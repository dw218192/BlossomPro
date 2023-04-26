#pragma once
#include "include/ui_PhyllotaxisEditor.h"
#include "CurveLenFunction/UserCurveLenFunction.h"
#include "Grammar/PhyllotaxisGrammar.h"

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
    enum DensityFuncEditType {
	    KEYFRAME = 0,
        EXPRESSION = 1
    };
    void updateDensityFunc();
    MStatus updatePhyllotaxisNode();
    MStatus createNetwork(MSelectionList const& selection);

private slots: //void on_<object name>_<signal name>(<signal parameters>);
    void on_expressionPlainTextEdit_textChanged();
    void on_curveWidget_curveUpdated();
    void on_closeBtn_clicked();
    void on_createBtn_clicked();
    void on_mirrorCheckBox_stateChanged(int state);
    void on_numIterSpinBpx_valueChanged(int value);
    void on_integStepDoubleBox_valueChanged(double value);
    void on_tabWidget_currentChanged(int index);
    void on_keyframeCurveEditor_curveChanged();

private:
    std::string m_densityFuncExpr;
    bool m_densityFuncMirror;
    DensityFuncEditType m_densityFuncEditType;

    Ui::PhyllotaxisEditor m_ui;

    struct Network {
        MObject curveShape;
        MObject phyllotaxisNode;
        MObject polySphereNode;
        MObject instancer;
        MObject meshShape;
        MObject meshTransform;
    };
    Network m_network;

	std::shared_ptr<UserCurveLenFunction> m_func;
};