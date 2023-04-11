#include "phyllotaxisEditor.h"
#include "Phyllotaxis/PhyllotaxisNode.h"
#include "Phyllotaxis/ExpressionCurveLenFunction.h"

#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MDGModifier.h>
#include <maya/MFnTransform.h>

#include <QSpinbox>

#include "ExpressionParser.h"

static MStatus createPhyllotaxisNodeInstance(MObject& phyllotaxisNode, MString const& curveName) {
    using pn = PhyllotaxisNode;

    MStatus status;
    phyllotaxisNode = MFnDependencyNode{}.create(pn::s_id, &status);
    CHECK(status, status);

    MFnDependencyNode fnPhyllotaxisNode{ phyllotaxisNode, &status };
    CHECK(status, status);

    // connect instancer
    static char const* const melCmdFmt =
        "connectAttr %s.worldSpace %s.%s; \n"
        "$ins = `createNode instancer`; \n"
        "$sphere = `polySphere`;\n"
		"setAttr ($sphere[0] + \".visibility\") 0;\n" // hide the sphere
        "connectAttr ($sphere[0] + \".matrix\") ($ins + \".inputHierarchy[0]\"); \n"
        "connectAttr %s.%s ($ins + \".inputPoints\"); \n"; // connect output
    static char buf[512];
    int const check = std::snprintf(buf, sizeof(buf), melCmdFmt,
        curveName.asChar(), fnPhyllotaxisNode.name().asChar(), pn::longName(pn::s_curve),
        fnPhyllotaxisNode.name().asChar(), pn::longName(pn::s_output)
    );
    if(check < 0) {
        ERROR_MESSAGE("snprintf failed");
        return MStatus::kFailure;
    }

    MGlobal::displayInfo(buf);
    status = MGlobal::executeCommand(buf);
    CHECK(status, status);

    return MStatus::kSuccess;
}

MStatus PhyllotaxisEditor::updatePhyllotaxisNode() {
    using pn = PhyllotaxisNode;

    MStatus status;
    MFnDependencyNode fnPhyllotaxisNode{ m_phyllotaxisNodeInstance };

    int const numIter = m_ui.numIterSpinBpx->value();
    double const step = m_ui.integStepDoubleBox->value();

    MPlug plug = fnPhyllotaxisNode.findPlug(pn::longName(pn::s_serializedCurveFunc), false, &status);
    CHECK(status, status);
    {
        MString test;
        status = plug.getValue(test);
        CHECK(status, status);

        std::string const str = m_func->serialize();
        if (test != str) {
            status = plug.setString(str.c_str());
            CHECK(status, status);
        }
    }

    plug = fnPhyllotaxisNode.findPlug(pn::longName(pn::s_numIter), false, &status);
    CHECK(status, status);
    {
        int test;
        status = plug.getValue(test);
        CHECK(status, status);

        if (test != numIter) {
            status = plug.setInt(numIter);
            CHECK(status, status);
        }
    }

    plug = fnPhyllotaxisNode.findPlug(pn::longName(pn::s_step), false, &status);
    CHECK(status, status);
    {
        double test;
        status = plug.getValue(test);
        CHECK(status, status);

        if (std::abs(test - step) > std::numeric_limits<double>::epsilon()) {
            status = plug.setDouble(step);
            CHECK(status, status);
        }
    }
    return MStatus::kSuccess;
}

PhyllotaxisEditor::PhyllotaxisEditor(QWidget* parent) :
    QDialog(parent) {
    m_ui.setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::NonModal);

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
            m_ui.curveWidget->setCurve(m_func);
        }
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

    MItSelectionList iter { selection, MFn::kNurbsCurve, &status };
    CHECK(status, (void)0);

    if (!iter.isDone()) {
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
        MGlobal::displayError("Please Select a NURBS curve first");
    }
}