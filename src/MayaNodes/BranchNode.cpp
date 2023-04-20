#include "BranchNode.h"
#include "Grammar/GeneralizedCylinderGrammar.h"

#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnNumericAttribute.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>

#include <format>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>

void* BranchNode::creator() {
	return new BranchNode;
}

MStatus BranchNode::initialize() {
	MStatus status;

	MFnTypedAttribute typedAttribute;
	MFnNumericAttribute numericAttribute;

	s_generatingCurve = typedAttribute.create(
		longName(s_generatingCurve),
		shortName(s_generatingCurve),
		MFnData::kNurbsCurve,
		MObject::kNullObj,
		&status);
	CHECK(status, status);

	s_carrierCurve = typedAttribute.create(
		longName(s_carrierCurve),
		shortName(s_carrierCurve),
		MFnData::kNurbsCurve,
		MObject::kNullObj,
		&status);
	CHECK(status, status);

	s_generatingCurveName = typedAttribute.create(
		longName(s_generatingCurveName),
		shortName(s_generatingCurveName),
		MFnData::kString,
		MObject::kNullObj,
		&status
	);
	CHECK(status, status);

	for (auto&& func : s_funcs) {
		func = typedAttribute.create(
			longName(func),
			shortName(func),
			MFnData::kString,
			MObject::kNullObj,
			&status);
		CHECK(status, status);
	}

	s_numIter = numericAttribute.create(
		longName(s_numIter),
		shortName(s_numIter),
		MFnNumericData::Type::kInt,
		1,
		&status);
	CHECK(status, status);

	s_step = numericAttribute.create(
		longName(s_step),
		shortName(s_step),
		MFnNumericData::Type::kDouble,
		0.1,
		&status
	);
	CHECK(status, status);

	s_output = typedAttribute.create(
		longName(s_output),
		shortName(s_output),
		MFnData::kMesh,
		MObject::kNullObj,
		&status
	);
	CHECK(status, status);

	status = addAttribute(s_generatingCurve);
	CHECK(status, status);
	status = addAttribute(s_carrierCurve);
	CHECK(status, status);
	status = addAttribute(s_generatingCurveName);
	CHECK(status, status);

	for (auto const& func : s_funcs) {
		status = addAttribute(func);
		CHECK(status, status);
	}
	status = addAttribute(s_numIter);
	CHECK(status, status);
	status = addAttribute(s_step);
	CHECK(status, status);
	status = addAttribute(s_output);
	CHECK(status, status);

	status = attributeAffects(s_generatingCurve, s_output);
	CHECK(status, status);
	status = attributeAffects(s_carrierCurve, s_output);
	CHECK(status, status);
	status = attributeAffects(s_generatingCurveName, s_output);
	CHECK(status, status);

	for (auto const& func : s_funcs) {
		status = attributeAffects(func, s_output);
		CHECK(status, status);
	}
	status = attributeAffects(s_numIter, s_output);
	CHECK(status, status);
	status = attributeAffects(s_step, s_output);
	CHECK(status, status);

	return MStatus::kSuccess;
}

Result<BranchNode::Inputs> BranchNode::getInputs(MDataBlock& data) {
	MStatus status;
	Inputs ret;
	GeneralizedCylinderGrammar::Functions& funcs = ret.funcs;
	{
		auto it = s_funcs.begin();
		for (auto const ptr : { & funcs.yawRate, & funcs.pitchRate, & funcs.rollRate, & funcs.twistRate, & funcs.widthRate }) {
			std::shared_ptr<UserCurveLenFunction> pfunc;
			HANDLE_EXCEPTION((it++)->inputValue(pfunc, data, &status));
			*ptr = pfunc;
		}
	}
	CHECK(status, status);

	ret.carrierCurveObj = data.inputValue(s_carrierCurve, &status).asNurbsCurveTransformed();
	CHECK(status, status);

	ret.generatingCurveObj = data.inputValue(s_generatingCurve, &status).asNurbsCurveTransformed();
	CHECK(status, status);

	ret.generatingCurveName = data.inputValue(s_generatingCurveName, &status).asString();
	CHECK(status, status);

	ret.numIter = data.inputValue(s_numIter, &status).asInt();
	CHECK(status, status);

	ret.step = data.inputValue(s_step, &status).asDouble();
	CHECK(status, status);

	return ret;
}

MStatus BranchNode::compute(MPlug const& plug, MDataBlock& data) {
	MStatus status;

	if (plug != s_output) {
		return MStatus::kUnknownParameter;
	}
	auto res = getInputs(data);
	if(!res.valid()) {
		status = res.error();
		CHECK(status, status);
	}

	auto [funcs, carrierCurveObj, generatingCurveObj, generatingCurveName, numIter, step] = res.value();

	auto curveInfo = std::make_unique<CurveInfo>(carrierCurveObj, &status);
	CHECK(status, status);

	auto grammar = std::make_unique<GeneralizedCylinderGrammar>(std::move(curveInfo), funcs, step);
	HANDLE_EXCEPTION(grammar->process(numIter));

	MString loftCmd { "loft -ch 1 -rn 0 -ar 1 -ss 1 -u 1 -c 0 -rsn true -po 1 " };
	loftCmd += generatingCurveName;

	int cnt = 0;
	for(auto const& [pos, rot, scale] : grammar->result()) {
		std::string tmpCurveName = std::vformat("excurve{}", std::make_format_args(cnt++));
		auto const cmdFmt =
			"string $names[] = `duplicate {}`;\n"
			"move {} {} {} $names[0];\n"
			"rename $names[0] {};\n"
//			"scale {} {} {} {};\n"
//			"rotate -os -fo 0 0 0;\n"
		;

		std::string cmd = std::vformat(cmdFmt, std::make_format_args(
				generatingCurveName.asChar(),
				pos.x, pos.y, pos.z,
				tmpCurveName.c_str()
			));

		status = MGlobal::executeCommand(cmd.c_str());
		CHECK(status, status);

		loftCmd += " ";
		loftCmd += tmpCurveName.c_str();
	}
	loftCmd += ";";

	MGlobal::displayInfo(loftCmd);

	MStringArray result;
	status = MGlobal::executeCommand(loftCmd, result);
	CHECK(status, status);

	MSelectionList selList;
	selList.add(result[0]);
	MDagPath dagPath;
	selList.getDagPath(0, dagPath);
	MObject loftedSurfaceObj = dagPath.node();

	MDataHandle outputSurfaceHandle = data.outputValue(s_output);
	outputSurfaceHandle.setMObject(loftedSurfaceObj);
	outputSurfaceHandle.setClean();

	data.setClean(plug);

	return MS::kSuccess;
}
