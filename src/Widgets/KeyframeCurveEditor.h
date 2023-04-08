#pragma once
#include <QWidget>
#include "KeyframeCurveWidget.h"

struct UserCurveLenFunction;

class KeyframeCurveEditor : public QWidget {
	Q_OBJECT

private:
	KeyframeCurveWidget* m_curveWidget;

public:
	explicit KeyframeCurveEditor(QWidget* parent = nullptr);
	std::shared_ptr<UserCurveLenFunction> getFunction() {
		return m_curveWidget->getFunction();
	}

signals:
	void curveChanged();
};