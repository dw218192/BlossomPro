#include "PhyllotaxisNode.h"
#include "UserCurveLenFunction.h"
#include "PhyllotaxisGrammar.h"
#include "../Utils.h"
#include <maya/MGlobal.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MVectorArray.h>

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

	s_curveFunc = typedAttribute.create(
		longName(s_curveFunc),
		shortName(s_curveFunc),
		MFnData::kString,
		&status);
	CHECK(status, status);

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
		MFnArrayAttrsData::kDynArrayAttrs,
		MObject::kNullObj,
		&status
	);
	CHECK(status, status);

	status = addAttribute(s_curve);
	CHECK(status, status);
	status = addAttribute(s_curveFunc);
	CHECK(status, status);
	status = addAttribute(s_numIter);
	CHECK(status, status);
	status = addAttribute(s_step);
	CHECK(status, status);
	status = addAttribute(s_output);
	CHECK(status, status);

	status = attributeAffects(s_curve, s_output);
	CHECK(status, status);
	status = attributeAffects(s_curveFunc, s_output);
	CHECK(status, status);
	status = attributeAffects(s_numIter, s_output);
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

	int const numIter = data.inputValue(s_numIter, &status).asInt();
	CHECK(status, status);

	HANDLE_EXCEPTION(s_curveFunc.inputValue(m_curveFunc, data, &status));
	CHECK(status, status);

	double const step = data.inputValue(s_step, &status).asDouble();
	CHECK(status, status);

	auto curveInfo = std::make_unique<CurveInfo>(curveObj, &status);
	CHECK(status, status);

	m_grammar = std::make_unique<PhyllotaxisGrammar>(std::move(curveInfo), m_curveFunc, step);
	HANDLE_EXCEPTION(m_grammar->process(numIter));

	MFnArrayAttrsData arrayAttrsData;
	MObject aadObj = arrayAttrsData.create(&status);
	CHECK(status, status);

	MVectorArray positions = arrayAttrsData.vectorArray("position", &status);
	CHECK(status, status);

	MVectorArray scales = arrayAttrsData.vectorArray("scale", &status);
	CHECK(status, status);

	for (auto [pos, scale] : m_grammar->result()) {
		status = positions.append(pos);
		CHECK(status, status);

		status = scales.append(scale);
		CHECK(status, status);
	}

	MGlobal::displayInfo(MString{ "num of instances = " } + positions.length());

	MDataHandle handle = data.outputValue(s_output, &status);
	CHECK(status, status);

	status = handle.setMObject(aadObj);
	CHECK(status, status);

	status = data.setClean(plug);
	CHECK(status, status);

	return MStatus::kSuccess;
}