#include "BranchEditor.h"
#include "Utils.h"
#include "MayaNodes/BranchNode.h"


static MStatus createBranchNodeInstance(MObject& node, MString const& curveName) {
    using bn = BranchNode;

    MStatus status;
    node = MFnDependencyNode{}.create(bn::s_id, &status);
    CHECK(status, status);

    MFnDependencyNode fnBranchNode { node, &status };
    CHECK(status, status);

    //std::string melCmdTemplate = loadResource("MEL/createPhyllotaxis.mel");
    //std::string melCmd = std::vformat(melCmdTemplate, std::make_format_args(
    //    curveName.asChar(),
    //    fnPhyllotaxisNode.name().asChar(),
    //    pn::longName(pn::s_curve),
    //    fnPhyllotaxisNode.name().asChar(),
    //    pn::longName(pn::s_output)
    //));

    //MGlobal::displayInfo(melCmd.c_str());
    //status = MGlobal::executeCommand(melCmd.c_str());
    //CHECK(status, status);

    return MStatus::kSuccess;
}

BranchEditor::BranchEditor(QWidget* parent) : QDialog{ parent } {
	m_ui.setupUi(this);
}

BranchEditor::~BranchEditor() {
	
}