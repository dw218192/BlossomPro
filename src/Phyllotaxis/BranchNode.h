#pragma once

#include <array>
#include <maya/MPxNode.h>
#include <memory>
#include "UserCurveLenFuncAttribute.h"

struct UserCurveLenFunction;

class BranchNode : public MPxNode
{
public:
	static constexpr char const* nodeName() {
		return "BranchNode";
	}

#define ATTR_LN(name) if(&attr == &(s_##name)) return #name
#define ATTR_SN(name, sn) if(&attr == &(s_##name)) return sn
	static constexpr char const* longName(MObject const& attr) {
		ATTR_LN(curve);
		ATTR_LN(numIter);
		ATTR_LN(step);
		ATTR_LN(output);
		ATTR_SN(funcs[0], "func0");
		ATTR_SN(funcs[1], "func1");
		ATTR_SN(funcs[2], "func2");
		ATTR_SN(funcs[3], "func3");
		ATTR_SN(funcs[4], "func4");
		return "";
	}
	static constexpr char const* shortName(MObject const& attr) {
		ATTR_SN(curve, "cv");
		ATTR_SN(numIter, "ni");
		ATTR_SN(step, "st");
		ATTR_SN(output, "ot");
		ATTR_SN(funcs[0], "f0");
		ATTR_SN(funcs[1], "f1");
		ATTR_SN(funcs[2], "f2");
		ATTR_SN(funcs[3], "f3");
		ATTR_SN(funcs[4], "f4");
		return "";
	}
#undef ATTR_LN
#undef ATTR_SN

	static void* creator();
	static MStatus initialize();

	static inline MTypeId s_id{ 0xdeae };
	static inline MObject s_curve;
	static inline MObject s_numIter;
	static inline MObject s_step;
	static inline MObject s_output;
	static inline std::array<UserCurveLenFuncAttribute, 5> s_funcs;

	BranchNode() = default;
	~BranchNode() override = default;

	MStatus compute(const MPlug& plug, MDataBlock& data) override;
private:
};
