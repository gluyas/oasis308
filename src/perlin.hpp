#pragma once

#include <cmath>
#include <string>
#include <vector>

#include "cgra_math.hpp"
#include "opengl.hpp"

#ifndef _PERLIN_H_
#define _PERLIN_H_


class Perlin {
public:
	Perlin(int seed);
	~Perlin();

	// Generates a Perlin (smoothed) noise value between -1 and 1, at the given 3D position.
	float noise(float x, float z);
	float simpleNoise(int x, int z);


private:
	int *p;
	float *Gx;
	float *Gz;
};

#endif