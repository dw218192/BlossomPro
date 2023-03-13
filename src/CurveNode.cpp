#include "CurveNode.h"
#include "CurveNode.h"
#include "CurveNode.h"
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
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CurveNode::s_step = numericAttribute.create(
		"step",
		"st",
		MFnNumericData::Type::kDouble,
		0.1,
		&status
	);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CurveNode::s_percent = numericAttribute.create(
		"percent",
		"pc",
		MFnNumericData::Type::kDouble,
		0.1,
		&status
	);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CurveNode::s_output = typedAttribute.create(
		"out_arr",
		"oa",
		MFnArrayAttrsData::kDynArrayAttrs,
		MObject::kNullObj,
		&status
	);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = addAttribute(CurveNode::s_curve);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = addAttribute(CurveNode::s_step);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = addAttribute(CurveNode::s_percent);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = addAttribute(CurveNode::s_output);
	CHECK_MSTATUS_AND_RETURN_IT(status);


	status = attributeAffects(CurveNode::s_curve, CurveNode::s_output);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = attributeAffects(CurveNode::s_step, CurveNode::s_output);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = attributeAffects(CurveNode::s_percent, CurveNode::s_output);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MStatus::kSuccess;
}
auto CurveNode::compute(const MPlug& plug, MDataBlock& data) -> MStatus {
	// findParamFromLength
	// getPointAtParam

	MStatus status;

	if (plug != CurveNode::s_output) {
		return MStatus::kUnknownParameter;
	}

	MObject curveObj = data.inputValue(CurveNode::s_curve, &status).asNurbsCurveTransformed();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	const double step = data.inputValue(CurveNode::s_step, &status).asDouble();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	const double percent = data.inputValue(CurveNode::s_percent, &status).asDouble();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MFnNurbsCurve curve{ curveObj, &status };
	CHECK_MSTATUS_AND_RETURN_IT(status);

	const double totalLen = percent * curve.length(0.01, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MFnArrayAttrsData arrayAttrsData;

	MObject aadObj = arrayAttrsData.create(&status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MVectorArray positions = arrayAttrsData.vectorArray("position", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	for(double len = 0; len < totalLen; len += step) {
		const double param = curve.findParamFromLength(len, 0.01, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MPoint point;
		status = curve.getPointAtParam(param, point, MSpace::kWorld);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		positions.append(point);
	}

	data.outputValue(CurveNode::s_output).setMObject(aadObj);
	data.setClean(plug);
	return MStatus::kSuccess;
}