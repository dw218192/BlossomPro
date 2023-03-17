#include "BsplineSurface.h"

#include <iostream>;
#include <vector>;
#include <stdexcept>;

void BsplineSurface::makeKnots(int size, int degree, std::vector<float>& knotVector)
{ // make uniform internal knots
	// n + 1 > p
	// n + 1 : number of control points in u-direction
	// r == n + 1 + p (last index)
	// p: degree of u-direction

	// m + 1 > q
	// m + 1 : number of control points in v-direction
	// s == m + 1 + q (last index)
	// q: degree of v-direction

	if (size <= degree)
	{
		std::cout << "makeKnots() : not enough control points to make knot vector. add more points.\n";
		return;
	}

	knotVector.clear();
	knotVector.resize(size + degree + 1); // vector size: r + 1 or s + 1
	for (int i{}; i <= degree; ++i)
		knotVector[i] = 0.0;

	int denominator{ size - degree};

	for (int i{ degree + 1 }; i < size; ++i)
	{
		knotVector[i] = static_cast<float>(i - degree) / denominator;
	}

	int m{ size + degree};
	for (int i{ size }; i <= m; ++i)
		knotVector[i] = 1.0;
}

int BsplineSurface::findKnotSpan(int size, int degree, float x, const std::vector<float>& knotVector) const
{
	if (knotVector.empty())
	{
		std::cerr << "findKnotSpan() knot vector is empty\n";
		throw std::runtime_error("findKnotSpan() knot vector is empty");
	}

	if (x < knotVector.front() || x > knotVector.back())
	{
		std::cerr << "findKnotSpan() x is outside of range\n";
		throw std::runtime_error("findKnotSpan() x is outside of range");
	}

	//algorithm A2.1 FindSpan pp68
	if (x == knotVector.back()) // special case
		return size - 1;

	int low{ degree };
	int high{ size };
	int mid{ (low + high) / 2 };

	while (x < knotVector[mid] || x >= knotVector[mid + 1])
	{
		if (x < knotVector[mid])
			high = mid;
		else
			low = mid;

		mid = (low + high) / 2;
	}

	return mid;
}

void BsplineSurface::basisFuns(int span, float x, int degree, std::vector<float>& knotVector, std::vector<float>& basis)
{
	// Algorithm A2.2 pp70
	// Compute the nonvanishing basis functions
	float temp, saved;
	std::vector<float> left(degree + 1);
	std::vector<float> right(degree + 1);

	basis[0] = 1.0;

	for (int j{ 1 }; j <= degree; ++j)
	{
		left[j] = x - knotVector[span + 1 - j];
		right[j] = knotVector[span + j] - x;
		saved = 0.0;
		for (int r{}; r < j; ++r)
		{
			temp = basis[r] / (right[r + 1] + left[j - r]);
			basis[r] = saved + right[r + 1] * temp;
			saved = left[j - r] * temp;
		}
		basis[j] = saved;
	}
}

// algorithm A3.5 pp. 103
void BsplineSurface::surfacePoint(float u, float v, glm::vec3& S)
{
	// add num check

	int uspan{ findKnotSpan(static_cast<int>(controlPoints.size()), p_degree, u, uKnots) };
	basisFuns(uspan, u, p_degree, uKnots, uBasis);
	int vspan{ findKnotSpan(static_cast<int>(controlPoints.front().size()), q_degree, v, vKnots) };
	basisFuns(vspan, v, q_degree, vKnots, vBasis);

	std::vector<glm::vec3> temp(q_degree + 1);

	for (int L{}; L <= q_degree; ++L)
	{
		temp[L].x = temp[L].y = temp[L].z = 0.0;
		for (int k{}; k <= p_degree; ++k)
		{
			temp[L].x += uBasis[k] * controlPoints[uspan - p_degree + k][vspan - q_degree + L].x;
			temp[L].y += uBasis[k] * controlPoints[uspan - p_degree + k][vspan - q_degree + L].y;
			temp[L].z += uBasis[k] * controlPoints[uspan - p_degree + k][vspan - q_degree + L].z;
		}
	}

	S.x = S.y = S.z = 0.0;

	for (int L{}; L <= q_degree; ++L)
	{
		S += vBasis[L] * temp[L];
	}
}

void BsplineSurface::addVector(const std::vector<glm::vec3>& vPt)
{
	controlPoints.push_back(vPt);
}

void BsplineSurface::makeKnots()
{
	makeKnots(static_cast<int>(controlPoints.size()), p_degree, uKnots);
	makeKnots(static_cast<int>(controlPoints.front().size()), q_degree, vKnots);
}
