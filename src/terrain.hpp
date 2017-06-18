#pragma once

#include <cmath>
#include <string>
#include <vector>

#include "cgra_math.hpp"
#include "opengl.hpp"

/*
* File:   Terrain.h
* Author: foleylliam
*
* Created on 16 June 2017, 6:27 AM
*/

#ifndef TERRAIN_H
#define	TERRAIN_H

class Terrain {
public:
	float waterHeight;
	Terrain(float waterHeight);
	Terrain(const Terrain& orig);
	virtual ~Terrain();
	GLuint makeWater(int xPoint, int zPoint, int width, int length);

private:

	struct point {
		int x = 0;
		float y = 0;
		int z = 0;
	};

};

#endif	/* TERRAIN_H */