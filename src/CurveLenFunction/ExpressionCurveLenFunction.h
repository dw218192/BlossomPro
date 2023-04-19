#pragma once

#include <stdexcept>
#include "ExpressionParser.h"
#include "UserCurveLenFunction.h"

struct ExpressionCurveLenFunction : public UserCurveLenFunction {
	ExpressionCurveLenFunction(std::string expr, bool mirror = false) noexcept;
private:
	double eval(double s);
public:
	/**
	 * \brief evaluates the user-defined function
	 * \param s curve length
	 * \return the function output
	 */
	double operator()(double s) const override;
	bool operator==(UserCurveLenFunction const& other) const override;
	bool operator==(ExpressionCurveLenFunction const& other) const;
	bool valid() const override {
		return m_valid;
	}

	std::string serialize() const override;
	void deserialize(std::istringstream& ss) override;
private:
	bool m_mirror;
	bool m_valid;
	std::string m_expr;

	// just a cache, not treated as the object's state.
	std::unordered_map<std::string, double> m_varMap;
};