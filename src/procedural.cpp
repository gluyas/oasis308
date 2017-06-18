#include <cmath>
#include <string>
#include <stdexcept>
#include <vector>

#include "cgra_math.hpp"
#include "procedural.hpp"
#include "opengl.hpp"
#include "terrain.hpp"
#include "perlin.hpp"

using namespace std;
using namespace cgra;

#define CHUNK_RADIUS 15 
#define DRAW_DISTANCE 4

int renderDis = 10;
int seed = 1024;
float duneHeight = 5;
float addedDuneHeight = 0;
float mountainHeight = 15;
float addedMountainHeight = 10;
float frequency = 30;
float biomeSize = 200;
float waterHeight = -5;
float grassHeight = -2;
float oasisDiamiter = 33;
float oasisDepth = 5;
int biomeTransRoughness = 7; //the higher this number is the more instant the transition is
Terrain t(waterHeight);
Perlin p(seed);



Procedural::Procedural(GLuint *t, GLuint *tN, GLuint shader, Geometry *tree) {
	textures = t;
	texturesNormal = tN;
	g_normalMapShaderPCF = shader;
	WS = 0;
	AD = 0;
	QE = 0;
	trees = tree;
	CreateMap();
}


void Procedural::setTexture(float height) {
	if (height < grassHeight) {
		//grass
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		//dont use normal map
		glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 0);
	}
	else {
		if (height > 6) {

			if (height > 8) {
				//rock1
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[6]);
				//dont use normal map
				glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 0);
			}
			else {
				//rock2
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[7]);
				//dont use normal map
				glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 0);
			}

		}
		else {
			//sand
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[2]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texturesNormal[1]);
			// use normal map
			glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 1);
		}
	}
}

void Procedural::setNormal(vec3 a, vec3 b, vec3 c) {
	vec3 u;
	vec3 v;
	vec3 n;
	u = b - a;
	v = c - a;
	n = cross(u, v);
	glNormal3f(n.x, n.y, n.z);
}

///////////////////////////////////
///////////////////////////
//not used now left here for reference
void Procedural::drawQuad(vec3 x1, vec3 x2, vec3 x3, vec3 x4) {
	setTexture((x1.y + x2.y + x3.y) / 3);
	glBegin(GL_TRIANGLES);
	setNormal(x1, x2, x3);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x1.x, x1.y, x1.z);
	setNormal(x1, x2, x3);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x2.x, x2.y, x2.z);
	setNormal(x1, x2, x3);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x3.x, x3.y, x3.z);
	glTexCoord2f(1.0, 0.0);
	glEnd();
	setTexture((x3.y + x4.y + x1.y) / 3);
	glBegin(GL_TRIANGLES);
	setNormal(x3, x4, x1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x3.x, x3.y, x3.z);
	setNormal(x1, x2, x3);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x4.x, x4.y, x4.z);
	setNormal(x1, x2, x3);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x1.x, x1.y, x1.z);
	glTexCoord2f(0.0, 1.0);
	glEnd();
}

void Procedural::createDisplayList() {
	if (m_displayList) glDeleteLists(m_displayList, 1);

	// Create a new list
	m_displayList = glGenLists(1);
	glNewList(m_displayList, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	//glColor3f(1.0f, 0.0f, 0.0f);
	for (quad q : m_quads) {

		setTexture((q.p1.y + q.p2.y + q.p3.y) / 3);
		glBegin(GL_TRIANGLES);
		setNormal(q.p1, q.p2, q.p3);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(q.p1.x, q.p1.y, q.p1.z);
		setNormal(q.p1, q.p2, q.p3);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(q.p2.x, q.p2.y, q.p2.z);
		setNormal(q.p1, q.p2, q.p3);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(q.p3.x, q.p3.y, q.p3.z);
		glTexCoord2f(1.0, 0.0);
		glEnd();
		setTexture((q.p3.y + q.p4.y + q.p1.y) / 3);
		glBegin(GL_TRIANGLES);
		setNormal(q.p3, q.p4, q.p1);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(q.p3.x, q.p3.y, q.p3.z);
		setNormal(q.p1, q.p2, q.p3);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(q.p4.x, q.p4.y, q.p4.z);
		setNormal(q.p1, q.p2, q.p3);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(q.p1.x, q.p1.y, q.p1.z);
		glTexCoord2f(0.0, 1.0);
                glEnd();

	}

	glEnd();
	glEndList();
}

float Procedural::generateMountainHight(float x, float z) {
	float h = 0;
	h = p.noise(x / frequency, z / frequency) * mountainHeight;
	h = h + p.noise(x / frequency * 2, z / frequency * 2) * mountainHeight / 2;
	h = h + p.noise(x / frequency * 4, z / frequency * 4) * mountainHeight / 4;
	h = h + p.noise(x / frequency * 8, z / frequency * 8) * mountainHeight / 8;
	h = h + p.noise(x / frequency * 16, z / frequency * 16) * mountainHeight / 16;
	return h + addedMountainHeight;
}

float Procedural::generateHeight(float x, float z) {
	float h = 0;
	h = p.noise(x / frequency, z / frequency);
	h = h*duneHeight;
	h = h + addedDuneHeight;
	float dis = sqrt(x * x + z * z);
	if (dis < oasisDiamiter) {
		float smoothness = 1 - (oasisDiamiter - dis) / oasisDiamiter;
		h = h*smoothness;
		h = h - cos((dis / oasisDiamiter)*3.14159265) * oasisDepth - oasisDepth;
	}
	float newH = generateMountainHight(x, z);
	float biomeSeed = seed;
	biomeSeed = biomeSeed / 1.2345;
	float biome = p.noise(x / biomeSize + biomeSeed, z / biomeSize - biomeSeed);
	for (int i = 0; i < biomeTransRoughness; i++) {
		biome = sin(biome * 3.14159265359 / 2);
	}
	biome = (biome + 1) / 2;
	h = h * (1 - biome) + newH*biome;
	return h;
}

void Procedural::generateObjects(vec3 location) {
	//grass for now
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[5]);
	//dont use normal map
	glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 0);
	float genNum = abs(p.simpleNoise(location.x, location.z));
	if (genNum < 0.01 && location.y < grassHeight && location.y > waterHeight) {
		glPushMatrix();
		glTranslatef(location.x, location.y, location.z);
		glScalef(0.001f, 0.001f, 0.001f);
		//glCallList(m_displayListPoly[1]);
		trees->renderGeometry(vec3(0.0));
		glPopMatrix();
	}
}

void Procedural::CreateChunk(int xPoint, int zPoint) {

	//textures
	//water
	
	m_displayListWater.push_back(t.makeWater(xPoint, zPoint, CHUNK_RADIUS, CHUNK_RADIUS));

	//sand
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	//use normal map
	glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 1);


	//////////////////////
	float heights[(CHUNK_RADIUS + 1)*(CHUNK_RADIUS + 1)];
	vec3 normals[(CHUNK_RADIUS + 1)*(CHUNK_RADIUS + 1)];
	int numx = 0;
	for (int x = xPoint * CHUNK_RADIUS; x < xPoint * CHUNK_RADIUS + (CHUNK_RADIUS + 1); x++) {
		int numz = 0;
		for (int z = zPoint * CHUNK_RADIUS; z < zPoint * CHUNK_RADIUS + (CHUNK_RADIUS + 1); z++) {
			heights[numx * (CHUNK_RADIUS + 1) + numz] = generateHeight(x, z);
			numz++;
		}
		numx++;
	}
	numx = 0;
	for (int x = xPoint * CHUNK_RADIUS; x < xPoint * CHUNK_RADIUS + (CHUNK_RADIUS + 1) - 1; x++) {
		int numz = 0;
		for (int z = zPoint * CHUNK_RADIUS; z < zPoint * CHUNK_RADIUS + (CHUNK_RADIUS + 1) - 1; z++) {
			vec3 a(x, heights[numx * (CHUNK_RADIUS + 1) + numz], z);
			vec3 b(x, heights[numx * (CHUNK_RADIUS + 1) + numz + 1], z + 1);
			vec3 c(x + 1, heights[(numx + 1) * (CHUNK_RADIUS + 1) + numz + 1], z + 1);
			vec3 d(x + 1, heights[(numx + 1) * (CHUNK_RADIUS + 1) + numz], z);
			quad q;
			q.p1 = a;
			q.p2 = b;
			q.p3 = c;
			q.p4 = d;
			//drawQuad(a, b, c, d);
			m_quads.push_back(q);
			//generateObjects(a);
			numz++;
		}
		numx++;
	}
}

void Procedural::CreateMap() {
	m_quads.clear();
	m_displayListWater.clear();

	int addedX = -(WS / CHUNK_RADIUS);
	int addedY = (AD / CHUNK_RADIUS);
	for (int i = -DRAW_DISTANCE + addedX; i < DRAW_DISTANCE + addedX; i++) {
		for (int j = -DRAW_DISTANCE + addedY; j < DRAW_DISTANCE + addedY; j++) {
			CreateChunk(i, j);
		}
	}
	createDisplayList();
}

void Procedural::placeTrees() {
    //trees
    	for (quad q : m_quads) {
            generateObjects(q.p1);
        }
}

void Procedural::renderGeometry(cgra::vec3 g_rot) {
	glShadeModel(GL_SMOOTH);
	glCallList(m_displayList);
        placeTrees();
	//water
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[8]);
	//dont use normal map
	glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 0);
	for (GLuint water : m_displayListWater) {
		glCallList(water);
	}
}