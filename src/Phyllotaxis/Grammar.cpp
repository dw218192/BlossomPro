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
		throw status;
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

	/*
	MVector translation = m_transform.getTranslation(MSpace::kWorld, &status);
	if (MFAIL(status)) {
		throw status;
	}

	MVector rotation = m_transform.eulerRotation().asVector();
	const auto rotOrder = m_transform.rotationOrder(&status);
	if (MFAIL(status)) {
		throw status;
	}

	const MString rotOrderStr = [rotOrder]() -> MString {
		switch (rotOrder) {
		case MTransformationMatrix::kXYZ: return "xyz ";
		case MTransformationMatrix::kXZY: return "xzy ";
		case MTransformationMatrix::kYXZ: return "yxz ";
		case MTransformationMatrix::kYZX: return "yzx ";
		case MTransformationMatrix::kZXY: return "zxy ";
		case MTransformationMatrix::kZYX: return "zyx ";
		default:
			std::string msg = "unknown rot order: ";
			msg += std::to_string(static_cast<int>(rotOrder));
			throw std::runtime_error(msg);
		}
	} ();
	auto vecToStr = [](MVector const& vec)->MString {
		MString ret;
		ret += vec.x; ret += " ";
		ret += vec.y; ret += " ";
		ret += vec.z; ret += " ";
		return ret;
	};
	const MString rotationStr = vecToStr(rotation);
	const MString translationStr = vecToStr(translation);
	*/

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
		throw status;
	}
	return *this;
}

Grammar::Turtle& Grammar::Turtle::drawSphere(double radius) noexcept {
	HANDLE_EXCEPTION(draw("polySphere"));
	return *this;
}

Grammar::Turtle& Grammar::Turtle::drawCube(MVector const& dimension) noexcept {
	HANDLE_EXCEPTION(draw("polyCube"));
	return *this;
}