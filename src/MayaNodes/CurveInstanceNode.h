#pragma once

#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MIntArray.h>
#include <maya/MFnIntArrayData.h>


class CurveInstanceNode : public MPxNode
{
public:
	CurveInstanceNode() {}
	virtual ~CurveInstanceNode() = default;

	virtual MStatus compute(const MPlug& plug, MDataBlock& data);

public:
	static constexpr char const* nodeName() {
		return "CurveInstanceNode";
	}

public:
	static MTypeId  id;

	static MObject instanceCount;
	static MObject inputCenter;
	static MObject inputRotate;
	
	static MObject rotateAttenuation;

	static MObject inputCurve;

	static MObject outTransforms;

public:
	static  void* creator();
	static  MStatus initialize();
};

