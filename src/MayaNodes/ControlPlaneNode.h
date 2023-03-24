#pragma once

#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MIntArray.h>
#include <maya/MFnIntArrayData.h>

#include <glm/glm.hpp>
#include <vector>
#include "BsplineSurface.h"

class ControlPlaneNode : public MPxNode
{
public:
	ControlPlaneNode() {}
	virtual ~ControlPlaneNode() = default;

	virtual MStatus compute(const MPlug& plug, MDataBlock& data);

public:
	static constexpr char const* nodeName() {
		return "ControlPlaneNode";
	}

	static void GetControlPoints(const int& rowCount, 
								 const int& columnCount, 
								 MItMeshVertex& vertexIter,
								 std::vector<std::vector<glm::vec3>>& controlPoints);

protected:
	// record the RowCount and Column before changing
	int m_PreRC = 2;
	int m_PreCC = 3;

protected:
	void UpdateVertices(const int& rowCount, const int& columnCount, std::vector<std::vector<glm::vec3>>& cps,  MPointArray& points);

protected:
	static void ConnectVertices(const int& row, const int& column, MIntArray& faceCounts, MIntArray& faceConnects);
	static void InitPlane(const int& defaultRc, const int& defaultCc, MPointArray& points, MStatus& stat);
	static void Init(const int& defaultRc, const int& defaultCc, MObject& outData, MStatus& stat);

public:
	static MTypeId  id;

	static MObject iRowCount;
	static MObject iColumCount;

	static MObject iCurrentPlane;
	static MObject oControlPlane;
public:
	static  void* creator();
	static  MStatus initialize();
};

