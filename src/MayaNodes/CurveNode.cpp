#include "CurveNode.h"
#include "../CurveLenFunction/CurveInfo.h"
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

	s_curve = typedAttribute.create(
		longName(s_curve),
		shortName(s_curve),
		MFnData::kNurbsCurve,
		MObject::kNullObj,
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

	s_percent = numericAttribute.create(
		longName(s_percent),
		shortName(s_percent),
		MFnNumericData::Type::kDouble,
		0.1,
		&status
	);
	CHECK(status, status);

	s_output = typedAttribute.create(
		longName(s_output),
		shortName(s_output),
		MFnArrayAttrsData::kDynArrayAttrs,
		MObject::kNullObj,
		&status
	);
	CHECK(status, status);

	status = addAttribute(s_curve);
	CHECK(status, status);

	status = addAttribute(s_step);
	CHECK(status, status);

	status = addAttribute(s_percent);
	CHECK(status, status);

	status = addAttribute(s_output);
	CHECK(status, status);


	status = attributeAffects(s_curve, s_output);
	CHECK(status, status);

	status = attributeAffects(s_step, s_output);
	CHECK(status, status);

	status = attributeAffects(s_percent, s_output);
	CHECK(status, status);

	return MStatus::kSuccess;
}
auto CurveNode::compute(const MPlug& plug, MDataBlock& data) -> MStatus {
	MStatus status;

	if (plug != s_output) {
		return MStatus::kUnknownParameter;
	}

	MObject curveObj = data.inputValue(s_curve, &status).asNurbsCurveTransformed();
	CHECK(status, status);

	const double step = data.inputValue(s_step, &status).asDouble();
	CHECK(status, status);

	const double percent = data.inputValue(s_percent, &status).asDouble();
	CHECK(status, status);

	MFnNurbsCurve curve{ curveObj, &status };
	CHECK(status, status);

	const double totalLen = percent * curve.length(0.01, &status);
	CHECK(status, status);

	MFnArrayAttrsData arrayAttrsData;

	MObject aadObj = arrayAttrsData.create(&status);
	CHECK(status, status);

	MVectorArray positions = arrayAttrsData.vectorArray("position", &status);
	CHECK(status, status);

	CurveInfo curveInfo{ curveObj , &status };
	CHECK(status, status);

	for(double len = 0.1; len < totalLen; len += step) {
		HANDLE_EXCEPTION(status = positions.append(curveInfo.getPoint(len)));
		CHECK(status, status);
	}
	MGlobal::displayInfo(MString{ "num of instances = " } + positions.length());
	data.outputValue(CurveNode::s_output).setMObject(aadObj);
	data.setClean(plug);
	return MStatus::kSuccess;
}