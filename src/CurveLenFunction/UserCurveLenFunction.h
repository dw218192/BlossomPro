#pragma once
#include <maya/MStatus.h>
#include <string>

#include "Utils.h"

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
	static Result<std::shared_ptr<UserCurveLenFunction>> deserialize(char const* raw) noexcept;
	static Result<MString> serialize(UserCurveLenFunction const& func) noexcept;

	UserCurveLenFunction() = default;
	virtual ~UserCurveLenFunction() = default;

	/**
	 * \brief evaluates the user-defined function
	 * \param s curve length
	 * \return the function output
	 */
	virtual double operator()(double s) const = 0;
	virtual bool operator==(UserCurveLenFunction const&) const = 0;
	bool operator!=(UserCurveLenFunction const& other) const {
		return !(*this == other);
	}

	virtual bool valid() const = 0;

protected:
	virtual std::string serialize() const = 0;
	virtual void deserialize(std::istringstream&) = 0;
};