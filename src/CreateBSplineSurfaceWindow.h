#pragma once
#include "include/ui_CreateBSplineSurfaceWindow.h"

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
namespace Ui { class CreateBSplineSurfaceWindow; }
QT_END_NAMESPACE

class CreateBSplineSurfaceWindow : public QDialog
{
    Q_OBJECT
public:
    explicit CreateBSplineSurfaceWindow(QWidget* parent);
    ~CreateBSplineSurfaceWindow() override;

private slots:
    void on_CreateBN_clicked();
    void on_CancelBN_clicked();

private:
    Ui::CreateBSplineSurfaceWindow ui;
};