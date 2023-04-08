#include "KeyframeCurveEditor.h"
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>

using kfce = KeyframeCurveEditor;

template<Qt::Orientation orientation = Qt::Horizontal>
static QWidget* lbl(const QString& labelText, QWidget* widget, std::optional<int> width = std::nullopt) {
    QBoxLayout* layout = nullptr;
    if constexpr (orientation == Qt::Horizontal) {
        layout = new QHBoxLayout();
    } else if constexpr (orientation == Qt::Vertical) {
        layout = new QVBoxLayout();
    }
	auto const label = new QLabel(labelText);
	layout->addWidget(label);
	layout->addWidget(widget);
	auto const wrapperWidget = new QWidget(widget->parentWidget());
	wrapperWidget->setLayout(layout);
    if (width) {
        wrapperWidget->setMaximumWidth(*width);
    }
	return wrapperWidget;
}

kfce::KeyframeCurveEditor(QWidget* parent)
	: QWidget(parent) {

    m_curveWidget = new KeyframeCurveWidget{ this };

    auto const slider = new QSlider{ Qt::Vertical, this };
    slider->setRange(1, 1000);
    slider->setSingleStep(1);
    m_curveWidget->setYScale(1);

    auto const combo = new QComboBox{ this };

    using stype = KeyframeCurveWidget::SplineType;
    combo->addItem("Linear", static_cast<int>(stype::Linear));
    combo->addItem("Cubic C1", static_cast<int>(stype::C1));
    combo->addItem("Cubic C2", static_cast<int>(stype::C2));
    m_curveWidget->setSplineType(stype::Linear);

    auto const sliderAndCurve = new QHBoxLayout{};
    sliderAndCurve->addWidget(m_curveWidget);
    sliderAndCurve->addWidget(lbl<Qt::Vertical>("Y Scale", slider, 60));

    auto const mainLayout = new QVBoxLayout{ this };
    mainLayout->addLayout(sliderAndCurve);
    mainLayout->addWidget(lbl<Qt::Horizontal>("Curve Type", combo));

    connect(slider, &QSlider::valueChanged, [this](int value) {
        m_curveWidget->setYScale(value);
    });
    connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        m_curveWidget->setSplineType(static_cast<stype>(index));
    });
    connect(m_curveWidget, &KeyframeCurveWidget::curveChanged, [this]() {
        emit curveChanged();
    });
}