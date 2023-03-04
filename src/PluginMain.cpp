#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MDoubleArray.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MDGModifier.h>
#include <maya/MPlugArray.h>
#include <maya/MVector.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>

#include <QFile>
#include "CurveNode.h"
#include "UnitTestCmd.h"

static void loadAndExecuteMelScript(char const* scriptFileName) {
    QString const filePath = QString{ ":/" } + scriptFileName;
    QFile file { filePath };
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        MGlobal::displayError(MQtUtil::toMString(QString{ "Failed to load " } + filePath));
        return;
    }
    QString const script = QTextStream{ &file }.readAll();
    file.close();
    MGlobal::executeCommand(MQtUtil::toMString(script), true, false);
}

MStatus initializePlugin( MObject obj )
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj, "BlossomPro", "1.0", "Any");

    status = plugin.registerCommand("test", UnitTestCmd::creator);
    CHECK_MSTATUS_AND_RETURN(status, MStatus::kSuccess);

    // Register Node
    status = plugin.registerNode("CurveNode", CurveNode::s_id, CurveNode::creator, CurveNode::initialize);
    CHECK_MSTATUS_AND_RETURN(status, MStatus::kSuccess);

    // init qt resource
    Q_INIT_RESOURCE(resources);
    loadAndExecuteMelScript("MEL/startup.mel");

    return status;
}

MStatus uninitializePlugin( MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj );

    status = plugin.deregisterCommand("test");
    CHECK_MSTATUS_AND_RETURN(status, MStatus::kSuccess);

    status = plugin.deregisterNode(CurveNode::s_id);
    CHECK_MSTATUS_AND_RETURN(status, MStatus::kSuccess);

    loadAndExecuteMelScript("MEL/cleanup.mel");

    return status;
}