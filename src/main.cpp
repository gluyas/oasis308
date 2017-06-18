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
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>

#include "cgra_geometry.hpp"
#include "cgra_math.hpp"
#include "simple_image.hpp"
#include "simple_shader.hpp"
#include "opengl.hpp"
#include "geometry.hpp"
#include "spotlight.hpp"
#include "skybox.hpp"

//Liams code
#include "perlin.hpp"
#include "terrain.hpp"
#include "procedural.hpp"


using namespace std;
using namespace cgra;

// Window
//
GLFWwindow* g_window;


// Projection values
//
float g_fovy = 50.0;
float g_znear = 0.1;
float g_zfar = 1000.0;


//Shadow Map
float shadowMapSize = 4096;

float cameraPosition[] = { 5.0f,7.0f,5.0f };
float lightPosition[] = { -5.0f, 10.0f, 0.0f, 1.0f };

//Matrices
mat4 lightProjectionMatrix;
mat4 lightViewMatrix;
mat4 lightSpaceMatrix;
mat4 cameraViewMatrix;

GLfloat lvm[16];
GLfloat lpm[16];
GLfloat lsm[16];
GLfloat cvm[16];

//scene rotation
vec3 g_rot = { 0,0,0 };

//scene translation
vec3 g_trans = { 0,0,0 };

//light direction
float lightDirection[] = { -1.0f, 1.0f, 1.0f, 0.0f };

// Mouse controlled Camera values
//
bool g_leftMouseDown = false;
bool g_ctrlDown = false;
vec2 g_mousePosition;
float g_pitch = 0;
float g_yaw = 0;
float g_zoom = 1.0;

//Spotlight values
vec3 trans = { 5.0f,7.0f,5.0f };
vec3 dir = { -0.5f,-0.5f,-0.5f };
vec3 rot = { 0.0f,0.0f,0.0f };
float angle = 45.0f;
float scale = 1.0f;

//light source controllers
bool l1 = true;
bool l2 = true;
bool l3 = true;
bool l4 = true;
float dirRot = 0;

// Values and fields to showcase the use of shaders
// Remove when modifying main.cpp for Assignment 3
//
bool g_useShader = true;
GLuint *textures = new GLuint[9];
GLuint *texturesNormal = new GLuint[2];
GLuint *texturesSpecular = new GLuint[1];
GLuint shadowMapTexture = 0;

GLuint shadowMapFBO = 0;

GLuint g_testShader = 0;
GLuint g_skyboxShader = 0;
GLuint g_normalMapShader = 0;
GLuint g_normalMapShaderPCF = 0;
GLuint g_phongTexShader = 0;
GLuint g_phongShader = 0;

// Geometry loader and drawer
//
Geometry *g_table = nullptr;
Geometry *g_sphere = nullptr;
Geometry *g_torus = nullptr;
Geometry *g_box = nullptr;
Geometry *g_teapot = nullptr;
Geometry *g_bunny = nullptr;
Geometry *trees = nullptr;
Spotlight *spotlight = nullptr;
Skybox *skybox = nullptr;

//Liams Code
Procedural *procedural = nullptr;

// Mouse Button callback
// Called for mouse movement event on since the last glfwPollEvents
//
void cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
	// cout << "Mouse Movement Callback :: xpos=" << xpos << "ypos=" << ypos << endl;
	if (g_leftMouseDown) {
		g_yaw -= (g_mousePosition.x - xpos)/2;
		g_pitch -= (g_mousePosition.y - ypos)/2;
	}
	g_mousePosition = vec2(xpos, ypos);
}


// Mouse Button callback
// Called for mouse button event on since the last glfwPollEvents
//
void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
	// cout << "Mouse Button Callback :: button=" << button << "action=" << action << "mods=" << mods << endl;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		g_leftMouseDown = (action == GLFW_PRESS);
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (g_useShader) {
			g_useShader = false;
			cout << "Using the default OpenGL pipeline" << endl;
		}
		else {
			g_useShader = true;
			cout << "Using a shader" << endl;
		}
	}
}


// Scroll callback
// Called for scroll event on since the last glfwPollEvents
//
void scrollCallback(GLFWwindow *win, double xoffset, double yoffset) {
	// cout << "Scroll Callback :: xoffset=" << xoffset << "yoffset=" << yoffset << endl;
	g_zoom += yoffset*4/* * g_zoom * 0.2*/;
}


float WS = 0;
float AD = 0;
float QE = 0;

// Keyboard callback
// Called for every key event on since the last glfwPollEvents
//
void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	// cout << "Key Callback :: key=" << key << "scancode=" << scancode
	// 	<< "action=" << action << "mods=" << mods << endl;
	// YOUR CODE GOES HERE
	// ...
	if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		g_trans[0] += 5.0f;
	}
	if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		g_trans[0] -= 5.0f;
	}
	if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		g_trans[1] += 5.0f;
	}
	if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		g_trans[1] -= 5.0f;
	}
	if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		g_trans[2] += 5.0f;
	}
	if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		g_trans[2] -= 5.0f;
	}
	if (key == GLFW_KEY_KP_SUBTRACT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if (angle > 1) {
			angle -= 1;
		}
	}
	if (key == GLFW_KEY_KP_ADD && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if(angle < 89){
			angle += 1;
		}
	}
	if (key == GLFW_KEY_KP_2 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if (scale > 0.1) {
			scale -= 0.1;
		}
	}
	if (key == GLFW_KEY_KP_8 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if (scale < 2) {
			scale += 0.1;
		}
	}
	if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT) && g_ctrlDown) {
		rot[0] -= 5;
	}
	if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT) && !g_ctrlDown) {
		rot[0] += 5;
	}
	if (key == GLFW_KEY_Y && (action == GLFW_PRESS || action == GLFW_REPEAT) && g_ctrlDown) {
		rot[1] -= 5;
	}
	if (key == GLFW_KEY_Y && (action == GLFW_PRESS || action == GLFW_REPEAT) && !g_ctrlDown) {
		rot[1] += 5;
	}
	if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT) && g_ctrlDown) {
		rot[2] -= 5;
	}
	if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT) && !g_ctrlDown) {
		rot[2] += 5;
	}
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
		g_ctrlDown = true;
	}
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
		g_ctrlDown = false;
	}
	if (key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT) && g_ctrlDown) {
		g_rot[1] -= 2;
	}
	if (key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT) && !g_ctrlDown) {
		g_rot[1] += 2;
	}
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		l1 = !l1;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		l2 = !l2;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		l3 = !l3;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		l4 = !l4;
	}
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) { //set light dir as view dir
		GLfloat matrix[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
		lightDirection[0] = matrix[2];
		lightDirection[1] = matrix[6];
		lightDirection[2] = matrix[10];

		//int viewport[4];
		//// get matrixs and viewport:
		//glGetDoublev(GL_MODELVIEW_MATRIX, matModelView);
		//glGetDoublev(GL_PROJECTION_MATRIX, matProjection);
		//glGetIntegerv(GL_VIEWPORT, viewport);
		//gluUnProject((viewport[2] - viewport[0]) / 2, (viewport[3] - viewport[1]) / 2,
		//	0.0, matModelView, matProjection, viewport,
		//	&camera_pos[0], &camera_pos[1], &camera_pos[2]);
	}
	if (key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT) && g_ctrlDown) {
		//dirRot -= 5;
	}
	if (key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT) && !g_ctrlDown) {
		//dirRot += 5;
	}
	if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		procedural->WS += 2;
		WS += 2;
	}
	if(key == GLFW_KEY_LEFT && (action == GLFW_RELEASE)){
		procedural->CreateMap();
	}
	if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		procedural->WS -= 2;
		WS -= 2;
	}
	if (key == GLFW_KEY_RIGHT && (action == GLFW_RELEASE)) {
		procedural->CreateMap();
	}
	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		procedural->AD -= 2;
		AD -= 2;
	}
	if (key == GLFW_KEY_UP && (action == GLFW_RELEASE)) {
		procedural->CreateMap();
	}
	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		procedural->AD += 2;
		AD += 2;
	}
	if (key == GLFW_KEY_DOWN && (action == GLFW_RELEASE)) {
		procedural->CreateMap();
	}
}


// Character callback
// Called for every character input event on since the last glfwPollEvents
//
void charCallback(GLFWwindow *win, unsigned int c) {
	// cout << "Char Callback :: c=" << char(c) << endl;
	// Not needed for this assignment, but useful to have later on
	/*if (char(c) == 'w') {
		procedural->WS++;
	}
	if (char(c) == 's') {
		procedural->WS--;
	}
	if (char(c) == 'a') {
		procedural->AD++;
	}
	if (char(c) == 'd') {
		procedural->AD--;
	}
	if (char(c) == 'q') {
		procedural->QE++;
	}
	if (char(c) == 'e') {
		procedural->QE--;
	}*/
}


// Sets up where and what the light is
// Called once on start up
//
void initLight() {
	float direction[] = { 0.0f, 1.0f, 1.0f, 0.0f };
	float diffintensity[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	glLightfv(GL_LIGHT3, GL_POSITION, direction);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, diffintensity);
	glLightfv(GL_LIGHT3, GL_AMBIENT, ambient);

	glEnable(GL_LIGHT3);

}


void loadTexture(GLuint texture, const char* filename)
{
	cout << filename << endl;
	Image tex(filename);
	cout << "test" << endl;
	glBindTexture(GL_TEXTURE_2D, texture);

	/*glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //optional

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.w, tex.h, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.dataPointer());

	glGenerateMipmap(GL_TEXTURE_2D);*/

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tex.w, tex.h, tex.glFormat(), GL_UNSIGNED_BYTE, tex.dataPointer());
}


void loadCubemap(GLuint texture, const char* filename)
{
	Image tex(filename);

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	Image right = tex.subsection(tex.w / 2, tex.h / 3, tex.w / 4, tex.h / 3);
	gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 3, right.w, right.h, right.glFormat(), GL_UNSIGNED_BYTE, right.dataPointer());

	Image left = tex.subsection(0, tex.h / 3, tex.w / 4, tex.h / 3);
	gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 3, left.w, left.h, left.glFormat(), GL_UNSIGNED_BYTE, left.dataPointer());

	Image top = tex.subsection(tex.w / 4, 0, tex.w / 4, tex.h / 3);
	gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 3, top.w, top.h, top.glFormat(), GL_UNSIGNED_BYTE, top.dataPointer());

	Image bottom = tex.subsection(tex.w / 4, (tex.h / 3)*2, tex.w / 4, tex.h / 3);
	gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 3, bottom.w, bottom.h, bottom.glFormat(), GL_UNSIGNED_BYTE, bottom.dataPointer());

	Image front = tex.subsection(tex.w / 4, tex.h / 3, tex.w / 4, tex.h / 3);
	gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 3, front.w, front.h, front.glFormat(), GL_UNSIGNED_BYTE, front.dataPointer());

	Image back = tex.subsection((tex.w / 4)*3, tex.h / 3, tex.w / 4, tex.h / 3);
	gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 3, back.w, back.h, back.glFormat(), GL_UNSIGNED_BYTE, back.dataPointer());

	/*for (int i = 0; i < 6; i++) {
		Image tex(faces[i]);
		gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 3, tex.w, tex.h, tex.glFormat(), GL_UNSIGNED_BYTE, tex.dataPointer());
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, tex.w, tex.h, 0, tex.glFormat(), GL_UNSIGNED_BYTE, tex.dataPointer());
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);*/

	/*glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
	glEnable(GL_TEXTURE_GEN_R);*/
}

void createShadowMapTexture()
{
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize,
		shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// An example of how to load a texure from a hardcoded location
//
void initTexture() {


	glActiveTexture(GL_TEXTURE0); // Use slot 0, need to use GL_TEXTURE1 ... etc if using more than one texture PER OBJECT
	glGenTextures(9, textures); // Generate texture ID

	loadTexture(textures[0], "./work/res/textures/wood.jpg");
	loadTexture(textures[1], "./work/res/textures/brick.jpg");
	loadTexture(textures[2], "./work/res/textures/sand/sand1.jpg");
	/*vector<std::string> faces
	{
		"./work/res/textures/right.png",
		"./work/res/textures/left.png",
		"./work/res/textures/top.png",
		"./work/res/textures/bottom.png",
		"./work/res/textures/back.png",
		"./work/res/textures/front.png"
	};
	loadCubemap(textures[3], faces);*/
	loadCubemap(textures[3], "./work/res/textures/skybox/skybox.png");

	loadTexture(textures[4], "./work/res/textures/sand.jpg");
	loadTexture(textures[5], "./work/res/textures/grass.png");
	loadTexture(textures[6], "./work/res/textures/rock.png");
	loadTexture(textures[7], "./work/res/textures/rock2.png");
	loadTexture(textures[8], "./work/res/textures/water.jpg");
	
	glActiveTexture(GL_TEXTURE1); // Use slot 0, need to use GL_TEXTURE1 ... etc if using more than one texture PER OBJECT
	glGenTextures(2, texturesNormal); // Generate texture ID

	loadTexture(texturesNormal[0], "./work/res/textures/normalMap.jpg");
	loadTexture(texturesNormal[1], "./work/res/textures/sand/sand_normal.jpg");

	glActiveTexture(GL_TEXTURE2);
	createShadowMapTexture();

	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, texturesSpecular);
	loadTexture(texturesSpecular[0], "./work/res/textures/sand/sand_specular.jpg");
}


// An example of how to load a shader from a hardcoded location
//
void initShader() {
	// To create a shader program we use a helper function
	// We pass it an array of the types of shaders we want to compile
	// and the corrosponding locations for the files of each stage
	g_testShader = makeShaderProgramFromFile({ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, { "./work/res/shaders/shaderTest.vert", "./work/res/shaders/shaderTest.frag" });
	g_skyboxShader = makeShaderProgramFromFile({ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, { "./work/res/shaders/shaderSkybox.vert", "./work/res/shaders/shaderSkybox.frag" });
	g_normalMapShaderPCF = makeShaderProgramFromFile({GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, { "./work/res/shaders/shaderNormalMapPCF.vert", "./work/res/shaders/shaderNormalMapPCF.frag" });
	g_normalMapShader = makeShaderProgramFromFile({ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, { "./work/res/shaders/shaderNormalMap.vert", "./work/res/shaders/shaderNormalMap.frag" });
	g_phongTexShader = makeShaderProgramFromFile({ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, { "./work/res/shaders/shaderPhongTex.vert", "./work/res/shaders/shaderPhongTex.frag" });
	g_phongShader = makeShaderProgramFromFile({ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, { "./work/res/shaders/shaderPhong.vert", "./work/res/shaders/shaderPhong.frag" });
}


// Sets up where the camera is in the scene
//
void setupCamera(int width, int height) {
	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_fovy, width / float(height), g_znear, g_zfar);
	//glMultMatrixf(lpm);

	// Set up the view part of the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glRotatef(g_pitch, 1, 0, 0);
	glRotatef(g_yaw, 0, 1, 0);
	glTranslatef(g_trans[0], g_trans[1]+ g_zoom, g_trans[2]);
	glTranslatef(0, -10, -20);
	glTranslatef(0 + WS, 0 + QE, 0 - AD);
	
	//gluLookAt(-g_trans[0], -g_trans[1] - g_zoom, -g_trans[2] + 20,// position of camera
	//	-g_trans[0], -g_trans[1], -g_trans[2], // position to look at
	//	0.0, 1.0, 0.0);// up relative to camera

	//gluLookAt(0, 10,  20,// position of camera
	//	0,0, 0, // position to look at
	//	0.0, 1.0, 0.0);// up relative to camera

	glGetFloatv(GL_MODELVIEW_MATRIX, cvm);
	cameraViewMatrix = {
		cvm[0], cvm[1], cvm[2], cvm[3],
		cvm[4], cvm[5], cvm[6], cvm[7],
		cvm[8], cvm[9], cvm[10], cvm[11],
		cvm[12], cvm[13], cvm[14], cvm[15]
	};
	cameraViewMatrix = inverse(cameraViewMatrix);

	int i = 0;
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			cvm[i] = cameraViewMatrix[col][row];
			i++;
		}
	}
}




// Draws function
//
void render(int width, int height) {

	//Draw the scene
	// Set viewport to be the whole window
	glViewport(0, 0, width, height);

	// Grey/Blueish background
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f); black background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	setupCamera(width, height);

	glEnable(GL_TEXTURE_CUBE_MAP);
	glDepthMask(GL_FALSE);
	glUseProgram(g_skyboxShader);
	glUniform1i(glGetUniformLocation(g_skyboxShader, "texture0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[3]);
	skybox->renderSkybox();
	glUseProgram(0);
	glDepthMask(GL_TRUE);
	glDisable(GL_TEXTURE_CUBE_MAP);

	// Enable flags for normal rendering
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	float off[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	////strong spotlight
	//if (l1) {
	//	glPushMatrix();
	//	spotlight->renderSpotlight(trans, dir, angle, scale, rot);
	//	glPopMatrix();
	//}
	//else {
	//	glDisable(GL_LIGHT2);
	//	glLightfv(GL_LIGHT2, GL_DIFFUSE, off);
	//	glLightfv(GL_LIGHT2, GL_SPECULAR, off);
	//	glLightfv(GL_LIGHT2, GL_AMBIENT, off);
	//}

	//weak directional light
	if (l3) {
		glPushMatrix();
		glRotated(dirRot, 0, 1, 0);
		//float direction[] = { 0.0f, 0.0f, 1.0f, 0.0f };
		float diffintensity[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float specintensity[] = { 0.8f, 0.8f, 0.8f, 1.0f };
		float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

		glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffintensity);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specintensity);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

		glEnable(GL_LIGHT0);
		glPopMatrix();
	}
	else {
		glDisable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, off);
		glLightfv(GL_LIGHT0, GL_SPECULAR, off);
		glLightfv(GL_LIGHT0, GL_AMBIENT, off);
	}

	//weak ambient light
	if (l4) {
		float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	}
	else {
		float ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	}

	// With shaders
	// Uses the shaders that you bind for the graphics pipeline
	//
	if (g_useShader) {
		glEnable(GL_TEXTURE_2D);
		glUseProgram(g_normalMapShaderPCF);
		glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "texture0"), 0);
		glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "textureNormal"), 1);
		glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "textureShadowMap"), 2);
		glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "textureSpecular"), 3);
		glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 1);
		lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix * cameraViewMatrix;

		int i = 0;
		for (int col = 0; col < 4; col++) {
			for (int row = 0; row < 4; row++) {
				lsm[i] = lightSpaceMatrix[col][row];
				i++;
			}
		}
		glUniformMatrix4fv(glGetUniformLocation(g_normalMapShaderPCF, "lightSpaceMatrix"), 1, GL_FALSE, lsm);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texturesNormal[1]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texturesSpecular[0]);
		/*g_table->renderGeometry(g_rot);
		g_box->renderGeometry(g_rot);
		g_sphere->renderGeometry(g_rot);
		g_teapot->renderGeometry(g_rot);
		g_bunny->renderGeometry(g_rot);*/
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texturesNormal[0]);
		procedural->renderGeometry(g_rot);
		//g_torus->renderGeometry(g_rot);
		/*glPushMatrix();
		glTranslatef(0.0f, 0.0f, 10.0f);
		glScalef(0.001f, 0.001f, 0.001f);
		trees->renderGeometry(g_rot);
		glPopMatrix();*/
		glUseProgram(0);
		glActiveTexture(GL_TEXTURE0);
	}


	//without shader
	else {
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		procedural->renderGeometry(g_rot);
		glFlush();
	}


	// Disable flags for cleanup (optional)
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
}



void depthRender(int width, int height) {
	glViewport(0, 0, shadowMapSize, shadowMapSize);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(g_fovy, width / float(height), g_znear, g_zfar);
	glOrtho(-75.0f, 75.0f, -75.0f, 75.0f, g_znear, 200.0f);
	glGetFloatv(GL_PROJECTION_MATRIX, lpm);
	lightProjectionMatrix = {
		lpm[0], lpm[1], lpm[2], lpm[3],
		lpm[4], lpm[5], lpm[6], lpm[7],
		lpm[8], lpm[9], lpm[10], lpm[11],
		lpm[12], lpm[13], lpm[14], lpm[15]
	};

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -50.f);//offset
	
	
	gluLookAt(lightDirection[0]*10, lightDirection[1]*10, lightDirection[2]*10,// position of light
		0,0, 0, // position to look at
		0.0, 1.0, 0.0);// up relative to camera
	
	glTranslatef(g_trans[0], g_trans[1] + g_zoom, g_trans[2]);
	glTranslatef(0 + WS, 0 + QE, 0 - AD);
	/*glRotatef(45, 1, 0, 0);
	glRotatef(45, 0, 1, 0);
	glTranslatef(10.0f, -10.0f, -10.0f);*/
	glGetFloatv(GL_MODELVIEW_MATRIX, lvm);
	lightViewMatrix = {
		lvm[0], lvm[1], lvm[2], lvm[3],
		lvm[4], lvm[5], lvm[6], lvm[7],
		lvm[8], lvm[9], lvm[10], lvm[11],
		lvm[12], lvm[13], lvm[14], lvm[15]
	};

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glUseProgram(g_testShader);
	/*g_table->renderGeometry(g_rot);
	g_box->renderGeometry(g_rot);
	g_sphere->renderGeometry(g_rot);
	g_teapot->renderGeometry(g_rot);
	g_bunny->renderGeometry(g_rot);
	g_torus->renderGeometry(g_rot);*/
	procedural->renderGeometry(g_rot);
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}




// Forward decleration for cleanliness (Ignore)
void APIENTRY debugCallbackARB(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, GLvoid*);

//Main program
//
int main(int argc, char **argv) {

	// Initialize the GLFW library
	if (!glfwInit()) {
		cerr << "Error: Could not initialize GLFW" << endl;
		abort(); // Unrecoverable error
	}

	// Get the version for GLFW for later
	int glfwMajor, glfwMinor, glfwRevision;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);

	// Create a windowed mode window and its OpenGL context
	g_window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	if (!g_window) {
		cerr << "Error: Could not create GLFW window" << endl;
		abort(); // Unrecoverable error
	}

	// Make the g_window's context is current.
	// If we have multiple windows we will need to switch contexts
	glfwMakeContextCurrent(g_window);



	// Initialize GLEW
	// must be done after making a GL context current (glfwMakeContextCurrent in this case)
	glewExperimental = GL_TRUE; // required for full GLEW functionality for OpenGL 3.0+
	GLenum err = glewInit();
	if (GLEW_OK != err) { // Problem: glewInit failed, something is seriously wrong.
		cerr << "Error: " << glewGetErrorString(err) << endl;
		abort(); // Unrecoverable error
	}



	// Print out our OpenGL verisions
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using GLFW " << glfwMajor << "." << glfwMinor << "." << glfwRevision << endl;



	// Attach input callbacks to g_window
	glfwSetCursorPosCallback(g_window, cursorPosCallback);
	glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
	glfwSetScrollCallback(g_window, scrollCallback);
	glfwSetKeyCallback(g_window, keyCallback);
	glfwSetCharCallback(g_window, charCallback);



	// Enable GL_ARB_debug_output if available. Not nessesary, just helpful
	if (glfwExtensionSupported("GL_ARB_debug_output")) {
		// This allows the error location to be determined from a stacktrace
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// Set the up callback
		glDebugMessageCallbackARB(debugCallbackARB, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
		cout << "GL_ARB_debug_output callback installed" << endl;
	}
	else {
		cout << "GL_ARB_debug_output not available. No worries." << endl;
	}


	// Initialize Geometry/Material/Lights
	// YOUR CODE GOES HERE
	// ...
	//initLight(); //using my own 4 light sources as per assignment brief
	initTexture();
	initShader();

	trees = new Geometry("./work/res/assets/PalmTree.obj", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	g_table = new Geometry("./work/res/assets/table.obj", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,10.0f);
	g_sphere = new Geometry("./work/res/assets/sphere.obj", 0.0f, 0.0f, 0.0f, -5.0f, 2.0f+1.0f, 5.0f, 1.0f, 10.0f);
	g_box = new Geometry("./work/res/assets/box.obj", 0.0f, 0.0f, 0.0f, 5.0f, 2.5f, -5.0f, 1.0f, 10.0f);
	g_bunny = new Geometry("./work/res/assets/bunny.obj", 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 1.0f, 10.0f);
	g_torus = new Geometry("./work/res/assets/torus.obj", 0.0f, 0.0f, 0.0f, 5.0f, 1.0f, 5.0f, 1.0f, 1.0f);
	g_teapot = new Geometry("./work/res/assets/teapot.obj", 0.0f, 0.0f, 0.0f, -5.0f, 0.5f, -5.0f, 1.0f, 10.0f);
	
	spotlight = new Spotlight();
	skybox = new Skybox();
	procedural = new Procedural(textures, texturesNormal, g_normalMapShaderPCF, trees);


	// Loop until the user closes the window
	while (!glfwWindowShouldClose(g_window)) {

		// Make sure we draw to the WHOLE window
		int width, height;
		glfwGetFramebufferSize(g_window, &width, &height);

		depthRender(width, height);

		// Main Render
		render(width, height);

		// Swap front and back buffers
		glfwSwapBuffers(g_window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
}


//-------------------------------------------------------------
// Fancy debug stuff
//-------------------------------------------------------------

// function to translate source to string
string getStringForSource(GLenum source) {

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		return("API");
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return("Window System");
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return("Shader Compiler");
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return("Third Party");
	case GL_DEBUG_SOURCE_APPLICATION:
		return("Application");
	case GL_DEBUG_SOURCE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// function to translate severity to string
string getStringForSeverity(GLenum severity) {

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		return("HIGH!");
	case GL_DEBUG_SEVERITY_MEDIUM:
		return("Medium");
	case GL_DEBUG_SEVERITY_LOW:
		return("Low");
	default:
		return("n/a");
	}
}

// function to translate type to string
string getStringForType(GLenum type) {
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		return("Error");
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return("Deprecated Behaviour");
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return("Undefined Behaviour");
	case GL_DEBUG_TYPE_PORTABILITY:
		return("Portability Issue");
	case GL_DEBUG_TYPE_PERFORMANCE:
		return("Performance Issue");
	case GL_DEBUG_TYPE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// actually define the function
void APIENTRY debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, GLvoid*) {
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) return;

	cerr << endl; // extra space

	cerr << "Type: " <<
		getStringForType(type) << "; Source: " <<
		getStringForSource(source) << "; ID: " << id << "; Severity: " <<
		getStringForSeverity(severity) << endl;

	cerr << message << endl;

	if (type == GL_DEBUG_TYPE_ERROR_ARB) throw runtime_error("");
}
