#pragma once
#include <QWidget>
#include "KeyframeCurveWidget.h"
#include "include/ui_KeyframeCurveEditor.h"

struct UserCurveLenFunction;

class KeyframeCurveEditor : public QWidget {
	Q_OBJECT
	Q_CLASSINFO("version", "0.0")
	Q_PROPERTY(QStringList prop_SavedCurves MEMBER m_savedCurves)

public:
	explicit KeyframeCurveEditor(QWidget* parent = nullptr);
	~KeyframeCurveEditor() override;

	std::shared_ptr<UserCurveLenFunction> getFunction() const {
		return m_ui.curveWidget->getFunction();
	}

protected:
	void resizeEvent(QResizeEvent* event) override;

signals:
	void curveChanged();

private slots:
	void on_slider_valueChanged(int value) const;
	void on_combo_currentIndexChanged(int index) const;
	void on_curveWidget_curveChanged();
	void on_saveBtn_clicked();
	void on_loadBtn_clicked();
	void on_deleteBtn_clicked();

private:
	void addSavedCurve(char const* serializedFunc, bool displayOnly = true);
	void addSavedCurve(UserCurveLenFunction const& func, bool displayOnly = true);

	Ui::KeyframeCurveEditor m_ui;
	WidgetDataSaver m_dataSaver;
	QStringList m_savedCurves;
};