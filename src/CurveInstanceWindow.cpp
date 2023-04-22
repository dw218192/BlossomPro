#include "CurveInstanceWindow.h"

#include <QSpinbox>
#include <string>
#include <format>

#include <QFile>
#include <QTextStream>

#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MDGModifier.h>
#include <maya/MFnDagNode.h >

#include "Utils.h"

CurveInstanceWindow::CurveInstanceWindow(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

CurveInstanceWindow::~CurveInstanceWindow()
{
}

void CurveInstanceWindow::on_Create_BN_clicked()
{
	QString curve_name = ui.Curve_Name->text();
	QString obj_name = ui.ObjectName->text();

	int instance_count = ui.InstanceNumberSpinBox->value();

	if (curve_name.length() > 0 && obj_name.length() > 0)
	{
		// run mel script to instance object
		std::string mel_template = loadResource("MEL/curveInstance.mel");
		std::string cmds = std::vformat(mel_template, std::make_format_args(curve_name.toStdString(), instance_count, obj_name.toStdString(), obj_name.toStdString()));

		MString M_cmd(cmds.c_str());

		MGlobal::executeCommand(M_cmd);
	}

	accept();
}

void CurveInstanceWindow::on_Cancel_BN_clicked()
{
	reject();
}

void CurveInstanceWindow::on_SelectCurveBN_clicked()
{
	MSelectionList selection;
	MStatus status = MGlobal::getActiveSelectionList(selection);
	CHECK(status, (void)0);

	MItSelectionList iter{ selection, MFn::kNurbsCurve, &status };
	CHECK(status, (void)0);

	if (!iter.isDone()) {
		MDagPath dagPath;
		status = selection.getDagPath(0, dagPath);
		CHECK(status, (void)0);

		MFnDagNode const dagNode{ dagPath };
		MString const curveObjName = dagNode.name(&status);

		QString curve_name(curveObjName.asChar());
		ui.Curve_Name->setText(curve_name);
	}
	else {
		MGlobal::displayError("Please Select a NURBS curve first");
	}
}

void CurveInstanceWindow::on_SelectObjectBN_clicked()
{
	MSelectionList selection;
	MStatus status = MGlobal::getActiveSelectionList(selection);
	CHECK(status, (void)0);

	MItSelectionList iter{ selection, MFn::kMesh, &status };
	CHECK(status, (void)0);

	if (!iter.isDone()) {
		MDagPath dagPath;
		status = selection.getDagPath(0, dagPath);
		CHECK(status, (void)0);

		MFnDagNode const dagNode{ dagPath };
		MString const meshName = dagNode.name(&status);
		QString object_name(meshName.asChar());
		ui.ObjectName->setText(object_name);
	}
	else {
		MGlobal::displayError("Please Select a Object");
	}
}
