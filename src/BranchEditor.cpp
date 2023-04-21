#include "BranchEditor.h"

#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>

#include "Utils.h"
#include "MayaNodes/BranchNode.h"


Result<BranchEditor::BranchNodeNetwork> BranchEditor::createNetwork(MObject const& carrierCurve, MObject const& generatingCurve) {
    using bn = BranchNode;

    MStatus status = MStatus::kSuccess;
    MObject branchNode = MFnDependencyNode{}.create(bn::s_id, &status);
    CHECK(status, status);

    MObject const loftNode = MFnDependencyNode{}.create("loft", &status);
    CHECK(status, status);

    MObject const tessNode = MFnDependencyNode{}.create("nurbsTessellate", &status);
    CHECK(status, status);

    MObject const transformNode = MFnTransform{}.create(MObject::kNullObj, &status);
    CHECK(status, status);

    MObject const meshNode = MFnMesh{}.create(0, 0, MPointArray{}, MIntArray{}, MIntArray{}, transformNode, &status);
    CHECK(status, status);

    auto const res = getName(generatingCurve);
    if(!res.valid()) {
        status = res.error();
        CHECK(status, status);
    }

    updateAttr(branchNode, bn::longName(bn::s_generatingCurveName), res.value());
    connectAttr(loftNode, "outputSurface", tessNode, "inputSurface");
    connectAttr(tessNode, "outputPolygon", meshNode, "inMesh");

/*
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
*/
    BranchNodeNetwork ret;
    ret.branchNodeObj = branchNode;
    ret.loftNodeObj = loftNode;

    updateNetwork(ret);
    return ret;
}

BranchEditor::BranchEditor(QWidget* parent) : QDialog{ parent } {
	m_ui.setupUi(this);
}

BranchEditor::~BranchEditor() {
	
}

MStatus BranchEditor::updateNetwork(BranchNodeNetwork const& network) {
    using bn = BranchNode;

    if(network.branchNodeObj.isNull()) {
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
	    status = updateAttr(network.branchNodeObj,
	                        bn::longName(*attr),
	                        MString{curveEditor->getFunction()->serialize().c_str()});
	    CHECK(status, status);
    }

    status = updateAttr(network.branchNodeObj,
        bn::longName(bn::s_numIter),
        m_ui.numIterSpinBpx->value());
    CHECK(status, status);

    status = updateAttr(network.branchNodeObj,
        bn::longName(bn::s_step),
        m_ui.integStepDoubleBox->value());
    CHECK(status, status);

    connectAttr(network.branchNodeObj, bn::longName(bn::s_output), network.loftNodeObj, "inputCurve");

    return MStatus::kSuccess;
}
void BranchEditor::on_createBtn_clicked() {
    MSelectionList selection;
    MStatus status = MGlobal::getActiveSelectionList(selection);
    CHECK(status, (void)0);

	if (selection.length() == 2) {
        std::array<MObject, 2> curves;

        status = selection.getDependNode(0, curves[0]);
        CHECK(status, (void)0);

        status = selection.getDependNode(1, curves[1]);
        CHECK(status, (void)0);

        auto const res = createNetwork(curves[0], curves[1]);
        if(!res.valid()) {
            CHECK(res.error(), (void)0);
        }
        m_network = res.value();
    }
    else {
        MGlobal::displayError("Please Select exactly two NURBS curves, carrier curve followed by generating curve");
    }
}

void BranchEditor::on_numIterSpinBpx_valueChanged([[maybe_unused]] int value) {
	MStatus const status = updateNetwork(m_network);
	CHECK(status, (void)(0));
}

void BranchEditor::on_integStepDoubleBox_valueChanged([[maybe_unused]] double value) {
    MStatus const status = updateNetwork(m_network);
    CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_2_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_3_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_4_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK(status, (void)(0));
}

void BranchEditor::on_keyframeCurveEditor_5_curveChanged() {
    MStatus const status = updateNetwork(m_network);
    CHECK(status, (void)(0));
}