#include "CreateBSplineSurfaceWindow.h"

#include <QSpinbox>
#include <string>
#include <format>

#include <QFile>
#include <QTextStream>

#include "Utils.h"

CreateBSplineSurfaceWindow::CreateBSplineSurfaceWindow(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

CreateBSplineSurfaceWindow::~CreateBSplineSurfaceWindow()
{
}

void CreateBSplineSurfaceWindow::on_CreateBN_clicked()
{
	std::string mel_template = loadResource("MEL/createBSplineSurface.mel");
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
