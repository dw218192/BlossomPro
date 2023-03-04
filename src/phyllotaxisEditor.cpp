#include "phyllotaxisEditor.h"
#include <maya/MGlobal.h>

PhyllotaxisEditor::PhyllotaxisEditor(QWidget* parent) :
    QDialog(parent), ui(new Ui::PhyllotaxisEditor) {
    ui->setupUi(this);
}

PhyllotaxisEditor::~PhyllotaxisEditor() {
    delete ui;
}

void PhyllotaxisEditor::on_expressionTextEdit_textChanged() {
    MGlobal::displayInfo("ha!");
}