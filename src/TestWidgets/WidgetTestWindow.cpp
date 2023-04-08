#include "WidgetTestWindow.h"
#include "../Widgets/KeyframeCurveEditor.h"
#include <QPushButton>
#include <QVBoxLayout>

using wtw = WidgetTestWindow;

wtw::WidgetTestWindow(QWidget* parent, Qt::WindowFlags flags) {
    if (objectName().isEmpty())
        setObjectName(QString::fromUtf8("widgetTest"));
    resize(400, 400);

    auto const curveWidget = new KeyframeCurveEditor{ this };
	setCentralWidget(curveWidget);
}