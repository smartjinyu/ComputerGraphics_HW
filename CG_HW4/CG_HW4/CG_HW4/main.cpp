/* Computer Graphics 2016
 * NTHU CS CGV Lab
 * Author: Yi-Hsiang Lo (seanyhl@cgv.cs.nthu.edu.tw)
 * Last-modified: 5/18, 2016
 */
#include <stdio.h>
#include <stdlib.h>
 
#include <GL/glew.h>
#include <freeglut/glut.h>
#include "textfile.h"
#include "GLM.h"
#include "Matrices.h"
#include "Texture.h"
#include "main.h"

#define MAX_TEXTURE_NUM 50
#define MAXSIZE 200000
#define MAXGROUPSIZE 50
#define myMax(a,b) (((a)>(b))?(a):(b))

#pragma warning (disable: 4996)

#ifndef GLUT_WHEEL_UP
# define GLUT_WHEEL_UP   0x0003
# define GLUT_WHEEL_DOWN 0x0004
#endif

// semaphore
int isModelLoading = 0;

// toggles
int isAutoRotating = 0;

int currentModel = 0;
char* filename[] = 
{
	/*00*/"TextureModels/teemo.obj",
	/*01*/"TextureModels/satellitetrap.obj",
	/*02*/"TextureModels/cube.obj",
	/*03*/"TextureModels/texturedknot.obj",
	/*04*/"TextureModels/laurana500.obj",
	/*05*/"TextureModels/duck.obj",
	/*06*/"TextureModels/ball.obj",
	/*07*/"TextureModels/Nala.obj",
	/*08*/"TextureModels/Digda.obj",
	/*09*/"TextureModels/Fushigidane.obj",
	/*10*/"TextureModels/Golonya.obj",
	/*11*/"TextureModels/Hitokage.obj",
	/*12*/"TextureModels/Mew.obj",
	/*13*/"TextureModels/Nyarth.obj",
	/*14*/"TextureModels/Zenigame.obj",
};
const int NUM_OF_MODEL = sizeof(filename) / sizeof(char*);

// textures
GLuint texNum[MAX_TEXTURE_NUM];
unsigned char image[MAX_TEXTURE_NUM][MAXSIZE]; //image data array 
FileHeader FH[MAX_TEXTURE_NUM]; //BMP FileHeader 
InfoHeader IH[MAX_TEXTURE_NUM]; //BMP InfoHeader
GLint iLocTexCoord;
GLint texture_wrap_mode = GL_REPEAT;
GLint texture_mag_filter = GL_LINEAR;
GLint texture_min_filter = GL_LINEAR;

// window size
const unsigned int uiWidth  = 500;
const unsigned int uiHeight = 500;
int screenWidth;
int screenHeight;

// eye position
GLfloat eyeVec[3] = {0.0f, 0.0f, 5.0f};

// option keys
#define PROJECTION_PERS 0x01
#define PROJECTION_PARA 0x02
char projectionMode = PROJECTION_PERS;

// projection matrix parameters
const float xmin = -1.0, xmax = 1.0;
const float ymin = -1.0, ymax = 1.0;
const float znear = eyeVec[2]-sqrtf(3.0f), zfar = eyeVec[2]+sqrtf(3.0f);

// normalization matrix
Matrix4 normalization_matrix;

// global view port matrix
Matrix4 viewPort;

// global projection matrix
Matrix4 parallelProjectionMatrix = myParallelProjectionMatrix(xmin, xmax, ymin, ymax, znear, zfar);
Matrix4 perspectiveProjectionMatrix = myFrustumMatrix(xmin, xmax, ymin, ymax, znear, zfar);

// Shader attributes
GLuint v, f, f2, p;

// Shader Locations
GLint iLocModelMatrix;

GLint iLocPosition;
GLint iLocMVP;

float rotateDeltaX, rotateDeltaY;
float rotateStartX, rotateStartY;
float rotateX, rotateY;
Matrix4 preM;
int isRBtnDown = 0;
int isMouseMoving = 0;

float preTranslateX, preTranslateY;
float translateX, translateY;
float translateStartX, translateStartY;
int isLBtnDown = 0;

float vertices[MAXGROUPSIZE][MAXSIZE];
float vtextures[MAXGROUPSIZE][MAXSIZE];

float aMVP[16];

GLMmodel* OBJ;
GLfloat scale;

// unpack bmp file
void LoadTextures(char* filename, int index)
{
	unsigned long size;
	FILE *file = fopen(filename, "rb");
	fread(&FH[index], sizeof(FileHeader), 1, file);
	fread(&IH[index], sizeof(InfoHeader), 1, file);
	size = IH[index].Width * IH[index].Height * 3;
	fread(image[index], size*sizeof(char), 1, file);
	fclose(file);
}

void initTextures(int index)
{
	glBindTexture(GL_TEXTURE_2D, texNum[index]);

	// TODO: A Text2D texture is created there.
	// NOTE: texture width x height = IH[index].Width x IH[index].Height
	//       image[i] means the texture image material used by i^{th} group
	// HINT: https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glTexImage2D.xml
	// glTexImage2D();
	// GL_TEXTURE_2D, GL_RGB, GL_UNSIGNED_BYTE

	// TODO: Generate mipmap by using gl function NOT glu function.
	// HINT: https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glGenerateMipmap.xml
	// glGenerateMipmap();
	// GL_TEXTURE_2D
}

void setTexture() 
{
	int tex_count = 0;
	GLMgroup* group = OBJ->groups;
	while(group){
		if(strlen(OBJ->materials[group->material].textureImageName) != 0){
			char TexNameStr[200];
			sprintf(TexNameStr, "TextureModels/%s", OBJ->materials[group->material].textureImageName);
			printf("read %s\n", TexNameStr);
			LoadTextures(TexNameStr, tex_count);
		}
		tex_count++;
		group = group->next;
	}
	glGenTextures(tex_count, texNum);
	for(int i = 0; i < tex_count; i++){
		initTextures(i);
	}
}

// parse texture model and align the vertices
void TextureModel()
{
	GLMgroup* group = OBJ->groups;
	float maxx, maxy, maxz;
	float minx, miny, minz;
	float dx, dy, dz;

	maxx = minx = OBJ->vertices[3];
	maxy = miny = OBJ->vertices[4];
	maxz = minz = OBJ->vertices[5];
	for(unsigned int i=2; i<=OBJ->numvertices; i++){
		GLfloat vx, vy, vz;
		vx = OBJ->vertices[i*3+0];
		vy = OBJ->vertices[i*3+1];
		vz = OBJ->vertices[i*3+2];
		if(vx > maxx) maxx = vx;  if(vx < minx) minx = vx; 
		if(vy > maxy) maxy = vy;  if(vy < miny) miny = vy; 
		if(vz > maxz) maxz = vz;  if(vz < minz) minz = vz; 
	}
	//printf("max\n%f %f, %f %f, %f %f\n", maxx, minx, maxy, miny, maxz, minz);
	dx = maxx - minx;
	dy = maxy - miny;
	dz = maxz - minz;
	//printf("dx,dy,dz = %f %f %f\n", dx, dy, dz);
	GLfloat normalizationScale = myMax(myMax(dx, dy), dz)/2;
	OBJ->position[0] = (maxx + minx) / 2;
	OBJ->position[1] = (maxy + miny) / 2;
	OBJ->position[2] = (maxz + minz) / 2;

	int gCount = 0;
	while(group){
		for(unsigned int i=0; i<group->numtriangles; i++){
			// triangle index
			int triangleID = group->triangles[i];

			// the index of each vertex
			int indv1 = OBJ->triangles[triangleID].vindices[0];
			int indv2 = OBJ->triangles[triangleID].vindices[1];
			int indv3 = OBJ->triangles[triangleID].vindices[2];

			// vertices
			GLfloat vx, vy, vz;
			vx = OBJ->vertices[indv1*3];
			vy = OBJ->vertices[indv1*3+1];
			vz = OBJ->vertices[indv1*3+2];
			//printf("%f %f %f\n", vx, vy, vz);
			vertices[gCount][i*9+0] = vx;
			vertices[gCount][i*9+1] = vy;
			vertices[gCount][i*9+2] = vz;
			vx = OBJ->vertices[indv2*3];
			vy = OBJ->vertices[indv2*3+1];
			vz = OBJ->vertices[indv2*3+2];
			//printf("%f %f %f\n", vx, vy, vz);
			vertices[gCount][i*9+3] = vx;
			vertices[gCount][i*9+4] = vy;
			vertices[gCount][i*9+5] = vz;
			vx = OBJ->vertices[indv3*3];
			vy = OBJ->vertices[indv3*3+1];
			vz = OBJ->vertices[indv3*3+2];
			//printf("%f %f %f\n", vx, vy, vz);
			vertices[gCount][i*9+6] = vx;
			vertices[gCount][i*9+7] = vy;
			vertices[gCount][i*9+8] = vz;
			//printf("\n");
			
			int indt1 = OBJ->triangles[triangleID].tindices[0]; 
			int indt2 = OBJ->triangles[triangleID].tindices[1]; 
			int indt3 = OBJ->triangles[triangleID].tindices[2];
			
			// TODO: texture coordinates should be aligned by yourself
			//OBJ->texcoords[indt1*2];
			//OBJ->texcoords[indt1*2+1];
			//OBJ->texcoords[indt2*2];
			//OBJ->texcoords[indt2*2+1];
			//OBJ->texcoords[indt3*2];
			//OBJ->texcoords[indt3*2+1];
		}
		group = group->next;
		gCount++;
	}

	// custom normalization matrix for each loaded model
	normalization_matrix.identity();
	normalization_matrix = normalization_matrix * myTranslateMatrix(-OBJ->position[0], -OBJ->position[1], -OBJ->position[2]);
	normalization_matrix = normalization_matrix * myScaleMatrix(1/normalizationScale, 1/normalizationScale, 1/normalizationScale);
}

void loadOBJModel(int index)
{
	// load model through glm parser
	OBJ = glmReadOBJ(filename[index]);
	printf("%s\n", filename[index]);

	// glm will calculate the vertex normals for you
	// uncomment them only if there is no vertex normal info in your .obj file
//	glmFacetNormals(OBJ);         
//	glmVertexNormals(OBJ, 90.0);  

	// parse texture model and align the vertices
	TextureModel();
}

void setViewPortMatrix()
{
	glViewport(0, 0, screenWidth, screenHeight);
	viewPort.identity();
	float ratio = (float)screenWidth / (float)screenHeight;
	if(ratio >= 1){
		viewPort = viewPort * myScaleMatrix(1.0f/ratio, 1, 1);
	}else{
		viewPort = viewPort * myScaleMatrix(1, ratio, 1);
	}
}

void loadModel(int i)
{
	isModelLoading = 1;
	loadOBJModel(i);
	setTexture();
	isModelLoading = 0;
}

// Callback function of window size changing
void onWindowReshape(int w, int h) 
{
    screenWidth = w;
	screenHeight = h;

	// view port matrix
	setViewPortMatrix();	
}

Matrix4 myTranslateMatrix(float tx, float ty, float tz)
{
	Matrix4 mat;
	mat.identity();
	mat[12] = tx;
	mat[13] = ty;
	mat[14] = tz;
	return mat;
}

Matrix4 myScaleMatrix(float sx, float sy, float sz)
{
	Matrix4 mat;
	mat.identity();
	mat[0] = sx;
	mat[5] = sy;
	mat[10] = sz;
	return mat;
}

Matrix4 myRotateMatrix(float angle, float x, float y, float z)
{
    float c = cosf(angle / 180.0f * M_PI);    // cos
    float s = sinf(angle / 180.0f * M_PI);    // sin
    float xx = x * x;
    float xy = x * y;
    float xz = x * z;
    float yy = y * y;
    float yz = y * z;
    float zz = z * z;

    Matrix4 m;
    m[0] = xx * (1 - c) + c;
    m[1] = xy * (1 - c) - z * s;
    m[2] = xz * (1 - c) + y * s;
    m[3] = 0;
    m[4] = xy * (1 - c) + z * s;
    m[5] = yy * (1 - c) + c;
    m[6] = yz * (1 - c) - x * s;
    m[7] = 0;
    m[8] = xz * (1 - c) - y * s;
    m[9] = yz * (1 - c) + x * s;
    m[10]= zz * (1 - c) + c;
    m[11]= 0;
    m[12]= 0;
    m[13]= 0;
    m[14]= 0;
    m[15]= 1;
	
    return m;
}

/* glmDot: compute the dot product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static GLfloat
glmDot(GLfloat* u, GLfloat* v)
{   
    return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

/* glmCross: compute the cross product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 * n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
 */
static GLvoid
glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
    n[0] = u[1]*v[2] - u[2]*v[1];
    n[1] = u[2]*v[0] - u[0]*v[2];
    n[2] = u[0]*v[1] - u[1]*v[0];
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
static GLvoid
glmNormalize(GLfloat* v)
{
    GLfloat l;
    l = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}


Matrix4 myLookAtMatrix(
	float eyex, float eyey, float eyez, 
	float cx, float cy, float cz, 
	float upx, float upy, float upz)
{
	// http://publib.boulder.ibm.com/infocenter/pseries/v5r3/index.jsp?topic=/com.ibm.aix.opengl/doc/openglrf/gluLookAt.htm
	/*
	Let E be the 3d column vector (eyeX, eyeY, eyeZ).
	Let C be the 3d column vector (centerX, centerY, centerZ).
	Let U be the 3d column vector (upX, upY, upZ).
	Compute L = C - E.
	Normalize L.
	Compute S = L x U.
	Normalize S.
	Compute U' = S x L.
	*/
	float E[3] = {eyex, eyey, eyez};
	float L[3] = {cx-eyex, cy-eyey, cz-eyez};
	glmNormalize(L);
	float U[3] = {upx, upy, upz};
	glmNormalize(U);
	float S[3];
	glmCross(L, U, S);
	glmNormalize(S);
	glmCross(S, L, U);

	Matrix4 m;
	m[0]  = S[0];
	m[1]  = S[1];
	m[2]  = S[2];
	m[3]  = 0;
	m[4]  = U[0];
	m[5]  = U[1];
	m[6]  = U[2];
	m[7]  = 0;
	m[8]  = -L[0];
	m[9]  = -L[1];
	m[10] = -L[2];
	m[11] = 0;
	m[12] = -E[0];
	m[13] = -E[1];
	m[14] = -E[2];
	m[15] = 1;

	return m;
}

Matrix4 myFrustumMatrix(float l, float r, float b, float t, float n, float f)
{
    Matrix4 mat;
    mat[0]  = 2 * n / (r - l);
	mat[1]  = 0;
    mat[2]  = 0;
	mat[3]  = 0;

	mat[4]  = 0;
    mat[5]  = 2 * n / (t - b);
    mat[6]  = 0;
	mat[7]  = 0;
	
	mat[8]  = (r + l) / (r - l);
	mat[9]  = (t + b) / (t - b);
    mat[10] = -(f + n) / (f - n);
    mat[11] = -1;
	
	mat[12] = 0;
	mat[13] = 0;
    mat[14] = -(2 * f * n) / (f - n);
    mat[15] = 0;
    return mat;
}

Matrix4 myPerspectiveProjectionMatrix(float fovY, float aspect, float front, float back)
{
    float tangent = tanf(fovY/2.0f * 180.0f/M_PI); // tangent of half fovY
    float height = front * tangent;         // half height of near plane
    float width = height * aspect;          // half width of near plane

    // params: left, right, bottom, top, near, far
    return myFrustumMatrix(-width, width, -height, height, front, back);
}

Matrix4 myParallelProjectionMatrix(float l, float r, float b, float t, float n, float f)
{
    Matrix4 mat;
    mat[0]  = 2 / (r - l);
	mat[1]  = 0;
	mat[2]  = 0;
	mat[3]  = 0;

	mat[4]  = 0;
    mat[5]  = 2 / (t - b);
	mat[6]  = 0;
	mat[7]  = 0;

	mat[8]  = 0;
	mat[9]  = 0;
    mat[10] = -2 / (f - n);
	mat[11] = 0;

    mat[12] = -(r + l) / (r - l);
    mat[13] = -(t + b) / (t - b);
    mat[14] = -(f + n) / (f - n);
	mat[15] = 1;
    return mat;
}

void showMatrix(Matrix4 m)
{
	printf("===========================\n");
	printf("%lf %lf %lf %lf\n", m[0], m[4], m[8], m[12]);
	printf("%lf %lf %lf %lf\n", m[1], m[5], m[9], m[13]);
	printf("%lf %lf %lf %lf\n", m[2], m[6], m[10], m[14]);
	printf("%lf %lf %lf %lf\n", m[3], m[7], m[11], m[15]);
}

void onDisplay(void) 
{
	while(isModelLoading == 1){
		//printf("waiting\n");
	}

	glClearColor(0.3f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


	Matrix4 M;
	// aMVP = identity() * M_normalize(Tn*Sn) * M_transformation(S or R or T) * V(lookat) * P(projection)
			
	M.identity();
			
	// normalize
	M = M * normalization_matrix;

	// previous rotate matrix
	M = M * preM;

	// mouse rotate
	if(isRBtnDown && isMouseMoving){
		float temp = sqrt(rotateX * rotateX + rotateY * rotateY);
		if(temp != 0){
			Matrix4 newM = myRotateMatrix(-temp / 4.0f, rotateY/temp, rotateX/temp, 0.0f);
			// divided by 4 in order to rotate slower
			M = M * newM;
			preM = preM * newM;
		}
	}
	
	// auto rotate
	if (isAutoRotating) {
		Matrix4 newM = myRotateMatrix(3.0f, 0.0f, 1.0f, 0.0f);
		M = M * newM;
		preM = preM * newM;
	}
	
	// scale
	M = M * myScaleMatrix(scale, scale, scale);

	// mouse translate
	M = M * myTranslateMatrix(preTranslateX, preTranslateY, 0);
	if(isMouseMoving && isLBtnDown){
		float ratio = (float)screenWidth / (float)screenHeight;
		float alphaX = (xmax - xmin) / (float)screenWidth;
		float alphaY = (ymax - ymin) / (float)screenHeight;
		if(ratio >= 1){
			alphaX *= ratio;
		}else{
			alphaY /= ratio;
		}
		M = M * myTranslateMatrix(translateX*alphaX, translateY*alphaY, 0);
		preTranslateX += translateX * alphaX;
		preTranslateY += translateY * alphaY;
	}

	Matrix4 modelMatrix;
	// Model matrix ends here
	for(int i=0;i<16;i++){
		modelMatrix[i] = M[i];
	}

	// V (lookat)
	M = M * myLookAtMatrix(eyeVec[0],eyeVec[1],eyeVec[2],  eyeVec[0],eyeVec[1],eyeVec[2]-1.0f,  0,1,0);

	// P (project)
	if(projectionMode == PROJECTION_PARA)
	{
		M = M * parallelProjectionMatrix;
	}
	else if(projectionMode == PROJECTION_PERS)
	{
		M = M * perspectiveProjectionMatrix;
	}
	else 
	{
		fprintf(stderr, "ERROR!\n");
		system("pause");
		exit(-1);
	}

	// V (view port)
	M = M * viewPort;

	for(int i=0;i<16;i++) aMVP[i] = M[i];

	// Matrices
	glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, M.get());
	glUniformMatrix4fv(iLocModelMatrix, 1, GL_FALSE, modelMatrix.get());

	GLMgroup* group = OBJ->groups;
	int gCount = 0;
	while(group){
		// enable attributes array
		glEnableVertexAttribArray(iLocPosition);
		// TODO: texture VertexAttribArray is enabled here
		// HINT: https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glEnableVertexAttribArray.xml
		// glEnableVertexAttribArray();

		// bind attributes array
		glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, vertices[gCount]);
		// TODO: bind texture vertex attribute pointer here
		// HINT: https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glVertexAttribPointer.xml
		// glVertexAttribPointer();

		// bind texture material group by group
		// This will bind your texture to the variable which type is "Sampler2D" in your shader.
		// e.g. uniform sampler2D us2dtexture; // in shader.frag
		// TODO: bind texture here
		// HINT: https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glBindTexture.xml
		// glBindTexture
		// GL_TEXTURE_2D

		// texture mag/min filter
		// TODO: texture mag/min filters are defined here
		// HINT: https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml
		// glTexParameteri();
		// GL_TEXTURE_2D
		// GL_TEXTURE_MAG_FILTER, GL_TEXTURE_MIN_FILTER
		// GL_LINEAR, GL_NEAREST
		// GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST
		// GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST
		
		// texture wrap mode s/t
		// TODO: texture wrap modes are defined here
		// HINT: https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml
		// glTexParameteri();
		// GL_TEXTURE_2D
		// GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T
		// GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE

		// draw arrays
		glDrawArrays(GL_TRIANGLES, 0, group->numtriangles*3);

		gCount++;
		group = group->next;
	}

	isMouseMoving = 0;

	Sleep(20);

	glutSwapBuffers();
}

void showHelp()
{
	printf("=============================\n");
	printf("press H / h to show this menu\n");
	printf("\n");
	printf("PROJECTION:        press P / p to switch perspective / orthographic projection\n");
	printf("\n");
	printf("TRANSLATION:       press the left button and drag the mouse to translate\n");
	printf("\n");
	printf("ROTATION:          press the right button and drag the mouse to rotate\n");
	printf("\n");
	printf("SCALE:             scroll mouse wheel up and down to scale\n");
	printf("\n");
	printf("MODEL SWITCHING:   press Z / z to switch previous model.\n");
	printf("                   press X / x to switch next model.\n");
	printf("\n");
}

void onKeyboardSpecial(int key, int x, int y){
	/*
	GLUT_KEY_F1			F1 function key.
	GLUT_KEY_F2			F2 function key.
	GLUT_KEY_F3			F3 function key.
	GLUT_KEY_F4			F4 function key.
	GLUT_KEY_F5			F5 function key.
	GLUT_KEY_F6			F6 function key.
	GLUT_KEY_F7			F7 function key.
	GLUT_KEY_F8			F8 function key.
	GLUT_KEY_F9			F9 function key.
	GLUT_KEY_F10		F10 function key.
	GLUT_KEY_F11		F11 function key.
	GLUT_KEY_F12		F12 function key.
	GLUT_KEY_LEFT		Left directional key.
	GLUT_KEY_UP			Up directional key.
	GLUT_KEY_RIGHT		Right directional key.
	GLUT_KEY_DOWN		Down directional key.
	GLUT_KEY_PAGE_UP	Page up directional key.
	GLUT_KEY_PAGE_DOWN	Page down directional key.
	GLUT_KEY_HOME		Home directional key.
	GLUT_KEY_END		End directional key.
	GLUT_KEY_INSERT		Inset directional key.
	*/
}

void onKeyboard(unsigned char key, int x, int y) {
	int modifier = glutGetModifiers();
	//printf("modifier = %d\n", modifier);
	//printf("key = %d\n", key);
	switch(modifier){
		case GLUT_ACTIVE_CTRL:{
			switch(key) {
				/* ctrl+q */ case 17: break;
				/* ctrl+a */ case  1: break;
				/* ctrl+w */ case 23: break;
				/* ctrl+s */ case 19: break;
				/* ctrl+e */ case  5: break;
				/* ctrl+d */ case  4: break;
			}
			break;
		}
	}

	// normal keys
	switch(key){
		case 'r': case 'R': 
			isAutoRotating = isAutoRotating ^ 1;
			break;

		case 'Z': case 'z':
			currentModel = (currentModel + NUM_OF_MODEL - 1) % NUM_OF_MODEL;
			loadModel(currentModel);
			printf("loading %s\n", filename[currentModel]);
			break;

		case 'X': case 'x':
			currentModel = (currentModel + 1) % NUM_OF_MODEL;
			loadModel(currentModel);
			printf("loading %s\n", filename[currentModel]);
			break;

		case 'H': case 'h':
			showHelp();
			break;

		case 'P': case 'p':
			if(projectionMode != PROJECTION_PERS){
				projectionMode = PROJECTION_PERS;
				printf("perspective projection\n");
			}else if(projectionMode != PROJECTION_PARA){
				projectionMode = PROJECTION_PARA;
				printf("parallel projection\n");
			}
			break;

		case 27: // esc key
			exit(0);
	}
	//glutPostRedisplay();
}

void setShaders() {
	char *vs = NULL,*fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("shader.vert");
	fs = textFileRead("shader.frag");

	const char * ff = fs;
	const char * vv = vs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);

	GLint vCompiled;
	glGetShaderiv(v, GL_COMPILE_STATUS, &vCompiled);
	if (!vCompiled) { 
		GLint length; 
		GLchar* log; 
 
		glGetShaderiv(v, GL_INFO_LOG_LENGTH, &length ); 
 
		log = (GLchar*) malloc(length); 
		glGetShaderInfoLog(v, length, &length, log); 
		fprintf(stderr, "[v] compile log = '%s'\n", log); 
		free(log); 
	} 

	GLint fCompiled;
	glGetShaderiv(f, GL_COMPILE_STATUS, &fCompiled);
	if (!fCompiled) { 
		GLint length; 
		GLchar* log; 
 
		glGetShaderiv(f, GL_INFO_LOG_LENGTH, &length ); 
 
		log = (GLchar*) malloc(length); 
		glGetShaderInfoLog(f, length, &length, log); 
		fprintf(stderr, "[f] compile log = '%s'\n", log); 
		free(log); 
	} 

	p = glCreateProgram();
	glAttachShader(p,f);
	glAttachShader(p,v);

	glLinkProgram(p);
	GLint linked = 0;
	glGetProgramiv(p, GL_LINK_STATUS, &linked );
	if (linked) {
		glUseProgram(p);
	} else {
		GLint length;
		GLchar* log;
		glGetProgramiv(p, GL_INFO_LOG_LENGTH, &length );
		log = (GLchar*) malloc(length);
		glGetProgramInfoLog(p, length, &length, log);
		fprintf(stderr, "link log = '%s'\n", log);
		free(log);
	}

	iLocPosition = glGetAttribLocation(p, "av4position");
	iLocTexCoord = glGetAttribLocation(p, "av2texCoord");

	iLocMVP = glGetUniformLocation(p, "mvp");

	iLocModelMatrix  = glGetUniformLocation(p, "um4modelMatrix");
	
	glUseProgram(p);
}

void onMouse(int who, int state, int x, int y){
	if(who == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && isLBtnDown == 0)
	{
		rotateStartX = (float)x;
		rotateStartY = (float)y;
		isRBtnDown = 1;
	}
	else if(who == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		isRBtnDown = 0;
	}
	else if(who == GLUT_LEFT_BUTTON && state == GLUT_DOWN && isRBtnDown == 0)
	{
		translateX = x - translateStartX;
		translateY = -(y - translateStartY);
		translateStartX = (float)x;
		translateStartY = (float)y;
		isLBtnDown = 1;
	}
	else if(who == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		isLBtnDown = 0;
	}
	else if(who == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{
		scale = 1.0f;
	}
	else if(who == GLUT_WHEEL_UP && state == GLUT_DOWN)
	{
		if(scale < 2.29) scale *= 1.05f;
	}
	else if(who == GLUT_WHEEL_DOWN && state == GLUT_DOWN)
	{
		if(scale > 0.25) scale /= 1.05f;
	}
}

// callback on mouse drag
void onMouseMotion(int x, int y)
{
	if(isRBtnDown){
		rotateX = x - rotateStartX;
		rotateY = y - rotateStartY;
		rotateStartX = (float)x;
		rotateStartY = (float)y;
	}
	if(isLBtnDown){
		translateX = x - translateStartX;
		translateY = -(y - translateStartY);
		translateStartX = (float)x;
		translateStartY = (float)y;
	}
	isMouseMoving = 1;
}

int main(int argc, char **argv)
{
	// glut init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	// create window
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(uiWidth, uiHeight);
	glutCreateWindow("10420 CS550000 CG HW4 TA");

	glewInit();
	if(glewIsSupported("GL_VERSION_2_0")){
		printf("Ready for OpenGL 2.0\n");
	}else{
		printf("OpenGL 2.0 not supported\n");
		system("pause");
		exit(1);
	}

	glutDisplayFunc(onDisplay);
	glutIdleFunc(onDisplay);
	glutReshapeFunc(onWindowReshape);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMouseMotion);
	glutKeyboardFunc(onKeyboard);
	glutSpecialFunc(onKeyboardSpecial);

	// init
	preTranslateX = preTranslateY = 0;
	scale = 1.0;
	currentModel = 0;
	loadModel(currentModel);
	setShaders();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glutMainLoop();

	glmDelete(OBJ);
	return 0;
}

