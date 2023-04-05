#include "WidgetTestWindow.h"
#include "../Widgets/KeyframeCurveWidget.h"
#include <QPushButton>
#include <QVBoxLayout>

using wtw = WidgetTestWindow;

wtw::WidgetTestWindow(QWidget* parent, Qt::WindowFlags flags) {
    if (objectName().isEmpty())
        setObjectName(QString::fromUtf8("widgetTest"));
    resize(400, 400);

    // Create a QVBoxLayout
    QVBoxLayout* layout = new QVBoxLayout(this);
    // Create a KeyframeCurveWidget
    KeyframeCurveWidget* curveWidget = new KeyframeCurveWidget(this);
    layout->addWidget(curveWidget);
    curveWidget->move(QPoint {10, 10});
    curveWidget->setFixedSize(QSize{ 300, 300 });
}