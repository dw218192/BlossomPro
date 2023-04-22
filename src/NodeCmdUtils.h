#pragma once
#include <maya/MDagPath.h>

#include "Utils.h"
#include <maya/MPlug.h>
#include <maya/MObject.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDGModifier.h>

namespace NodeCmdUtils {
    template<typename T>
    MStatus updateAttr(MObject const& node, char const* attrName, T&& value) noexcept {
        MStatus status = MStatus::kSuccess;
        MFnDependencyNode const fnNode{ node };

        MPlug plug = fnNode.findPlug(attrName, false, &status);
        CHECK(status, status);
        {
            std::decay_t<T> test;
            status = plug.getValue(test);
            CHECK(status, status);

            if (test != value) {
                status = plug.setValue(std::forward<T>(value));
                CHECK(status, status);
            }
        }
        return status;
    }
    template<typename T>
    MStatus updateArrayAttr(MObject const& node, char const* attrName, unsigned int i, T&& value) noexcept {
        MStatus status = MStatus::kSuccess;
        MFnDependencyNode const fnNode{ node };

        MPlug const plug = fnNode.findPlug(attrName, false, &status);
        CHECK(status, status);
        {
            if (plug.isArray()) {
                MPlug elementPlug = plug.elementByLogicalIndex(i, &status);
                CHECK(status, status);

                std::decay_t<T> test;
                elementPlug.getValue(test);

                if (test != value) {
                    status = elementPlug.setValue(std::forward<T>(value));
                    CHECK(status, status);
                }
            }
        }
        return status;
    }
    template<typename T>
    Result<T> getAttrValue(MObject const& node, char const* attrName) noexcept {
        MStatus status = MStatus::kSuccess;

        MFnDependencyNode const fnNode{ node };

        MPlug const plug = fnNode.findPlug(attrName, false, &status);
        CHECK(status, status);

        std::decay_t<T> ret;
        status = plug.getValue(ret);
        CHECK(status, status);

        return ret;
    }

    inline MStatus connectAttr(MObject const& from, char const* fromAttr, MObject const& to, char const* toAttr) noexcept {
        MStatus status = MStatus::kSuccess;
        MDGModifier dgModifier;
        MFnDependencyNode const fnFrom{ from }, fnTo{ to };
        MPlug const fromPlug = fnFrom.findPlug(fromAttr, false, &status);
        CHECK(status, status);

        MPlug const toPlug = fnTo.findPlug(toAttr, false, &status);
        CHECK(status, status);

        if (fromPlug.isArray() && toPlug.isArray()) {
            unsigned int const numElements = fromPlug.numElements();
            for (unsigned int i = 0; i < numElements; ++i) {
                MPlug fromElement = fromPlug.elementByLogicalIndex(i, &status);
                MPlug toElement = toPlug.elementByLogicalIndex(i, &status);
                dgModifier.connect(fromElement, toElement);
            }
        }
        else {
            dgModifier.connect(fromPlug, toPlug);
        }

        dgModifier.doIt();

        return status;
    }

    inline Result<MString> getName(MObject const& obj) noexcept {
        MStatus status = MStatus::kSuccess;
        MFnDependencyNode const fnBranchNode{ obj, &status };
        CHECK(status, status);
        MString ret = fnBranchNode.name(&status);
        CHECK(status, status);
        return ret;
    }

    inline MStatus rename(MObject const& obj, MString const& newName) noexcept {
        MStatus status = MStatus::kSuccess;
        MFnDependencyNode depNode{ obj, &status };
        CHECK(status, status);

        depNode.setName(newName, false, &status);
        CHECK(status, status);

        return status;
    }

    inline Result<MObject> getShape(MObject const& transformObj) noexcept {
        MStatus status = MStatus::kSuccess;
        MFnDagNode const dagNode{ transformObj, &status };
        CHECK(status, status);

        MDagPath dagPath;
        status = dagNode.getPath(dagPath);
        CHECK(status, status);

        status = dagPath.extendToShape();
        CHECK(status, status);

        MObject shapeObj = dagPath.node(&status);
        CHECK(status, status);

        return shapeObj;
    }
};