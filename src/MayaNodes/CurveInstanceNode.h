#pragma once

#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MIntArray.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MRandom.h>

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
	
	static MObject randSeed;
	static MObject rotateMagnitude;
	static MObject scaleMagnitude;
	static MObject yRandMagnitude;
	static MObject offsetRandMagnitude;
	static MObject rotateRandMagnitude;

	static MObject inputCurve;

	static MObject outTransforms;

public:
	static  void* creator();
	static  MStatus initialize();
};

class MyRand
{
public:
	MyRand(unsigned long long seed)
		:i(0), seed(seed)
	{}
	inline float Rand(float min, float max) { return MRandom::Rand_f(i++, seed, 0, max - min) + min; }
protected:
	unsigned long long i;
	unsigned long long seed;
};
