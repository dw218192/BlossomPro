#include "CurveInstanceNode.h"
#include "../Utils.h"
#include <memory>
#include "Phyllotaxis/CurveInfo.h"

#include <maya/MGlobal.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>

#include <maya/MObject.h>
#include <maya/MPoint.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MPointArray.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MVectorArray.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MEulerRotation.h>

#include <glm/glm.hpp>
#include <format>

#define MAKE_INPUT(attr) //attr.setKeyable(true);\
                         //attr.setStorable(true);\
                         //attr.setReadable(true);\
                         //attr.setWritable(true);
                         //
#define MAKE_OUT(attr)   //attr.setKeyable(false);\
                         //attr.setStorable(false);\
                         //attr.setReadable(true);\
                         //attr.setWritable(false);
//attr.setWritable(false);

MTypeId CurveInstanceNode::id(0x80002);
MObject CurveInstanceNode::instanceCount;
MObject CurveInstanceNode::inputCenter;
MObject CurveInstanceNode::inputRotate;
MObject CurveInstanceNode::inputCurve;
MObject CurveInstanceNode::outTransforms;

template<typename T>
inline T Lerp(const T& a, const T&b, const T& u)
{
    return a - a * u + u * b;
}

inline float toDegree(float radiance) { return glm::degrees(radiance); }

void* CurveInstanceNode::creator()
{
    return new CurveInstanceNode();
}

MStatus CurveInstanceNode::initialize()
{
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    MFnUnitAttribute uAttr;

    MStatus returnStatus;

    // input attributes
    CurveInstanceNode::instanceCount = nAttr.create("InstanceCount", "icount", MFnNumericData::kInt, 3);
    nAttr.setMin(4.0);
    MAKE_INPUT(nAttr)
    addAttribute(CurveInstanceNode::instanceCount);

    CurveInstanceNode::inputRotate = nAttr.create("InputRotate", "ir", MFnNumericData::k3Float);
    MAKE_INPUT(tAttr)
    addAttribute(CurveInstanceNode::inputRotate);

    CurveInstanceNode::inputCenter = nAttr.create("InputCenter", "icenter", MFnNumericData::k3Float);
    MAKE_INPUT(tAttr)
    addAttribute(CurveInstanceNode::inputCenter);

    CurveInstanceNode::inputCurve = tAttr.create("InputCurve", "icurve", MFnData::kNurbsCurve, MObject::kNullObj, &returnStatus);
    MAKE_INPUT(tAttr)
    addAttribute(CurveInstanceNode::inputCurve);

    // output attributes
    CurveInstanceNode::outTransforms = tAttr.create("OutTransforms", "otrans", MFnArrayAttrsData::kDynArrayAttrs, MObject::kNullObj, &returnStatus);
    MAKE_OUT(tAttr)
    addAttribute(CurveInstanceNode::outTransforms);

    returnStatus = attributeAffects(CurveInstanceNode::inputCurve, CurveInstanceNode::outTransforms);

    returnStatus = attributeAffects(CurveInstanceNode::inputCenter, CurveInstanceNode::outTransforms);
    returnStatus = attributeAffects(CurveInstanceNode::inputRotate, CurveInstanceNode::outTransforms);
    returnStatus = attributeAffects(CurveInstanceNode::instanceCount, CurveInstanceNode::outTransforms);

    return returnStatus;
}

MStatus CurveInstanceNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus returnStatus;
    if (plug == CurveInstanceNode::outTransforms)
    {
        // get inputs from data
        int instanceCount = data.inputValue(CurveInstanceNode::instanceCount, &returnStatus).asInt();
        CHECK(returnStatus, returnStatus);

        float3& rotate = data.inputValue(CurveInstanceNode::inputRotate, &returnStatus).asFloat3();
        CHECK(returnStatus, returnStatus);

        float3& c = data.inputValue(CurveInstanceNode::inputCenter, &returnStatus).asFloat3();
        CHECK(returnStatus, returnStatus);

        MVector center(c[0], c[1], c[2]);

        MObject curveObj = data.inputValue(CurveInstanceNode::inputCurve, &returnStatus).asNurbsCurveTransformed();
        CHECK(returnStatus, returnStatus);

        // get curve
        auto curveInfo = std::make_unique<CurveInfo>(curveObj, &returnStatus);
        CHECK(returnStatus, returnStatus);

        // configurate output array
        MFnArrayAttrsData arrayAttrsData;
        MObject aadObj = arrayAttrsData.create(&returnStatus);
        CHECK(returnStatus, returnStatus);

        MVectorArray positions = arrayAttrsData.vectorArray("position", &returnStatus);
        CHECK(returnStatus, returnStatus);
        MVectorArray scales = arrayAttrsData.vectorArray("scale", &returnStatus);
        CHECK(returnStatus, returnStatus);
        MVectorArray rotations = arrayAttrsData.vectorArray("rotation", &returnStatus);
        CHECK(returnStatus, returnStatus);

        float length = curveInfo->length();

        MVector baseVec = center - curveInfo->getPoint(0.f);

        for (int i = 0; i < instanceCount; ++i)
        {
            MVector point = curveInfo->getPoint(Lerp<float>(0.f, length, static_cast<float>(i) / static_cast<float>(instanceCount - 1)));
            positions.append(point);

            // make the instanced object toward the center
            MVector dir = center - point;
            MEulerRotation eulerRotate(rotate);
            MQuaternion quat = baseVec.rotateTo(dir);
            MEulerRotation euler = quat.asEulerRotation();
            
            MVector rv;
            rv.x = toDegree(euler.x);
            rv.y = toDegree(euler.y);
            rv.z = toDegree(euler.z);
            
            rotations.append(rv);
        }

        data.outputValue(CurveInstanceNode::outTransforms).set(aadObj);
    }
    else
        return MS::kUnknownParameter;
    return MS::kSuccess;
}