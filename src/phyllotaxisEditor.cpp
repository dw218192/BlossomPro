#include "phyllotaxisEditor.h"
#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <QSpinbox>

PhyllotaxisEditor::PhyllotaxisEditor(QWidget* parent) :
    QDialog(parent) {
    m_ui.setupUi(this);

    m_densityFuncExpr = m_ui.expressionPlainTextEdit->toPlainText().toStdString();
	m_densityFuncMirror = m_ui.mirrorCheckBox->isChecked();
    updateDensityFunc();
}

PhyllotaxisEditor::~PhyllotaxisEditor() { }

void PhyllotaxisEditor::updateDensityFunc() {
    m_func = std::make_shared<UserCurveLenFunction>(m_densityFuncExpr, m_densityFuncMirror);
    if (!m_func->valid()) {
        MGlobal::displayInfo(ExpressionParser::getLastError().c_str());
    }
    m_ui.curveWidget->setCurve(m_func);
}

void PhyllotaxisEditor::on_expressionPlainTextEdit_textChanged() {
    m_densityFuncExpr = m_ui.expressionPlainTextEdit->toPlainText().toStdString();
    updateDensityFunc();
}

void PhyllotaxisEditor::on_mirrorCheckBox_stateChanged(int state) {
    m_densityFuncMirror = state > 0;
    updateDensityFunc();
}

void PhyllotaxisEditor::on_curveWidget_curveUpdated() {
    m_ui.yMinLabel->setText(QString::number(m_ui.curveWidget->getYMin(), 'f', 1));
    m_ui.yMaxLabel->setText(QString::number(m_ui.curveWidget->getYMax(), 'f', 1));
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
        MObject nurbsCurveObj;
        status = iter.getDependNode(nurbsCurveObj);
        CHECK(status, (void)0);
        MFnNurbsCurve fnNurbsCurve { nurbsCurveObj, &status };
        CHECK(status, (void)0);

        m_grammar = std::make_unique<PhyllotaxisGrammar>(CurveInfo{fnNurbsCurve}, *m_func, m_ui.integStepDoubleBox->value());
        m_grammar->process(m_ui.numIterSpinBpx->value());
    } else {
        MGlobal::displayError("Please Select a NURBS curve first");
    }
}