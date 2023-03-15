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

#include <QTextStream>

#include "CurveNode.h"
#include "testwindow.h"
#include "phyllotaxisEditor.h"

#include "Cmds/UnitTestCmd.h"
#include "Cmds/WindowCmd.h"

static constexpr std::pair<char const*, MCreatorFunction> g_cmds[] = {
    { "unitTest", UnitTestCmd::creator },
    { "createTestWindow", WindowCmd<TestWindow>::creator },
    { "createPhyllotaxisWindow", WindowCmd<PhyllotaxisEditor>::creator }
};

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
    MStatus status = MStatus::kSuccess;
    MFnPlugin plugin( obj, "BlossomPro", "1.0", "Any");

    // register commands
    for(auto&& [cmdDesc, func] : g_cmds) {
        status = plugin.registerCommand(cmdDesc, func);
        CHECK(status);
    }

    // Register Node
    status = plugin.registerNode("CurveNode", CurveNode::s_id, CurveNode::creator, CurveNode::initialize);
    CHECK(status);

    // init qt resource
    Q_INIT_RESOURCE(resources);
    loadAndExecuteMelScript("MEL/startup.mel");
    
    return status;
}

MStatus uninitializePlugin( MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj );

    // deregister commands
    for (auto&& [cmdDesc, func] : g_cmds) {
        status = plugin.deregisterCommand(cmdDesc);
        CHECK(status);
    }

    status = plugin.deregisterNode(CurveNode::s_id);
    CHECK(status);

    loadAndExecuteMelScript("MEL/cleanup.mel");

    return status;
}