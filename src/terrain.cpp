/*
* File:   Terrain.cpp
* Author: foleylliam
*
* Created on 16 June 2017, 6:27 AM
*/

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "terrain.hpp"
#include "cgra_math.hpp"
#include "opengl.hpp"

using namespace std;
using namespace cgra;


Terrain::Terrain(float waterHeight) {
	this->waterHeight = waterHeight;
}

Terrain::Terrain(const Terrain& orig) {
}

Terrain::~Terrain() {

}

GLuint Terrain::makeWater(int xPoint, int zPoint, int width, int length) {
	GLuint m_displayList = 0;

	if (m_displayList) glDeleteLists(m_displayList, 1);

	// Create a new list
	m_displayList = glGenLists(1);
	glNewList(m_displayList, GL_COMPILE);

	point w1, w2, w3, w4;
	glBegin(GL_QUADS);
	glNormal3f(0, 1.0, 0.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(xPoint * width, waterHeight, zPoint * length);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(xPoint * width, waterHeight, zPoint * length + length);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(xPoint * width + width, waterHeight, zPoint * length + length);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(xPoint * width + width, waterHeight, zPoint * length);
	glEnd();
	glEndList();

	return m_displayList;
}



