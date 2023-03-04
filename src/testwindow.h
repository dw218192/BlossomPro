#pragma once
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

#include "include/ui_testwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TestWindow; }
QT_END_NAMESPACE

class TestWindow : public QDialog
{
    Q_OBJECT
public:
    explicit TestWindow(QWidget* parent) :
        QDialog(parent), ui(new Ui::TestWindow)
    {
        ui->setupUi(this);
    }
    ~TestWindow() override {
        delete ui;
    }
private slots:
    void accept() override {
        MGlobal::displayInfo("accept");
    }
    void reject() override {
        MGlobal::displayInfo("reject");
        QDialog::reject();
    }
private:
    Ui::TestWindow* ui;
};