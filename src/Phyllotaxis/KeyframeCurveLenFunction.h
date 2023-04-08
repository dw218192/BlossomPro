#pragma once

#include <vector>
#include <spline.h>
#include "UserCurveLenFunction.h"
#include "ControlPointArray.h"

struct KeyframeCurveLenFunction : public UserCurveLenFunction {
    enum class SplineType {
        Linear = 0,
        C1 = 1,
        C2 = 2,
    };

    KeyframeCurveLenFunction() = default;
    KeyframeCurveLenFunction(ControlPointArray controlPoints, SplineType type);
	double operator()(double s) const override;
	bool operator==(UserCurveLenFunction const&) const override;
	bool operator==(KeyframeCurveLenFunction const&) const;
	bool valid() const override;
	std::string serialize() const override;
    void setType(SplineType type);
    void setYScale(double scale) {
        m_yScale = scale;
    }
    void addControlPoint(double x, double y);
    void insert(ControlPointArray::ConstIterator it, double x, double y);
    void setControlPoint(ControlPointArray::ConstIterator it, double x, double y);
    SplineType getType() const;
    double getScale() const {
        return m_yScale;
    }
    ControlPointArray const& getControlPoints() const {
        return m_controlPoints;
    }
private:
	void deserialize(std::istringstream&) override;

// serialized
    ControlPointArray m_controlPoints;
    tk::spline::spline_type m_splineType;
    double m_yScale;
// not serialized
	tk::spline m_spline;
};
