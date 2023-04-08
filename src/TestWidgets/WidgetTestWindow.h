#pragma once
#include <QMainWindow>

class WidgetTestWindow : public QMainWindow
{
    Q_OBJECT
public:
    WidgetTestWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
};