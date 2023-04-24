#pragma once

#include <maya/MObjectArray.h>
#include <bitset>

#include "include/ui_BranchEditor.h"

#include "Utils.h"
#include "Grammar/GeneralizedCylinderGrammar.h"

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
    struct Inputs {
        GeneralizedCylinderGrammar::Functions funcs;
        int numIter;
        double step;
        double length;
    };

    struct BranchNodeNetwork {
        MObject generatingCurve;
        MObject generatingCurveShape;

        MObject loftNodeObj, tessNodeObj, transformObj, meshObj;
    };

    MStatus pushLoftCurve(MObject const& curveObj);
    MStatus popLoftCurve();
    Inputs getInputs() const;
    MStatus createNetwork(MSelectionList const& selection);
    MStatus updateNetwork(BranchNodeNetwork const& network);

private slots:
    void on_createBtn_clicked();
    void on_numIterSpinBpx_valueChanged(int value);
    void on_integStepDoubleBox_valueChanged(double value);
    void on_keyframeCurveEditor_1_curveChanged();
    void on_keyframeCurveEditor_2_curveChanged();
    void on_keyframeCurveEditor_3_curveChanged();
    void on_keyframeCurveEditor_4_curveChanged();
    void on_keyframeCurveEditor_5_curveChanged();
    void on_radioButton_1_toggled(bool checked);
    void on_radioButton_2_toggled(bool checked);
    void on_radioButton_3_toggled(bool checked);
    void on_radioButton_4_toggled(bool checked);
    void on_radioButton_5_toggled(bool checked);
private:
	MObjectArray m_curvePool;
    BranchNodeNetwork m_network;
    Ui::BranchEditor m_ui{};
};