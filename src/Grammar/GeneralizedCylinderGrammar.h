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

	GeneralizedCylinderGrammar(std::unique_ptr<CurveInfo> info, Functions functions, double step = 0.02)
		: m_step{step}, m_s{0.0}, m_phi{0.0}, m_info{std::move(info)}, m_funcs{functions}
	{
		m_turtle.pitchUp(90); // start facing +y axis
	}

	bool hasNext() const override {
		return m_s < m_info->length();
	}
	void nextIter() override {
		if (!hasNext()) {
			return;
		}

		double const s = m_s / m_info->length();

		auto const& yawFunc = *m_funcs.yawRate;
		auto const& pitchFunc = *m_funcs.pitchRate;
		auto const& rollFunc = *m_funcs.rollRate;
		auto const& twistFunc = *m_funcs.twistRate;
		auto const& widthFunc = *m_funcs.widthRate;
		m_phi += twistFunc(s);

		m_turtle
			.rotateLeft(yawFunc(s) * m_step)
			.pitchDown(pitchFunc(s) * m_step)
			.rollRight(rollFunc(s) * m_step)
			.rollRight(m_phi);

		double const scale = widthFunc(s);
		m_result.emplace_back(m_turtle.getPos(), m_turtle.getRot(), MVector { scale, scale, scale });

		m_turtle
			.forward(m_step)
			.rollLeft(m_phi);

		m_s += m_step;
	}

private:
	double const m_step;
	double m_s, m_phi;
	std::unique_ptr<CurveInfo> m_info;
	Functions const m_funcs;
};