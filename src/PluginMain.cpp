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
#include <list>

#include "CurveNode.h"
#include "UnitTestCmd.h"

MStatus initializePlugin( MObject obj )
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj, "MyPlugin", "1.0", "Any");
/*
	char buffer[2048];
    int check = sprintf_s(buffer, 2048, "source \"%s/MyPluginDialog\";", plugin.loadPath().asChar());
    if(check < 0) {
        return MStatus::kFailure;
    }

	status = MGlobal::executeCommand(buffer, true);
    if (!status) {
        status.perror("registerCommand");
        return status;
    }
*/

    status = plugin.registerCommand("test", UnitTestCmd::creator);
    CHECK_MSTATUS_AND_RETURN(status, MStatus::kSuccess);

    // Register Node
    status = plugin.registerNode("CurveNode", CurveNode::s_id, CurveNode::creator, CurveNode::initialize);
    CHECK_MSTATUS_AND_RETURN(status, MStatus::kSuccess);

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

    return status;
}