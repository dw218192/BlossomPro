#pragma once
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
		double const param = m_curve.findParamFromLength(s, &status);
		if (MFAIL(status)) {
			throw status;
		}
		MPoint point;
		status = m_curve.getPointAtParam(param, point, MSpace::kWorld);
		if (MFAIL(status)) {
			throw status;
		}
		return MVector { point };
	}
	auto length() const -> double {
		MStatus status;
		double ret = m_curve.length(kMFnNurbsEpsilon, &status);
		if (MFAIL(status)) {
			throw status;
		}
		return ret;
	}
private:
	MFnNurbsCurve const& m_curve;
};
