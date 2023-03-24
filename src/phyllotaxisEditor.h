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

private:
    void updateDensityFunc();
    MStatus updatePhyllotaxisNode();

private slots: //void on_<object name>_<signal name>(<signal parameters>);
    void on_expressionPlainTextEdit_textChanged();
    void on_curveWidget_curveUpdated();
    void on_closeBtn_clicked();
    void on_createBtn_clicked();
    void on_mirrorCheckBox_stateChanged(int state);
    void on_numIterSpinBpx_valueChanged(int value);
    void on_integStepDoubleBox_valueChanged(double value);

private:
    std::string m_densityFuncExpr;
    bool m_densityFuncMirror;

    Ui::PhyllotaxisEditor m_ui;
    MObject m_phyllotaxisNodeInstance;
    std::shared_ptr<UserCurveLenFunction> m_func;
    std::unique_ptr<PhyllotaxisGrammar> m_grammar;
};