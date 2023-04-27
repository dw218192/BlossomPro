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

    KeyframeCurveLenFunction(ControlPointArray controlPoints, SplineType type, double scale);
	double operator()(double s) const override;
	bool operator==(UserCurveLenFunction const&) const override;
	bool operator==(KeyframeCurveLenFunction const&) const;
    bool operator!=(KeyframeCurveLenFunction const& other) const {
        return !(*this == other);
    }

	bool valid() const override;
    void setType(SplineType type);
    void setYScale(double scale) {
        m_yScale = scale;
    }
    void addControlPoint(double x, double y);
    void insert(ControlPointArray::ConstIterator it, double x, double y);
    void setControlPoint(ControlPointArray::ConstIterator it, double x, double y);
    void setControlPoints(ControlPointArray::ConstIterator first, ControlPointArray::ConstIterator last);

    SplineType getType() const;
    double getScale() const {
        return m_yScale;
    }
    ControlPointArray const& getControlPoints() const {
        return m_controlPoints;
    }

protected:
    KeyframeCurveLenFunction() = default;
    std::string serialize() const override;
    void deserialize(std::istringstream&) override;

private:
// serialized
    ControlPointArray m_controlPoints;
    tk::spline::spline_type m_splineType;
    double m_yScale;
// not serialized
	tk::spline m_spline;
};
