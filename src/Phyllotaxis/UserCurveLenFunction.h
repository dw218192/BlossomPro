#pragma once
#include <stdexcept>
#include "ExpressionParser.h"

/**
 * \brief
 * represents a user-defined function of curve length ratio\n
 * for a given curve length ratio x = s / total_curve_length in the range [0, 1]\n
 * the f(x) outputs a numerical value in the range [0, inf)\n
 *
 * if mirror is on, the function's domain will be squeezed into [0, 0.5] with y values unchanged\n
 * and then "mirrored" around 0.5
 */
struct UserCurveLenFunction
{
private:
	static inline std::vector<UserCurveLenFunction*> s_objs;
	static void registerInstance(UserCurveLenFunction* ins) {
		for (size_t i = 0; i < s_objs.size(); ++i) {
			if(!s_objs[i]) {
				s_objs[i] = ins;
				ins->m_id = static_cast<int>(i);
			}
		}
		ins->m_id = static_cast<int>(s_objs.size());
		s_objs.push_back(ins);
	}
	static void deregisterInstance(UserCurveLenFunction* ins) {
		s_objs[ins->m_id] = nullptr;
	}

public:
	static UserCurveLenFunction* getInstance(int i) {
		if(i < 0 || i >= s_objs.size()) {
			return nullptr;
		}
		return s_objs[i];
	}

public:
	UserCurveLenFunction(std::string expr, bool mirror = false) noexcept
		: m_mirror(mirror), m_valid(true), m_expr(std::move(expr))
	{
		registerInstance(this);

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
	virtual ~UserCurveLenFunction() {
		deregisterInstance(this);
	}
private:
	double eval(double s) {
		m_varMap.begin()->second = s;
		auto const ret = ExpressionParser::evalExpression(m_expr, m_varMap);
		if (!ret) {
			throw std::runtime_error{ std::string { "failed to evaluate " } + m_expr };
		}
		return *ret;
	}
public:
	/**
	 * \brief evaluates the user-defined function
	 * \param s curve length
	 * \return the function output
	 */
	double operator()(double s) {
		if (!m_valid) {
			return 0.0;
		}
		if (s < 0 || s >= 1.01) {
			throw std::runtime_error{ std::string { "curve function's domain not in [0,1]" } + m_expr };
		}
		double ret;
		if(m_mirror) {
			if(s <= 0.5) {
				ret = eval(s * 2);
			} else {
				ret = eval(1 - (s - 0.5) * 2);
			}
		} else {
			ret = eval(s);
		}
		return ret;
	}

	bool valid() const {
		return m_valid;
	}
	int id() const {
		return m_id;
	}
private:
	int m_id;
	bool m_mirror;
	bool m_valid;
	std::string m_expr;
	std::unordered_map<std::string, double> m_varMap;
};