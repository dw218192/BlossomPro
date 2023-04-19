#pragma once
#include "Grammar.h"
#include "CurveInfo.h"
#include "UserCurveLenFunction.h"

struct GeneralizedCylinderGrammar : public Grammar {
	GeneralizedCylinderGrammar() = default;
	bool hasNext() const override {
		return false;
	}
	void nextIter() override {
		if (!hasNext()) {
			return;
		}
	}
private:
};