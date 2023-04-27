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
        double step;
        double length;
    };

    struct BranchNodeNetwork {
        // user provided
        MObject generatingCurve;
        MObject generatingCurveShape;
        MObject flowerHeadTransform;

        // automatically generated
        MObject loftNodeObj, tessNodeObj, transformObj, meshObj;
    };

    enum UpdateCurveFlag {
	    CURVE_1 = 1 << 0,
    	CURVE_2 = 1 << 1,
        CURVE_3 = 1 << 2,
        CURVE_4 = 1 << 3,
        CURVE_5 = 1 << 4
    };

    MStatus pushLoftCurve(MObject const& curveObj);
    MStatus popLoftCurve();
    Inputs getInputs(unsigned char flags) const;
    MStatus createNetwork(MSelectionList const& selection);
    MStatus updateNetwork(unsigned char flags);

private slots:
    void on_createBtn_clicked();
    void on_selectFlowerHeadBtn_clicked();
    void on_numStepsSpinBox_valueChanged(int value);
    void on_lengthDoubleBox_valueChanged(double value);
    void on_FHeadOffsetXDblBox_valueChanged(double value);
    void on_FHeadOffsetYDblBox_valueChanged(double value);
    void on_FHeadOffsetZDblBox_valueChanged(double value);
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
    GeneralizedCylinderGrammar::Functions m_cachedFuncs;
	MObjectArray m_curvePool;
    BranchNodeNetwork m_network;
    Ui::BranchEditor m_ui{};
};