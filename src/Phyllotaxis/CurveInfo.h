#pragma once
#include "Utils.h"
#include <maya/MFnNurbsCurve.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>

struct CurveInfo {
	CurveInfo() = delete;
	CurveInfo(CurveInfo&) = delete;
	CurveInfo(CurveInfo&&) = delete;
	CurveInfo(MFnNurbsCurve const& curve) : m_curve(curve) { }

	auto getPoint(double s) const -> MVector  {
		MStatus status;
		double const param = m_curve.findParamFromLength(s, 0.01, &status);
		if (MFAIL(status)) {
			throw MAYA_EXCEPTION(status);
		}
		MPoint point;
		status = m_curve.getPointAtParam(param, point, MSpace::kObject);
		if (MFAIL(status)) {
			throw MAYA_EXCEPTION(status);
		}
		return MVector { point };
	}
	auto length() const -> double {
		MStatus status;
		double ret = m_curve.length(kMFnNurbsEpsilon, &status);
		if (MFAIL(status)) {
			throw MAYA_EXCEPTION(status);
		}
		return ret;
	}
private:
	MFnNurbsCurve const& m_curve;
};
