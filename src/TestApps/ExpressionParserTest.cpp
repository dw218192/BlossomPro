#include <cassert>
#include <iostream>
#include "ExpressionParser.h"

using namespace ExpressionParser;
int main()
{
    auto doParsing = [](std::string const& str) {
        auto tokens = tokenize(str);
        assert(tokens);
        auto postFix = toPostFix(*tokens);
        assert(postFix);
        std::cout << *postFix << std::endl;
    };
    auto doEval = [](std::string const& str) {
        auto res = evalExpression(str, {});
        assert(res);
        std::cout << *res << std::endl;
    };
    auto doEvalUnknown = [](std::string const& str, std::unordered_map<std::string, double> const& mp) {
        auto res = evalExpression(str, mp);
        assert(res);
        std::cout << *res << std::endl;
    };
    try {
        // simple tests
        doParsing("2 + 4");
        // long variable name
        doParsing("happy * 2 - 1");
        // precedence
        doParsing("sad + happy * cold");
        // bracket
        doParsing("(sad + 2 * dead) * cold");
        // minus sign
        doParsing("-2 + (sad + 2 * 5) * c");
        // -2 sad 2 5 * + c * +
        // precedence2
        doParsing("3 + 4 * 2 / (1 - 5) ^ 2 ^ 3"); // 3 4 2 * 1 5 - 2 3 ^ ^ / +
        doEval("0.5");
        doEval("3 * 4");
        doEval("9 * 99 * 1");
    	doEval("3 + 4 * 2 / (1 - 5) ^ 2 ^ 3"); // ~ 3.000
        doEvalUnknown("a + b * 2 / (1-5)^2^3", {
            {"a", 3},
            {"b", 4}
        });
        doEvalUnknown("2 * 2 * (2 * x + 5) ^ 2", {
		    {"x", 3}
        });

    } catch(std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
	return 0;
}