/* Computer Graphics 2015
 * NTHU CS CGV Lab
 * Author: Yi-Hsiang Lo (LF21201@gmail.com)
 * Last-modified: 5/21, 2015
 */
#pragma once
#include "Matrices.h"

void LoadTextures(char* filename, int index);
void initTextures(int index);
void setTexture() ;
void TextureModel();
void loadOBJModel(int index);
void setViewPortMatrix();
void loadModel(int i);
void changeSize(int w, int h) ;
Matrix4 myTranslateMatrix(float tx, float ty, float tz);
Matrix4 myScaleMatrix(float sx, float sy, float sz);
Matrix4 myRotateMatrix(float angle, float x, float y, float z);
static GLfloat glmDot(GLfloat* u, GLfloat* v);
static GLvoid glmCross(GLfloat* u, GLfloat* v, GLfloat* n);
static GLvoid glmNormalize(GLfloat* v);
Matrix4 myLookAtMatrix(float eyex, float eyey, float eyez, float cx, float cy, float cz, float upx, float upy, float upz);
Matrix4 myFrustumMatrix(float l, float r, float b, float t, float n, float f);
Matrix4 myPerspectiveProjectionMatrix(float fovY, float aspect, float front, float back);
Matrix4 myParallelProjectionMatrix(float l, float r, float b, float t, float n, float f);
void showMatrix(Matrix4 m);
void renderScene(void) ;
void showHelp();
void processSpecialKeys(int key, int x, int y);
void processNormalKeys(unsigned char key, int x, int y);
void setShaders();
void processMouse(int who, int state, int x, int y);
void myMotionFunc(int x, int y);
