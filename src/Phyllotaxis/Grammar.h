#pragma once
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <stack>
#include <vector>

struct GrammarResult {
	GrammarResult(MVector const& pos, MVector const& scale) : pos(pos), scale(scale) {}
	MVector pos;
	MVector scale;
};

/// <summary>
/// represents the turtle, the L-System grammar, and the parser all together
/// </summary>
struct Grammar {
	/// <summary>
	/// represents the turtle
	/// convention:
	/// Z is forward; Y is upward; X is right
	/// all rotation is around local Y
	/// all translation is along local Z
	/// </summary>
	struct Turtle {
		Turtle();

		Turtle(Turtle&) = delete;
		Turtle(Turtle&&) = delete;
		Turtle& operator=(Turtle&) = delete;
		Turtle& operator=(Turtle&&) = delete;
		// operations in line with the operations available in the cpfg language
		Turtle& forward(double length);
		Turtle& rotateLeft(double angleDegrees) noexcept;
		Turtle& rotateRight(double angleDegrees) noexcept;
		Turtle& rollLeft(double angleDegrees) noexcept;
		Turtle& rollRight(double angleDegrees) noexcept;
		Turtle& pitchUp(double angleDegrees) noexcept;
		Turtle& pitchDown(double angleDegrees) noexcept;
		Turtle& pushState();
		Turtle& popState();
		Turtle& drawSphere(double radius);
		Turtle& drawCube(MVector const& dimension = {0,0,0});

		MVector getPos() const;

	private:
		Turtle& doLocalEulerDegrees(double x, double y, double z) noexcept;
		Turtle& draw(MString const& melCommand);

		MMatrix m_transform;
		std::stack<MMatrix> m_transformStack;
	};

	Grammar() = default;
	virtual ~Grammar() = default;
	Grammar(Grammar&) = delete;
	Grammar(Grammar&&) = delete;
	Grammar& operator=(Grammar&) = delete;
	Grammar& operator=(Grammar&&) = delete;

	/// <summary>
	/// evaluates the next iteration of the grammar
	virtual void nextIter() = 0;
	/// <summary>
	/// returns if there is a next iteration
	virtual bool hasNext() const = 0;
	/// <summary>
	/// evaluate for a given number of iterations
	void process(int numIter) {
		for (int i = 0; i < numIter && hasNext(); ++i) {
			nextIter();
		}
	}
	auto const& result() const {
		return m_result;
	}
protected:
	std::vector<GrammarResult> m_result;

	Turtle m_turtle;
};