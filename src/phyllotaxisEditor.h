#pragma once
#include "include/ui_phyllotaxisEditor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PhyllotaxisEditor; }
QT_END_NAMESPACE

class PhyllotaxisEditor : public QDialog
{
    Q_OBJECT
public:
    explicit PhyllotaxisEditor(QWidget* parent);
    ~PhyllotaxisEditor() override;
private slots: //void on_<object name>_<signal name>(<signal parameters>);
    void on_expressionTextEdit_textChanged();

private:
    Ui::PhyllotaxisEditor* ui;
};