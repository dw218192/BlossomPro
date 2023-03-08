#include "phyllotaxisEditor.h"
#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>

PhyllotaxisEditor::PhyllotaxisEditor(QWidget* parent) :
    QDialog(parent) {
    m_ui.setupUi(this);
}

PhyllotaxisEditor::~PhyllotaxisEditor() { }

void PhyllotaxisEditor::on_expressionPlainTextEdit_textChanged() {
    auto const str = m_ui.expressionPlainTextEdit->toPlainText().toStdString();
    m_func = std::make_shared<UserCurveLenFunction>(str);

    if (!*m_func) {
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
    MGlobal::displayInfo("clicked");
}