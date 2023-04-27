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

MTypeId ControlPlaneNode::id(0x80001);

MObject ControlPlaneNode::iRowCount;
MObject ControlPlaneNode::iColumCount;

MObject ControlPlaneNode::oPreRC;
MObject ControlPlaneNode::oPreCC;

MObject ControlPlaneNode::iCurrentPlane;
MObject ControlPlaneNode::oControlPlane;

#define MAKE_INPUT(attr) //attr.setKeyable(true);\
                         //attr.setStorable(true);\
                         //attr.setReadable(true);\
                         //attr.setWritable(true);
                         //
#define MAKE_OUT(attr)   //attr.setKeyable(false);\
                         //attr.setStorable(false);\
                         //attr.setReadable(true);\
                         //attr.setWritable(false);

#define Index(h_size, w, h) ((h_size) * (w) + (h))
#define Lerp(a, b, t) ((1.f - (t)) * (a) + (t) * (b))

void ControlPlaneNode::GetControlPoints(const int& rowCount,
                                        const int& columnCount,
                                        MItMeshVertex& vertexIter,
                                        std::vector<std::vector<glm::vec3>>& controlPoints)
{
    int rc = rowCount << 1;
    for (int w = 0; w <= rc; ++w)
    {
        std::vector<glm::vec3> cp;
        for (int h = 0; h <= columnCount; ++h)
        {
            const MPoint& point = vertexIter.position();
            cp.emplace_back(point.x, point.y, point.z);
            vertexIter.next();
        }
        controlPoints.emplace_back(std::move(cp));
    }

    const MPoint& endpoint = vertexIter.position();
    glm::vec3 ep{ endpoint.x, endpoint.z, endpoint.z };

    for (auto& cp : controlPoints)
    {
        cp.emplace_back(ep);
    }
}

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
            faceConnects.append(Index(column + 1, w, h - 1));
            faceConnects.append(Index(column + 1, w, h));
            faceConnects.append(Index(column + 1, w - 1, h));
        }
    }
}
void ControlPlaneNode::InitPlane(const int& defaultRc, const int& defaultCc, MPointArray& points, MStatus& stat)
{
    glm::vec3 pMin(-1.f, 0.f, -1.f);
    glm::vec3 pMax(1.f, 0.f, 1.f);
    pMin.x += -2.f;
    pMax.x += -2.f;

    // add points
    int vertexR = 2 * defaultRc + 1;
    int vertexC = defaultCc + 1;
    for (int r = 0; r <= 2 * defaultRc; ++r)
    {
        for (int c = 0; c <= defaultCc; ++c)
        {
            glm::vec3 p = glm::mix(pMin, pMax, glm::vec3(static_cast<float>(c) / static_cast<float>(defaultCc), 0.f, static_cast<float>(r) / static_cast<float>(2 * defaultRc)));
            MPoint point = MPoint(p.x, p.y, p.z);
            points.append(point);
        }
    }
    glm::vec3 root(pMax.x + 0.5 * (pMax - pMin).x, 0.f, 0.5 * (pMax + pMin).z);
    points.append(MPoint(root.x, root.y, root.z));
}
void ControlPlaneNode::Init(const int& defaultRc, const int& defaultCc, MObject& outData, MStatus& stat)
{
    // make new mesh
    MPointArray points;
    MIntArray faceCounts;
    MIntArray faceConnects;

    float min_x = -1.f;
    float min_z = -1.f;
    float max_x =  1.f;
    float max_z =  1.f;

    InitPlane(defaultRc, defaultCc, points, stat);

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

    // output data
    ControlPlaneNode::oPreRC = nAttr.create("PreviousRC", "prc", MFnNumericData::kInt, defaultRC);
    MAKE_INPUT(nAttr)
    addAttribute(ControlPlaneNode::oPreRC);

    // output data
    ControlPlaneNode::oPreCC = nAttr.create("PreviousCC", "pcc", MFnNumericData::kInt, defaultCC);
    MAKE_INPUT(nAttr)
    addAttribute(ControlPlaneNode::oPreCC);

    // output geometry
    ControlPlaneNode::iCurrentPlane = tAttr.create("CurrentPlane", "curp", MFnData::kMesh, &returnStatus);
    MAKE_INPUT(tAttr)
    addAttribute(ControlPlaneNode::iCurrentPlane);

    // output geometry
    ControlPlaneNode::oControlPlane = tAttr.create("ContorlPlane", "cp", MFnData::kMesh, &returnStatus);
    tAttr.setDefault(newOutputData);
    MAKE_OUT(tAttr)
    addAttribute(ControlPlaneNode::oControlPlane);

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

        MDataHandle prcData = data.outputValue(ControlPlaneNode::oPreRC, &returnStatus);
        int prc = prcData.asInt();
        MDataHandle pccData = data.outputValue(ControlPlaneNode::oPreCC, &returnStatus);
        int pcc = pccData.asInt();
        
        if (prc == rowCount && pcc == columnCount)
        {
            return MS::kSuccess;
        }

        // clear tweak of mesh
        if (plug.isConnected())
        {

            MDataHandle planeData = data.inputValue(ControlPlaneNode::iCurrentPlane, &returnStatus);
            MObject controlPlane = planeData.asMesh();

            MPointArray points;
            MPointArray initPoints;
            MIntArray faceCounts;
            MIntArray faceConnects;

            // get control points
            MDagPath dummyDagPath;
            MItMeshVertex vertexIter(dummyDagPath);
            vertexIter.reset(controlPlane);
            std::vector<std::vector<glm::vec3>> controlPoints;
            GetControlPoints(prc, pcc, vertexIter, controlPoints);

            // compute new control plane (initial state) with new row & column count
            InitPlane(rowCount, columnCount, initPoints, returnStatus);

            // compute the new control plane (with offset) after interpolation
            UpdateVertices(rowCount << 1, columnCount, prc << 1, pcc, controlPoints, points);

            // setup face connections with new row & column count
            ConnectVertices(rowCount << 1, columnCount, faceCounts, faceConnects);

            // set new out mesh
            MFnMeshData dataCreator;
            MObject newOutputData = dataCreator.create(&returnStatus);
            MObject newMesh = MFnMesh().create(initPoints.length(), faceCounts.length(), initPoints, faceCounts, faceConnects, newOutputData, &returnStatus);
            MDataHandle outputPlaneData = data.outputValue(ControlPlaneNode::oControlPlane, &returnStatus);
            outputPlaneData.set(newOutputData);
            outputPlaneData.setClean();

            // TODO: move to a function
            // set pnts for output mesh
            // must be called AFTER setting out mesh
            MPlugArray connections;
            plug.connectedTo(connections, false, true);
            MString name = connections[0].name();
            MStringArray strArr;
            name.split('.', strArr);
            MString templateStr = R"(setAttr("^1s.pnts[^2s]") - type "float3" ^3s;)";
            int count = ((rowCount << 1) + 1) * (columnCount + 1) + 1;
            
            for (int i = 0; i < count; ++i)
            {
                MPoint p = points[i] - initPoints[i];
                MString pos;
                pos += p.x;
                pos += " ";
                pos += p.y;
                pos += " ";
                pos += p.z;

                MString id;
                id += i;

                MString cmd;
                cmd.format(templateStr, strArr[0], id, pos);

                MGlobal::displayInfo(cmd);
                MGlobal::executeCommand(cmd);
            }
        }
        // upadte preRC and preCC
        prcData.setInt(rowCount);
        pccData.setInt(columnCount);
        prcData.setClean();
        pccData.setClean();

        data.setClean(plug);
    }
    else
        return MS::kUnknownParameter;
    return MS::kSuccess;
}

glm::vec3 interpolate(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec2 t)
{
    glm::vec3 p1 = glm::mix(a, b, glm::vec3(t.x));
    glm::vec3 p2 = glm::mix(c, d, glm::vec3(t.x));
    return glm::mix(p1, p2, glm::vec3(t.y));
}

void ControlPlaneNode::UpdateVertices(int rowCount,
                                      int columnCount,
                                      int prc,
                                      int pcc,
                                      std::vector<std::vector<glm::vec3>>& cps,
                                      MPointArray& points)
{
    points.clear();
    glm::vec3 pt;
    for (int u = 0; u <= rowCount; ++u)
    {
        for (int v = 0; v <= columnCount; ++v)
        {
            float x = glm::mix(0.f, static_cast<float>(pcc), static_cast<float>(v) / static_cast<float>(columnCount));
            float y = glm::mix(0.f, static_cast<float>(prc), static_cast<float>(u) / static_cast<float>(rowCount));
            int ix = static_cast<int>(glm::floor(x));
            int iy = static_cast<int>(glm::floor(y));
            int bx = ix + 1;
            int cy = iy + 1;
            int dx = ix + 1;
            int dy = iy + 1;
            if (u == rowCount)
            {
                cy = dy = iy;
            }
            if (v == columnCount)
            {
                bx = dx = ix;
            }

            pt = interpolate(cps[iy][ix], cps[iy][bx], cps[cy][ix], cps[dy][dx], glm::vec2(x - ix, y - iy));

            points.append(MPoint(pt.x, pt.y, pt.z));
        }
    }

    pt = cps.back().back();
    points.append(MPoint(pt.x, pt.y, pt.z));
}