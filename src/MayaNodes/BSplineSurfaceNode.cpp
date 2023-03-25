#include "BSplineSurfaceNode.h"

#include <maya/MGlobal.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>

#include <maya/MIOStream.h>
#include <maya/MTime.h>
#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>

#include <maya/MDagPath.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>
#include <maya/MVector.h>

#include <maya/MFnStringData.h>
#include <maya/MFnMeshData.h>

#include "BsplineSurface.h"
#include "ControlPlaneNode.h"

MTypeId BSplineSurfaceNode::id(0x80000);

MObject BSplineSurfaceNode::inSW;
MObject BSplineSurfaceNode::inSH;

MObject BSplineSurfaceNode::controlPlane;
MObject BSplineSurfaceNode::inCPSW;
MObject BSplineSurfaceNode::inCPSH;
MObject BSplineSurfaceNode::outSurface;

#define MAKE_INPUT(attr) attr.setKeyable(true);\
                         attr.setStorable(true);\
                         attr.setReadable(true);\
                         attr.setWritable(true);

#define MAKE_OUT(attr)   attr.setKeyable(false);\
                         attr.setStorable(false);\
                         attr.setReadable(true);\
                         attr.setWritable(false);

#define Index(h_size, w, h) ((h_size) * (w) + (h))

void* BSplineSurfaceNode::creator()
{
    return new BSplineSurfaceNode();
}

MStatus BSplineSurfaceNode::initialize()
{
    
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    MFnUnitAttribute uAttr;

    MStatus returnStatus;

    // output geometry
    BSplineSurfaceNode::outSurface = tAttr.create("OSurface", "osurface", MFnData::kMesh, &returnStatus);
    MAKE_OUT(tAttr)
    addAttribute(BSplineSurfaceNode::outSurface);

    // input attributes
    BSplineSurfaceNode::controlPlane = tAttr.create("ControlPlane", "cp", MFnData::kMesh, &returnStatus);
    MAKE_INPUT(tAttr)
    addAttribute(BSplineSurfaceNode::controlPlane);

    // input attributes
    BSplineSurfaceNode::inCPSW = nAttr.create("CPSubdivisionWidth", "cpsw", MFnNumericData::kInt, 4);
    MAKE_INPUT(nAttr)
    addAttribute(BSplineSurfaceNode::inCPSW);

    // input attributes
    BSplineSurfaceNode::inCPSH = nAttr.create("CPSubdivisionHeight", "cpsh", MFnNumericData::kInt, 4);
    MAKE_INPUT(nAttr)
    addAttribute(BSplineSurfaceNode::inCPSH);

    returnStatus = attributeAffects(BSplineSurfaceNode::controlPlane, BSplineSurfaceNode::outSurface);
    return returnStatus;
}

MStatus BSplineSurfaceNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus returnStatus;
    if (plug == BSplineSurfaceNode::outSurface) {

        /* Get controlPlane's subdivision width & height */
        MDataHandle swData = data.inputValue(BSplineSurfaceNode::inCPSW, &returnStatus);
        int cpsw = swData.asInt();
        MDataHandle shData = data.inputValue(BSplineSurfaceNode::inCPSH, &returnStatus);
        int cpsh = shData.asInt();

        MDataHandle controlPlaneData = data.inputValue(BSplineSurfaceNode::controlPlane, &returnStatus);
        MObject controlPlane = controlPlaneData.asMesh();

        // get control points from control plane
        MDagPath dummyDagPath;
        MItMeshVertex vertexIter(dummyDagPath);
        vertexIter.reset(controlPlane);
        std::vector<std::vector<glm::vec3>> controlPoints;
        ControlPlaneNode::GetControlPoints(cpsw, cpsh, vertexIter, controlPoints);
        
        MFnMeshData dataCreator;
        MObject newOutputData = dataCreator.create(&returnStatus);

        // make new Mesh for output BSPlineSurface surface
        this->MakeMesh(controlPoints, newOutputData, returnStatus);

        MDataHandle outputHandle = data.outputValue(BSplineSurfaceNode::outSurface, &returnStatus);
        outputHandle.set(newOutputData);
        data.setClean(plug);
    }
    else
        return MS::kUnknownParameter;
    return MS::kSuccess;
}

void BSplineSurfaceNode::MakeMesh(std::vector<std::vector<glm::vec3>>& controlPoints, MObject& meshData, MStatus& status)
{
    // construct BSpline Surface
    BsplineSurface bs00{ 3, 3 };

    for (auto& cp : controlPoints)
    {
        bs00.addVector(cp);
    }
    bs00.makeKnots();

    // make new mesh
    MPointArray points;
    MIntArray faceCounts;
    MIntArray faceConnects;

    float size = 15.f;
    glm::vec3 pt;

    for (int u = 0; u <= static_cast<int>(size); ++u)
    {
        for (int v = 0; v <= static_cast<int>(size); ++v)
        {
            bs00.surfacePoint(static_cast<float>(u) / size, static_cast<float>(v) / size, pt);
            points.append(MPoint(pt.x, pt.y, pt.z));
        }
    }

    // make points, faceCounts, and face Connects
    for (int w = 1; w <= size; ++w) // totoally size * size faces
    {
        for (int h = 1; h <= size; ++h)
        {
            faceConnects.append(Index(size + 1, w - 1, h - 1));
            faceConnects.append(Index(size + 1, w - 1, h));
            faceConnects.append(Index(size + 1, w, h));
            faceConnects.append(Index(size + 1, w, h - 1));
            faceCounts.append(4);
        }
    }
    
    MObject newMesh = MFnMesh().create(points.length(), faceCounts.length(), points, faceCounts, faceConnects, meshData, &status);
}