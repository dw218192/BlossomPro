#pragma once
#include "Grammar.h"
#include "CurveInfo.h"
#include "UserCurveLenFunction.h"

struct PhyllotaxisGrammar : public Grammar {
	PhyllotaxisGrammar() = delete;
	PhyllotaxisGrammar(CurveInfo const& info, UserCurveLenFunction func, double integrationStep = 0.0001)
		: m_info(info), m_densityFunc(std::move(func)), m_integrationStep(integrationStep), m_a(0.0), m_s(0.0) {
		m_turtle.pitchUp(90); // start facing +y axis
	}
	bool hasNext() const override {
		return m_s < m_info.length();
	}
	void nextIter() override {
		if(!hasNext()) {
			return;
		}
		double x = m_info.getPoint(m_s).x;
		// perform integration
		while (m_a < 1 && m_s < m_info.length()) {
			x = m_info.getPoint(m_s).x;
			double const density = m_densityFunc(m_s);
			m_a += 2 * x / (density * density) * m_integrationStep;
			m_s += m_integrationStep;
		}
		m_a = std::max(0.0, m_a - 1.0);
		double const y = m_info.getPoint(m_s).y;

		m_turtle
			.pushState()
				.forward(y)
				.rotateRight(90)
				.forward(x)
				.drawSphere(m_densityFunc(m_s))
			.popState()
			.rollLeft(137.5);
	}
private:
	CurveInfo const& m_info;
	UserCurveLenFunction m_densityFunc;
	double const m_integrationStep;

	// state variables
	double m_a, m_s;
};