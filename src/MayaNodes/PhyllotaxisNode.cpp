#include "PhyllotaxisNode.h"
#include "../CurveLenFunction/UserCurveLenFunction.h"
#include "../Grammar/PhyllotaxisGrammar.h"
#include "../Utils.h"
#include <maya/MGlobal.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MVectorArray.h>

void* PhyllotaxisNode::creator() {
	return new PhyllotaxisNode;
}

MStatus PhyllotaxisNode::initialize() {
	MStatus status;

	MFnTypedAttribute typedAttribute;
	MFnUnitAttribute unitAttribute;
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
		MObject::kNullObj,
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

	s_curveRadiusOutput = numericAttribute.create(
		longName(s_curveRadiusOutput),
		shortName(s_curveRadiusOutput),
		MFnNumericData::Type::kDouble,
		0.0,
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
	status = addAttribute(s_curveRadiusOutput);
	CHECK(status, status);

	status = attributeAffects(s_curve, s_output);
	CHECK(status, status);
	status = attributeAffects(s_curveFunc, s_output);
	CHECK(status, status);
	status = attributeAffects(s_numIter, s_output);
	CHECK(status, status);
	status = attributeAffects(s_step, s_output);
	CHECK(status, status);

	status = attributeAffects(s_curve, s_curveRadiusOutput);
	CHECK(status, status);
	status = attributeAffects(s_curveFunc, s_curveRadiusOutput);
	CHECK(status, status);

	return MStatus::kSuccess;
}

MStatus PhyllotaxisNode::compute(const MPlug& plug, MDataBlock& data) {
	MStatus status;

	if (plug != s_output && plug != s_curveRadiusOutput) {
		return MStatus::kUnknownParameter;
	}

	// validate input
	MObject curveObj = data.inputValue(s_curve, &status).asNurbsCurveTransformed();
	CHECK_RET(status);

	CurveInfo const info{ curveObj, &status };
	CHECK_RET(status);

	if (plug == s_curveRadiusOutput) {
		MVector const basePoint = info.getPoint(0);

		MDataHandle handle = data.outputValue(s_curveRadiusOutput, &status);
		CHECK_RET(status);

		handle.setDouble(std::abs(basePoint.x));
		CHECK_RET(status);
	} else {
		status = s_curveFunc.inputValue(m_curveFunc, data);
		CHECK_RET(status);

		int const numIter = data.inputValue(s_numIter, &status).asInt();
		CHECK_RET(status);

		double const step = data.inputValue(s_step, &status).asDouble();
		CHECK_RET(status);

		PhyllotaxisGrammar grammar{ info , m_curveFunc, step };
		HANDLE_EXCEPTION(grammar.process(numIter));

		MFnArrayAttrsData arrayAttrsData;
		MObject aadObj = arrayAttrsData.create(&status);
		CHECK_RET(status);

		MVectorArray positions = arrayAttrsData.vectorArray("position", &status);
		CHECK_RET(status);

		MVectorArray scales = arrayAttrsData.vectorArray("scale", &status);
		CHECK_RET(status);

		for (auto&& [pos, rot, scale] : grammar.result()) {
			status = positions.append(pos);
			CHECK_RET(status);

			status = scales.append(scale);
			CHECK_RET(status);
		}

		MGlobal::displayInfo(MString{ "num of instances = " } + positions.length());

		MDataHandle handle = data.outputValue(s_output, &status);
		CHECK_RET(status);

		status = handle.setMObject(aadObj);
		CHECK_RET(status);
	}

	status = data.setClean(plug);
	CHECK(status, status);

	return MStatus::kSuccess;
}