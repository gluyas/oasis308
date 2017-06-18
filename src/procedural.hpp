#pragma once


#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "cgra_math.hpp"
#include "opengl.hpp"
#include "geometry.hpp"

struct quad {
	cgra::vec3 p1;
	cgra::vec3 p2;
	cgra::vec3 p3;
	cgra::vec3 p4;
};

class Procedural {
private:

	GLuint *textures;
	GLuint *texturesNormal;
	GLuint g_normalMapShaderPCF;
	Geometry *trees;

	std::vector<quad> m_quads;

	void setTexture(float height);
	void setNormal(cgra::vec3 a, cgra::vec3 b, cgra::vec3 c);
	void drawQuad(cgra::vec3 x1, cgra::vec3 x2, cgra::vec3 x3, cgra::vec3 x4);
	float generateMountainHight(float x, float z);
	float generateHeight(float x, float z);
	void generateObjects(cgra::vec3 location);
	void CreateChunk(int xPoint, int zPoint);
	void createDisplayList();
        void placeTrees();

	GLuint m_displayList = 0;
	std::vector<GLuint> m_displayListWater;

public:

	float WS;
	float AD;
	float QE;


	Procedural(GLuint *textures, GLuint *texturesNormal, GLuint g_normalMapShaderPCF, Geometry *tree);
	~Procedural();

	void renderGeometry(cgra::vec3 g_rot);
	void CreateMap();
};