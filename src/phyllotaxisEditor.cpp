#include "phyllotaxisEditor.h"
#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>

PhyllotaxisEditor::PhyllotaxisEditor(QWidget* parent) :
    QDialog(parent), m_ui(new Ui::PhyllotaxisEditor) {
    m_ui->setupUi(this);
}

PhyllotaxisEditor::~PhyllotaxisEditor() {
    delete m_ui;
}

void PhyllotaxisEditor::on_expressionPlainTextEdit_textChanged() {
    auto const str = m_ui->expressionPlainTextEdit->toPlainText().toStdString();
    m_func = std::make_shared<UserCurveLenFunction>(str);

    if (*m_func) {
        MGlobal::displayInfo("expression is valid");
    } else {
        MGlobal::displayInfo(ExpressionParser::getLastError().c_str());
    }

    m_ui->curveWidget->setCurve(m_func);
    MGlobal::displayInfo(m_ui->curveWidget->valid() ? "valid" : "not valid");
}