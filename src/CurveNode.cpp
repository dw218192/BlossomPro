#include "CurveNode.h"
#include "Phyllotaxis/CurveInfo.h"
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MGlobal.h>
#include <maya/MVectorArray.h>

auto CurveNode::creator()->void* {
	return new CurveNode;
}
auto CurveNode::initialize() -> MStatus {
	MStatus status;

	MFnTypedAttribute typedAttribute;
	MFnNumericAttribute numericAttribute;

	CurveNode::s_curve = typedAttribute.create(
		"curve", 
		"cv",
		MFnData::kNurbsCurve,
		MObject::kNullObj,
		&status);
	CHECK(status);

	CurveNode::s_step = numericAttribute.create(
		"step",
		"st",
		MFnNumericData::Type::kDouble,
		0.1,
		&status
	);
	CHECK(status);

	CurveNode::s_percent = numericAttribute.create(
		"percent",
		"pc",
		MFnNumericData::Type::kDouble,
		0.1,
		&status
	);
	CHECK(status);

	CurveNode::s_output = typedAttribute.create(
		"out_arr",
		"oa",
		MFnArrayAttrsData::kDynArrayAttrs,
		MObject::kNullObj,
		&status
	);
	CHECK(status);

	status = addAttribute(CurveNode::s_curve);
	CHECK(status);

	status = addAttribute(CurveNode::s_step);
	CHECK(status);

	status = addAttribute(CurveNode::s_percent);
	CHECK(status);

	status = addAttribute(CurveNode::s_output);
	CHECK(status);


	status = attributeAffects(CurveNode::s_curve, CurveNode::s_output);
	CHECK(status);

	status = attributeAffects(CurveNode::s_step, CurveNode::s_output);
	CHECK(status);

	status = attributeAffects(CurveNode::s_percent, CurveNode::s_output);
	CHECK(status);

	return MStatus::kSuccess;
}
auto CurveNode::compute(const MPlug& plug, MDataBlock& data) -> MStatus {
	MStatus status;

	if (plug != CurveNode::s_output) {
		return MStatus::kUnknownParameter;
	}

	MObject curveObj = data.inputValue(CurveNode::s_curve, &status).asNurbsCurveTransformed();
	CHECK(status);

	const double step = data.inputValue(CurveNode::s_step, &status).asDouble();
	CHECK(status);

	const double percent = data.inputValue(CurveNode::s_percent, &status).asDouble();
	CHECK(status);

	MFnNurbsCurve curve{ curveObj, &status };
	CHECK(status);

	const double totalLen = percent * curve.length(0.01, &status);
	CHECK(status);

	MFnArrayAttrsData arrayAttrsData;

	MObject aadObj = arrayAttrsData.create(&status);
	CHECK(status);

	MVectorArray positions = arrayAttrsData.vectorArray("position", &status);
	CHECK(status);

	CurveInfo curveInfo{ curve };
	for(double len = 0.1; len < totalLen; len += step) {
		HANDLE_EXCEPTION(status = positions.append(curveInfo.getPoint(len)));
		CHECK(status);
	}
	MGlobal::displayInfo(MString{"num of instances = "} + positions.length());
	data.outputValue(CurveNode::s_output).setMObject(aadObj);
	data.setClean(plug);
	return MStatus::kSuccess;
}