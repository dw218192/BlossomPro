#pragma once
#include "Grammar.h"
#include "../CurveLenFunction/CurveInfo.h"
#include "../CurveLenFunction/UserCurveLenFunction.h"

struct GeneralizedCylinderGrammar : public Grammar {
	struct Functions {
		std::shared_ptr<UserCurveLenFunction> yawRate;
		std::shared_ptr<UserCurveLenFunction> pitchRate;
		std::shared_ptr<UserCurveLenFunction> rollRate;
		std::shared_ptr<UserCurveLenFunction> widthRate;
		std::shared_ptr<UserCurveLenFunction> twistRate;
	};

	GeneralizedCylinderGrammar(Functions functions, double length = 1, double step = 0.02)
		: m_step{step}, m_length{length}, m_s{0.0}, m_phi{0.0}, m_funcs{std::move(functions)} { }

	bool hasNext() const override {
		return m_s < m_length;
	}
	void nextIter() override {
		if (!hasNext()) {
			return;
		}

		double const s = m_s / m_length;

#define EVAL(pfunc, dflt) (m_funcs.pfunc ? (*m_funcs.pfunc)(s) : (dflt))
		auto const yawAngle = EVAL(yawRate, 0) * m_step;
		auto const pitchAngle = EVAL(pitchRate, 0) * m_step;
		auto const rollAngle = EVAL(rollRate, 0) * m_step;
		auto const twistAngle = EVAL(twistRate, 0) * m_step;
		auto const width = EVAL(widthRate, 1);
#undef EVAL

		m_phi += twistAngle;

		// roll --> rotate
		// rotate --> roll

		m_turtle
			.rollLeft(yawAngle * m_step)
			.pitchDown(pitchAngle * m_step)
			.rotateRight(rollAngle * m_step)
			.rotateRight(m_phi);

		double const scale = width;
		m_result.emplace_back(m_turtle.getPos(), m_turtle.getRot(), MVector { scale, scale, scale });

		m_turtle
			.pitchUp(90)
			.forward(m_step)
			.pitchDown(90)
			.rotateLeft(m_phi);

		m_s += m_step;
	}

private:
	double const m_step, m_length;
	double m_s, m_phi;
	Functions const m_funcs;
};