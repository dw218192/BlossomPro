#include "phyllotaxisEditor.h"
#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

PhyllotaxisEditor::PhyllotaxisEditor(QWidget* parent) :
    QDialog(parent) {
    m_ui.setupUi(this);
}

PhyllotaxisEditor::~PhyllotaxisEditor() { }

void PhyllotaxisEditor::on_expressionPlainTextEdit_textChanged() {
    auto const str = m_ui.expressionPlainTextEdit->toPlainText().toStdString();
    m_func = std::make_shared<UserCurveLenFunction>(str);
    if (!m_func->valid()) {
        MGlobal::displayInfo(ExpressionParser::getLastError().c_str());
    }

    m_ui.curveWidget->setCurve(m_func);
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
        //TODO: create the grammar
    } else {
        MGlobal::displayError("Please Select a NURBS curve first");
    }
}