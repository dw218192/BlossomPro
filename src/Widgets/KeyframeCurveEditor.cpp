#include "KeyframeCurveEditor.h"
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>

KeyframeCurveEditor::KeyframeCurveEditor(QWidget* parent)
	: QWidget(parent) {
    m_ui.setupUi(this);
    m_ui.slider->setRange(1, 1000);
    m_ui.slider->setSingleStep(1);
    m_ui.curveWidget->setYScale(1);
    using stype = KeyframeCurveWidget::SplineType;
    m_ui.combo->addItem("Linear", static_cast<int>(stype::Linear));
    m_ui.combo->addItem("Cubic C1", static_cast<int>(stype::C1));
    m_ui.combo->addItem("Cubic C2", static_cast<int>(stype::C2));

	connect(m_ui.slider, &QSlider::valueChanged, [this](int value) {
        m_ui.curveWidget->setYScale(value);
    });
    connect(m_ui.combo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        m_ui.curveWidget->setSplineType(static_cast<stype>(index));
    });
    connect(m_ui.curveWidget, &KeyframeCurveWidget::curveChanged, [this]() {
        emit curveChanged();
    });
}