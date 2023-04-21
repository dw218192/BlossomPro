#include "BranchNode.h"
#include "Grammar/GeneralizedCylinderGrammar.h"

#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MGlobal.h>

#include <format>
#include <maya/MDagPath.h>
#include <maya/MFnTransform.h>
#include <maya/MPointArray.h>
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
		MFnData::kNurbsCurve,
		MObject::kNullObj,
		&status
	);
	CHECK(status, status);
	status = typedAttribute.setArray(true);
	CHECK(status, status);
	status = typedAttribute.setUsesArrayDataBuilder(true);
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

	auto const grammar = std::make_unique<GeneralizedCylinderGrammar>(std::move(curveInfo), funcs, step);
	HANDLE_EXCEPTION(grammar->process(numIter));

	MArrayDataHandle outCurveArrayHandle = data.outputArrayValue(s_output, &status);
	CHECK(status, status);

	MArrayDataBuilder outCurveArrayBuilder{ &data, s_output, static_cast<unsigned>(grammar->result().size()), &status };
	CHECK(status, status);

	int idx = 0;
	for(auto const& [pos, rot, scale] : grammar->result()) {
		MFnNurbsCurve curveFn;
		MObject obj = curveFn.copy(generatingCurveObj, MObject::kNullObj, &status);
		CHECK(status, status);

		// Transform the duplicated curve to [pos, rot, scale]
		MFnTransform transformFn;
		MObject transformObj = transformFn.create(MObject::kNullObj, &status);
		CHECK(status, status);

		transformFn.setTranslation(pos, MSpace::kWorld);
		transformFn.setRotation(rot);
		double scales[3] = { scale.x, scale.y, scale.z };
		transformFn.setScale(scales);

		MFnDagNode dagNodeFn(transformObj, &status);
		status = dagNodeFn.addChild(obj, MFnDagNode::kNextPos, true);
		CHECK(status, status);

		// Add it to the output array
		MDataHandle elementHandle = outCurveArrayBuilder.addElement(idx++, &status);
		CHECK(status, status);
		elementHandle.setMObject(obj);
		elementHandle.setClean();
	}

	outCurveArrayHandle.set(outCurveArrayBuilder);
	outCurveArrayHandle.setAllClean();
	data.setClean(plug);

	return MS::kSuccess;
}
