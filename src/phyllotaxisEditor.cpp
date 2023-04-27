#include "PhyllotaxisEditor.h"
#include "MayaNodes/PhyllotaxisNode.h"
#include "CurveLenFunction/ExpressionCurveLenFunction.h"

#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MDGModifier.h>
#include <maya/MFnTransform.h>

#include <QSpinbox>

#include "ExpressionParser.h"
#include "NodeCmdUtils.h"
#include "MayaNodes/CurveInstanceNode.h"

MStatus PhyllotaxisEditor::updateNetwork() {
    using pn = PhyllotaxisNode;
    using namespace NodeCmdUtils;

    if(m_network.phyllotaxisNode.isNull()) {
        return MStatus::kSuccess;
    }

    auto serializedFunc = UserCurveLenFunction::serialize(*m_func);
    CHECK_RES(serializedFunc);

    MStatus status = Attribute{m_network.phyllotaxisNode, pn::longName(pn::s_curveFunc)}
	    .setValue(serializedFunc.value());
    CHECK_RET(status);

	status = Attribute{ m_network.phyllotaxisNode, pn::longName(pn::s_numIter) }
		.setValue(m_ui.numIterSpinBpx->value());
    CHECK_RET(status);

    status = Attribute{ m_network.phyllotaxisNode, pn::longName(pn::s_step) }
		.setValue(m_ui.integStepDoubleBox->value());
    CHECK_RET(status);

    status = Attribute{ m_network.curveInstanceNode, "InstanceCount" }
		.setValue(m_ui.numPetalsSpinBox->value());
    CHECK_RET(status);

	return MStatus::kSuccess;
}

MStatus PhyllotaxisEditor::createNetwork() {
    /*
	 *             creates the following node graph
	 *
	 *           |-------------------(sphere shape)----> instancer --- (instances)--> [phyllotaxis pattern]
	 *  ----- (inputs)----> phyllotaxis node ---(attr array)---|
	 *           |                 |
	 *           |                 |----(radius)------> makeNurbsCircle ---(circle)----> curveInstanceNode --(attr array)------> instancer ---> [petals]
	 *           -------------------------------------------------------------------------------------------(patal shape)---------|
	 */

    using pn = PhyllotaxisNode;
    using cn = CurveInstanceNode;
    using namespace NodeCmdUtils;

    Network network = m_network;
    MStatus status = MStatus::kSuccess;

    if (network.curveShape.isNull()) {
        MGlobal::displayError("you must selected at least a NURBS curve to create a phyllotaxis pattern");
        return status;
    }

    if (network.phyllotaxisNode.isNull()) {
        auto res = createNode(pn::s_id);
        CHECK_RES(res);

        network.phyllotaxisNode = res.value();
        CHECK_RET(status);
    }
    if (network.phyInstancer.isNull()) {
        auto res = createNode("instancer");
        CHECK_RES(res);

        network.phyInstancer = res.value();
        CHECK_RET(status);
    }
    if (network.petalInstancer.isNull()) {
        auto res = createNode("instancer");
        CHECK_RES(res);

        network.petalInstancer = res.value();
        CHECK_RET(status);

        MFnTransform transform{ network.phyInstancer, &status };
        CHECK_RET(status);

        status = transform.addChild(network.petalInstancer);
        CHECK_RET(status);
    }
    if (network.meshShape.isNull() || network.meshTransform.isNull()) {
        if (network.polySphereNode.isNull()) {
            auto res = createNode("polySphere");
            CHECK_RES(res);

            network.polySphereNode = res.value();
            CHECK_RET(status);
        }

        auto const res = createEmptyMesh();
        CHECK_RES(res);

        auto const& mesh = res.value();
        network.meshShape = mesh.shape;
        network.meshTransform = mesh.transform;

        status = addDefaultShadingGroup(network.meshTransform);
        CHECK_RET(status);
    }

    if(network.makeCurveNode.isNull()) {
        auto res = createNode("makeNurbCircle");
        CHECK_RES(res);

        network.makeCurveNode = res.value();
    }

    if (network.circleCurveShape.isNull() || network.circleCurveTransform.isNull()) {
        auto res = createEmptyNurbs();
        CHECK_RES(res);

        auto const& mesh = res.value();
        network.circleCurveShape = mesh.shape;
        network.circleCurveTransform = mesh.transform;

        // hide the circle from the user
        status = Attribute{ network.circleCurveTransform, "visibility"}.setValue(false);
        CHECK_RET(status);
    }

    if(network.curveInstanceNode.isNull()) {
        auto res = createNode(cn::id);
        CHECK_RES(res);

        network.curveInstanceNode = res.value();
    }

    Attribute const polySphereOutput{ network.polySphereNode, "output" };

	Attribute const meshInput{ network.meshShape, "inMesh" };
    Attribute const meshMatrix{ network.meshTransform, "matrix" };

	Attribute const phyInstancerInputHierarchy{ network.phyInstancer, "inputHierarchy" };
    Attribute const phyInstancerInputPoints{ network.phyInstancer, "inputPoints" };

    Attribute const phyllotaxisPointsOutput{ network.phyllotaxisNode, pn::longName(pn::s_output) };
    Attribute const phyllotaxisRadiusOutput{ network.phyllotaxisNode, pn::longName(pn::s_curveRadiusOutput) };

    Attribute const phyllotaxisCurveInput{ network.phyllotaxisNode, pn::longName(pn::s_curve) };
    Attribute const curveWorldspace{ network.curveShape, "worldSpace"};
    Attribute const makeCircleOutput{ network.makeCurveNode, "outputCurve"};
    Attribute const makeCurveRadiusInput{ network.makeCurveNode, "radius" };

    Attribute const circleInput{ network.circleCurveShape, "create" };
	Attribute const circleWorldSpace{ network.circleCurveShape, "worldSpace" };

    Attribute const petalInstancerInputHierarchy{ network.petalInstancer, "inputHierarchy" };
    Attribute const petalInstancerInputPoints{ network.petalInstancer, "inputPoints" };

    Attribute const curveInstanceNodeCurveInput{ network.curveInstanceNode, "InputCurve" };
    Attribute const curveInstanceNodeInsCountInput{ network.curveInstanceNode, "InstanceCount" };
    Attribute const curveInstanceNodeOutput{ network.curveInstanceNode, "OutTransforms" };

    status = curveWorldspace[0].connect(phyllotaxisCurveInput);
    CHECK_RET(status);
    {
        status = polySphereOutput.connect(meshInput);
        CHECK_RET(status);
        status = meshMatrix.connect(phyInstancerInputHierarchy[0]);
        CHECK_RET(status);
        status = phyllotaxisPointsOutput.connect(phyInstancerInputPoints);
        CHECK_RET(status);
    }

    // if we have petals
    if(!network.petalTransform.isNull()) {
        Attribute const petalMeshMatrix{ network.petalTransform, "matrix" };
        Attribute makeCircleNormal{ network.makeCurveNode, "normal" };

    	status = makeCircleNormal.child(0).setValue(0.0);
        CHECK_RET(status);
        status = makeCircleNormal.child(1).setValue(1.0);
        CHECK_RET(status);
        status = makeCircleNormal.child(2).setValue(0.0);
        CHECK_RET(status);

        status = phyllotaxisRadiusOutput.connect(makeCurveRadiusInput);
        CHECK_RET(status);
        status = makeCircleOutput.connect(circleInput);
        CHECK_RET(status);
    	status = circleWorldSpace[0].connect(curveInstanceNodeCurveInput);
        CHECK_RET(status);
        status = petalMeshMatrix.connect(petalInstancerInputHierarchy[0]);
        CHECK_RET(status);
        status = curveInstanceNodeOutput.connect(petalInstancerInputPoints);
        CHECK_RET(status);
    }

    // hide the sphere from the user
    status = Attribute{ network.meshTransform, "visibility" }.setValue(false);
    CHECK_RET(status);

    m_network = network;
    return status;
}

PhyllotaxisEditor::PhyllotaxisEditor(QWidget* parent) :
    QDialog(parent) {
    m_ui.setupUi(this);

    m_densityFuncExpr = m_ui.expressionPlainTextEdit->toPlainText().toStdString();
	m_densityFuncMirror = m_ui.mirrorCheckBox->isChecked();
    m_densityFuncEditType = static_cast<DensityFuncEditType>(m_ui.tabWidget->currentIndex());

    updateDensityFunc();
}

PhyllotaxisEditor::~PhyllotaxisEditor() { }

void PhyllotaxisEditor::updateDensityFunc() {
    switch (m_densityFuncEditType) {
    case KEYFRAME:
        m_func = m_ui.densityCurveEditor->getFunction();
        break;
    case EXPRESSION:
        m_func = std::make_shared<ExpressionCurveLenFunction>(m_densityFuncExpr, m_densityFuncMirror);
        if (!m_func->valid()) {
            MGlobal::displayInfo(ExpressionParser::getLastError().c_str());
        }
        m_ui.curveWidget->setCurve(m_func);

        break;
    }

    MStatus const status = updateNetwork();
    CHECK_NO_RET(status);
}

void PhyllotaxisEditor::on_expressionPlainTextEdit_textChanged() {
    m_densityFuncExpr = m_ui.expressionPlainTextEdit->toPlainText().toStdString();
    updateDensityFunc();
}

void PhyllotaxisEditor::on_mirrorCheckBox_stateChanged(int state) {
    m_densityFuncMirror = state > 0;
    updateDensityFunc();
}

void PhyllotaxisEditor::on_numIterSpinBpx_valueChanged([[maybe_unused]] int value) {
    MStatus const status = updateNetwork();
    CHECK_NO_RET(status);
}

void PhyllotaxisEditor::on_numPetalsSpinBox_valueChanged([[maybe_unused]] int value) {
    MStatus const status = updateNetwork();
    CHECK_NO_RET(status);
}

void PhyllotaxisEditor::on_integStepDoubleBox_valueChanged([[maybe_unused]] double value) {
    MStatus const status = updateNetwork();
    CHECK_NO_RET(status);
}

void PhyllotaxisEditor::on_tabWidget_currentChanged(int index) {
    m_densityFuncEditType = static_cast<DensityFuncEditType>(index);
    updateDensityFunc();
}

void PhyllotaxisEditor::on_densityCurveEditor_curveChanged() {
    updateDensityFunc();
}

void PhyllotaxisEditor::on_selectPhyCurveBtn_clicked() {
    using namespace NodeCmdUtils;

	MStatus status;
    MSelectionList selection;

    status = MGlobal::getActiveSelectionList(selection);
    CHECK_NO_RET(status);

    if (selection.length() == 1) {
        MObject transform;
        status = selection.getDependNode(0, transform);
        CHECK_NO_RET(status);

        auto const shapeObj = getShape(transform);
        CHECK_RES_NO_RET(shapeObj);

        auto name = getName(transform);
        CHECK_RES_NO_RET(name);

        m_network.curveShape = shapeObj.value();
        m_ui.selectedPhyLabel->setText(MQtUtil::toQString(name.value()));

        if(!m_network.phyllotaxisNode.isNull()) {
            Attribute attr{ m_network.phyllotaxisNode, PhyllotaxisNode::longName(PhyllotaxisNode::s_curve) };
            status = attr.setValue(m_network.curveShape);
            CHECK_NO_RET(status);
        }
    } else {
        MGlobal::displayError("Please Select exactly one NURBS curve");
    }
}

void PhyllotaxisEditor::on_selecPetalMeshBtn_clicked() {
    using namespace NodeCmdUtils;

    MStatus status;
    MSelectionList selection;

    status = MGlobal::getActiveSelectionList(selection);
    CHECK_NO_RET(status);

    if (selection.length() == 1) {
        MObject transform;
        status = selection.getDependNode(0, transform);
        CHECK_NO_RET(status);

        auto const shape = getShape(transform);
        CHECK_RES_NO_RET(shape);

        auto name = getName(transform);
        CHECK_RES_NO_RET(name);

        m_network.petalTransform = transform;
        m_ui.selectedPetalMeshLabel->setText(MQtUtil::toQString(name.value()));

        if(!m_network.petalInstancer.isNull()) {
            Attribute const inputHierarchy{ m_network.petalInstancer, "inputHierarchy"};
            Attribute const transformMatrix{ m_network.petalTransform, "matrix" };
            status = transformMatrix.connect(inputHierarchy[0]);
            CHECK_NO_RET(status);
        }
    } else {
        MGlobal::displayError("Please Select exactly one object");
    }
}

void PhyllotaxisEditor::on_curveWidget_curveUpdated() {
    m_ui.yMinLabel->setText(QString::number(m_ui.curveWidget->getViewYMin(), 'f', 1));
    m_ui.yMaxLabel->setText(QString::number(m_ui.curveWidget->getViewYMax() + 1, 'f', 1));
    //m_ui.minValueLabel->setText(QString{ "Min Value: " } + QString::number(m_ui.curveWidget->getYMin(), 'f', 1));
    //m_ui.maxValueLabel->setText(QString{ "Max Value: " } + QString::number(m_ui.curveWidget->getYMax(), 'f', 1));
}

void PhyllotaxisEditor::on_closeBtn_clicked() {
    reject();
}

void PhyllotaxisEditor::on_createBtn_clicked() {
    if (!m_func || !m_func->valid()) {
        MGlobal::displayError("Please Enter a valid expression for the density function");
        return;
    }

    MStatus status = createNetwork();
    CHECK_NO_RET(status);

    status = updateNetwork();
    CHECK_NO_RET(status);
}