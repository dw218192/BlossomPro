#include "CreateBSplineSurfaceWindow.h"

#include <QSpinbox>
#include <string>
#include <format>

#include <QFile>
#include <QTextStream>

CreateBSplineSurfaceWindow::CreateBSplineSurfaceWindow(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
}

CreateBSplineSurfaceWindow::~CreateBSplineSurfaceWindow()
{
}

void CreateBSplineSurfaceWindow::on_CreateBN_clicked()
{
	QString const filePath = QString{ ":/" } + "MEL/createBSplineSurface.mel";

	QFile file{ filePath };
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		MString error;
		error += "Fail to Load: ";
		error += filePath.toStdString().c_str();
		MGlobal::displayError(error);
		return;
	}
	std::string mel_template = QTextStream{ &file }.readAll().toStdString();

	std::string cmds = std::vformat(mel_template, std::make_format_args(
								          ui.CPXSpinBox->value(),
								          ui.CPYSpinBox->value(),
								          ui.BSSXSpinBox->value(),
								          ui.BSSYSpinBox->value()));

	MString M_cmd(cmds.c_str());

	MGlobal::displayInfo(M_cmd);
	MGlobal::executeCommand(M_cmd);
	accept();
}

void CreateBSplineSurfaceWindow::on_CancelBN_clicked()
{
	reject();
}
