#include "KeyframeCurveLenFunction.h"
#include "../Utils.h"

#include <sstream>
#include <utility>

using kfclf = KeyframeCurveLenFunction;

static tk::spline::spline_type trans(kfclf::SplineType type) {
	switch (type) {
	case kfclf::SplineType::Linear: 
		return tk::spline::linear;
	case kfclf::SplineType::C1: 
		return tk::spline::cspline_hermite;
	case kfclf::SplineType::C2: 
		return tk::spline::cspline;
	default: return tk::spline::linear;
	}
}
static kfclf::SplineType trans(tk::spline::spline_type type) {
	switch (type) {
	case tk::spline::spline_type::linear:
		return kfclf::SplineType::Linear;
	case tk::spline::cspline_hermite:
		return kfclf::SplineType::C1;
	case tk::spline::cspline:
		return kfclf::SplineType::C2;
	default: return kfclf::SplineType::Linear;
	}
}

kfclf::KeyframeCurveLenFunction(ControlPointArray controlPoints, SplineType type, double scale)
	: m_controlPoints{std::move(controlPoints)}, m_splineType{trans(type)}, m_yScale(scale) {
	if (m_controlPoints.size() >= 3) {
		m_spline.set_points(m_controlPoints.getX(), m_controlPoints.getY(), m_splineType);
	}
}

double kfclf::operator()(double s) const {
	return m_yScale * std::max(0.0, m_spline(s));
}

bool kfclf::operator==(UserCurveLenFunction const& other) const {
	if (auto const ptr = dynamic_cast<KeyframeCurveLenFunction const*>(&other); !ptr) {
		return false;
	}
	else {
		return this->operator==(*ptr);
	}
}

bool kfclf::operator==(kfclf const& other) const {
	return m_controlPoints == other.m_controlPoints && 
		m_splineType == other.m_splineType;
}

bool kfclf::valid() const {
	return m_controlPoints.size() >= 3;
}

static constexpr char k_sep = ' ';
std::string kfclf::serialize() const {
	std::ostringstream ss;
	ss.exceptions(std::ios::failbit);

	ss << STR(KeyframeCurveLenFunction) << k_sep;
	ss << m_controlPoints.size() << k_sep;
	for (auto [x, y] : m_controlPoints) {
		ss << x << k_sep << y << k_sep;
	}
	ss << m_splineType << k_sep << m_yScale;

	return ss.str();
}

void kfclf::setType(SplineType type) {
	m_splineType = trans(type);
	if (valid()) {
		m_spline.set_points(m_controlPoints.getX(), m_controlPoints.getY(), m_splineType);
	}
}

void kfclf::addControlPoint(double x, double y) {
	m_controlPoints.add(x, y);
	if (valid()) {
		m_spline.set_points(m_controlPoints.getX(), m_controlPoints.getY(), m_splineType);
	}
}

void kfclf::insert(ControlPointArray::ConstIterator cit, double x, double y) {
	m_controlPoints.insert(m_controlPoints.begin() + (cit - m_controlPoints.cbegin()), x, y);
	if (valid()) {
		m_spline.set_points(m_controlPoints.getX(), m_controlPoints.getY(), m_splineType);
	}
}

void kfclf::setControlPoint(ControlPointArray::ConstIterator cit, double x, double y) {
	auto it = m_controlPoints.begin() + (cit - m_controlPoints.cbegin());
	it->first = x;
	it->second = y;
	if (valid()) {
		m_spline.set_points(m_controlPoints.getX(), m_controlPoints.getY(), m_splineType);
	}
}

void kfclf::setControlPoints(ControlPointArray::ConstIterator first, ControlPointArray::ConstIterator last) {
	m_controlPoints.clear();
	for (; first != last; ++first) {
		auto [x, y] = *first;
		m_controlPoints.add(x, y);
	}
	if(valid()) {
		m_spline.set_points(m_controlPoints.getX(), m_controlPoints.getY(), m_splineType);
	}
}

kfclf::SplineType kfclf::getType() const {
	return trans(m_splineType);
}

void kfclf::deserialize(std::istringstream& ss) {
	ss.exceptions(std::ios::failbit);

	size_t sz;
	ControlPointArray arr;
	int type;
	double yScale;

	ss >> sz;
	for (size_t i = 0; i < sz; ++i) {
		double x, y;
		ss >> x >> y;
		arr.add(x, y);
	}
	ss >> type >> yScale;

	m_controlPoints = std::move(arr);
	m_splineType = static_cast<tk::spline::spline_type>(type);
	if (valid()) {
		m_spline.set_points(m_controlPoints.getX(), m_controlPoints.getY(), m_splineType);
	}
	m_yScale = yScale;
}
