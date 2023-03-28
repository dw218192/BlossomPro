#pragma once

#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MIntArray.h>
#include <maya/MFnIntArrayData.h>

#include <vector>
#include <glm/glm.hpp>

namespace BSplineSurfaceNode
{
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

		static void ComputeInitPlane(const int& defaultRc, const int& defaultCc, MPointArray& points);

	protected:
		static void ConnectVertices(const int& row, const int& column, MIntArray& faceCounts, MIntArray& faceConnects);
		static void Init(const int& defaultRc, const int& defaultCc, MObject& outData, MStatus& stat);

	public:
		static MTypeId  id;

		static MObject iRowCount;
		static MObject iColumCount;

		static MObject iControlPlane;
		static MObject oControlPlane;
		static MObject oOldPointArray;
	public:
		static  void* creator();
		static  MStatus initialize();
	};
}



