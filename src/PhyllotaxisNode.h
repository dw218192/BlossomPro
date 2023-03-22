#pragma once
#include <maya/MPxNode.h>
#include <memory>

struct PhyllotaxisGrammar;

class PhyllotaxisNode : public MPxNode
{
public:
	static void* creator();
	static MStatus initialize();
	static inline MTypeId s_id{ 0xdead };
	static inline MObject s_curve;
	static inline MObject s_step;
	static inline MObject s_output;

	PhyllotaxisNode() = default;
	~PhyllotaxisNode() override = default;

	MStatus compute(const MPlug& plug, MDataBlock& data);

private:
	std::unique_ptr<PhyllotaxisGrammar> m_grammar;
};
