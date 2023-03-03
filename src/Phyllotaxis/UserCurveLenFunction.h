#pragma once
#include <stdexcept>

#include "ExpressionParser.h"

/**
 * \brief represents a user-defined function of curve length
 */
struct UserCurveLenFunction
{
	explicit UserCurveLenFunction(std::string expr) : m_expr(std::move(expr)) {
		auto const tokens = ExpressionParser::tokenize(m_expr);
		if(!tokens) {
			throw std::runtime_error{ std::string { "failed to tokenize: " } + m_expr };
		}
		auto postFix = ExpressionParser::toPostFix(*tokens);
		if(!postFix) {
			throw std::runtime_error{ std::string { "failed to convert to postfix: " } + m_expr };
		}

		int unknownCount = 0;
		for(auto&& token : *postFix) {
			if(token->type() == ExpressionParser::Token::Type::variable) {
				++unknownCount;
				if(unknownCount > 1) {
					throw std::runtime_error{ std::string { "expression can only have 1 variable: " } + m_expr };
				}
				m_varMap[token->value()];
			}
		}
	}

	/**
	 * \brief evaluates the user-defined function
	 * \param s curve length
	 * \return the function output
	 */
	double operator()(double s) {
		m_varMap.begin()->second = s;
		auto const ret = ExpressionParser::evalExpression(m_expr, m_varMap);
		if(!ret) {
			throw std::runtime_error{ std::string { "failed to evaluate " } + m_expr };
		}
		return *ret;
	}
private:
	std::string m_expr;
	std::unordered_map<std::string, double> m_varMap;
};