#include "BranchEditor.h"
#include "Utils.h"
#include "NodeCmdUtils.h"

#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MFnSet.h>

#include <array>

using namespace NodeCmdUtils;

BranchEditor::Inputs BranchEditor::getInputs() const {
    Inputs ret;
    ret.funcs.yawRate = m_ui.keyframeCurveEditor->getFunction();
    ret.funcs.pitchRate = m_ui.keyframeCurveEditor_2->getFunction();
    ret.funcs.rollRate = m_ui.keyframeCurveEditor_3->getFunction();
    ret.funcs.twistRate = m_ui.keyframeCurveEditor_4->getFunction();
    ret.funcs.widthRate = m_ui.keyframeCurveEditor_5->getFunction();
    ret.numIter = m_ui.numIterSpinBpx->value();
    ret.step = m_ui.integStepDoubleBox->value();
    ret.length = m_ui.lengthDoubleBox->value();

    return ret;
}

MStatus BranchEditor::createNetwork(MSelectionList const& selection) {
    MStatus status = MStatus::kSuccess;

    status = selection.getDependNode(0, m_network.generatingCurve);
    CHECK_RET(status);

    auto const shapeObj = getShape(m_network.generatingCurve);
    CHECK_RES(shapeObj);

    m_network.generatingCurveShape = shapeObj.value();

    if (m_network.loftNodeObj.isNull()) {
        m_network.loftNodeObj = MFnDependencyNode{}.create("loft", &status);
        CHECK_RET(status);
    }

    updateAttr(m_network.loftNodeObj, "reverseSurfaceNormals", true);

    if (m_network.tessNodeObj.isNull()) {
        m_network.tessNodeObj = MFnDependencyNode{}.create("nurbsTessellate", &status);
        CHECK_RET(status);
    }
    if (m_network.transformObj.isNull()) {
        m_network.transformObj = MFnTransform{}.create(MObject::kNullObj, &status);
        CHECK_RET(status);
    }

    if (m_network.meshObj.isNull()) {
        m_network.meshObj = MFnMesh{}.create(0, 0, MPointArray{}, MIntArray{}, MIntArray{}, m_network.transformObj, &status);
        CHECK_RET(status);
    }

    connectAttr(m_network.loftNodeObj, "outputSurface", m_network.tessNodeObj, "inputSurface");
    connectAttr(m_network.tessNodeObj, "outputPolygon", m_network.meshObj, "inMesh");

    // Add mesh to the initial shading group
    MSelectionList shadingGroupList;
    shadingGroupList.add("initialShadingGroup");

    MObject shadingGroupNode;
    shadingGroupList.getDependNode(0, shadingGroupNode);

    MFnSet shadingGroupSet{ shadingGroupNode, &status };
    CHECK_RET(status);

    status = shadingGroupSet.addMember(m_network.meshObj);
    CHECK_RET(status);

    status = updateNetwork(m_network);
    CHECK_RET(status);

    return status;
}

BranchEditor::BranchEditor(QWidget* parent) : QDialog{ parent } {
	m_ui.setupUi(this);
}

BranchEditor::~BranchEditor() {
	
}

MStatus BranchEditor::updateNetwork(BranchNodeNetwork const& network) {
    if(network.loftNodeObj.isNull()) {
        return MStatus::kSuccess;
    }

    MStatus status;
	auto [funcs, numIter, step, length] = getInputs();

    auto const grammar = std::make_unique<GeneralizedCylinderGrammar>(funcs, length, step);
    HANDLE_EXCEPTION(grammar->process(numIter));

    MPlug const inputCurvePlug = MFnDependencyNode{ network.loftNodeObj }
		.findPlug("inputCurve", false, &status);
    CHECK_RET(status);

    // clear inputCurve Plug
    MDGModifier dgModifier;

    unsigned int const numConnectedElements = inputCurvePlug.numConnectedElements(&status);
    CHECK_RET(status);
    for (unsigned int i = 0; i < numConnectedElements; ++i) {
        MPlug connectedPlug = inputCurvePlug.connectionByPhysicalIndex(i, &status);
        CHECK_RET(status);
        status = dgModifier.disconnect(connectedPlug, inputCurvePlug.elementByLogicalIndex(i));
        CHECK_RET(status);
    }

	dgModifier.doIt();

    auto addCurve = [&inputCurvePlug, &dgModifier](unsigned int index, MObject curveShapeObj) {
        MStatus status = MStatus::kSuccess;
        MPlug const plug = inputCurvePlug.elementByLogicalIndex(index, &status);
        CHECK_RET(status);

        MPlug const worldSpaceArrayPlug = MFnDependencyNode{ curveShapeObj }
    		.findPlug("worldSpace", false, &status);
        CHECK_RET(status);

        MPlug const worldSpacePlug = worldSpaceArrayPlug.elementByLogicalIndex(0, &status);
        CHECK_RET(status);

        status = dgModifier.connect(worldSpacePlug, plug);
        CHECK_RET(status);

        return status;
    };

    unsigned int idx = 0;
    status = addCurve(idx++, network.generatingCurveShape);
    CHECK_RET(status);

    for (auto const& [pos, rot, scale] : grammar->result()) {
        // NOTE: this creates a transform node with a shape node all at once
    	MObject curveObj = MFnNurbsCurve{}.copy(m_network.generatingCurveShape,MObject::kNullObj, &status);
        CHECK_RET(status);

        // Transform the duplicated curve to [pos, rot, scale]
        MFnTransform transformFn{ curveObj };
        status = transformFn.setTranslation(pos, MSpace::kTransform);
        CHECK_RET(status);

        status = transformFn.setRotation(rot);
        CHECK_RET(status);

        double const scales[3] = { scale.x, scale.y, scale.z };
        status = transformFn.setScale(scales);
        CHECK_RET(status);

        auto curveShape = getShape(curveObj);
        if(!curveShape.valid()) {
            status = curveShape.error();
            CHECK_RET(status);
        }

        //MFnDagNode dagNodeFn(curveObj, &status);
        //MObject const curveShapeObj = dagNodeFn.child(0, &status);
        //CHECK(status, status);

        status = addCurve(idx++, curveShape.value());
        CHECK_RET(status);
    }
    dgModifier.doIt();

    return MStatus::kSuccess;
}
void BranchEditor::on_createBtn_clicked() {
    MSelectionList selection;
    MStatus status = MGlobal::getActiveSelectionList(selection);
    CHECK_NO_RET(status);

	if (selection.length() == 1) {
        status = createNetwork(selection);
        CHECK_NO_RET(status);
    }
    else {
        MGlobal::displayError("Please Select exactly one NURBS curve to represent the cross-section of the limb");
    }
}

void BranchEditor::on_numIterSpinBpx_valueChanged([[maybe_unused]] int value) {
	MStatus const status = updateNetwork(m_network);
	CHECK(status, (void)(0));
}

void BranchEditor::on_integStepDoubleBox_valueChanged([[maybe_unused]] double value) {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_keyframeCurveEditor_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_keyframeCurveEditor_2_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_keyframeCurveEditor_3_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_keyframeCurveEditor_4_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_keyframeCurveEditor_5_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}