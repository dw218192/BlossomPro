#pragma once
#include <maya/MPxNode.h>

class CurveNode : public MPxNode
{
public:
	static constexpr char const* nodeName() {
		return "CurveNode";
	}
	static constexpr char const* longName(MObject const& attr) {
		if (&attr == &s_percent) {
			return "percent";
		} else if(&attr == &s_curve) {
			return "curve";
		} else if (&attr == &s_step) {
			return "step";
		} else if (&attr == &s_output) {
			return "outArr";
		}
		return "";
	}
	static constexpr char const* shortName(MObject const& attr) {
		if (&attr == &s_percent) {
			return "pr";
		} else if (&attr == &s_curve) {
			return "cv";
		} else if (&attr == &s_step) {
			return "st";
		} else if (&attr == &s_output) {
			return "oa";
		}
		return "";
	}

	static void* creator();
	static MStatus initialize();
	static inline MTypeId s_id{ 0xdead };
	static inline MObject s_percent;
	static inline MObject s_curve;
	static inline MObject s_step;
	static inline MObject s_output;

	CurveNode() = default;
	~CurveNode() override = default;

	MStatus compute(const MPlug& plug, MDataBlock& data) override;
};