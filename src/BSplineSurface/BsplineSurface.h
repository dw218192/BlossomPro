#pragma once

#include <glm/glm.hpp>
#include <vector>

class BsplineSurface
{
public:
	BsplineSurface(int p, int q) : p_degree{ p }, q_degree{ q } { uBasis.resize(p + 1); vBasis.resize(q + 1); }
	void surfacePoint(float u, float v, glm::vec3& S);
	void addVector(const std::vector<glm::vec3>& vPt);
	void makeKnots();
private:
	void basisFuns(int span, float x, int degree, std::vector<float>& knotVector, std::vector<float>& basis);
	void makeKnots(int size, int degree, std::vector<float>& knotVector);
	int findKnotSpan(int size, int degree, float x, const std::vector<float>& knotVector) const;

	int p_degree{}; // degree p
	int q_degree{}; // degree q
	std::vector<std::vector<glm::vec3>> controlPoints; // n + 1 by m + 1 matrix
	std::vector<float> uKnots; // r == n + p + 1, p + 1 duplicates at each end
	std::vector<float> vKnots; // q == m + q + 1, q + 1 duplicates at each end
	std::vector<float> uBasis;
	std::vector<float> vBasis;
};