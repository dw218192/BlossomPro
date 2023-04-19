#include "BranchNode.h"

void* BranchNode::creator() {
	return new BranchNode;
}

MStatus BranchNode::initialize() {
	return MStatus::kSuccess;
}

MStatus BranchNode::compute(const MPlug& plug, MDataBlock& data) {
	return MPxNode::compute(plug, data);
}
