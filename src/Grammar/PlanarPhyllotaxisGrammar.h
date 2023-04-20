#pragma once
#include "Grammar.h"

struct PlanarPhyllotaxisGrammar : public Grammar {
	PlanarPhyllotaxisGrammar() : m_cur(0) {}

	bool hasNext() const override {
		return true;
	}
	void nextIter() override {
		m_turtle
			.rotateLeft(137.5)
			.pushState()
				.forward(std::sqrt(m_cur))
				.drawSphere(0.2)
			.popState();
		++m_cur;
	}
private:
	int m_cur;
};