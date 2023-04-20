#include "BranchEditor.h"

#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>

#include "Utils.h"
#include "MayaNodes/BranchNode.h"


static Result<MObject> createBranchNodeInstance(MString const& carrierName, MString const& generatingName) {
    using bn = BranchNode;

    MStatus status = MStatus::kSuccess;
    MObject node = MFnDependencyNode{}.create(bn::s_id, &status);
    CHECK(status, status);

    MFnDependencyNode const fnBranchNode { node, &status };
    CHECK(status, status);

    std::string const melCmdTemplate = loadResource("MEL/createBranchNode.mel");
    std::string const melCmd = std::vformat(melCmdTemplate, std::make_format_args(
	                                            carrierName.asChar(),
	                                            generatingName.asChar(),
	                                            fnBranchNode.name().asChar(),
	                                            bn::longName(bn::s_carrierCurve),
	                                            bn::longName(bn::s_generatingCurve),
												bn::longName(bn::s_generatingCurveName),
												bn::longName(bn::s_output)
                                            ));
    MString const mstrCmd{ melCmd.c_str() };
    MGlobal::displayInfo(mstrCmd);
    status = MGlobal::executeCommand(mstrCmd);
    CHECK(status, status);

    return node;
}

BranchEditor::BranchEditor(QWidget* parent) : QDialog{ parent } {
	m_ui.setupUi(this);
}

BranchEditor::~BranchEditor() {
	
}

MStatus BranchEditor::updateNode() {
    using bn = BranchNode;

    if(m_nodeInstance.isNull()) {
        return MStatus::kSuccess;
    }

    MStatus status;
    for (auto [attr, curveEditor] : {
	         std::pair{&(bn::s_funcs[0]), m_ui.keyframeCurveEditor},
	         std::pair{&(bn::s_funcs[1]), m_ui.keyframeCurveEditor_2},
	         std::pair{&(bn::s_funcs[2]), m_ui.keyframeCurveEditor_3},
	         std::pair{&(bn::s_funcs[3]), m_ui.keyframeCurveEditor_4},
	         std::pair{&(bn::s_funcs[4]), m_ui.keyframeCurveEditor_5},
         }) {
	    status = updateAttr(m_nodeInstance,
	                        bn::longName(*attr),
	                        MString{curveEditor->getFunction()->serialize().c_str()});
	    CHECK(status, status);
    }

    status = updateAttr(m_nodeInstance,
        bn::longName(bn::s_numIter),
        m_ui.numIterSpinBpx->value());
    CHECK(status, status);

    status = updateAttr(m_nodeInstance,
        bn::longName(bn::s_step),
        m_ui.integStepDoubleBox->value());
    CHECK(status, status);

    return MStatus::kSuccess;
}
void BranchEditor::on_createBtn_clicked() {
    MSelectionList selection;
    MStatus status = MGlobal::getActiveSelectionList(selection);
    CHECK(status, (void)0);

	if (selection.length() == 2) {
        std::array<MString, 2> curveNames;
        for (int i = 0; i < 2; ++i) {
            MDagPath dagPath;
            status = selection.getDagPath(i, dagPath);
            CHECK(status, (void)0);

            MFnDagNode const dagNode{ dagPath };
            curveNames[i] = dagNode.name(&status);
            CHECK(status, (void)0);
        }
        auto const res = createBranchNodeInstance(curveNames[0], curveNames[1]);
        if(!res.valid()) {
            CHECK(res.error(), (void)0);
        }
        m_nodeInstance = res.value();
        updateNode();
    }
    else {
        MGlobal::displayError("Please Select exactly two NURBS curves, carrier curve followed by generating curve");
    }
}

void BranchEditor::on_numIterSpinBpx_valueChanged(int value) {
	MStatus const status = updateNode();
	CHECK(status, (void)(0));
}

void BranchEditor::on_integStepDoubleBox_valueChanged(double value) {
	MStatus const status = updateNode();
	CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_curveChanged() {
	MStatus const status = updateNode();
	CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_2_curveChanged() {
	MStatus const status = updateNode();
	CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_3_curveChanged() {
	MStatus const status = updateNode();
	CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_4_curveChanged() {
	MStatus const status = updateNode();
	CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_5_curveChanged() {
	MStatus const status = updateNode();
	CHECK(status, (void)(0));
}