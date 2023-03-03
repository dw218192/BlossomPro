#include "ExpressionParser.h"

#include <algorithm>
#include <vector>
#include <functional>
#include <iostream>
#include <optional>
#include <memory>


// unsupported expression types:
// 1. 5(6+a)  omitted multiplication
// 2. +34     unary plus
// 3. -(2+3)  unary operation on brackets

namespace ExpressionParser {
	auto Token::isOperator(std::string const& val) noexcept -> bool {
		return val == "+" || val == "-" || val == "*" || val == "/" || val == "^";
	}
	auto Token::getPrecdence(std::string const& val) -> int {
		if (!isOperator(val)) {
			throw std::runtime_error("unknown operator found: " + val);
		}
		if (val == "^") return 4;
		else if (val == "*" || val == "/") return 3;
		else return 2;
	}
	auto Token::isLeftAssociative(std::string const& val) -> bool {
		if (!isOperator(val)) {
			throw std::runtime_error("unknown operator found: " + val);
		}
		return val != "^";
	}
	auto BinaryOp::eval(double x, double y) const -> double {
		if (m_value == "+") {
			return x + y;
		} else if (m_value == "-") {
			return x - y;
		} else if (m_value == "*") {
			return x * y;
		} else if (m_value == "/") {
			return x / y;
		} else if (m_value == "^") {
			return std::pow(x, y);
		} else {
			throw std::runtime_error(std::string{ "unknown binary operation " } + m_value);
		}
	}

	Variable::Variable(std::string val) noexcept : Token(std::move(val), Type::variable) {
		try {
			m_numericVal = std::stod(m_value);
		} catch (std::exception const& ex) {
			m_numericVal = std::nullopt;
		}
	}
	Variable::Variable(double val) noexcept : Token("", Type::variable) {
		m_numericVal = val;
	}

	auto Variable::eval(std::unordered_map<std::string, double> const& variableValues) const noexcept -> double {
		if (m_numericVal) {
			return *m_numericVal;
		} else {
			auto const it = variableValues.find(m_value);
			if(it != variableValues.end()) {
				return it->second;
			} else {
				std::cerr << std::string{"Variable not found, assuming 0 : "} + m_value;
				return 0;
			}
		}
	}
	auto operator<<(std::ostream& os, Token::Type type) noexcept -> std::ostream& {
		switch(type) {
		case Token::Type::binaryOperator: os << "bop"; break;
		case Token::Type::leftBracket: os << "lb"; break;
		case Token::Type::rightBracket: os << "rb"; break;
		case Token::Type::variable: os << "var"; break;
		default: os << "unknown"; break;
		}
		return os;
	}

	auto operator<<(std::ostream& os, Token const& token) noexcept -> std::ostream& {
		//os << '{' << token.type() << ':' << token.value() << '}';
		os << token.value();
		return os;
	}
	auto operator<<(std::ostream& os, TokenList const& tokens) noexcept -> std::ostream& {
		os << "token list: [ ";
		for(auto&& token : tokens) {
			os << *token << ' ';
		}
		os << "]";
		return os;
	}
	
	auto tokenize(std::string const& infixExpression) noexcept -> std::optional<TokenList> {
		std::string expr;
		std::vector<std::shared_ptr<Token>> tokens;
		std::vector<std::shared_ptr<Token>> finalTokens;

		// remove all whitespaces
		for(char c : infixExpression) {
			if(c != ' ') {
				expr.push_back(c);
			}
		}

		for (size_t i = 0; i < expr.size(); ++i) {
			std::string str{ expr[i] };

			bool isNegativeSign = str == "-" && (tokens.empty() || (tokens.back()->type() != Token::Type::variable && tokens.back()->type() != Token::Type::rightBracket));
			if (Token::isOperator(str) && !isNegativeSign) {
				tokens.emplace_back(std::make_shared<BinaryOp>(str));
			} else if (str == "(") {
				tokens.emplace_back(std::make_shared<Token>(str, Token::Type::leftBracket));
			} else if (str == ")") {
				tokens.emplace_back(std::make_shared<Token>(str, Token::Type::rightBracket));
			} else {
				size_t j = i + 1;
				for (; j < expr.size() && !Token::isOperator({ expr[j] }) && expr[j] != '(' && expr[j] != ')'; ++j) {
					str.push_back(expr[j]);
				}
				tokens.emplace_back(std::make_shared<Variable>(str));
				i = j - 1;
			}
		}
		// check if the expression is legal
		/*
		 * all token types: bop, lb, rb, var, nothing
		 *
		 *  variable can be preceded by
		 *  bop, lb, nothing
		 *
		 *  binary operators can be preceded by
		 *  rb, var
		 *
		 *  left brackets can be preceded by
		 *	bop, lb, nothing
		 *
		 *	right brackets can be preceded by
		 *	rb, var
		*/

		for (auto it = tokens.begin(), prevIt = tokens.end(); it != tokens.end(); prevIt = it, ++it) {
			Token const& token = **it;
			if (token.type() == Token::Type::variable) {
				if (prevIt != tokens.end()) {
					Token const& prevToken = **prevIt;
					if (prevToken.type() != Token::Type::binaryOperator &&
						prevToken.type() != Token::Type::leftBracket) {
						return std::nullopt;
					}
				}
			} else if (token.type() == Token::Type::binaryOperator) {
				if (prevIt != tokens.end()) {
					Token const& prevToken = **prevIt;
					if (prevToken.type() != Token::Type::variable &&
						prevToken.type() != Token::Type::rightBracket) {
						return std::nullopt;
					}
				} else {
					return std::nullopt;
				}
			} else if (token.type() == Token::Type::leftBracket) {
				if (prevIt != tokens.end()) {
					Token const& prevToken = **prevIt;
					if (prevToken.type() != Token::Type::binaryOperator && 
						prevToken.type() != Token::Type::leftBracket) {
						return std::nullopt;
					}
				}
			} else if (token.type() == Token::Type::rightBracket) {
				if (prevIt != tokens.end()) {
					Token const& prevToken = **prevIt;
					if (prevToken.type() != Token::Type::variable && 
						prevToken.type() != Token::Type::rightBracket) {
						return std::nullopt;
					}
				} else {
					return std::nullopt;
				}
			} else {
				return std::nullopt;
			}
		}

		return std::make_optional(std::move(tokens));
	}

	auto toPostFix(std::vector<std::shared_ptr<Token>> const& tokens) noexcept -> std::optional<TokenList> {
		TokenList stk;
		TokenList processed;

		for (auto&& token : tokens) {
			if (token->type() == Token::Type::variable) {
				processed.emplace_back(token);
			} else if (token->type() == Token::Type::binaryOperator) {
				while (!stk.empty() && 
					stk.back()->type() == Token::Type::binaryOperator && 
					(Token::getPrecdence(token->value()) < Token::getPrecdence(stk.back()->value()) ||
						(Token::getPrecdence(token->value()) == Token::getPrecdence(stk.back()->value()) && 
						Token::isLeftAssociative(token->value())))) {
					processed.emplace_back(stk.back());
					stk.pop_back();
				}
				stk.emplace_back(token);
			} else if (token->type() == Token::Type::leftBracket) {
				stk.emplace_back(token);
			} else if (token->type() == Token::Type::rightBracket) {
				while (!stk.empty() && stk.back()->type() != Token::Type::leftBracket) {
					processed.emplace_back(stk.back());
					stk.pop_back();
				}
				if (!stk.empty() && stk.back()->type() == Token::Type::leftBracket) {
					stk.pop_back();
				} else {
					// ill-formed expression
					return std::nullopt;
				}
			}
		}
		while (!stk.empty()) {
			if(stk.back()->type() == Token::Type::leftBracket) {
				// ill-formed expression
				return std::nullopt;
			}
			processed.emplace_back(stk.back());
			stk.pop_back();
		}

		return std::make_optional(std::move(processed));
	}

	auto evalExpression(TokenList const& postFixTokens, std::unordered_map<std::string, double> const& values) noexcept -> std::optional<double> {
		// evaluation
		TokenList stk;
		auto curToken = postFixTokens.rbegin();
		while(curToken != postFixTokens.rend() || stk.size() > 1) {
			if (curToken != postFixTokens.rend()) {
				stk.emplace_back(*curToken);
				++curToken;
			}

			while(stk.size() > 2 && 
				stk[stk.size() - 1]->type() == Token::Type::variable && 
				stk[stk.size() - 2]->type() == Token::Type::variable) {
				// evaluate a binary expression
				size_t const n = stk.size();
				if(stk[n - 3]->type() != Token::Type::binaryOperator) {
					std::cerr << "ill-formed expr\n";
					return std::nullopt;
				}
				auto op = stk[n - 3];
				auto operand1 = stk[n-1];
				auto operand2 = stk[n - 2];
				stk.pop_back();
				stk.pop_back();
				stk.pop_back();

				double val = std::dynamic_pointer_cast<BinaryOp>(op)->eval(
					std::dynamic_pointer_cast<Variable>(operand1)->eval(values),
					std::dynamic_pointer_cast<Variable>(operand2)->eval(values));
				stk.emplace_back(std::make_shared<Variable>(val));
			}
			// std::cout << stk << std::endl;
		}
		if(stk.size() != 1) {
			std::cerr << "ill-formed expr\n";
			return std::nullopt;
		}
		double ret = std::dynamic_pointer_cast<Variable>(stk.back())->eval({});
		return std::make_optional(ret);
	}

	auto evalExpression(std::string const& expression, std::unordered_map<std::string, double> const& values) noexcept -> std::optional<double> {
		auto const tokens = tokenize(expression);
		if(!tokens) {
			std::cerr << "tokenize failed\n";
			return std::nullopt;
		}
		auto const postFixTokens = toPostFix(*tokens);
		if(!postFixTokens) {
			std::cerr << "toPostFix failed\n";
			return std::nullopt;
		}
		return evalExpression(*postFixTokens, values);
	}
}