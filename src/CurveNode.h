#pragma once
#include <maya/MPxNode.h>

class CurveNode : public MPxNode
{
public:
	static void* creator();
	static MStatus initialize();
	static inline MTypeId s_id{ 0xdead };
	static inline MObject s_percent;
	static inline MObject s_curve;
	static inline MObject s_step;
	static inline MObject s_output;

	CurveNode() = default;
	~CurveNode() override = default;

	MStatus compute(const MPlug& plug, MDataBlock& data);
};