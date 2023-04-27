#include "UserCurveLenFunction.h"
#include "ExpressionCurveLenFunction.h"
#include "KeyframeCurveLenFunction.h"
#include <sstream>

Result<std::shared_ptr<UserCurveLenFunction>> UserCurveLenFunction::deserialize(char const* raw) noexcept {
	std::string const str{ raw };
	std::istringstream ss{ str };

	std::string typeStr;
	ss >> typeStr;

	try {
		if (typeStr == STR(ExpressionCurveLenFunction)) {
			struct Type : public ExpressionCurveLenFunction {};

			auto ret = std::dynamic_pointer_cast<UserCurveLenFunction>(std::make_shared<Type>());
			ret->deserialize(ss);
			return ret;
		}
		else if (typeStr == STR(KeyframeCurveLenFunction)) {
			struct Type : public KeyframeCurveLenFunction {};

			auto ret = std::dynamic_pointer_cast<UserCurveLenFunction>(std::make_shared<Type>());
			ret->deserialize(ss);
			return ret;
		}
		else {
			return MStatus{ MStatus::kInvalidParameter };
		}
	} catch (...) {
		return MStatus{ MStatus::kInvalidParameter };
	}
}

Result<MString> UserCurveLenFunction::serialize(UserCurveLenFunction const& func) noexcept {
	try {
		std::string const ret = func.serialize();
		return MString{ ret.c_str() };
	} catch (...) {
		return MStatus{ MStatus::kInvalidParameter };
	}
}