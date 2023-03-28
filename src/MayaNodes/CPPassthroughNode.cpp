#include "CPPassthroughNode.h"

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

#define MAKE_INPUT(attr) attr.setKeyable(true);\
                         attr.setStorable(true);\
                         attr.setReadable(true);\
                         attr.setWritable(true);

#define MAKE_OUT(attr)   attr.setKeyable(false);\
                         attr.setStorable(true);\
                         attr.setReadable(true);
                         //attr.setWritable(false);

#define Index(h_size, w, h) ((h_size) * (w) + (h))
#define Lerp(a, b, t) ((1.f - (t)) * (a) + (t) * (b))

namespace BSplineSurfaceNode
{
    MTypeId CPPassthroughNode::id(0x80002);

    MObject CPPassthroughNode::iRowCount;
    MObject CPPassthroughNode::iColumCount;

    MObject CPPassthroughNode::oPreRC;
    MObject CPPassthroughNode::oPreCC;

    MObject CPPassthroughNode::iOldPointArray;

    MObject CPPassthroughNode::iControlPlane;
    MObject CPPassthroughNode::oControlPlane;

    void CPPassthroughNode::GetControlPoints(const int& rowCount,
                                             const int& columnCount,
                                             MItMeshVertex& vertexIter,
                                             std::vector<std::vector<glm::vec3>>& controlPoints)
    {
        MGlobal::displayInfo("From CPPT");
        int rc = rowCount << 1;
        for (int w = 0; w <= rc; ++w)
        {
            std::vector<glm::vec3> cp;
            for (int h = 0; h <= columnCount; ++h)
            {
                const MPoint& point = vertexIter.position();

                MString pos;
                pos += point.x;
                pos += " ";
                pos += point.y;
                pos += " ";
                pos += point.z;

                MGlobal::displayInfo(pos);

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

    void CPPassthroughNode::SetPnts(int count, const MString& name, const MPointArray& offsets)
    {
        for (int i = 0; i < count; ++i)
        {
            const MPoint& p = offsets[i];
            MString pos;
            pos += p.x;
            pos += " ";
            pos += p.y;
            pos += " ";
            pos += p.z;

            MString id;
            id += i;

            MString cmd;
            cmd.format(SetPntsTemplateStr, name, id, pos);
            MGlobal::displayInfo(cmd);
            MGlobal::executeCommand(cmd);
        }
    }

    void* CPPassthroughNode::creator()
    {
        return new CPPassthroughNode();
    }

    MStatus CPPassthroughNode::initialize()
    {
        MFnNumericAttribute nAttr;
        MFnTypedAttribute tAttr;
        MFnUnitAttribute uAttr;

        MStatus returnStatus;

        // input attributes
        CPPassthroughNode::iRowCount = nAttr.create("RowCount", "rc", MFnNumericData::kInt, 0);
        nAttr.setMin(2.0);
        MAKE_INPUT(nAttr)
        addAttribute(CPPassthroughNode::iRowCount);

        // input attributes
        CPPassthroughNode::iColumCount = nAttr.create("ColumCount", "cc", MFnNumericData::kInt, 0);
        nAttr.setMin(3.0);
        MAKE_INPUT(nAttr)
        addAttribute(CPPassthroughNode::iColumCount);

        // output data
        CPPassthroughNode::oPreRC = nAttr.create("PreviousRC", "prc", MFnNumericData::kInt, -1);
        MAKE_INPUT(nAttr)
        addAttribute(CPPassthroughNode::oPreRC);

        // output data
        CPPassthroughNode::oPreCC = nAttr.create("PreviousCC", "pcc", MFnNumericData::kInt, -1);
        MAKE_INPUT(nAttr)
        addAttribute(CPPassthroughNode::oPreCC);

        // input attributes
        CPPassthroughNode::iOldPointArray = tAttr.create("iOldPointArray", "iopa", MFnData::kPointArray, &returnStatus);
        MAKE_INPUT(tAttr)
        addAttribute(CPPassthroughNode::iOldPointArray);

        // output geometry
        CPPassthroughNode::iControlPlane = tAttr.create("inContorlPlane", "icp", MFnData::kMesh, &returnStatus);
        MAKE_INPUT(tAttr)
        addAttribute(CPPassthroughNode::iControlPlane);

        // output geometry
        CPPassthroughNode::oControlPlane = tAttr.create("outContorlPlane", "ocp", MFnData::kMesh, &returnStatus);
        MAKE_OUT(tAttr)
        addAttribute(CPPassthroughNode::oControlPlane);

        returnStatus = attributeAffects(CPPassthroughNode::iControlPlane, CPPassthroughNode::oControlPlane);
        return returnStatus;
    }

    MStatus CPPassthroughNode::compute(const MPlug& plug, MDataBlock& data)
    {
        MStatus returnStatus;
        

        if (plug == CPPassthroughNode::oControlPlane)
        {
            MGlobal::displayInfo("PassThroughNode Compute");

            /* Get subdivision width & height */
            MDataHandle rcData = data.inputValue(CPPassthroughNode::iRowCount, &returnStatus);
            int rowCount = rcData.asInt();
            MDataHandle ccaData = data.inputValue(CPPassthroughNode::iColumCount, &returnStatus);
            int columnCount = ccaData.asInt();

            MDataHandle prcData = data.outputValue(CPPassthroughNode::oPreRC, &returnStatus);
            int prc = prcData.asInt();
            MDataHandle pccData = data.outputValue(CPPassthroughNode::oPreCC, &returnStatus);
            int pcc = pccData.asInt();

            MDataHandle inPlaneData = data.inputValue(CPPassthroughNode::iControlPlane, &returnStatus);
            MObject inControlPlane = inPlaneData.asMesh();

            //if (prc != rowCount || pcc != columnCount)
            {
                if (prc > 0 && pcc > 0)
                {
                    if (pass = !pass)
                    {
                        return MS::kSuccess;
                    }

                    MPointArray oldPoints;

                    MObject thisNode = thisMObject();
                    MPlug plug_aPoints(thisNode, CPPassthroughNode::iOldPointArray);
                    MObject o_aPoints;
                    plug_aPoints.getValue(o_aPoints);
                    MFnPointArrayData fn_aPoints(o_aPoints);
                    fn_aPoints.copyTo(oldPoints);

                    std::vector<std::vector<glm::vec3>> controlPoints;

                    int rc = prc << 1;
                    for (int w = 0; w <= rc; ++w)
                    {
                        std::vector<glm::vec3> cp;
                        for (int h = 0; h <= pcc; ++h)
                        {
                            const MPoint& point = oldPoints[Index(pcc + 1, w, h)];

                            MString pos;
                            pos += point.x;
                            pos += " ";
                            pos += point.y;
                            pos += " ";
                            pos += point.z;

                            MGlobal::displayInfo(pos);

                            cp.emplace_back(point.x, point.y, point.z);
                        }
                        controlPoints.emplace_back(std::move(cp));
                    }

                    MPoint& endpoint = oldPoints[oldPoints.length() - 1];

                    glm::vec3 ep{ endpoint.x, endpoint.z, endpoint.z };

                    for (auto& cp : controlPoints)
                    {
                        cp.emplace_back(ep);
                    }

                    //GetControlPoints(prc, pcc, vertexIter, controlPoints);

                    MPointArray initPoints;
                    MPointArray newPoints;

                    // compute new control plane (initial state) with new row & column count
                    ControlPlaneNode::ComputeInitPlane(rowCount, columnCount, initPoints);

                    // compute the new control plane (with offset) after interpolation
                    ComputeNewVertices(rowCount << 1, columnCount, prc << 1, pcc, controlPoints, newPoints);

                    // compute offset
                    MPointArray offsets;
                    for (unsigned int i = 0; i < newPoints.length(); ++i)
                    {
                        offsets.append(newPoints[i] - initPoints[i]);
                    }

                    MFnMesh controlPlane(inControlPlane, &returnStatus);

                    // set pnts
                    MPlugArray connections;
                    plug.connectedTo(connections, true, true);
                    for (auto& plug : connections)
                    {
                        MGlobal::displayInfo(plug.name());
                    }
                    MString name = "cShape1";
                    //MStringArray strArr;
                    //name.split('.', strArr);
                    MString templateStr = R"(setAttr("^1s.pnts[^2s]") - type "float3" ^3s;)";
                    int count = ((rowCount << 1) + 1) * (columnCount + 1) + 1;
                    SetPnts(count, name, offsets);
                }
            }

            // set new out mesh
            MDataHandle outputPlaneData = data.outputValue(CPPassthroughNode::oControlPlane, &returnStatus);
            outputPlaneData.set(inControlPlane);
            //outputPlaneData.setClean();
            
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

    void CPPassthroughNode::ComputeNewVertices(int rowCount,
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
}
