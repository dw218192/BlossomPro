#pragma once
#include <stdexcept>

#include "ExpressionParser.h"

/**
 * \brief represents a user-defined function of curve length
 */
struct UserCurveLenFunction
{
	explicit UserCurveLenFunction(std::string expr) noexcept : m_valid(true), m_expr(std::move(expr)) {
		auto const tokens = ExpressionParser::tokenize(m_expr);
		if(!tokens) {
			m_valid = false;
			return;
		}
		auto const postFix = ExpressionParser::toPostFix(*tokens);
		if(!postFix) {
			m_valid = false;
			return;
		}
		int unknownCount = 0;
		for(auto&& token : *postFix) {
			if(token->type() == ExpressionParser::Token::Type::variable) {
				auto const var = std::dynamic_pointer_cast<ExpressionParser::Variable>(token);
				// if it's not a constant and we have not seen it before
				if(!var->isConstant() && !m_varMap.count(var->value())) {
					++unknownCount;

					if (unknownCount > 1) {
						m_valid = false;
						m_varMap.clear();
						return;
					}

					m_varMap[token->value()];
				}
			}
		}
	}

	/**
	 * \brief evaluates the user-defined function
	 * \param s curve length
	 * \return the function output
	 */
	auto operator()(double s) {
		if(!m_valid) {
			return 0.0;
		}

		m_varMap.begin()->second = s;
		auto const ret = ExpressionParser::evalExpression(m_expr, m_varMap);
		if(!ret) {
			throw std::runtime_error{ std::string { "failed to evaluate " } + m_expr };
		}
		return *ret;
	}

	bool valid() const {
		return m_valid;
	}
private:
	bool m_valid;
	std::string m_expr;
	std::unordered_map<std::string, double> m_varMap;
};