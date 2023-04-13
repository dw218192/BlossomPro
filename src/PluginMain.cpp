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
#include <maya/MQtUtil.h>
#include <QTextStream>

#include "Phyllotaxis/PhyllotaxisNode.h"

#include "testwindow.h"
#include "phyllotaxisEditor.h"
#include "CreateBSplineSurfaceWindow.h"

#include "Cmds/UnitTestCmd.h"
#include "Cmds/WindowCmd.h"

#include "MayaNodes/ControlPlaneNode.h"
#include "MayaNodes/BSPlineSurfaceNode.h"
#include "MayaNodes/CurveNode.h"
#include "MayaNodes/CurveInstanceNode.h"
#include "Utils.h"

using CmdCleanupFunction = void(*)();
static constexpr std::tuple<char const*, MCreatorFunction, CmdCleanupFunction> g_cmds[] = {
    { "unitTest", UnitTestCmd::creator, nullptr },
    { "createTestWindow", WindowCmd<TestWindow>::creator, WindowCmd<TestWindow>::cleanup },
    { "createPhyllotaxisWindow", WindowCmd<PhyllotaxisEditor>::creator, WindowCmd<PhyllotaxisEditor>::cleanup },
    { "createBSplineSurfaceWindow", WindowCmd<CreateBSplineSurfaceWindow>::creator,  WindowCmd<CreateBSplineSurfaceWindow>::cleanup }
};
static constexpr std::tuple<char const*, MTypeId*, MCreatorFunction, MInitializeFunction> g_nodes[] = {
    { CurveNode::nodeName(), &CurveNode::s_id, CurveNode::creator, CurveNode::initialize },
    { PhyllotaxisNode::nodeName(), &PhyllotaxisNode::s_id, PhyllotaxisNode::creator, PhyllotaxisNode::initialize },
    { ControlPlaneNode::nodeName(), &ControlPlaneNode::id, ControlPlaneNode::creator, ControlPlaneNode::initialize },
    { BSplineSurfaceNode::nodeName(), &BSplineSurfaceNode::id, BSplineSurfaceNode::creator, BSplineSurfaceNode::initialize},
    { CurveInstanceNode::nodeName(), &CurveInstanceNode::id, CurveInstanceNode::creator, CurveInstanceNode::initialize}
};

MStatus initializePlugin( MObject obj )
{
    MStatus status = MStatus::kSuccess;
    MFnPlugin plugin( obj, "BlossomPro", "1.0", "Any");


	// register commands
    for(auto&& [cmdDesc, func, _] : g_cmds) {
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
    for (auto&& [cmdDesc, func, cleanup] : g_cmds) {
        status = plugin.deregisterCommand(cmdDesc);
        CHECK(status, status);

        if(cleanup) {
            cleanup();
        }
    }
    // Deregister nodes
    for (auto&& [nodeName, id, creator, initializer] : g_nodes) {
        status = plugin.deregisterNode(*id);
        CHECK(status, status);
    }

    loadAndExecuteMelScript("MEL/cleanup.mel");

    return status;
}