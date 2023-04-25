#pragma once
#include "../Utils.h"
#include <maya/MFnNurbsCurve.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>

struct CurveInfo {
	// disable move because we have maya type fields
	CurveInfo(CurveInfo&&) = delete;
	CurveInfo(CurveInfo const& other) : m_obj{ other.m_obj }, m_curve{ other.m_obj } {  }
	CurveInfo(MObject const& curveObj, MStatus* status = nullptr) : m_obj{ curveObj }, m_curve{ curveObj, status } { }

	[[nodiscard]] auto getPoint(double s) const -> MVector  {
		MStatus status;
		double const param = m_curve.findParamFromLength(s, kMFnNurbsEpsilon, &status);
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
	[[nodiscard]] auto length() const -> double {
		MStatus status;
		double const ret = m_curve.length(kMFnNurbsEpsilon, &status);
		if (MFAIL(status)) {
			throw MAYA_EXCEPTION(status);
		}
		return ret;
	}
private:
	MObject m_obj;
	MFnNurbsCurve m_curve;
};
