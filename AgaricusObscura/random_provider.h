#pragma once

#include <random>

using namespace std;

// class to abstract RNG functionality for gameplay randomisation
class random_provider
{
private:
	// random state members
	mt19937 mt;
	uniform_real_distribution<double> dist;

public:
	// initialiser
	random_provider(unsigned int seed)
	{
		mt.seed(seed);
		dist = uniform_real_distribution<double>(0.0, 1.0);
	}

	// get the next double in the sequence
	inline double next() { return dist(mt); }

	// reset the seed of the PRNG source
	inline void seed(unsigned int seed) { mt.seed(seed); }
};