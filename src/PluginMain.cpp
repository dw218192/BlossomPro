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

#include "Phyllotaxis/PhyllotaxisNode.h"

#include "testwindow.h"
#include "phyllotaxisEditor.h"

#include "Cmds/UnitTestCmd.h"
#include "Cmds/WindowCmd.h"

#include "MayaNodes/ControlPlaneNode.h"
#include "MayaNodes/BSPlineSurfaceNode.h"

static constexpr std::pair<char const*, MCreatorFunction> g_cmds[] = {
    { "unitTest", UnitTestCmd::creator },
    { "createTestWindow", WindowCmd<TestWindow>::creator },
    { "createPhyllotaxisWindow", WindowCmd<PhyllotaxisEditor>::creator }
};
static constexpr std::tuple<char const*, MTypeId*, MCreatorFunction, MInitializeFunction> g_nodes[] = {
    { PhyllotaxisNode::nodeName(), &PhyllotaxisNode::s_id, PhyllotaxisNode::creator, PhyllotaxisNode::initialize },
    { ControlPlaneNode::nodeName(), &ControlPlaneNode::id, ControlPlaneNode::creator, ControlPlaneNode::initialize },
    { BSplineSurfaceNode::nodeName(), &BSplineSurfaceNode::id, BSplineSurfaceNode::creator, BSplineSurfaceNode::initialize}
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
        CHECK(status, status);
    }

    // Register Node
    for (auto&& [nodeName, id, creator, initializer] : g_nodes) {
        status = plugin.registerNode(nodeName, *id, creator, initializer);
        CHECK(status, status);
    }

    // init qt resource
    Q_INIT_RESOURCE(resources);
    loadAndExecuteMelScript("MEL/startup.mel");
    
    return status;
}

MStatus uninitializePlugin( MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj );

    // Deregister commands
    for (auto&& [cmdDesc, func] : g_cmds) {
        status = plugin.deregisterCommand(cmdDesc);
        CHECK(status, status);
    }

    // Deregister nodes
    for (auto&& [nodeName, id, creator, initializer] : g_nodes) {
        status = plugin.deregisterNode(*id);
        CHECK(status, status);
    }

    loadAndExecuteMelScript("MEL/cleanup.mel");

    return status;
}