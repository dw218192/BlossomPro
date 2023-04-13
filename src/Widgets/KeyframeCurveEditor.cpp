#include "KeyframeCurveEditor.h"
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>
#include <QPainter>
#include <QPen>
#include <memory>

using stype = KeyframeCurveWidget::SplineType;


static QPixmap getPreview(UserCurveLenFunction const& func) {
    static constexpr int width = 256;
    static constexpr int height = 256;
    QPixmap preview { width, height };
    preview.fill(Qt::black);

    QPainter painter{ &preview };
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen { Qt::green, 2, Qt::SolidLine };
    painter.setPen(pen);

    // Determine the range of x values for the function
    double yMin = std::numeric_limits<double>::max(),
		yMax = std::numeric_limits<double>::lowest();
    for (double x = 0; x <= 1; x += 0.01) {
        double y = func(x);
        yMin = std::min(yMin, y);
        yMax = std::max(yMax, y);
    }
    std::optional<QPointF> lastPoint;
    for (double x = 0; x < 1; x += 0.01) {
        double y = func(x);
        QPointF point{ x * width, (1 - y) * height };
        if (lastPoint) {
        	painter.drawLine(*lastPoint, point);
        }
        lastPoint = point;
    }
    return preview;
}

KeyframeCurveEditor::KeyframeCurveEditor(QWidget* parent)
	: QWidget(parent), m_dataSaver{this} {
    m_ui.setupUi(this);
    m_ui.slider->setRange(1, 1000);
    m_ui.slider->setSingleStep(1);
    m_ui.curveWidget->setYScale(1);
    m_ui.combo->addItem("Linear", static_cast<int>(stype::Linear));
    m_ui.combo->addItem("Cubic C1", static_cast<int>(stype::C1));
    m_ui.combo->addItem("Cubic C2", static_cast<int>(stype::C2));

    // populate the save curve list
    m_dataSaver.loadData();

    auto listWidget = m_ui.savedCurveList;
    listWidget->clear();
	for (auto&& curve : m_savedCurves) {
        auto pfunc = UserCurveLenFunction::deserialize(curve.toUtf8());
        auto const& func = *pfunc;
        listWidget->addItem(new QListWidgetItem{ getPreview(func),"" });
    }
}

KeyframeCurveEditor::~KeyframeCurveEditor() {
    m_dataSaver.saveData();
}

void KeyframeCurveEditor::resizeEvent(QResizeEvent* event) {
	QWidget::resizeEvent(event);
    int w = m_ui.savedCurveList->width();
    m_ui.savedCurveList->setIconSize({ w, w });
}

void KeyframeCurveEditor::on_slider_valueChanged(int value) const {
    m_ui.curveWidget->setYScale(value);
}

void KeyframeCurveEditor::on_combo_currentIndexChanged(int index) const {
    m_ui.curveWidget->setSplineType(static_cast<stype>(index));
}

void KeyframeCurveEditor::on_curveWidget_curveChanged() {
    emit curveChanged();
}

void KeyframeCurveEditor::on_saveBtn_clicked() {
    auto const cw = m_ui.curveWidget;
    auto const pfunc = cw->getFunction();
    auto const& func = *pfunc;

    m_ui.savedCurveList->addItem(new QListWidgetItem{ getPreview(func),"" });
    auto const serialized = cw->getFunction()->serialize();
    m_savedCurves.push_back(QString::fromUtf8(serialized.c_str()));
}

void KeyframeCurveEditor::on_loadBtn_clicked() {
    auto const cw = m_ui.curveWidget;
    auto const lw = m_ui.savedCurveList;
    if (int const index = lw->currentRow(); index >= 0) {
	    if (auto const pfunc = std::dynamic_pointer_cast<KeyframeCurveLenFunction>(
		    UserCurveLenFunction::deserialize(m_savedCurves[index].toUtf8()))) {
            auto const& func = *pfunc;
            cw->setSplineType(func.getType());
            cw->setYScale(func.getScale());
            cw->setControlPoints(func.getControlPoints().cbegin(), func.getControlPoints().cend());
        }
    }
}

void KeyframeCurveEditor::on_deleteBtn_clicked() {
    auto const lw = m_ui.savedCurveList;
    if (int const index = lw->currentRow(); index >= 0) {
        auto const item = lw->takeItem(index);
        m_savedCurves.removeAt(index);
    	delete item;
    }
}
