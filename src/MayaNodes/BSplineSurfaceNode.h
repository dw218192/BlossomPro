#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MObject.h>

namespace BSplineSurfaceNode
{
	class BSplineSurfaceNode : public MPxNode
	{
	public:
		BSplineSurfaceNode() {}
		virtual ~BSplineSurfaceNode() = default;

		virtual MStatus compute(const MPlug& plug, MDataBlock& data);

	private:
		void MakeMesh(std::vector<std::vector<glm::vec3>>& controlPoints,
			int subdivisionWidth,
			int subdivisionHeight,
			MObject& meshData,
			MStatus& status);

	public:
		static MTypeId  id;

		static MObject inSW;
		static MObject inSH;

		static  MObject controlPlane;
		static  MObject inCPSW;
		static  MObject inCPSH;

		static  MObject outSurface;

	public:
		static constexpr char const* nodeName() {
			return "BSplineSurfaceNode";
		}

		static  void* creator();
		static  MStatus initialize();
	};
}
