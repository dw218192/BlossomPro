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
	void inputValue(std::shared_ptr<UserCurveLenFunction>& outVal, MDataBlock& data, MStatus* status) const {
		MString const serializedCurve = data.inputValue(m_obj, status).asString();
		if(MFAIL(*status)) {
			return;
		}
		if (!outVal || outVal->serialize() != serializedCurve) {
			outVal = UserCurveLenFunction::deserialize(serializedCurve.asChar());
		}
		if (!outVal || !outVal->valid()) {
			*status = MStatus::kInvalidParameter;
		}
	}
private:
	MObject m_obj;
};