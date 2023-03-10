#pragma once
#include <QPointer>

#include <maya/MPxCommand.h>
#include <maya/MQtUtil.h>

#include "CurveNode.h"
#include "Phyllotaxis/Grammar.h"
#include "Phyllotaxis/PlanarPhyllotaxisGrammar.h"

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
		return MStatus::kSuccess;
	}

	static auto creator() -> void* {
		return new UnitTestCmd;
	}
};