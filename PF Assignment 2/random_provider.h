#pragma once

#include <random>

using namespace std;

class random_provider
{
private:
	mt19937 mt;
	uniform_real_distribution<double> dist;
public:
	random_provider(unsigned int seed)
	{
		mt.seed(seed);
		dist = uniform_real_distribution<double>(0.0, 1.0);
	}

	inline double next() { return dist(mt); }

	inline void seed(unsigned int seed) { mt.seed(seed); }
};