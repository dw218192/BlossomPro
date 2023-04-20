#pragma once

#include "include/ui_BranchEditor.h"
#include "MayaNodes/BranchNode.h"

QT_BEGIN_NAMESPACE
namespace Ui { class BranchEditor; }
QT_END_NAMESPACE

class BranchEditor : public QDialog
{
    Q_OBJECT
public:
    explicit BranchEditor(QWidget* parent);
    ~BranchEditor() override;

private:
    MStatus updateNode();

private slots:
    void on_createBtn_clicked();
    void on_numIterSpinBpx_valueChanged(int value);
    void on_integStepDoubleBox_valueChanged(double value);
    void on_keyframeCurveEditor_curveChanged();
    void on_keyframeCurveEditor_2_curveChanged();
    void on_keyframeCurveEditor_3_curveChanged();
    void on_keyframeCurveEditor_4_curveChanged();
    void on_keyframeCurveEditor_5_curveChanged();

private:
    MObject m_nodeInstance;
    Ui::BranchEditor m_ui{};
};