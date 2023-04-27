#pragma once
#include <maya/MDagPath.h>

#include "Utils.h"
#include <maya/MPlug.h>
#include <maya/MObject.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDGModifier.h>
#include <maya/MFnSet.h>
#include <maya/MSelectionList.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>

namespace NodeCmdUtils {
    struct Attribute {
        Attribute(MObject const& obj, char const* attrName) noexcept;

    	template<typename T>
        [[nodiscard]] auto value() const noexcept -> Result<T>;
        template<typename T>
        [[nodiscard]] auto setValue(T&& val) noexcept -> MStatus;
        [[nodiscard]] auto operator[](unsigned int i) const noexcept -> Attribute;

        [[nodiscard]] auto connect(Attribute const& other) const noexcept -> MStatus;
        [[nodiscard]] auto disconnect(Attribute const& other) const noexcept -> MStatus;
    private:
        Attribute() noexcept {}
        bool m_valid;
        MPlug m_plug;
        MObject m_object;
    };


    inline Attribute::Attribute(MObject const& obj, char const* attrName) noexcept {
        MStatus status;
        MFnDependencyNode const fnNode{ obj };
        m_plug = fnNode.findPlug(attrName, false, &status);
        if(MFAIL(status)) {
            m_valid = false;
            return;
        }

        m_object = obj;
        m_valid = true;
    }

    inline auto Attribute::operator[](unsigned i) const noexcept -> Attribute {
        Attribute ret;
        MStatus status;
        ret.m_plug = m_plug.elementByLogicalIndex(i, &status);
        if (MFAIL(status)) {
            ret.m_valid = false;
            return ret;
        }
    	ret.m_object = m_object;
        ret.m_valid = true;
        return ret;
    }

    inline auto Attribute::connect(Attribute const& other) const noexcept -> MStatus {
	    MDGModifier dgModifier;
        MFnDependencyNode const fnFrom{ m_object }, fnTo{ other.m_object };
        MStatus status;
    	if(m_plug.isConnected(&status)) {
            CHECK_RET(status);

            status = dgModifier.disconnect(m_plug, other.m_plug);
            CHECK_RET(status);
    	}
        status = dgModifier.connect(m_plug, other.m_plug);
        CHECK_RET(status);

        status = dgModifier.doIt();
        CHECK_RET(status);

        return status;
    }

    inline auto Attribute::disconnect(Attribute const& other) const noexcept -> MStatus {
        MDGModifier dgModifier;
        MFnDependencyNode const fnFrom{ m_object }, fnTo{ other.m_object };
        MStatus status;

        if (m_plug.isConnected(&status)) {
            CHECK_RET(status);

            status = dgModifier.disconnect(m_plug, other.m_plug);
            CHECK_RET(status);
        }

        status = dgModifier.doIt();
        CHECK_RET(status);

        return status;
    }

    template <typename T>
    auto Attribute::value() const noexcept -> Result<T> {
        if (!m_valid) {
            return MStatus::kInvalidParameter;
        }

        std::decay_t<T> ret;
        MStatus status = m_plug.getValue(ret);
        CHECK_RET(status);

        return ret;
    }

    template <typename T>
    auto Attribute::setValue(T&& val) noexcept -> MStatus {
        if(!m_valid) {
            return MStatus::kInvalidParameter;
        }

        std::decay_t<T> test;

    	MStatus status = m_plug.getValue(test);
        CHECK_RET(status);

        if (test != val) {
            status = m_plug.setValue(std::forward<T>(val));
            CHECK_RET(status);
        }
        return status;
    }

    [[nodiscard]] inline Result<MString> getName(MObject const& obj) noexcept {
        MStatus status = MStatus::kSuccess;
        MFnDependencyNode const fnBranchNode{ obj, &status };
        CHECK(status, status);
        MString ret = fnBranchNode.name(&status);
        CHECK(status, status);
        return ret;
    }

    [[nodiscard]] inline MStatus rename(MObject const& obj, MString const& newName) noexcept {
        MStatus status = MStatus::kSuccess;
        MFnDependencyNode depNode{ obj, &status };
        CHECK(status, status);

        depNode.setName(newName, false, &status);
        CHECK(status, status);

        return status;
    }

    [[nodiscard]] inline MStatus addDefaultShadingGroup(MObject const& meshTransform) {
        MStatus status = MStatus::kSuccess;
        MSelectionList shadingGroupList;
        shadingGroupList.add("initialShadingGroup");

        MObject shadingGroupNode;
        shadingGroupList.getDependNode(0, shadingGroupNode);

        MFnSet shadingGroupSet{ shadingGroupNode, &status };
        CHECK_RET(status);

        status = shadingGroupSet.addMember(meshTransform);
        CHECK_RET(status);
        return status;
    }

    [[nodiscard]] inline Result<MObject> getShape(MObject const& transformObj) noexcept {
        MStatus status = MStatus::kSuccess;
        MFnDagNode const dagNode{ transformObj, &status };
        CHECK_RET(status);

        MDagPath dagPath;
        status = dagNode.getPath(dagPath);
        CHECK_RET(status);

        status = dagPath.extendToShape();
        CHECK_RET(status);

        MObject shapeObj = dagPath.node(&status);
        CHECK_RET(status);

        return shapeObj;
    }

    struct CreateResult {
        MObject shape;
        MObject transform;
    };
    [[nodiscard]] inline Result<CreateResult> createEmptyMesh() noexcept {
        MStatus status;
        CreateResult ret;
        ret.transform = MFnTransform{}.create(MObject::kNullObj, &status);
        CHECK_RET(status);

        ret.shape = MFnMesh{}.create(0, 0, MPointArray{}, MIntArray{}, MIntArray{}, ret.transform, &status);
        CHECK_RET(status);

        return ret;
    }
    [[nodiscard]] inline Result<CreateResult> createEmptyNurbs() noexcept {
        MStatus status;
        CreateResult ret;
        ret.transform = MFnTransform{}.create(MObject::kNullObj, &status);
        CHECK_RET(status);

        MPoint const points[]{
            {0, 1},
            {0, 2},
            {0, 3}
        };
        ret.shape = MFnNurbsCurve{}.createWithEditPoints(
            MPointArray{ points, 3 }, 
            3, MFnNurbsCurve::kOpen, true, true, true, 
            ret.transform, &status);

        CHECK_RET(status);

        return ret;
    }
    [[nodiscard]] inline Result<MObject> createNode(char const* typeName) noexcept {
        MStatus status;
        MObject ret = MFnDependencyNode{}.create(typeName, &status);
        CHECK_RET(status);
    	return ret;
    }
    [[nodiscard]] inline Result<MObject> createNode(MTypeId typeId) noexcept {
        MStatus status;
        MObject ret = MFnDependencyNode{}.create(typeId, &status);
        CHECK_RET(status);
        return ret;
    }
};