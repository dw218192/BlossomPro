#pragma once

#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MIntArray.h>
#include <maya/MFnIntArrayData.h>

#include <glm/glm.hpp>
#include <vector>
#include "BsplineSurface.h"

namespace BSplineSurfaceNode
{
	class CPPassthroughNode : public MPxNode
	{
	public:
		CPPassthroughNode() {}
		virtual ~CPPassthroughNode() = default;

		virtual MStatus compute(const MPlug& plug, MDataBlock& data);
		bool pass = true;
	public:
		static constexpr char const* nodeName() {
			return "CPPassthroughNode";
		}

		static void GetControlPoints(const int& rowCount,
									 const int& columnCount,
									 MItMeshVertex& vertexIter,
									 std::vector<std::vector<glm::vec3>>& controlPoints);

	protected:
		static void ComputeNewVertices(int rowCount,
									   int columnCount,
									   int prc,
									   int pcc,
									   std::vector<std::vector<glm::vec3>>& cps,
									   MPointArray& points);

		static void SetPnts(int count, const MString& name, const MPointArray& offsets);

	public:
		static MTypeId  id;

		static MObject iRowCount;
		static MObject iColumCount;

		static MObject oPreRC;
		static MObject oPreCC;

		static MObject iOldPointArray;

		static MObject iControlPlane;
		static MObject oControlPlane;

		inline static const MString SetPntsTemplateStr = R"(setAttr("^1s.pnts[^2s]") - type "float3" ^3s;)";
	public:
		static  void* creator();
		static  MStatus initialize();
	};
}
