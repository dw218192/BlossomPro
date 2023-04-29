#include "CurveInstanceNode.h"
#include "../Utils.h"
#include <memory>
#include "CurveLenFunction/CurveInfo.h"

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
#include <maya/MMatrix.h>
#include <maya/MTime.h>


#include <glm/glm.hpp>
#include <format>
#include <ctime>

#define MAKE_INPUT(attr) //attr.setKeyable(true);\
                         //attr.setStorable(true);\
                         //attr.setReadable(true);\
                         //attr.setWritable(true);
                         //
#define MAKE_OUT(attr)   //attr.setKeyable(false);\
                         //attr.setStorable(false);\
                         //attr.setReadable(true);\
                         //attr.setWritable(false);

MTypeId CurveInstanceNode::id(0x80002);
MObject CurveInstanceNode::instanceCount;
MObject CurveInstanceNode::rotateMagnitude;
MObject CurveInstanceNode::scaleMagnitude;
MObject CurveInstanceNode::randSeed;
MObject CurveInstanceNode::yRandMagnitude;
MObject CurveInstanceNode::offsetRandMagnitude;
MObject CurveInstanceNode::rotateRandMagnitude;

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
inline float toRadians(float radiance) { return glm::radians(radiance); }

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
    nAttr.setMin(3.0);
    MAKE_INPUT(nAttr)
    addAttribute(CurveInstanceNode::instanceCount);

    CurveInstanceNode::rotateMagnitude = nAttr.create("RotateMagnitude", "rmagnitude", MFnNumericData::kFloat, 1.f);
    nAttr.setMin(-1.0);
    nAttr.setMax(2.0);
    MAKE_INPUT(nAttr)
    addAttribute(CurveInstanceNode::rotateMagnitude);

    CurveInstanceNode::scaleMagnitude = nAttr.create("ScaleMagnitude", "smagnitude", MFnNumericData::kFloat, 1.f);
    nAttr.setMin(0.1f);
    nAttr.setMax(2.0);
    MAKE_INPUT(nAttr)
    addAttribute(CurveInstanceNode::scaleMagnitude);

    CurveInstanceNode::yRandMagnitude = nAttr.create("YRandMagnitude", "yrand", MFnNumericData::kFloat, 0.f);
    nAttr.setMin(0.f);
    nAttr.setMax(5.f);
    MAKE_INPUT(nAttr)
    addAttribute(CurveInstanceNode::yRandMagnitude);

    CurveInstanceNode::offsetRandMagnitude = nAttr.create("OffsetRandMagnitude", "offsetrand", MFnNumericData::kFloat, 0.f);
    nAttr.setMin(0.f);
    nAttr.setMax(2.f);
    MAKE_INPUT(nAttr)
    addAttribute(CurveInstanceNode::offsetRandMagnitude);

    CurveInstanceNode::rotateRandMagnitude = nAttr.create("RotateRandMagnitude", "rotaterand", MFnNumericData::kFloat, 0.f);
    nAttr.setMin(0.f);
    nAttr.setMax(10.f);
    MAKE_INPUT(nAttr)
    addAttribute(CurveInstanceNode::rotateRandMagnitude);

    unsigned long long seed = std::time(0);
    CurveInstanceNode::randSeed = nAttr.create("RandSeed", "seed", MFnNumericData::kLong, seed);
    nAttr.setMin(0);
    MAKE_INPUT(nAttr)
    addAttribute(CurveInstanceNode::randSeed);

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
    returnStatus = attributeAffects(CurveInstanceNode::rotateMagnitude, CurveInstanceNode::outTransforms);
    returnStatus = attributeAffects(CurveInstanceNode::scaleMagnitude, CurveInstanceNode::outTransforms);
    returnStatus = attributeAffects(CurveInstanceNode::randSeed, CurveInstanceNode::outTransforms);
    returnStatus = attributeAffects(CurveInstanceNode::yRandMagnitude, CurveInstanceNode::outTransforms);
    returnStatus = attributeAffects(CurveInstanceNode::offsetRandMagnitude, CurveInstanceNode::outTransforms);
    returnStatus = attributeAffects(CurveInstanceNode::rotateRandMagnitude, CurveInstanceNode::outTransforms);

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

        float rotate_magnitude = data.inputValue(CurveInstanceNode::rotateMagnitude, &returnStatus).asFloat();
        CHECK(returnStatus, returnStatus);

        float scale_magnitude = data.inputValue(CurveInstanceNode::scaleMagnitude, &returnStatus).asFloat();
        CHECK(returnStatus, returnStatus);

        unsigned long long seed = data.inputValue(CurveInstanceNode::randSeed, &returnStatus).asLong();
        CHECK(returnStatus, returnStatus);

        float yrand_magnitude = data.inputValue(CurveInstanceNode::yRandMagnitude, &returnStatus).asFloat();
        CHECK(returnStatus, returnStatus);

        float offset_magnitude = data.inputValue(CurveInstanceNode::offsetRandMagnitude, &returnStatus).asFloat();
        CHECK(returnStatus, returnStatus);

        float rrand_magnitude = data.inputValue(CurveInstanceNode::rotateRandMagnitude, &returnStatus).asFloat();
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

        MVector baseVec(1, 0, 0);

        MyRand rng(seed);

        for (int i = 0; i < instanceCount; ++i)
        {
            float t = i + rng.Rand(-offset_magnitude, offset_magnitude);
            float m = instanceCount;
            t = glm::mod(glm::mod(t, m) + m, m);

            MVector point = curveInfo->getPoint(Lerp<float>(0.f, length, static_cast<float>(t) / static_cast<float>(instanceCount)));
            point.y += rng.Rand(-yrand_magnitude, yrand_magnitude);
            positions.append(point);

            // make the instanced object toward the center
            MVector dir = center - point;
            dir.y = 0.f;

            MEulerRotation inv_obj_rotate(toRadians(rotate[0]), toRadians(rotate[1]), toRadians(rotate[2]));
            inv_obj_rotate = inv_obj_rotate.inverse();

            MQuaternion toward_center = baseVec.rotateTo(dir);

            MEulerRotation obj_rotate(toRadians(rotate[0] + rng.Rand(-rrand_magnitude, rrand_magnitude)), 
                                      toRadians(rotate[1] + rng.Rand(-rrand_magnitude, rrand_magnitude)), 
                                      rotate_magnitude * toRadians(rotate[2] + rng.Rand(-rrand_magnitude, rrand_magnitude)));

            MEulerRotation result;
            result = obj_rotate.asMatrix() * inv_obj_rotate.asMatrix() * toward_center.asMatrix();

            MVector rv;
            rv.x = toDegree(result.x);
            rv.y = toDegree(result.y);
            rv.z = toDegree(result.z);
            
            rotations.append(rv);
            MVector s(scale_magnitude, scale_magnitude, scale_magnitude);
            scales.append(s);
        }

        data.outputValue(CurveInstanceNode::outTransforms).set(aadObj);
    }
    else
        return MS::kUnknownParameter;
    return MS::kSuccess;
}