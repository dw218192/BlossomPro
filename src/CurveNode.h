#pragma once
#include <maya/MPxNode.h>

class CurveNode : public MPxNode
{
public:
	static void* creator();
	static MStatus initialize();
	static MTypeId s_id;
	static MObject s_curve;
	static MObject s_step;
	static MObject s_output;

	CurveNode() = default;
	~CurveNode() override = default;

	MStatus compute(const MPlug& plug, MDataBlock& data);
};