#include "PhyllotaxisNode.h"
#include "Phyllotaxis/UserCurveLenFunction.h"

#include "Utils.h"
#include <maya/MGlobal.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnNumericAttribute.h>

void* PhyllotaxisNode::creator() {
	return new PhyllotaxisNode;
}

MStatus PhyllotaxisNode::initialize() {
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

	s_curveFuncId = numericAttribute.create(
		longName(s_curveFuncId),
		shortName(s_curveFuncId),
		MFnNumericData::Type::kInt,
		-1,
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
		MFnArrayAttrsData::kDynArrayAttrs,
		MObject::kNullObj,
		&status
	);
	CHECK(status, status);

	status = addAttribute(s_curve);
	CHECK(status, status);
	status = addAttribute(s_curveFuncId);
	CHECK(status, status);
	status = addAttribute(s_step);
	CHECK(status, status);
	status = addAttribute(s_output);
	CHECK(status, status);

	status = attributeAffects(s_curveFuncId, s_output);
	CHECK(status, status);
	status = attributeAffects(s_curve, s_output);
	CHECK(status, status);
	status = attributeAffects(s_step, s_output);
	CHECK(status, status);

	return MStatus::kSuccess;
}

MStatus PhyllotaxisNode::compute(const MPlug& plug, MDataBlock& data) {
	MStatus status;

	if (plug != PhyllotaxisNode::s_output) {
		return MStatus::kUnknownParameter;
	}

	// validate input
	MObject curveObj = data.inputValue(s_curve, &status).asNurbsCurveTransformed();
	CHECK(status, status);

	int const curveFuncId = data.inputValue(s_curveFuncId, &status).asInt();
	CHECK(status, status);

	UserCurveLenFunction* curveFunc = UserCurveLenFunction::getInstance(curveFuncId);
	if(!curveFunc || !curveFunc->valid()) {
		return MStatus::kInvalidParameter;
	}

	double const step = data.inputValue(s_step, &status).asDouble();
	CHECK(status, status);

	return MStatus::kSuccess;
}