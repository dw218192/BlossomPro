#pragma once
#include "include/ui_CurveInstanceWindow.h"

// Qt header
#include <QDialog>
#include <QWidget>
#include <QPointer>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>

// Maya header
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MArgList.h>

QT_BEGIN_NAMESPACE
namespace Ui { class CurveInstanceWindow; }
QT_END_NAMESPACE

class CurveInstanceWindow : public QDialog
{
    Q_OBJECT
public:
    explicit CurveInstanceWindow(QWidget* parent);
    ~CurveInstanceWindow() override;

private slots:
    void on_Create_BN_clicked();
    void on_Cancel_BN_clicked();

    void on_SelectCurveBN_clicked();
    void on_SelectObjectBN_clicked();

private:
    Ui::CurveInstanceWindow ui;
};