#include "ExpressionCurveLenFunction.h"
#include "../Utils.h"

#include <stdexcept>
#include <sstream>

using eclf = ExpressionCurveLenFunction;

eclf::ExpressionCurveLenFunction(std::string expr, bool mirror) noexcept
	: m_mirror(mirror), m_valid(true), m_expr(std::move(expr))
{
	auto const tokens = ExpressionParser::tokenize(m_expr);
	if (!tokens) {
		m_valid = false;
		return;
	}
	auto const postFix = ExpressionParser::toPostFix(*tokens);
	if (!postFix) {
		m_valid = false;
		return;
	}
	int unknownCount = 0;
	for (auto&& token : *postFix) {
		if (token->type() == ExpressionParser::Token::Type::variable) {
			auto const var = std::dynamic_pointer_cast<ExpressionParser::Variable>(token);
			// if it's not a constant and we have not seen it before
			if (!var->isConstant() && !m_varMap.count(var->value())) {
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
double eclf::eval(double s) {
	m_varMap.begin()->second = s;
	auto const ret = ExpressionParser::evalExpression(m_expr, m_varMap);
	if (!ret) {
		throw std::runtime_error{ std::string { "failed to evaluate " } + m_expr };
	}
	return *ret;
}
double eclf::operator()(double s) const {
	auto const self = const_cast<ExpressionCurveLenFunction*>(this);
	if (!m_valid) {
		return 0.0;
	}
	if (s < 0 || s >= 1.01) {
		throw std::runtime_error{ std::string { "curve function's domain not in [0,1]" } + m_expr };
	}
	double ret;
	if (m_mirror) {
		if (s <= 0.5) {
			ret = self->eval(s * 2);
		} else {
			ret = self->eval(1 - (s - 0.5) * 2);
		}
	} else {
		ret = self->eval(s);
	}
	return ret;
}

bool eclf::operator==(UserCurveLenFunction const& other) const {
	if (auto const ptr = dynamic_cast<ExpressionCurveLenFunction const*>(&other); !ptr) {
		return false;
	} else {
		return this->operator==(*ptr);
	}
}
bool eclf::operator==(ExpressionCurveLenFunction const& other) const {
	return m_mirror == other.m_mirror &&
		m_valid == other.m_valid &&
		m_expr == other.m_expr;
}

static constexpr char k_sep = ' ';
std::string eclf::serialize() const {
	std::ostringstream ss;
	ss.exceptions(std::ios::failbit);

	ss << STR(ExpressionCurveLenFunction) << k_sep;
	ss << m_mirror << k_sep << m_valid << k_sep << m_expr << k_sep;

	if (!m_varMap.empty()) {
		ss << "v:" << k_sep;
		auto const it = m_varMap.begin();
		ss << it->first << k_sep << it->second;
	}

	return ss.str();
}

void eclf::deserialize(std::istringstream& ss) {
	ss.exceptions(std::ios::failbit);

	bool mirror;
	bool valid;
	std::string expr;
	std::unordered_map<std::string, double> varMap;

	ss >> mirror >> valid >> expr;
	std::string flag;

	if (ss >> flag) {
		if (flag == "v:") {
			std::string key;
			double val;
			ss >> key >> val;

			varMap[key] = val;
		}
	}

	m_mirror = mirror;
	m_valid = valid;
	m_expr = std::move(expr);
	m_varMap = std::move(varMap);
}