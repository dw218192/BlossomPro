#pragma once
#include <QPointer>

#include <maya/MPxCommand.h>
#include <maya/MQtUtil.h>
#include <maya/MFnDependencyNode.h>

#include "Grammar/Grammar.h"
#include "Grammar/PlanarPhyllotaxisGrammar.h"
#include "../MayaNodes/CurveNode.h"
#include "../Utils.h"

#define CURVE_NODE_TEST
struct UnitTestCmd : public MPxCommand {
	auto doIt(const MArgList& args) -> MStatus override {
		Grammar::Turtle t;
#ifdef TURTLE_TEST_1
		HANDLE_EXCEPTION(
			t.forward(1)
			.forward(2)
			.rotateLeft(90)
			.forward(3)
			.rotateRight(45)
			.forward(3)
			.drawCube()
		);
#endif

#ifdef TURTLE_TEST_2
		HANDLE_EXCEPTION(
			t.pitchUp(90)
			.forward(2)
			.drawCube({0.1,0.1,0.1})
			.forward(-3)
			.drawSphere(0.1)
			.forward(3)
			.pushState()
				.rollLeft(45)
				.forward(1)
				.drawCube()
			.popState()
				.rollRight(20)
				.forward(2)
				.drawCube()
		);
#endif

#ifdef PLANAR_TEST
		PlanarPhyllotaxisGrammar ppg;
		HANDLE_EXCEPTION(
			ppg.process(50);
		);
#endif

#ifdef CURVE_NODE_TEST
		static char const* const fmt =
			"$curveName = `ls -sl`;\n"
			"$curveNode = `createNode %s`;\n"
			"connectAttr ($curveName[0] + \".worldSpace\") ($curveNode + \".%s\");\n"
			"$obj = `polyCube`;\n"
			"$ins = `createNode instancer`;\n"
			"connectAttr ($obj[0] + \".matrix\") ($ins + \".inputHierarchy[0]\");\n"
			"connectAttr ($curveNode + \".%s\") ($ins + \".inputPoints\");";
		static char buf[1024];
		int const check = std::snprintf(buf, sizeof(buf), fmt, 
			CurveNode::nodeName(),
			CurveNode::longName(CurveNode::s_curve),
			CurveNode::longName(CurveNode::s_output)
		);
		MGlobal::displayInfo(buf);
		if(check < 0) {
			ERROR_MESSAGE("snprintf failed\n");
			return MStatus::kFailure;
		}
		MStatus status = MGlobal::executeCommand(buf);
		CHECK(status, status);
		/*
		 *
		 * createNode CurveNode;
			connectAttr curve1.worldSpace CurveNode1.curve;
			createNode instancer;
			polyCube;
			connectAttr pCube1.matrix instancer1.inputHierarchy[0];
			connectAttr CurveNode1.out_arr instancer1.inputPoints;
		 */
#endif
		return MStatus::kSuccess;
	}

	static auto creator() -> void* {
		return new UnitTestCmd;
	}
};