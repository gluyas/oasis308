#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "cgra_geometry.hpp"
#include "cgra_math.hpp"
#include "opengl.hpp"
#include "spotlight.hpp"

using namespace std;
using namespace cgra;

Spotlight::Spotlight() {

}

void Spotlight::renderSpotlight(vec3 t, vec3 d, float a, float s, vec3 r) {
	glEnable(GL_COLOR_MATERIAL);
	normalize(d); //normalize direction
	glPushMatrix();
	float position[] = { t[0], t[1], t[2], 1.0f };
	float direction[] = { d[0],d[1], d[2] };
	GLfloat exp = 2;
	GLfloat atten = 0.001;
	GLfloat ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };

	glLightfv(GL_LIGHT2, GL_POSITION, position);
	glRotatef(r[0], 1, 0, 0);
	glRotatef(r[1], 0, 1, 0);
	glRotatef(r[2], 0, 0, 1);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, direction);
	glLightfv(GL_LIGHT2, GL_SPOT_EXPONENT, &exp);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, a);
	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT2, GL_SPECULAR, white_light);
	glLightfv(GL_LIGHT2, GL_LINEAR_ATTENUATION, &atten);
	glEnable(GL_LIGHT2);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(t[0], t[1], t[2]);
	glRotatef(r[0], 1, 0, 0);
	glRotatef(r[1], 0, 1, 0);
	glRotatef(r[2], 0, 0, 1);

	glColor3f(1, 1, 1);
	//cgraSphere(s/2,10,10, false);

	float angle = dot(vec3(0, 0, 1), d);
	vec3 perp = cross(vec3(0, 0, 1), d);
	float theta = acos(angle);
	float thetaD = degrees(theta);
	glRotatef(thetaD, perp[0], perp[1], perp[2]);
	glTranslatef(0.0f, 0.0f, s*1.5);
	glRotatef(180, 1, 0, 0);
	//cgraCone(s*tan(radians(a)) , s, 10, 10, true);
	glRotatef(180, 1, 0, 0);
	//cgraCylinder(s/100, s/100, s*2, 10, 10, false);
	glTranslatef(0.0f, 0.0f, s*2);
	//cgraCone(s/4, s/2, 10, 10, false);
	// Clean up
	glPopMatrix();
	glDisable(GL_COLOR_MATERIAL);
}
