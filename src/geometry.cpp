//---------------------------------------------------------------------------
//
// Copyright (c) 2016 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>

#include "cgra_math.hpp"
#include "geometry.hpp"
#include "opengl.hpp"

using namespace std;
using namespace cgra;

Geometry::Geometry(string filename, float rotx, float roty, float rotz, float tranx, float trany, float tranz, float scale, float uvScale) {
	m_filename = filename;
	uv = uvScale;
	readOBJ(filename);
	if (m_triangles.size() > 0) {
		createDisplayListPoly();
		createDisplayListWire();
	}
	r = 0;
	rx = rotx;
	ry = roty;
	rz = rotz;
	tx = tranx;
	ty = trany;
	tz = tranz;
	s = scale;
}


Geometry::~Geometry() { }


void Geometry::readOBJ(string filename) {

	// Make sure our geometry information is cleared
	m_points.clear();
	m_uvs.clear();
	m_normals.clear();
	m_triangles.clear();
	m_quadsG.clear();

	// Load dummy points because OBJ indexing starts at 1 not 0
	m_points.push_back(vec3(0,0,0));
	m_uvs.push_back(vec2(0,0));
	m_normals.push_back(vec3(0,0,1));


	ifstream objFile(filename);

	if(!objFile.is_open()) {
		cerr << "Error reading " << filename << endl;
		throw runtime_error("Error :: could not open file.");
	}

	cout << "Reading file " << filename << endl;

	// good() means that failbit, badbit and eofbit are all not set
	while(objFile.good()) {

		// Pull out line from file
		string line;
		std::getline(objFile, line);
		istringstream objLine(line);

		// Pull out mode from line
		string mode;
		objLine >> mode;

		// Reading like this means whitespace at the start of the line is fine
		// attempting to read from an empty string/line will set the failbit
		if (!objLine.fail()) {

			if (mode == "v") {
				vec3 v;
				objLine >> v.x >> v.y >> v.z;
				m_points.push_back(v);

			} else if(mode == "vn") {
				vec3 vn;
				objLine >> vn.x >> vn.y >> vn.z;
				m_normals.push_back(vn);

			} else if(mode == "vt") {
				vec2 vt;
				objLine >> vt.x >> vt.y;
				m_uvs.push_back(vt*uv);

			} else if(mode == "f") {

				vector<vertex> verts;
				while (objLine.good()){
					vertex v;

					//-------------------------------------------------------------
					// [Assignment 1] :
					// Modify the following to parse the bunny.obj. It has no uv
					// coordinates so each vertex for each face is in the format
					// v//vn instead of the usual v/vt/vn.
					//
					// Modify the following to parse the dragon.obj. It has no
					// normals or uv coordinates so the format for each vertex is
					// v instead of v/vt/vn or v//vn.
					//
					// Hint : Check if there is more than one uv or normal in
					// the uv or normal vector and then parse appropriately.
					//-------------------------------------------------------------

					// Assignment code (assumes you have all of v/vt/vn for each vertex)
					if (m_uvs.size() > 1){
						objLine >> v.p;		// Scan in position index
						objLine.ignore(1);	// Ignore the '/' character
						objLine >> v.t;		// Scan in uv (texture coord) index
						objLine.ignore(1);	// Ignore the '/' character
						objLine >> v.n;		// Scan in normal index
					}
					else if(m_normals.size() > 1){
						objLine >> v.p;		// Scan in position index
						objLine.ignore(2);	// Ignore the '//' character
						objLine >> v.n;		// Scan in normal index
					} else{
						objLine >> v.p;		// Scan in position index
					}
					if(v.p !=0 || v.t != 0 || v.n != 0) {
						verts.push_back(v);
					}
				}



				// IFF we have 3 verticies, construct a triangle
				if(verts.size() == 3){
					triangle tri;
					tri.v[0] = verts[0];
					tri.v[1] = verts[1];
					tri.v[2] = verts[2];
					m_triangles.push_back(tri);

				}

				if (verts.size() == 4) {
					quadG q;
					q.v[0] = verts[0];
					q.v[1] = verts[1];
					q.v[2] = verts[2];
					q.v[2] = verts[3];
					m_quadsG.push_back(q);
				}
			}
		}
	}

	cout << "Reading OBJ file is DONE." << endl;
	cout << m_points.size()-1 << " points" << endl;
	cout << m_uvs.size()-1 << " uv coords" << endl;
	cout << m_normals.size()-1 << " normals" << endl;
	cout << m_triangles.size() << " faces" << endl;
	cout << m_quadsG.size() << " quads" << endl;

	// If we didn't have any normals, create them
	if (m_normals.size() <= 1) createNormals();

	createTangents();
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to populate the normals for
// the model currently loaded. Compute per face normals
// first and get that working before moving onto calculating
// per vertex normals.
//-------------------------------------------------------------
void Geometry::createNormals() {
	std::vector<cgra::vec3> normals; //triangle face normals
	for (int i = 0; i < m_triangles.size(); i++) {
		vec3 v0 = m_points[m_triangles[i].v[0].p];
		vec3 v1 = m_points[m_triangles[i].v[1].p];
		vec3 v2 = m_points[m_triangles[i].v[2].p];
		
		vec3 deltaPos1 = v1 - v0;
		vec3 deltaPos2 = v2 - v0;

		vec3 normal(cross(deltaPos1, deltaPos2));
		normals.push_back(normal);

		m_triangles[i].v[0].n = m_triangles[i].v[0].p;	//set n as p because need normal for every vertex/point
		m_triangles[i].v[1].n = m_triangles[i].v[1].p;
		m_triangles[i].v[2].n = m_triangles[i].v[2].p;

		m_normals.push_back(vec3(0, 0, 0));	//create 0 normal for each point/vertex
		m_normals.push_back(vec3(0, 0, 0));
		m_normals.push_back(vec3(0, 0, 0));

	}
	

	std::vector<cgra::vec3> normalsQ; //triangle face normals
	for (int i = 0; i < m_quadsG.size(); i++) {
		vec3 v0 = m_points[m_quadsG[i].v[0].p];
		vec3 v1 = m_points[m_quadsG[i].v[1].p];
		vec3 v2 = m_points[m_quadsG[i].v[2].p];

		vec3 deltaPos1 = v1 - v0;
		vec3 deltaPos2 = v2 - v0;

		vec3 normal(cross(deltaPos1, deltaPos2));
		normalsQ.push_back(normal);

		m_quadsG[i].v[0].n = m_quadsG[i].v[0].p;	//set n as p because need normal for every vertex/point
		m_quadsG[i].v[1].n = m_quadsG[i].v[1].p;
		m_quadsG[i].v[2].n = m_quadsG[i].v[2].p;
		m_quadsG[i].v[3].n = m_quadsG[i].v[3].p;

		m_normals.push_back(vec3(0, 0, 0));	//create 0 normal for each point/vertex
		m_normals.push_back(vec3(0, 0, 0));
		m_normals.push_back(vec3(0, 0, 0));
		m_normals.push_back(vec3(0, 0, 0));

	}

	//tri
	for (int j = 0; j < m_triangles.size(); j++) {
		for (int k = 0; k < 3; k++) {
			m_normals[m_triangles[j].v[k].n] += normals[j]; //sum vertex normal with face normal
		}
	}
	for (int j = 0; j < m_triangles.size(); j++) {
		for (int k = 0; k < 3; k++) {
			normalize(m_normals[m_triangles[j].v[k].n]);
		}
	}

	//quads
	for (int j = 0; j < m_quadsG.size(); j++) {
		for (int k = 0; k < 4; k++) {
			m_normals[m_quadsG[j].v[k].n] += normalsQ[j]; //sum vertex normal with face normal
		}
	}
	for (int j = 0; j < m_quadsG.size(); j++) {
		for (int k = 0; k < 4; k++) {
			normalize(m_normals[m_quadsG[j].v[k].n]);
		}
	}
}

void Geometry::createTangents() {
	
	std::vector<cgra::vec3> tangents; //triangle face normals
	for (int i = 0; i < m_triangles.size(); i++) {
		
		vec3 v0 = m_points[m_triangles[i].v[0].p];
		vec3 v1 = m_points[m_triangles[i].v[1].p];
		vec3 v2 = m_points[m_triangles[i].v[2].p];

		vec2 uv0 = m_uvs[m_triangles[i].v[0].t];
		vec2 uv1 = m_uvs[m_triangles[i].v[1].t];
		vec2 uv2 = m_uvs[m_triangles[i].v[2].t];

		vec3 deltaPos1 = v1 - v0;
		vec3 deltaPos2 = v2 - v0;

		vec2 deltaUV1 = uv1 - uv0;
		vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		tangents.push_back(tangent);
		
		m_triangles[i].v[0].tang = m_triangles[i].v[0].p;	//set n as p because need normal for every vertex/point
		m_triangles[i].v[1].tang = m_triangles[i].v[1].p;
		m_triangles[i].v[2].tang = m_triangles[i].v[2].p;

		m_tangents.push_back(vec3(0, 0, 0));	//create 0 normal for each point/vertex
		m_tangents.push_back(vec3(0, 0, 0));
		m_tangents.push_back(vec3(0, 0, 0));
	}
	
	std::vector<cgra::vec3> tangentsQ; //triangle face normals
	for (int i = 0; i < m_quadsG.size(); i++) {

		vec3 v0 = m_points[m_quadsG[i].v[0].p];
		vec3 v1 = m_points[m_quadsG[i].v[1].p];
		vec3 v2 = m_points[m_quadsG[i].v[2].p];

		vec2 uv0 = m_uvs[m_quadsG[i].v[0].t];
		vec2 uv1 = m_uvs[m_quadsG[i].v[1].t];
		vec2 uv2 = m_uvs[m_quadsG[i].v[2].t];

		vec3 deltaPos1 = v1 - v0;
		vec3 deltaPos2 = v2 - v0;

		vec2 deltaUV1 = uv1 - uv0;
		vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		tangentsQ.push_back(tangent);

		m_quadsG[i].v[0].tang = m_quadsG[i].v[0].p;	//set n as p because need normal for every vertex/point
		m_quadsG[i].v[1].tang = m_quadsG[i].v[1].p;
		m_quadsG[i].v[2].tang = m_quadsG[i].v[2].p;
		m_quadsG[i].v[3].tang = m_quadsG[i].v[3].p;

		m_tangents.push_back(vec3(0, 0, 0));	//create 0 normal for each point/vertex
		m_tangents.push_back(vec3(0, 0, 0));
		m_tangents.push_back(vec3(0, 0, 0));
		m_tangents.push_back(vec3(0, 0, 0));
	}
	for (int j = 0; j < m_triangles.size(); j++) {
		for (int k = 0; k < 3; k++) {
			m_tangents[m_triangles[j].v[k].tang] += tangents[j]; //sum vertex normal with face normal
		}
	}
	for (int j = 0; j < m_quadsG.size(); j++) {
		for (int k = 0; k < 4; k++) {
			m_tangents[m_quadsG[j].v[k].tang] += tangentsQ[j]; //sum vertex normal with face normal
		}
	}
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to create display list
// of the obj file to show it as polygon model
//-------------------------------------------------------------
void Geometry::createDisplayListPoly() {
	// Delete old list if there is one
	if (m_displayListPoly) glDeleteLists(m_displayListPoly, 1);

	// Create a new list
	cout << "Creating Poly Geometry" << endl;
	m_displayListPoly = glGenLists(1);
	glNewList(m_displayListPoly, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	for (triangle t : m_triangles) {

		//v0
		glNormal3f(m_normals[t.v[0].n][0], m_normals[t.v[0].n][1], m_normals[t.v[0].n][2]);
		glTexCoord2f(m_uvs[t.v[0].t][0], m_uvs[t.v[0].t][1]);
		glColor3f(m_tangents[t.v[0].tang][0], m_tangents[t.v[0].tang][1], m_tangents[t.v[0].tang][2]);
		glVertex3f(m_points[t.v[0].p][0], m_points[t.v[0].p][1], m_points[t.v[0].p][2]);


		//v1
		glNormal3f(m_normals[t.v[1].n][0], m_normals[t.v[1].n][1], m_normals[t.v[1].n][2]);
		glTexCoord2f(m_uvs[t.v[1].t][0], m_uvs[t.v[1].t][1]);
		glColor3f(m_tangents[t.v[1].tang][0], m_tangents[t.v[1].tang][1], m_tangents[t.v[1].tang][2]);
		glVertex3f(m_points[t.v[1].p][0], m_points[t.v[1].p][1], m_points[t.v[1].p][2]);


		//v2
		glNormal3f(m_normals[t.v[2].n][0], m_normals[t.v[2].n][1], m_normals[t.v[2].n][2]);
		glTexCoord2f(m_uvs[t.v[2].t][0], m_uvs[t.v[2].t][1]);
		glColor3f(m_tangents[t.v[2].tang][0], m_tangents[t.v[2].tang][1], m_tangents[t.v[2].tang][2]);
		glVertex3f(m_points[t.v[2].p][0], m_points[t.v[2].p][1], m_points[t.v[2].p][2]);

	}
	glBegin(GL_QUADS);
	for (quadG q : m_quadsG) {
		//v0
		glNormal3f(m_normals[q.v[0].n][0], m_normals[q.v[0].n][1], m_normals[q.v[0].n][2]);
		glTexCoord2f(m_uvs[q.v[0].t][0], m_uvs[q.v[0].t][1]);
		glColor3f(m_tangents[q.v[0].tang][0], m_tangents[q.v[0].tang][1], m_tangents[q.v[0].tang][2]);
		glVertex3f(m_points[q.v[0].p][0], m_points[q.v[0].p][1], m_points[q.v[0].p][2]);


		//v1
		glNormal3f(m_normals[q.v[1].n][0], m_normals[q.v[1].n][1], m_normals[q.v[1].n][2]);
		glTexCoord2f(m_uvs[q.v[1].t][0], m_uvs[q.v[1].t][1]);
		glColor3f(m_tangents[q.v[1].tang][0], m_tangents[q.v[1].tang][1], m_tangents[q.v[1].tang][2]);
		glVertex3f(m_points[q.v[1].p][0], m_points[q.v[1].p][1], m_points[q.v[1].p][2]);


		//v2
		glNormal3f(m_normals[q.v[2].n][0], m_normals[q.v[2].n][1], m_normals[q.v[2].n][2]);
		glTexCoord2f(m_uvs[q.v[2].t][0], m_uvs[q.v[2].t][1]);
		glColor3f(m_tangents[q.v[2].tang][0], m_tangents[q.v[2].tang][1], m_tangents[q.v[2].tang][2]);
		glVertex3f(m_points[q.v[2].p][0], m_points[q.v[2].p][1], m_points[q.v[2].p][2]);
		
		//v3
		glNormal3f(m_normals[q.v[3].n][0], m_normals[q.v[3].n][1], m_normals[q.v[3].n][2]);
		glTexCoord2f(m_uvs[q.v[3].t][0], m_uvs[q.v[3].t][1]);
		glColor3f(m_tangents[q.v[3].tang][0], m_tangents[q.v[3].tang][1], m_tangents[q.v[3].tang][2]);
		glVertex3f(m_points[q.v[3].p][0], m_points[q.v[3].p][1], m_points[q.v[3].p][2]);
	}
	glEnd();
	glEndList();
	cout << "Finished creating Poly Geometry" << endl;
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to create display list
// of the obj file to show it as wireframe model
//-------------------------------------------------------------
void Geometry::createDisplayListWire() {
	// Delete old list if there is one
	if (m_displayListWire) glDeleteLists(m_displayListWire, 1);

	// Create a new list
	cout << "Creating Wire Geometry" << endl;
	m_displayListWire = glGenLists(1);
	glNewList(m_displayListWire, GL_COMPILE);
	glBegin(GL_LINES);
	for (triangle t : m_triangles) {
		//join v0 to v1
		//v0
		glNormal3f(m_normals[t.v[0].n][0], m_normals[t.v[0].n][1], m_normals[t.v[0].n][2]);
		glTexCoord2f(m_uvs[t.v[0].t][0], m_uvs[t.v[0].t][1]);
		glVertex3f(m_points[t.v[0].p][0], m_points[t.v[0].p][1], m_points[t.v[0].p][2]);
		//v1
		glNormal3f(m_normals[t.v[1].n][0], m_normals[t.v[1].n][1], m_normals[t.v[1].n][2]);
		glTexCoord2f(m_uvs[t.v[1].t][0], m_uvs[t.v[1].t][1]);
		glVertex3f(m_points[t.v[1].p][0], m_points[t.v[1].p][1], m_points[t.v[1].p][2]);
		//join v1 to v2
		//v1
		glNormal3f(m_normals[t.v[1].n][0], m_normals[t.v[1].n][1], m_normals[t.v[1].n][2]);
		glTexCoord2f(m_uvs[t.v[1].t][0], m_uvs[t.v[1].t][1]);
		glVertex3f(m_points[t.v[1].p][0], m_points[t.v[1].p][1], m_points[t.v[1].p][2]);
		//v2
		glNormal3f(m_normals[t.v[2].n][0], m_normals[t.v[2].n][1], m_normals[t.v[2].n][2]);
		glTexCoord2f(m_uvs[t.v[2].t][0], m_uvs[t.v[2].t][1]);
		glVertex3f(m_points[t.v[2].p][0], m_points[t.v[2].p][1], m_points[t.v[2].p][2]);
		//join v2  to v0
		//v2
		glNormal3f(m_normals[t.v[2].n][0], m_normals[t.v[2].n][1], m_normals[t.v[2].n][2]);
		glTexCoord2f(m_uvs[t.v[2].t][0], m_uvs[t.v[2].t][1]);
		glVertex3f(m_points[t.v[2].p][0], m_points[t.v[2].p][1], m_points[t.v[2].p][2]);
		//v0
		glNormal3f(m_normals[t.v[0].n][0], m_normals[t.v[0].n][1], m_normals[t.v[0].n][2]);
		glTexCoord2f(m_uvs[t.v[0].t][0], m_uvs[t.v[0].t][1]);
		glVertex3f(m_points[t.v[0].p][0], m_points[t.v[0].p][1], m_points[t.v[0].p][2]);
	}
	glEnd();
	glEndList();
	cout << "Finished creating Wire Geometry" << endl;
}


void Geometry::renderGeometry(vec3 g_rot) {
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glRotatef(g_rot[0], 1.0f, 0.0f, 0.0f); //20
	glRotatef(g_rot[1], 0.0f, 1.0f, 0.0f); //-30
	glRotatef(g_rot[2], 0.0f, 0.0f, 1.0f);
	glTranslatef(tx, ty, tz);
	glRotatef(r, 0.0f, 1.0f, 0.0f);
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glScalef(s, s, s);
	if (m_wireFrameOn) {

		//-------------------------------------------------------------
		// [Assignment 1] :
		// When moving on to displaying your obj, comment out the
		// wire_cow function & uncomment the glCallList function
		//-------------------------------------------------------------

		glShadeModel(GL_SMOOTH);
		//wire_cow();
		glCallList(m_displayListWire);

	} else {

		//-------------------------------------------------------------
		// [Assignment 1] :
		// When moving on to displaying your obj, comment out the
		// cow function & uncomment the glCallList function
		//-------------------------------------------------------------

		glShadeModel(GL_SMOOTH);
		//cow();
		glCallList(m_displayListPoly);

	}
	glPopMatrix();
}


void Geometry::toggleWireFrame() {
	m_wireFrameOn = !m_wireFrameOn;
}
