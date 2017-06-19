#include <cmath>
#include <string>
#include <stdexcept>
#include <algorithm>    // std::find
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

Procedural::Procedural(GLuint *t, GLuint *tN, GLuint shader, Geometry *tree, Geometry *grass, int seedInput, float oasisDiamiterInput, float oasisDepthInput) {
    seed = seedInput;
    oasisDiamiter = oasisDiamiterInput;
    oasisDepth = oasisDepthInput;
    textures = t;
    texturesNormal = tN;
    g_normalMapShaderPCF = shader;
    WS = 0;
    AD = 0;
    QE = 0;
    trees = tree;
    grasss = grass;
    m_uvs.clear();
    m_uvs.push_back(vec2(0.0, 0.0)); // dummy
    m_uvs.push_back(vec2(0.0, 0.0));
    m_uvs.push_back(vec2(0.0, 1.0));
    m_uvs.push_back(vec2(1.0, 1.0));
    m_uvs.push_back(vec2(1.0, 0.0));
    CreateMap();
}

void Procedural::setTexture(float height) {
    if (height < grassHeight) {
        //grass
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[5]);
        //dont use normal map
        glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 0);
    } else {
        if (height > 6) {

            if (height > 8) {
                //rock1
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures[6]);
                //dont use normal map
                glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 0);
            } else {
                //rock2
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures[7]);
                //dont use normal map
                glUniform1i(glGetUniformLocation(g_normalMapShaderPCF, "useNorm"), 0);
            }

        } else {
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

void Procedural::createNormals() {
    std::vector<cgra::vec3> normals; //triangle face normals
    for (int i = 0; i < m_triangles.size(); i++) {
        vec3 v0 = m_points[m_triangles[i].v[0].p];
        vec3 v1 = m_points[m_triangles[i].v[1].p];
        vec3 v2 = m_points[m_triangles[i].v[2].p];

        vec3 deltaPos1 = v1 - v0;
        vec3 deltaPos2 = v2 - v0;

        vec3 normal(cross(deltaPos1, deltaPos2));
        normals.push_back(normal);

        m_triangles[i].v[0].n = m_triangles[i].v[0].p; //set n as p because need normal for every vertex/point
        m_triangles[i].v[1].n = m_triangles[i].v[1].p;
        m_triangles[i].v[2].n = m_triangles[i].v[2].p;

        m_normals.push_back(vec3(0, 0, 0)); //create 0 normal for each point/vertex
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

}

void Procedural::createTangents() {

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
        vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        tangents.push_back(tangent);

        m_triangles[i].v[0].tang = m_triangles[i].v[0].p; //set n as p because need normal for every vertex/point
        m_triangles[i].v[1].tang = m_triangles[i].v[1].p;
        m_triangles[i].v[2].tang = m_triangles[i].v[2].p;

        m_tangents.push_back(vec3(0, 0, 0)); //create 0 normal for each point/vertex
        m_tangents.push_back(vec3(0, 0, 0));
        m_tangents.push_back(vec3(0, 0, 0));
    }

    for (int j = 0; j < m_triangles.size(); j++) {
        for (int k = 0; k < 3; k++) {
            m_tangents[m_triangles[j].v[k].tang] += tangents[j]; //sum vertex normal with face normal
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
    //glTexCoord2f(1.0, 0.0);
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
    //glTexCoord2f(0.0, 1.0);
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
        //glTexCoord2f(1.0, 0.0);
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
        //glTexCoord2f(0.0, 1.0);
        glEnd();

    }

    glEnd();
    glEndList();
}

void Procedural::createDisplayListTri() {
    // Delete old list if there is one
    if (m_displayList) glDeleteLists(m_displayList, 1);

    // Create a new list
    //cout << "Creating Poly Geometry" << endl;
    m_displayList = glGenLists(1);
    glNewList(m_displayList, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);
    for (triangle t : m_triangles) {

        setTexture((m_points[t.v[0].p][1] + m_points[t.v[1].p][1] + m_points[t.v[2].p][1]) / 3);
        glBegin(GL_TRIANGLES);
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
        glEnd();
    }
    glEnd();
    glEndList();
    cout << "Finished creating Poly Geometry" << endl;
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

    if (genNum < 0.02 && location.y < grassHeight && location.y > waterHeight) {
        glPushMatrix();
        glTranslatef(location.x, location.y - 0.5, location.z);
        glScalef(0.001f, 0.001f, 0.001f);
        trees->renderGeometry(vec3(0.0));
        glPopMatrix();
    }
    if (genNum > 0.7 && location.y < grassHeight && location.y > waterHeight) {
        glPushMatrix();
        glTranslatef(location.x, location.y, location.z);
        glScalef(0.2f, 0.2f, 0.2f);
        //glRotatef(0,0,0);
        grasss->renderGeometry(vec3(0.0));
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

            vertex v1;
            vertex v2;
            vertex v3;
            vertex v4;

            //set uv coords
            v1.t = 1;
            v2.t = 2;
            v3.t = 3;
            v4.t = 4;

            auto p1 = find(m_points.begin(), m_points.end(), a);
            if (p1 != m_points.end()) {
                v1.p = std::distance(m_points.begin(), p1);
            } else {
                v1.p = m_points.size();
                m_points.push_back(a);
            }

            auto p2 = find(m_points.begin(), m_points.end(), b);
            if (p2 != m_points.end()) {
                v2.p = std::distance(m_points.begin(), p2);
            } else {
                v2.p = m_points.size();
                m_points.push_back(b);
            }

            auto p3 = find(m_points.begin(), m_points.end(), c);
            if (p3 != m_points.end()) {
                v3.p = std::distance(m_points.begin(), p3);
            } else {
                v3.p = m_points.size();
                m_points.push_back(c);
            }

            auto p4 = find(m_points.begin(), m_points.end(), d);
            if (p4 != m_points.end()) {
                v4.p = std::distance(m_points.begin(), p4);
            } else {
                v4.p = m_points.size();
                m_points.push_back(d);
            }

            triangle tri1;
            tri1.v[0] = v1;
            tri1.v[1] = v2;
            tri1.v[2] = v3;

            triangle tri2;
            tri2.v[0] = v3;
            tri2.v[1] = v4;
            tri2.v[2] = v1;

            m_triangles.push_back(tri1);
            m_triangles.push_back(tri2);

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

    // Make sure our geometry information is cleared
    m_points.clear();
    m_normals.clear();
    m_triangles.clear();

    // Load dummy points because OBJ indexing starts at 1 not 0
    m_points.push_back(vec3(0, 0, 0));
    m_normals.push_back(vec3(0, 0, 1));


    int addedX = -(WS / CHUNK_RADIUS);
    int addedY = (AD / CHUNK_RADIUS);
    for (int i = -DRAW_DISTANCE + addedX; i < DRAW_DISTANCE + addedX; i++) {
        for (int j = -DRAW_DISTANCE + addedY; j < DRAW_DISTANCE + addedY; j++) {
            CreateChunk(i, j);
        }
    }
    //createDisplayList();
    createNormals();
    createTangents();
    createDisplayListTri();
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
        //glCallList(water);
    }
}