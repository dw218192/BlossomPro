#pragma once
#include "Grammar.h"
#include "../CurveLenFunction/CurveInfo.h"
#include "../CurveLenFunction/UserCurveLenFunction.h"

struct PhyllotaxisGrammar : public Grammar {
	PhyllotaxisGrammar(std::unique_ptr<CurveInfo> info, std::shared_ptr<UserCurveLenFunction> func, double integrationStep = 0.0001)
		: m_info(std::move(info)), m_densityFunc(func), m_integrationStep(integrationStep), m_a(0.0), m_s(0.0)
	{
		m_turtle.pitchUp(90); // start facing +y axis
	}
	bool hasNext() const override {
		return m_s < m_info->length();
	}
	void nextIter() override {
		if(!hasNext()) {
			return;
		}

		double const curveLen = m_info->length();
		auto const& densityFunc = *m_densityFunc;
		// double x = m_info->getPoint(m_s).x;
		// static constexpr double k_pi = 3.14159265358979323846;

		// perform integration
		//while (m_a < 1 && m_s < curveLen) {
		//	x = m_info->getPoint(m_s).x;
		//	double const density = m_densityFunc(m_s / curveLen);
		//	m_a += 2 * x / (density * density) * m_integrationStep;
		//	m_s += m_integrationStep;
		//}
		//m_a = std::max(0.0, m_a - 1.0);

		m_s += m_integrationStep;
		double const x = m_info->getPoint(m_s).x;
		double const y = m_info->getPoint(m_s).y;

		double const scale = densityFunc(m_s / curveLen);
		m_turtle
			.pushState()
			.forward(y)
			.rotateRight(90)
			.forward(x);

		m_result.emplace_back(m_turtle.getPos(), m_turtle.getRot(), MVector{scale, scale, scale});

		m_turtle
			.popState()
			.rollLeft(137.5);
	}
private:
	std::unique_ptr<CurveInfo> m_info;
	std::shared_ptr<UserCurveLenFunction> m_densityFunc;
	double const m_integrationStep;

	// state variables
	double m_a, m_s;
};