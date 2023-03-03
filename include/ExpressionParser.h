#pragma once
#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace ExpressionParser {
	struct Token {
		enum class Type {
			binaryOperator,
			variable,
			leftBracket,
			rightBracket
		};

		Token(std::string val, Type type) : m_value(std::move(val)), m_type(type) { }
		Token(Token&&) noexcept = default;
		virtual ~Token() noexcept = default;
		auto value() const ->std::string { return m_value; }
		auto type() const { return m_type; }
		static auto isOperator(std::string const& val) noexcept -> bool;
		static auto getPrecdence(std::string const& val) -> int;
		static auto isLeftAssociative(std::string const& val) -> bool;

	protected:
		std::string m_value;
		Type m_type;
	};

	using TokenList = std::vector<std::shared_ptr<Token>>;

	/**
	 * \brief represents a binary operator (taking two arguments and outputting a single value)
	 */
	struct BinaryOp : public Token {
		BinaryOp(std::string val) : Token(std::move(val), Type::binaryOperator) { }
		BinaryOp(BinaryOp&&) = default;
		~BinaryOp() override = default;

		auto eval(double x, double y) const -> double;
	};

	/**
	 * \brief represents either an unknown variable or a constant
	 */
	struct Variable : public Token {
		explicit Variable(std::string val) noexcept;
		explicit Variable(double val) noexcept;
		Variable(Variable&&) = default;
		~Variable() override = default;
		auto eval(std::unordered_map<std::string, double> const& variableValues) const noexcept -> double;

		friend auto operator<<(std::ostream& os, Variable const& var) noexcept -> std::ostream&;
	private:
		// only set if the variable is actually a constant
		std::optional<double> m_numericVal;
	};

	auto operator<<(std::ostream& os, Token::Type type) noexcept -> std::ostream&;
	auto operator<<(std::ostream& os, Token const& token) noexcept -> std::ostream&;
	auto operator<<(std::ostream& os, TokenList const& tokens) noexcept -> std::ostream&;
	auto tokenize(std::string const& infixExpression) noexcept -> std::optional<TokenList>;
	auto toPostFix(std::vector<std::shared_ptr<Token>> const& tokens) noexcept -> std::optional<TokenList>;
	auto evalExpression(std::string const& expression, std::unordered_map<std::string, double> const& values) noexcept -> std::optional<double>;
	auto evalExpression(TokenList const& postFixTokens, std::unordered_map<std::string, double> const& values) noexcept -> std::optional<double>;
}