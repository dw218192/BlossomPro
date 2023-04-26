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

static MStatus createPhyllotaxisNodeInstance(MObject& phyllotaxisNode, MString const& curveName) {
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

    MStatus status;
    phyllotaxisNode = MFnDependencyNode{}.create(pn::s_id, &status);
    CHECK_RET(status);

    MFnDependencyNode fnPhyllotaxisNode{ phyllotaxisNode, &status };
    CHECK_RET(status);

    MObject instancer = MFnDependencyNode{}.create("instancer", &status);
    CHECK_RET(status);


    // connect instancer
    /*
    std::string melCmdTemplate = loadResource("MEL/createPhyllotaxis.mel");
    std::string melCmd = std::vformat(melCmdTemplate, std::make_format_args(
        curveName.asChar(),
        fnPhyllotaxisNode.name().asChar(),
        pn::longName(pn::s_curve),
        fnPhyllotaxisNode.name().asChar(),
        pn::longName(pn::s_output)
    ));  
    MGlobal::displayInfo(melCmd.c_str());
    status = MGlobal::executeCommand(melCmd.c_str());
    CHECK(status, status);
    */

    return MStatus::kSuccess;
}

MStatus PhyllotaxisEditor::updatePhyllotaxisNode() {
    using pn = PhyllotaxisNode;
    using namespace NodeCmdUtils;

    MStatus status = Attribute{m_phyllotaxisNodeInstance, pn::longName(pn::s_curveFunc)}
	    .setValue(MString { m_func->serialize().c_str() });
    CHECK(status, status);

	status = Attribute{ m_phyllotaxisNodeInstance, pn::longName(pn::s_numIter) }
		.setValue(m_ui.numIterSpinBpx->value());
    CHECK(status, status);

    status = Attribute{ m_phyllotaxisNodeInstance, pn::longName(pn::s_step) }
		.setValue(m_ui.integStepDoubleBox->value());
    CHECK(status, status);

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

    if (!m_phyllotaxisNodeInstance.isNull()) {
        CHECK(updatePhyllotaxisNode(), (void)0);
    }
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
    if (!m_phyllotaxisNodeInstance.isNull()) {
        CHECK(updatePhyllotaxisNode(), (void)0);
    }
}

void PhyllotaxisEditor::on_integStepDoubleBox_valueChanged(double value) {
    if (!m_phyllotaxisNodeInstance.isNull()) {
        CHECK(updatePhyllotaxisNode(), (void)0);
    }
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
    CHECK(status, (void)0);

    if (selection.length() == 1) {
        MDagPath dagPath;
        status = selection.getDagPath(0, dagPath);
        CHECK(status, (void)0);

        MFnDagNode const dagNode{ dagPath };
        MString const curveObjName = dagNode.name(&status);
        CHECK(status, (void)0);

        status = createPhyllotaxisNodeInstance(m_phyllotaxisNodeInstance, curveObjName);
        CHECK(status, (void)0);

        status = updatePhyllotaxisNode();
        CHECK(status, (void)0);
    } else {
        MGlobal::displayError("Please Select exactly one NURBS curve first");
    }
}