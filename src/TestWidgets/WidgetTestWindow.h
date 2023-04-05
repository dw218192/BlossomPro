#pragma once
#include <QMainWindow>

class WidgetTestWindow : public QMainWindow
{
    Q_OBJECT
public:
    WidgetTestWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
protected slots:
    // void on_btn_clicked();
};