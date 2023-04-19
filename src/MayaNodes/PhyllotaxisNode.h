#pragma once
#include <maya/MPxNode.h>
#include <memory>
#include "../CurveLenFunction/UserCurveLenFuncAttribute.h"

struct UserCurveLenFunction;
struct PhyllotaxisGrammar;

class PhyllotaxisNode : public MPxNode
{
public:
	static inline MTypeId s_id{ 0xdeae };
	static inline MObject s_curve;
	static inline UserCurveLenFuncAttribute s_curveFunc;
	static inline MObject s_numIter;
	static inline MObject s_step;
	static inline MObject s_output;

	static constexpr char const* nodeName() {
		return "PhyllotaxisNode";
	}
	static constexpr char const* longName(MObject const& attr) {
		if(&attr == &s_curve) {
			return "curve";
		} else if(&attr == &s_curveFunc) {
			return "serializedCurveFunc";
		} else if (&attr == &s_numIter) {
			return "numIter";
		} else if (&attr == &s_step) {
			return "step";
		} else if(&attr == &s_output) {
			return "outArr";
		}
		return "";
	}
	static constexpr char const* shortName(MObject const& attr) {
		if (&attr == &s_curve) {
			return "cv";
		} else if (&attr == &s_curveFunc) {
			return "scf";
		} else if (&attr == &s_numIter) {
			return "ni";
		} else if (&attr == &s_step) {
			return "st";
		} else if (&attr == &s_output) {
			return "oa";
		}
		return "";
	}
	static void* creator();
	static MStatus initialize();


	PhyllotaxisNode() = default;
	~PhyllotaxisNode() override = default;

	MStatus compute(const MPlug& plug, MDataBlock& data) override;
private:
	std::shared_ptr<UserCurveLenFunction> m_curveFunc;
	std::unique_ptr<PhyllotaxisGrammar> m_grammar;
};