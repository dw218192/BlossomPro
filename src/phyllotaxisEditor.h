#pragma once
#include "include/ui_phyllotaxisEditor.h"
#include "Phyllotaxis/UserCurveLenFunction.h"
#include "Phyllotaxis/PhyllotaxisGrammar.h"

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
    void on_expressionPlainTextEdit_textChanged();
    void on_curveWidget_curveUpdated();
    void on_closeBtn_clicked();
    void on_createBtn_clicked();

private:
    Ui::PhyllotaxisEditor m_ui;
    std::shared_ptr<UserCurveLenFunction> m_func;
    std::unique_ptr<PhyllotaxisGrammar> m_grammar;
};