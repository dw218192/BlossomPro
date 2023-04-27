#pragma once
#include <maya/MDataBlock.h>
#include <maya/MString.h>

#include "UserCurveLenFunction.h"
#include "../Utils.h"

/*
 * simple MObject wrapper to make it easier to work with UserCurveLenFunction
*/
struct UserCurveLenFuncAttribute {
	UserCurveLenFuncAttribute() = default;
	UserCurveLenFuncAttribute(MObject const& obj)
		: m_obj{obj} { }
	operator MObject() = delete;
	operator MObject const&() const {
		return m_obj;
	}
	operator MObject&() {
		return m_obj;
	}
	MObject const* operator&() const {
		return &m_obj;
	}
	MObject* operator&() {
		return &m_obj;
	}
	MStatus inputValue(std::shared_ptr<UserCurveLenFunction>& inOutVal, MDataBlock& data) const noexcept {
		MStatus status;
		MString const serializedCurve = data.inputValue(m_obj, &status).asString();
		CHECK_RET(status);

		auto res = UserCurveLenFunction::deserialize(serializedCurve.asChar());
		CHECK_RES(res);

		if (!inOutVal || *res.value() != *inOutVal) {
			inOutVal = res.value();
		}
		if (!inOutVal || !inOutVal->valid()) {
			return MStatus::kInvalidParameter;
		}
		return status;
	}
private:
	MObject m_obj;
};