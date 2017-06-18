#include "perlin.hpp"
#include "cgra_math.hpp"
#include "opengl.hpp"

#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;
using namespace cgra;


Perlin::Perlin(int seed) {
	p = new int[256];
	Gx = new float[256];
	Gz = new float[256];

	for (int i = 0; i<256; ++i) {
		p[i] = i;
		Gx[i] = simpleNoise(i + seed, i);
		Gz[i] = simpleNoise(i, i + seed);
	}

	int j = 0;
	int swp = 0;
	for (int i = 0; i<256; i++) {
		int rnd = simpleNoise(i + seed, i + seed) * 2147483648;
		j = rnd & 255;
		swp = p[i];
		p[i] = p[j];
		p[j] = swp;
	}
}

Perlin::~Perlin()
{
	delete p;
	delete Gx;
	delete Gz;
}

//code inspired by https://stackoverflow.com/questions/16569660/2d-perlin-noise-in-c
float Perlin::simpleNoise(int x, int y) {
	int nNum;
	nNum = x + y * 57;
	nNum = (nNum << 13) ^ nNum;
	return (1.0 - ((nNum * ((nNum * nNum * 15731) + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

//this function and the constructor were heavily influenced by a 3d perlin noise class I found online but I cant seem to find it again for referencing
float Perlin::noise(float x, float z) {
	//Get square corners
	int fx = int(floorf(x));
	int f1 = fx + 1;
	int fz = int(floorf(z));
	int f2 = fz + 1;

	//Get point position inside the square
	float pfx = x - float(fx);
	float pf1 = pfx - 1.0f;
	float pfz = z - float(fz);
	float pf2 = pfz - 1.0f;

	//Dot product between gradient and pos vector
	int gIndex = p[(fx + p[(p[fz & 255]) & 255]) & 255];
	float d000 = Gx[gIndex] * pfx + Gz[gIndex] * pfz;
	gIndex = p[(f1 + p[(p[fz & 255]) & 255]) & 255];
	float d001 = Gx[gIndex] * pf1 + Gz[gIndex] * pfz;

	gIndex = p[(fx + p[(1 + p[fz & 255]) & 255]) & 255];
	float d010 = Gx[gIndex] * pfx + Gz[gIndex] * pfz;
	gIndex = p[(f1 + p[(1 + p[fz & 255]) & 255]) & 255];
	float d011 = Gx[gIndex] * pf1 + Gz[gIndex] * pfz;

	gIndex = p[(fx + p[(p[f2 & 255]) & 255]) & 255];
	float d100 = Gx[gIndex] * pfx + Gz[gIndex] * pf2;
	gIndex = p[(f1 + p[(p[f2 & 255]) & 255]) & 255];
	float d101 = Gx[gIndex] * pf1 + Gz[gIndex] * pf2;

	gIndex = p[(fx + p[(1 + p[f2 & 255]) & 255]) & 255];
	float d110 = Gx[gIndex] * pfx + Gz[gIndex] * pf2;
	gIndex = p[(f1 + p[(1 + p[f2 & 255]) & 255]) & 255];
	float d111 = Gx[gIndex] * pf1 + Gz[gIndex] * pf2;

	//Use polynomial interpolation to get final value
	float wx = ((6 * pfx - 15)*pfx + 10)*pfx*pfx*pfx;
	float wz = ((6 * pfz - 15)*pfz + 10)*pfz*pfz*pfz;

	float xa = d000 + wx*(d001 - d000);
	float xb = d010 + wx*(d011 - d010);
	float xc = d100 + wx*(d101 - d100);
	float xd = d110 + wx*(d111 - d110);
	float value = xa + wz*(xc - xa);

	return value;
}