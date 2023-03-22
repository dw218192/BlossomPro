#include "Grammar.h"
#include "../Utils.h"
#include <maya/MEulerRotation.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MTransformationMatrix.h>
#include <sstream>

// warning: maya matrix is row-major
Grammar::Turtle::Turtle() {
	m_transform = MMatrix::identity;
}

Grammar::Turtle& Grammar::Turtle::forward(double length) {
	MTransformationMatrix transform;
	MStatus status = transform.setTranslation(MVector{ 0, 0, length }, MSpace::kTransform);
	if (MFAIL(status)) {
		throw MAYA_EXCEPTION(status);
	}
	m_transform = transform.asMatrix() * m_transform;
	return *this;
}

// ccw around y
Grammar::Turtle& Grammar::Turtle::rotateLeft(double angleDegrees) noexcept {
	return doLocalEulerDegrees(0, angleDegrees, 0);
}

// cw around y
Grammar::Turtle& Grammar::Turtle::rotateRight(double angleDegrees) noexcept {
	return doLocalEulerDegrees(0, -angleDegrees, 0);
}

// cw around z
Grammar::Turtle& Grammar::Turtle::rollLeft(double angleDegrees) noexcept {
	return doLocalEulerDegrees(0, 0, -angleDegrees);
}

// cw around z
Grammar::Turtle& Grammar::Turtle::rollRight(double angleDegrees) noexcept {
	return doLocalEulerDegrees(0, 0, angleDegrees);
}

// cw around x
Grammar::Turtle& Grammar::Turtle::pitchUp(double angleDegrees) noexcept {
	return doLocalEulerDegrees(-angleDegrees, 0, 0);
}

// ccw around x
Grammar::Turtle& Grammar::Turtle::pitchDown(double angleDegrees) noexcept {
	return doLocalEulerDegrees(angleDegrees, 0, 0);
}

Grammar::Turtle& Grammar::Turtle::pushState() {
	m_transformStack.push(m_transform);
	return *this;
}

Grammar::Turtle& Grammar::Turtle::popState() {
	if (m_transformStack.empty()) {
		throw std::runtime_error("matrix stack is empty");
	}
	m_transform = m_transformStack.top();
	m_transformStack.pop();
	return *this;
}

Grammar::Turtle& Grammar::Turtle::doLocalEulerDegrees(double x, double y, double z) noexcept {
	MEulerRotation rotation{
		x * 0.017453,
		y * 0.017453,
		z * 0.017453
	};
	m_transform = rotation.asMatrix() * m_transform;
	return *this;
}

Grammar::Turtle& Grammar::Turtle::draw(MString const& melCommand) {
	MStatus status;
	MString cmd;
	cmd += MString{ "$obj = `" } + melCommand + "`; ";
	cmd += MString{ "xform" };
	cmd += MString{ "-m " };
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			cmd += m_transform(i, j);
			cmd += " ";
		}
	}
	cmd += "$obj; ";

	MGlobal::displayInfo(cmd);

	status = MGlobal::executeCommand(cmd);
	if (MFAIL(status)) {
		throw MAYA_EXCEPTION(status);
	}
	return *this;
}

Grammar::Turtle& Grammar::Turtle::drawSphere(double radius) {
	return draw(MString{ "polySphere -r " } + radius);
}

Grammar::Turtle& Grammar::Turtle::drawCube(MVector const& dimension) {
	MString cmd = "polyCube -w ";
	cmd += dimension[0];
	cmd += "-h ";
	cmd += dimension[1];
	cmd += "-d ";
	cmd += dimension[2];
	return draw(cmd);
}