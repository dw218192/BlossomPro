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

class TestQtWindow : public QDialog
{
    Q_OBJECT
public:
    // Main constructor, first default argument is QWidget * get parent.
    // By default, we need parent to "Maya Main Window"(by a function).
    TestQtWindow(QWidget* parent) :
        QDialog(parent),
        ptr_button(new QPushButton("Push"))
    {
        this->setObjectName("customWindow");
        this->setWindowTitle("Test Window");
        this->setFixedSize(QSize(360, 240));
        QVBoxLayout* ptr_layout = new QVBoxLayout;
        ptr_layout->addWidget(new QLabel("Hello World"));
        ptr_layout->addWidget(ptr_button);
        this->setLayout(ptr_layout);
        // Do Qt connections
        connect(
            ptr_button,
            &QPushButton::clicked,
            this,
            &TestQtWindow::execute
        );
    }
    // Main virtual destructor, do nothing.
    virtual ~TestQtWindow() {}
private slots:
    // Create a private slots, here is optional.
    void execute() const {
        MGlobal::displayInfo("Hello World!");
	}
private:
    // Create a QT button. here is optional.
    QPushButton* ptr_button;
};