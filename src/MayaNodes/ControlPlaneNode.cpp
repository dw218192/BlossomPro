#include "ControlPlaneNode.h"

#include <maya/MGlobal.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>

#include <maya/MIOStream.h>
#include <maya/MTime.h>
#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MPlug.h>

#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFloatArray.h>
#include <maya/MVector.h>

#include <maya/MFnStringData.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnFloatVectorArrayData.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFnFloatArrayData.h>
#include "CPPassthroughNode.h"
#define MAKE_INPUT(attr) attr.setKeyable(true);\
                         attr.setStorable(true);\
                         attr.setReadable(true);\
                         attr.setWritable(true);

#define MAKE_OUT(attr)   attr.setKeyable(false);\
                         attr.setStorable(true);\
                         attr.setReadable(true);\
                         attr.setWritable(false);

#define Index(h_size, w, h) ((h_size) * (w) + (h))
#define Lerp(a, b, t) ((1.f - (t)) * (a) + (t) * (b))

namespace BSplineSurfaceNode
{
    MTypeId ControlPlaneNode::id(0x80001);

    MObject ControlPlaneNode::iRowCount;
    MObject ControlPlaneNode::iColumCount;

    MObject ControlPlaneNode::iControlPlane;
    MObject ControlPlaneNode::oControlPlane;
    MObject ControlPlaneNode::oOldPointArray;

    void ControlPlaneNode::ConnectVertices(const int& row, const int& column, MIntArray& faceCounts, MIntArray& faceConnects)
    {
        int endIdx = (row + 1) * (column + 1);
        for (int i = 1; i <= row; ++i)
        {
            faceCounts.append(3);
            faceConnects.append(endIdx);
            faceConnects.append(i * (column + 1) - 1);
            faceConnects.append((i + 1) * (column + 1) - 1);
        }

        // make face connects
        for (int w = 1; w <= row; ++w) // totoally size x size faces
        {
            for (int h = 1; h <= column; ++h)
            {
                faceCounts.append(4);
                faceConnects.append(Index(column + 1, w - 1, h - 1));
                faceConnects.append(Index(column + 1, w - 1, h));
                faceConnects.append(Index(column + 1, w, h));
                faceConnects.append(Index(column + 1, w, h - 1));
            }
        }
    }
    void ControlPlaneNode::ComputeInitPlane(const int& defaultRc, const int& defaultCc, MPointArray& points)
    {
        float min_x = -1.f;
        float min_z = -1.f;
        float max_x = 1.f;
        float max_z = 1.f;

        // add points
        int vertexR = 2 * defaultRc + 1;
        int vertexC = defaultCc + 1;
        for (int r = 0; r <= 2 * defaultRc; ++r)
        {
            for (int c = 0; c <= defaultCc; ++c)
            {
                MPoint point = MPoint(
                    Lerp(min_x, max_x, static_cast<float>(c) / static_cast<float>(defaultCc)),
                    0.f,
                    Lerp(min_z, max_z, static_cast<float>(r) / static_cast<float>(2 * defaultRc))
                );
                points.append(point);
            }
        }

        points.append(MPoint(
            max_x + 0.5 * (max_x - min_x),
            0.f,
            0.5 * (max_z + min_z)
        ));
    }

    void ControlPlaneNode::Init(const int& defaultRc, const int& defaultCc, MObject& outData, MStatus& stat)
    {
        // make new mesh
        MPointArray points;
        MIntArray faceCounts;
        MIntArray faceConnects;

        float min_x = -1.f;
        float min_z = -1.f;
        float max_x = 1.f;
        float max_z = 1.f;

        ComputeInitPlane(defaultRc, defaultCc, points);

        // setup faceCounts and faceConnects
        ConnectVertices(2 * defaultRc, defaultCc, faceCounts, faceConnects);

        MObject newMesh = MFnMesh().create(points.length(), faceCounts.length(), points, faceCounts, faceConnects, outData, &stat);
    }

    void* ControlPlaneNode::creator()
    {
        return new ControlPlaneNode();
    }

    MStatus ControlPlaneNode::initialize()
    {
        MFnNumericAttribute nAttr;
        MFnTypedAttribute tAttr;
        MFnUnitAttribute uAttr;

        MStatus returnStatus;

        //default values
        int defaultRC = 2;
        int defaultCC = 3;

        MFnMeshData dataCreator;
        MObject newOutputData = dataCreator.create(&returnStatus);
        Init(defaultRC, defaultCC, newOutputData, returnStatus);

        // input attributes
        ControlPlaneNode::iRowCount = nAttr.create("RowCount", "rc", MFnNumericData::kInt, defaultRC);
        nAttr.setMin(2.0);
        MAKE_INPUT(nAttr)
        addAttribute(ControlPlaneNode::iRowCount);

        // input attributes
        ControlPlaneNode::iColumCount = nAttr.create("ColumCount", "cc", MFnNumericData::kInt, defaultCC);
        nAttr.setMin(3.0);
        MAKE_INPUT(nAttr)
        addAttribute(ControlPlaneNode::iColumCount);

        ControlPlaneNode::iControlPlane = tAttr.create("InContorlPlane", "icp", MFnData::kMesh, &returnStatus);
        MAKE_INPUT(tAttr)
        addAttribute(ControlPlaneNode::iControlPlane);

        // output geometry
        ControlPlaneNode::oControlPlane = tAttr.create("ContorlPlane", "cp", MFnData::kMesh, &returnStatus);
        tAttr.setDefault(newOutputData);
        MAKE_OUT(tAttr)
        addAttribute(ControlPlaneNode::oControlPlane);

        // output geometry
        ControlPlaneNode::oOldPointArray = tAttr.create("OldPointArray", "opa", MFnData::kPointArray, &returnStatus);
        MAKE_OUT(tAttr)
        addAttribute(ControlPlaneNode::oOldPointArray);

        returnStatus = attributeAffects(ControlPlaneNode::iRowCount, ControlPlaneNode::oControlPlane);
        returnStatus = attributeAffects(ControlPlaneNode::iColumCount, ControlPlaneNode::oControlPlane);
        return returnStatus;
    }

    MStatus ControlPlaneNode::compute(const MPlug& plug, MDataBlock& data)
    {
        MStatus returnStatus;
        if (plug == ControlPlaneNode::oControlPlane)
        {
            /* Get subdivision width & height */
            MDataHandle rcData = data.inputValue(ControlPlaneNode::iRowCount, &returnStatus);
            int rowCount = rcData.asInt();
            MDataHandle ccaData = data.inputValue(ControlPlaneNode::iColumCount, &returnStatus);
            int columnCount = ccaData.asInt();

            MDataHandle inPlaneData = data.inputValue(ControlPlaneNode::iControlPlane, &returnStatus);
            MObject inControlPlane = inPlaneData.asMesh();

            MPointArray oldPoints;

            // get control points
            MDagPath dummyDagPath;
            MItMeshVertex vertexIter(dummyDagPath);
            vertexIter.reset(inControlPlane);
            MGlobal::displayInfo("From PT");
            while (!vertexIter.isDone())
            {
                MPoint point = vertexIter.position();
                oldPoints.append(point);
                MString pos;
                pos += point.x;
                pos += " ";
                pos += point.y;
                pos += " ";
                pos += point.z;

                MGlobal::displayInfo(pos);
                vertexIter.next();
            }
            MPointArray points;
            MIntArray faceCounts;
            MIntArray faceConnects;

            // compute control planes with new row & column count
            ComputeInitPlane(rowCount, columnCount, points);

            // setup face connections with new row & column count
            ConnectVertices(rowCount << 1, columnCount, faceCounts, faceConnects);

            // set OldPointArray
            MFnPointArrayData opadata;
            MObject opaObj = opadata.create(oldPoints, &returnStatus);

            MFnPointArrayData testData(opaObj);
            MPointArray arr = testData.array();
            MString info;
            info += " test arr len: ";
            info += arr.length();
            MGlobal::displayInfo(info);

            MDataHandle outputOldPointArray = data.outputValue(ControlPlaneNode::oOldPointArray, &returnStatus);
            outputOldPointArray.set(opaObj);
            outputOldPointArray.setClean();

            // set new out mesh
            MFnMeshData dataCreator;
            MObject newOutputData = dataCreator.create(&returnStatus);
            MObject newMesh = MFnMesh().create(points.length(), faceCounts.length(), points, faceCounts, faceConnects, newOutputData, &returnStatus);
            MDataHandle outputPlaneData = data.outputValue(ControlPlaneNode::oControlPlane, &returnStatus);
            outputPlaneData.set(newOutputData);
            outputPlaneData.setClean();

            data.setClean(plug);
        }
        else
            return MS::kUnknownParameter;
        return MS::kSuccess;
    }
}

