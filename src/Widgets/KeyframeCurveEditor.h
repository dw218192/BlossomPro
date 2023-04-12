#pragma once
#include <QWidget>
#include "KeyframeCurveWidget.h"
#include "include/ui_KeyframeCurveEditor.h"

struct UserCurveLenFunction;

class KeyframeCurveEditor : public QWidget {
	Q_OBJECT

private:
	Ui::KeyframeCurveEditor m_ui;

public:
	explicit KeyframeCurveEditor(QWidget* parent = nullptr);
	std::shared_ptr<UserCurveLenFunction> getFunction() const {
		return m_ui.curveWidget->getFunction();
	}
signals:
	void curveChanged();
};