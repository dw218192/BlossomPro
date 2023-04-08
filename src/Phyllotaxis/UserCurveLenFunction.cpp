#include "UserCurveLenFunction.h"
#include "ExpressionCurveLenFunction.h"
#include "KeyframeCurveLenFunction.h"

#include "../Utils.h"
#include <sstream>

std::shared_ptr<UserCurveLenFunction> UserCurveLenFunction::deserialize(char const* raw) {
	std::string const str{ raw };
	std::istringstream ss{ str };
	ss.exceptions(std::ios::failbit);

	std::string typeStr;
	ss >> typeStr;

	if(typeStr == STR(ExpressionCurveLenFunction)) {
		auto ret = std::make_shared<ExpressionCurveLenFunction>("");
		ret->deserialize(ss);
		return ret;
	} else if(typeStr == STR(KeyframeCurveLenFunction)) {
		auto ret = std::make_shared<KeyframeCurveLenFunction>(ControlPointArray{}, KeyframeCurveLenFunction::SplineType::Linear);
		ret->deserialize(ss);
		return ret;
	} else {
		throw std::runtime_error{ "unknown curve len function type " + typeStr };
	}
}