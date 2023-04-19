#include "KeyframeCurveEditor.h"
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>
#include <QPainter>
#include <QPen>
#include <memory>

#include "CurveLenFunction/KeyframeCurveLenFunction.h"

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

        // make sure y is in [0.25, 0.75]
        y -= yMin;
        y /= yMax - yMin;
        y *= 0.5;
        y += 0.25;

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

    auto const listWidget = m_ui.savedCurveList;
    listWidget->clear();

    // populate default curves if the user hasn't save any curves
    if (m_savedCurves.empty()) {
        addSavedCurve(KeyframeCurveLenFunction{
            ControlPointArray { {0, 0}, {0.5, 0.5}, {1, 1} },
            KeyframeCurveLenFunction::SplineType::Linear,
            1.0
        }, false);
        addSavedCurve(KeyframeCurveLenFunction{
            ControlPointArray { {0, 1}, {0.25, 0.2}, {0.5, 0.4}, {1, 1} },
            KeyframeCurveLenFunction::SplineType::C1,
            1.0
        }, false);
    } else {
        for (auto&& curve : m_savedCurves) {
            addSavedCurve(curve.toUtf8());
        }
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
    addSavedCurve(func, false);
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

void KeyframeCurveEditor::addSavedCurve(char const* serializedFunc, bool displayOnly) {
    auto const pfunc = UserCurveLenFunction::deserialize(serializedFunc);
    addSavedCurve(*pfunc, displayOnly);
}

void KeyframeCurveEditor::addSavedCurve(UserCurveLenFunction const& func, bool displayOnly) {
    m_ui.savedCurveList->addItem(new QListWidgetItem{ getPreview(func),"" });
    std::string const serialized = func.serialize();

    if (!displayOnly) {
        m_savedCurves.push_back(QString::fromUtf8(serialized.c_str()));
    }
}
