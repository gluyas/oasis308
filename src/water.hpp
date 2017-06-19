//
// Created by Marc on 17/06/2017.
//

#ifndef CGRA_PROJECT_A3_WATER_H
#define CGRA_PROJECT_A3_WATER_H

#define W_RES 128
#define W_G 9.807f   // gravity
#define W_DT 1.0f    // delta time per frame
#define W_DAMP 0.0f
#define W_FRAMES 3

#include "opengl.hpp"
#include "cgra_math.hpp"
#include "simple_shader.hpp"
#include "simple_image.hpp"

using namespace std;
using namespace cgra;

class Water {
public:
    Water(vec2 a, vec2 b, float height) {

        vec3 quad[4];
        if (a.x > b.x) {
            float temp = a.x;
            a.x = b.x;
            b.x = temp;
        }
        if (a.y > b.y) {
            float temp = a.y;
            a.y = b.y;
            b.y = temp;
        }
        quad[0] = vec3(a.x, height, a.y);
        quad[1] = vec3(b.x, height, a.y);
        quad[2] = vec3(b.x, height, b.y);
        quad[3] = vec3(a.x, height, b.y);

        boundA = a;
        boundB = b;

        // STORE GEOMETRY

        glGenVertexArrays(1, &vertArray);
        glBindVertexArray(vertArray);

        glGenBuffers(1, &vertBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 4, quad, GL_STATIC_READ);
        glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // SHADER

        shader = makeShaderProgramFromFile(
                {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
                { "./res/shaders/water.vert", "./res/shaders/water.frag"}
        );

        // GRADIENT MAP

        for (int i = 0; i < W_RES; i++) {
            for (int j = 0; j < W_RES; j++) {
                //gradientMap[i][j] = (i + j)%2 == 0? vec2(0,0) : vec2(1,1);
                //gradientMap[i][j] = j > 50 ? vec2(0,0) : vec2(1,1);
                //gradientMap[i][j] = vec2((float) i/W_RES, (float) j/W_RES);
                //gradientMap[i][j] = vec2(cos((float) i/W_RES * 12), sin((float) j/W_RES * 12));
            }
        }

        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &gradientTex); // Generate texture ID
        glBindTexture(GL_TEXTURE_2D, gradientTex); // Bind it as a 2D texture

        //loadTexture(gradientTex, "./res/textures/waterdudv.jpg");

        // INITIAL BASE, HEIGHT SAMPLING

        vec2 corner = vec2(a.x, a.y);
        unitX = (b.x - a.x)/W_RES;
        vec2 unitXVec = vec2(unitX, 0);
        unitY = (b.y - a.y)/W_RES;
        vec2 unitYVec = vec2(0, unitY);
        for (int x = 0; x < W_RES; x++) {
            for (int y = 0; y < W_RES; y++) {
                base[x][y] = 5-5.0f*((x + y) - W_RES)/W_RES;
                // corner + unitXVec * x + unitYVec * y
                heightPrev[x][y] = height + 4.0f*(x+y)/W_RES;
                heightCurrent[x][y] = height;
            }
        }


        // TRIDIAGONAL MATRICIES
        for (int i = 0; i <= 1; i++) {
            sliceView = (SliceView) i;  // alternate rows and columns
            for (unsigned int j = 0; j < W_RES; j++) {
                currentSlice = j;

                diagMajor[j][0][i] = e0();

                for (int k = 1; k < W_RES-1; k++) {
                    diagMajor[j][i][k] = e(k);
                    diagMinor[j][i][k-1] = f(k-1);
                }

                diagMajor[W_RES-1][i][W_RES-1] = eN();
                diagMinor[j][i][W_RES-2] = f(W_RES-2);
            }
        }
    }

    void render() {
        timeElapsed += 0.01f;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gradientTex);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, W_RES, W_RES, 0, GL_RG, GL_FLOAT, gradientMap);
        glUniform1i(glGetUniformLocation(GL_TEXTURE0, "waterGradientMap"), 0);
        glUniform1f(glGetUniformLocation(shader, "offset"), timeElapsed);

        glUniform2f(glGetUniformLocation(shader, "waterBoundA"), boundA.x, boundA.y);
        glUniform2f(glGetUniformLocation(shader, "waterBoundB"), boundB.x, boundB.y);

        glBindVertexArray(vertArray);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        glUseProgram(0);
        glDisable(GL_BLEND);
    }

    void simulate() {
        // water simulation
        for (int i = 0; i <= 1; i++) {
            sliceView = (SliceView) i;  // alternate rows and columns
            for (unsigned int j = 0; j < W_RES; j++) {
                currentSlice = j;

                // solve linear system Ax = b
                float hc[W_RES];
                hVec(hc, current);
                float hp[W_RES];
                hVec(hp, prev);

                float b[W_RES];
                for (int i = 0; i < W_RES; i++) {
                    b[i] = hc[i] + (1 - W_DAMP) * (hc[i] - hp[i]);
                }

                float result[W_RES];
                solve(diagMinor[currentSlice][sliceView], diagMajor[currentSlice][sliceView], result);

                if (sliceView == x) {
                    for (int i = 0; i < W_RES; i++) heightCurrent[i][currentSlice] = result[i];
                } else {
                    for (int i = 0; i < W_RES; i++) heightCurrent[currentSlice][i] = result[i];
                }
            }
            // rotate array refs
            auto temp = heightCurrent;
            heightCurrent = heightNext;
            heightNext = heightPrev;
            heightPrev = temp;
        }

        for (int x = 0; x < W_RES; x++) {
            for (int y = 0; y < W_RES; y++) {
                gradientMap[x][y] = vec2(heightCurrent[x][y], 0);
                cout << heightCurrent[x][y] << endl;
            }
        }

    }

private:

    float timeElapsed = 0;

    // RENDERING
    GLuint vertArray;
    GLuint vertBuffer;

    GLuint shader;

    vec2 boundA;
    vec2 boundB;

    GLuint gradientTex;
    vec2 gradientMap[W_RES][W_RES];

    // SIMULATION

    // first index is slice number, second is x/y direction, third is data string
    float diagMajor[W_RES][2][W_RES];
    float diagMinor[W_RES][2][W_RES-1];

    float unitX;
    float unitY;

    float base[W_RES][W_RES];

    float arrays[3][W_RES][W_RES];

    float (*heightCurrent)[W_RES] = arrays[0];
    float (*heightPrev)[W_RES] = arrays[1];
    float (*heightNext)[W_RES] = arrays[2];

    // state variables to simplify method signatures
    unsigned int currentSlice;
    enum SliceView : char {x = 0, z = 1};
    SliceView sliceView = x;

    enum Snapshot : char {next = 1, current = 0, prev = -1};

    void hVec(float * vec, Snapshot time) {
        float (*array)[W_RES];
        switch (time) {
            case next:
                array = heightNext;
                break;
            case current:
                array = heightCurrent;
                break;
            case prev:
                array = heightPrev;
                break;
        }

        if (currentSlice == x) {
            for (int i = 0; i < W_RES; i++) vec[i] = array[i][currentSlice];
        } else {
            for (int i = 0; i < W_RES; i++) vec[i] = array[currentSlice][i];
        }
    }

    // depth of water
    float d(int i) {
        return h(i) - b(i);
    }

    // height of water
    float h(int i) {
        if (sliceView == x) {
            return heightCurrent[i][currentSlice];
        } else {
            return heightCurrent[currentSlice][i];
        }
    }

    // base of geography
    float b(int i) {
        if (sliceView == x) {
            return base[i][currentSlice];
        } else {
            return base[currentSlice][i];
        }
    }

    // matrix building functions

    inline float unitLength() {
        if (sliceView == x) {
            return unitX;
        } else {
            return unitY;
        }
    }

    float e0() {
        return 1 + W_G * W_DT * W_DT * (d(0) + d(1)) / (2 * unitLength() * unitLength());
    }

    float eN() {
        return 1 + W_G * W_DT * W_DT * (d(W_RES - 2) + d(W_RES - 1)) / (2 * unitLength() * unitLength());
    }

    float e(int i) {
        return 1 + W_G * W_DT * W_DT * (d(i-1) + 2*d(i) + d(i+1)) / (2 * unitLength() * unitLength());
    }

    float f(int i) {
        return - W_G * W_DT * W_DT * (d(i) + d(i+1)) / (2 * unitLength() * unitLength());
    }

    // matrix solve method. modified version of:
    // https://en.wikibooks.org/wiki/Algorithm_Implementation/Linear_Algebra/Tridiagonal_matrix_algorithm#C.2B.2B
    // Written by Keivan Moradi, 2014
    void solve(float* diagMinor, float* diagMajor, float* x) {

        float c[W_RES -1];
        for (int i = 0; i < W_RES-1; i++) c[i] = diagMinor[i];  // our matricies are always symmetrical

        int n = W_RES-1; // since we start from x0 (not x1)
        c[0] /= diagMajor[0];
        x[0] /= diagMajor[0];

        for (int i = 1; i < n; i++) {
            c[i] /= diagMajor[i] - diagMinor[i]*c[i-1];
            x[i] = (x[i] - diagMinor[i]*x[i-1]) / (diagMajor[i] - diagMinor[i]*c[i-1]);
        }

        x[n] = (x[n] - diagMinor[n]*x[n-1]) / (diagMajor[n] - diagMinor[n]*c[n-1]);

        for (int i = n; i-- > 0;) {
            x[i] -= c[i]*x[i+1];
        }
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


};


#endif //CGRA_PROJECT_A3_WATER_H
