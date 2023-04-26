#include "PhyllotaxisEditor.h"
#include "MayaNodes/PhyllotaxisNode.h"
#include "CurveLenFunction/ExpressionCurveLenFunction.h"

#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MDGModifier.h>
#include <maya/MFnTransform.h>

#include <QSpinbox>

#include <format>

#include "ExpressionParser.h"
#include "NodeCmdUtils.h"

MStatus PhyllotaxisEditor::updatePhyllotaxisNode() {
    using pn = PhyllotaxisNode;
    using namespace NodeCmdUtils;

    if(m_network.phyllotaxisNode.isNull()) {
        return MStatus::kSuccess;
    }

    MStatus status = Attribute{m_network.phyllotaxisNode, pn::longName(pn::s_curveFunc)}
	    .setValue(MString { m_func->serialize().c_str() });
    CHECK(status, status);

	status = Attribute{ m_network.phyllotaxisNode, pn::longName(pn::s_numIter) }
		.setValue(m_ui.numIterSpinBpx->value());
    CHECK(status, status);

    status = Attribute{ m_network.phyllotaxisNode, pn::longName(pn::s_step) }
		.setValue(m_ui.integStepDoubleBox->value());
    CHECK(status, status);

    return MStatus::kSuccess;
}

MStatus PhyllotaxisEditor::createNetwork(MSelectionList const& selection) {
    /*
	 *             creates the following node graph
	 *
	 *           |-------------------(sphere shape)----> instancer --- (instances)--> [phyllotaxis pattern]
	 *  ----- (inputs)----> phyllotaxis node ---(attr array)---|
	 *           |                 |
	 *           |                 |----(radius)------> makeNurbsCircle ---(circle)----> curveInstanceNode --(attr array)------> instancer ---> [petals]
	 *           -------------------------------------------------------------------------------------------(patal shape)---------|
	 */

    using pn = PhyllotaxisNode;
    using namespace NodeCmdUtils;

    Network network = m_network;
    MStatus status;

    status = selection.getDependNode(0, network.curveShape);
    CHECK_RET(status);
    auto const shapeObj = getShape(network.curveShape);
    CHECK_RES(shapeObj);

    network.curveShape = shapeObj.value();

    if (network.phyllotaxisNode.isNull()) {
        network.phyllotaxisNode = MFnDependencyNode{}.create(pn::s_id, &status);
        CHECK_RET(status);
    }
    if (network.instancer.isNull()) {
        network.instancer = MFnDependencyNode{}.create("instancer", &status);
        CHECK_RET(status);
    }

    if (network.meshShape.isNull() || network.meshTransform.isNull()) {
        if (network.polySphereNode.isNull()) {
            network.polySphereNode = MFnDependencyNode{}.create("polySphere", &status);
            CHECK_RET(status);
        }

        auto const meshRes = createEmptyMesh();
        CHECK_RES(meshRes);

        auto const& mesh = meshRes.value();
        network.meshShape = mesh.shape;
        network.meshTransform = mesh.transform;

        status = addDefaultShadingGroup(network.meshTransform);
        CHECK_RET(status);
    }

    Attribute const polySphereOutput{ network.polySphereNode, "output" };
    Attribute const meshInput{ network.meshShape, "inMesh" };
    Attribute const meshMatrix{ network.meshTransform, "matrix" };
    Attribute const instancerHierarchyInput{ network.instancer, "inputHierarchy" };
    Attribute const instancerInputPoints{ network.instancer, "inputPoints" };
    Attribute const phyllotaxisOutputPoints{ network.phyllotaxisNode, pn::longName(pn::s_output) };
    Attribute const phyllotaxisCurveInput{ network.phyllotaxisNode, pn::longName(pn::s_curve) };
    Attribute const curveWorldspace{ network.curveShape, "worldSpace"};

    status = curveWorldspace[0].connect(phyllotaxisCurveInput);
    CHECK_RET(status);

    status = polySphereOutput.connect(meshInput);
    CHECK_RET(status);

    status = meshMatrix.connect(instancerHierarchyInput[0]);
    CHECK_RET(status);

    status = phyllotaxisOutputPoints.connect(instancerInputPoints);
    CHECK_RET(status);

    m_network = network;
    return MStatus::kSuccess;
}

PhyllotaxisEditor::PhyllotaxisEditor(QWidget* parent) :
    QDialog(parent) {
    m_ui.setupUi(this);

    m_densityFuncExpr = m_ui.expressionPlainTextEdit->toPlainText().toStdString();
	m_densityFuncMirror = m_ui.mirrorCheckBox->isChecked();
    m_densityFuncEditType = static_cast<DensityFuncEditType>(m_ui.tabWidget->currentIndex());

    updateDensityFunc();
}

PhyllotaxisEditor::~PhyllotaxisEditor() { }

void PhyllotaxisEditor::updateDensityFunc() {
    switch (m_densityFuncEditType) {
    case KEYFRAME:
        m_func = m_ui.keyframeCurveEditor->getFunction();
        break;
    case EXPRESSION:
        m_func = std::make_shared<ExpressionCurveLenFunction>(m_densityFuncExpr, m_densityFuncMirror);
        if (!m_func->valid()) {
            MGlobal::displayInfo(ExpressionParser::getLastError().c_str());
        }
        m_ui.curveWidget->setCurve(m_func);

        break;
    }

    CHECK_NO_RET(updatePhyllotaxisNode());
}

void PhyllotaxisEditor::on_expressionPlainTextEdit_textChanged() {
    m_densityFuncExpr = m_ui.expressionPlainTextEdit->toPlainText().toStdString();
    updateDensityFunc();
}

void PhyllotaxisEditor::on_mirrorCheckBox_stateChanged(int state) {
    m_densityFuncMirror = state > 0;
    updateDensityFunc();
}

void PhyllotaxisEditor::on_numIterSpinBpx_valueChanged(int value) {
    CHECK_NO_RET(updatePhyllotaxisNode());
}

void PhyllotaxisEditor::on_integStepDoubleBox_valueChanged(double value) {
    CHECK_NO_RET(updatePhyllotaxisNode());
}

void PhyllotaxisEditor::on_tabWidget_currentChanged(int index) {
    m_densityFuncEditType = static_cast<DensityFuncEditType>(index);
    updateDensityFunc();
}

void PhyllotaxisEditor::on_keyframeCurveEditor_curveChanged() {
    updateDensityFunc();
}

void PhyllotaxisEditor::on_curveWidget_curveUpdated() {
    m_ui.yMinLabel->setText(QString::number(m_ui.curveWidget->getViewYMin(), 'f', 1));
    m_ui.yMaxLabel->setText(QString::number(m_ui.curveWidget->getViewYMax() + 1, 'f', 1));
    //m_ui.minValueLabel->setText(QString{ "Min Value: " } + QString::number(m_ui.curveWidget->getYMin(), 'f', 1));
    //m_ui.maxValueLabel->setText(QString{ "Max Value: " } + QString::number(m_ui.curveWidget->getYMax(), 'f', 1));
}

void PhyllotaxisEditor::on_closeBtn_clicked() {
    reject();
}

void PhyllotaxisEditor::on_createBtn_clicked() {
    if (!m_func || !m_func->valid()) {
        MGlobal::displayError("Please Enter a valid expression for the density function");
        return;
    }

	MSelectionList selection;
    MStatus status = MGlobal::getActiveSelectionList(selection);
    CHECK_NO_RET(status);

    if (selection.length() == 1) {
        MDagPath dagPath;
        status = selection.getDagPath(0, dagPath);
        CHECK_NO_RET(status);

        MFnDagNode const dagNode{ dagPath };
        MString const curveObjName = dagNode.name(&status);
        CHECK_NO_RET(status);

        status = createNetwork(selection);
        CHECK_NO_RET(status);

        status = updatePhyllotaxisNode();
        CHECK_NO_RET(status);
    } else {
        MGlobal::displayError("Please Select exactly one NURBS curve first");
    }
}