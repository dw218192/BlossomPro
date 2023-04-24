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

MStatus BranchEditor::pushLoftCurve(MObject const& curveObj) {
    MStatus status = MStatus::kSuccess;
    MDGModifier dgModifier;

    MPlug const inputCurvePlug = MFnDependencyNode{ m_network.loftNodeObj }
		.findPlug("inputCurve", false, &status);
    CHECK_RET(status);

    auto const idx = m_curvePool.length();
    auto const shapeObj = getShape(curveObj);
    CHECK_RES(shapeObj);

    // connect: curveShape.worldSpace[0] --> loftNode.inputCurve[i]
    MPlug const worldSpaceArrayPlug = MFnDependencyNode{ shapeObj.value() }
		.findPlug("worldSpace", false, &status);
    CHECK_RET(status);

    MPlug const worldSpacePlug = worldSpaceArrayPlug.elementByLogicalIndex(0, &status);
    CHECK_RET(status);

    status = dgModifier.connect(worldSpacePlug, inputCurvePlug.elementByLogicalIndex(idx));
    CHECK_RET(status);

    // append to pool
    status = m_curvePool.append(curveObj);
    CHECK_RET(status);

    status = dgModifier.doIt();
    CHECK_RET(status);

    return status;
}

MStatus BranchEditor::popLoftCurve() {
    MStatus status = MStatus::kSuccess;
    MDGModifier dgModifier;

    MPlug const inputCurvePlug = MFnDependencyNode{ m_network.loftNodeObj }
		.findPlug("inputCurve", false, &status);
    CHECK_RET(status);

    auto const idx = m_curvePool.length() - 1;
    MObject const curveObj = m_curvePool[idx];

    auto const shapeObj = getShape(curveObj);
    CHECK_RES(shapeObj);

    // disconnect: curveShape.worldSpace[0] --> loftNode.inputCurve[i]
    MPlug const worldSpaceArrayPlug = MFnDependencyNode{ shapeObj.value() }
		.findPlug("worldSpace", false, &status);
    CHECK_RET(status);

    MPlug const worldSpacePlug = worldSpaceArrayPlug.elementByLogicalIndex(0, &status);
    CHECK_RET(status);

    status = dgModifier.disconnect(worldSpacePlug, inputCurvePlug.elementByLogicalIndex(idx));
    CHECK_RET(status);

    // remove from pool
    status = m_curvePool.remove(idx);
    CHECK_RET(status);

    auto curveName = getName(curveObj);
    CHECK_RES(curveName);

    // remove curve from the scene
    status = MGlobal::executeCommand(MString{ "delete " } + curveName.value());
    CHECK_RET(status);

    status = dgModifier.doIt();
    CHECK_RET(status);

    return status;
}

BranchEditor::Inputs BranchEditor::getInputs() const {
    Inputs ret;
    ret.funcs.yawRate = m_ui.radioButton_1->isChecked() ? m_ui.keyframeCurveEditor_1->getFunction() : nullptr;
    ret.funcs.pitchRate = m_ui.radioButton_2->isChecked() ? m_ui.keyframeCurveEditor_2->getFunction() : nullptr;
    ret.funcs.rollRate = m_ui.radioButton_3->isChecked() ? m_ui.keyframeCurveEditor_3->getFunction() : nullptr;
    ret.funcs.twistRate = m_ui.radioButton_4->isChecked() ? m_ui.keyframeCurveEditor_4->getFunction() : nullptr;
    ret.funcs.widthRate = m_ui.radioButton_5->isChecked() ? m_ui.keyframeCurveEditor_5->getFunction() : nullptr;
    ret.numIter = m_ui.numIterSpinBpx->value();
    ret.step = m_ui.integStepDoubleBox->value();
    ret.length = m_ui.lengthDoubleBox->value();

    return ret;
}

MStatus BranchEditor::createNetwork(MSelectionList const& selection) {
    MStatus status = MStatus::kSuccess;

    // get the generating curve's transform and shape
    status = selection.getDependNode(0, m_network.generatingCurve);
    CHECK_RET(status);
    auto const shapeObj = getShape(m_network.generatingCurve);
    CHECK_RES(shapeObj);

    m_network.generatingCurveShape = shapeObj.value();

    if (m_network.loftNodeObj.isNull()) {
        m_network.loftNodeObj = MFnDependencyNode{}.create("loft", &status);
        CHECK_RET(status);
    }

    // add to pool
    status = pushLoftCurve(m_network.generatingCurve);
    CHECK_RET(status);

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

    // validate curve pool
    {
        unsigned int const len = m_curvePool.length();
        for (int i = len-1; i >= 0; --i) {
            if (m_curvePool[i].isNull()) {
                m_curvePool.remove(i);
            }
        }
    }

    // connect any new curve's shape to the loft node
    unsigned int const numCurves = grammar->result().size();
    if(numCurves < m_curvePool.length()) {
        auto const diff = m_curvePool.length() - numCurves;
        for (unsigned int i = 0; i < diff;  ++i) {
            status = popLoftCurve();
            CHECK_RET(status);
        }
    } else if(numCurves > m_curvePool.length()) {
        auto const diff = numCurves - m_curvePool.length();
        for (unsigned int i = 0; i < diff; ++i) {
            MObject curveObj = MFnNurbsCurve{}.copy(m_network.generatingCurveShape, MObject::kNullObj, &status);
            CHECK_RET(status);

            status = pushLoftCurve(curveObj);
            CHECK_RET(status);
        }
    }

    // start at the second curve because the first curve stays in place
    unsigned int idx = 0;
    for (auto const& [pos, rot, scale] : grammar->result()) {
        // NOTE: this creates a transform node with a shape node all at once
    	MObject curveObj = m_curvePool[idx++];

        // Transform the duplicated curve to [pos, rot, scale]
        MFnTransform transformFn{ curveObj };
        status = transformFn.setTranslation(pos, MSpace::kTransform);
        CHECK_RET(status);

        status = transformFn.setRotation(rot);
        CHECK_RET(status);

        double const scales[3] = { scale.x, scale.y, scale.z };
        status = transformFn.setScale(scales);
        CHECK_RET(status);
    }

    return MStatus::kSuccess;
}
void BranchEditor::on_createBtn_clicked() {
    MSelectionList selection;
    MStatus status = MGlobal::getActiveSelectionList(selection);
    CHECK_NO_RET(status);

	if (selection.length() == 1) {
        status = createNetwork(selection);
        CHECK_NO_RET(status);
    } else {
        MGlobal::displayError("Please Select exactly one NURBS curve to represent the cross-section of the limb");
    }
}

void BranchEditor::on_numIterSpinBpx_valueChanged([[maybe_unused]] int value) {
	MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_integStepDoubleBox_valueChanged([[maybe_unused]] double value) {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_keyframeCurveEditor_1_curveChanged() {
	if (m_ui.radioButton_1->isChecked()) {
		MStatus const status = updateNetwork(m_network);
		CHECK_NO_RET(status);
	}
}

void BranchEditor::on_keyframeCurveEditor_2_curveChanged() {
	if (m_ui.radioButton_2->isChecked()) {
		MStatus const status = updateNetwork(m_network);
		CHECK_NO_RET(status);
	}
}

void BranchEditor::on_keyframeCurveEditor_3_curveChanged() {
	if (m_ui.radioButton_3->isChecked()) {
		MStatus const status = updateNetwork(m_network);
		CHECK_NO_RET(status);
	}
}

void BranchEditor::on_keyframeCurveEditor_4_curveChanged() {
	if (m_ui.radioButton_4->isChecked()) {
		MStatus const status = updateNetwork(m_network);
		CHECK_NO_RET(status);
	}
}

void BranchEditor::on_keyframeCurveEditor_5_curveChanged() {
	if (m_ui.radioButton_5->isChecked()) {
		MStatus const status = updateNetwork(m_network);
		CHECK_NO_RET(status);
	}
}

void BranchEditor::on_radioButton_1_toggled([[maybe_unused]] bool checked) {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_radioButton_2_toggled([[maybe_unused]] bool checked) {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_radioButton_3_toggled([[maybe_unused]] bool checked) {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_radioButton_4_toggled([[maybe_unused]] bool checked) {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}

void BranchEditor::on_radioButton_5_toggled([[maybe_unused]] bool checked) {
    MStatus const status = updateNetwork(m_network);
    CHECK_NO_RET(status);
}
