#pragma once

#include "include/ui_BranchEditor.h"
#include "CurveLenFunction/UserCurveLenFunction.h"

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

// private slots:

private:
    Ui::BranchEditor m_ui;
};