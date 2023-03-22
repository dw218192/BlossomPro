#include "PhyllotaxisNode.h"
#include "Utils.h"
#include <maya/MGlobal.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnNumericAttribute.h>

auto PhyllotaxisNode::creator()->void* {
	return new PhyllotaxisNode;
}

auto PhyllotaxisNode::initialize() -> MStatus {
	MStatus status;

	MFnTypedAttribute typedAttribute;
	MFnNumericAttribute numericAttribute;

	PhyllotaxisNode::s_curve = typedAttribute.create(
		"curve",
		"cv",
		MFnData::kNurbsCurve,
		MObject::kNullObj,
		&status);
	CHECK(status, status);

	PhyllotaxisNode::s_step = numericAttribute.create(
		"step",
		"st",
		MFnNumericData::Type::kDouble,
		0.1,
		&status
	);
	CHECK(status, status);

	PhyllotaxisNode::s_output = typedAttribute.create(
		"out_arr",
		"oa",
		MFnArrayAttrsData::kDynArrayAttrs,
		MObject::kNullObj,
		&status
	);
	CHECK(status, status);

	status = addAttribute(PhyllotaxisNode::s_curve);
	CHECK(status, status);

	status = addAttribute(PhyllotaxisNode::s_step);
	CHECK(status, status);

	status = addAttribute(PhyllotaxisNode::s_output);
	CHECK(status, status);


	status = attributeAffects(PhyllotaxisNode::s_curve, PhyllotaxisNode::s_output);
	CHECK(status, status);

	status = attributeAffects(PhyllotaxisNode::s_step, PhyllotaxisNode::s_output);
	CHECK(status, status);

	return MStatus::kSuccess;
}

MStatus PhyllotaxisNode::compute(const MPlug& plug, MDataBlock& data) {
	
}