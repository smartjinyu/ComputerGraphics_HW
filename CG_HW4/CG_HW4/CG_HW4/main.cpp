/* Computer Graphics 2017
 * NTHU CS CGV Lab
 * Last-modified: 5/24, 2017
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
#include<iostream>
#include<vector>
#include "Vec3.h"
using namespace std;
#define MAX_TEXTURE_NUM 50
#define MAXSIZE 300000
#define MAXGROUPSIZE 50
#define myMax(a,b) (((a)>(b))?(a):(b))

#pragma warning (disable: 4996)

#ifndef GLUT_WHEEL_UP
# define GLUT_WHEEL_UP   0x0003
# define GLUT_WHEEL_DOWN 0x0004
#endif

int isModelLoading = 0;
int isAutoRotating = 0;

int currentModel = 0;
#define NUM_OF_MODEL 15
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
// textures
GLuint texNum[MAX_TEXTURE_NUM];
unsigned char image[MAX_TEXTURE_NUM][MAXSIZE]; //image data array 
FileHeader FH[MAX_TEXTURE_NUM]; //BMP FileHeader 
InfoHeader IH[MAX_TEXTURE_NUM]; //BMP InfoHeader
int isTextureMapping = 1;
GLint iLocTexCoord;
GLint texture_wrap_mode = GL_REPEAT;
GLint texture_mag_filter = GL_LINEAR;
GLint texture_min_filter = GL_LINEAR;
int MAG_F = 0; // 0 nearest, 1 linear
int MIN_F = 0; // 0 nearest, 1 linear
int W_F = 0; // 0 clamp, 1 repeat
// window size
const unsigned int uiWidth = 500;
const unsigned int uiHeight = 500;
int screenWidth;
int screenHeight;

// eye position
GLfloat eyeVec[3] = { 0.0f, 0.0f, 5.0f };

// option keys
#define PROJECTION_PERS 0x01
#define PROJECTION_PARA 0x02
char projectionMode = PROJECTION_PERS;

// projection matrix parameters
const float xmin = -1.0, xmax = 1.0;
const float ymin = -1.0, ymax = 1.0;
const float znear = eyeVec[2] - sqrtf(3.0f), zfar = eyeVec[2] + sqrtf(3.0f);

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

GLint iLocIsTextureMapping;

GLint iLocPosition;
GLint iLocColour;
GLint iLocMVP;
GLint iLocNormal;
GLint iLoceyepos;
GLint iLightcount;
GLint iView, iProjection, iModel;
GLint iRotate;
GLint iPV;

GLint iModelKA, iModelKD, iModelKS;
GLint iModelShininess;


typedef struct LightLocate
{
	GLint iLightPosition;
	GLint iLightKA;
	GLint iLightKD;
	GLint iLightKS;

	GLint ispotDirection;

	GLint ispotExponent;
	GLint ispotCutoff;
	GLint ispotCosCutoff;
	GLint iconstantAttenuation;
	GLint ilinearAttenuation;
	GLint iquadraticAttenuation;
	GLint ilighttype;
	GLint ienable;
} LightLoc;
LightLoc m_LightLocate[3];

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
float vnormals[MAXGROUPSIZE][MAXSIZE];
float vtextures[MAXGROUPSIZE][MAXSIZE];

float *KA, *KD, *KS;
vector<float*> *KAs, *KDs, *KSs;
vector<float> Shin;
int PV_flag = 0;

// light parameters
typedef struct Light_Parm
{
	int m_light_type;


	Vec3 m_Position;

	Vec3 ambient;
	Vec3 diffuse;
	Vec3 specular;

	Vec3 spotDirection;

	int	Point_Light_Range;

	float m_spotExponent;
	float m_spotCutoff;
	float m_spotCosCutoff;
	int light_type;
	int enable;
} Light;
Light Dir, Spot, Pos;

float aMVP[16];

GLMmodel* OBJ;
GLfloat normalScale;
GLfloat scale;

// unpack bmp file
void LoadTextures(char* filename, int index)
{
	unsigned long size;
	char temp;
	FILE *file = fopen(filename, "rb");
	fread(&FH[index], sizeof(FileHeader), 1, file);
	fread(&IH[index], sizeof(InfoHeader), 1, file);
	size = IH[index].Width * IH[index].Height * 3; // IH.Width * IH.Height * 3
	//image[index] = (unsigned char*)malloc(size*sizeof(char));
	fread(image[index], size * sizeof(char), 1, file);
	fclose(file);

	//Swap channel => BRG----->RGB
	for (unsigned int i = 0; i < size; i += 3) {
		temp = image[index][i];
		image[index][i] = image[index][i + 2];
		image[index][i + 2] = temp;
	}
}

void initTextures(int index)
{
	glBindTexture(GL_TEXTURE_2D, texNum[index]);

	// TODO: Generate mipmap by using gl function NOT glu function.
	// TODO: A Text2D texture is created there.	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IH[index].Width, IH[index].Height, 0, GL_RGB, GL_UNSIGNED_BYTE, image[index]);
	/*
	* https://learnopengl.com/#!Getting-started/Textures
	* The first argument specifies the texture target; setting this to GL_TEXTURE_2D means this operation will generate a texture on the currently bound texture object at the same target (so any textures bound to targets GL_TEXTURE_1D or GL_TEXTURE_3D will not be affected).
	* The second argument specifies the mipmap level for which we want to create a texture for if you want to set each mipmap level manually, but we'll leave it at the base level which is 0.
	* The third argument tells OpenGL in what kind of format we want to store the texture. Our image has only RGB values so we'll store the texture with RGB values as well.
	* The 4th and 5th argument sets the width and height of the resulting texture. We stored those earlier when loading the image so we'll use the corresponding variables.
	* The next argument should always be 0 (some legacy stuff).
	* The 7th and 8th argument specify the format and datatype of the source image. We loaded the image with RGB values and stored them as chars (bytes) so we'll pass in the corresponding values.
	* The last argument is the actual image data.
	*/
	glGenerateMipmap(GL_TEXTURE_2D);
	// glBindTexture(GL_TEXTURE_2D, 0); // unbind texture

	// Hint: texture width x height = IH[index].Width x IH[index].Height
	//       image[index] means the texture image material used by which group
}

void setTexture()
{
	int tex_count = 0;
	GLMgroup* group = OBJ->groups;
	while (group) {
		if (strlen(OBJ->materials[group->material].textureImageName) != 0) {
			char TexNameStr[200];
			sprintf(TexNameStr, "TextureModels/%s", OBJ->materials[group->material].textureImageName);
			printf("read %s\n", TexNameStr);
			LoadTextures(TexNameStr, tex_count);
		}
		tex_count++;
		group = group->next;
	}
	glGenTextures(tex_count, texNum);
	for (int i = 0; i < tex_count; i++) {
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
	for (unsigned int i = 2; i <= OBJ->numvertices; i++) {
		GLfloat vx, vy, vz;
		vx = OBJ->vertices[i * 3 + 0];
		vy = OBJ->vertices[i * 3 + 1];
		vz = OBJ->vertices[i * 3 + 2];
		if (vx > maxx) maxx = vx;  if (vx < minx) minx = vx;
		if (vy > maxy) maxy = vy;  if (vy < miny) miny = vy;
		if (vz > maxz) maxz = vz;  if (vz < minz) minz = vz;
	}
	//printf("max\n%f %f, %f %f, %f %f\n", maxx, minx, maxy, miny, maxz, minz);
	dx = maxx - minx;
	dy = maxy - miny;
	dz = maxz - minz;
	//printf("dx,dy,dz = %f %f %f\n", dx, dy, dz);
	normalScale = myMax(myMax(dx, dy), dz) / 2;
	OBJ->position[0] = (maxx + minx) / 2;
	OBJ->position[1] = (maxy + miny) / 2;
	OBJ->position[2] = (maxz + minz) / 2;

	int gCount = 0;
	while (group) {
		KA = OBJ->materials[group->material].ambient;
		KD = OBJ->materials[group->material].diffuse;
		KS = OBJ->materials[group->material].specular;
		KAs->push_back(KA);
		KDs->push_back(KD);
		KSs->push_back(KS);
		Shin.push_back(5.0f);
		for (unsigned int i = 0; i < group->numtriangles; i++) {
			// triangle index
			int triangleID = group->triangles[i];

			// the index of each vertex
			int indv1 = OBJ->triangles[triangleID].vindices[0];
			int indv2 = OBJ->triangles[triangleID].vindices[1];
			int indv3 = OBJ->triangles[triangleID].vindices[2];

			// vertices
			GLfloat vx, vy, vz;
			vx = OBJ->vertices[indv1 * 3];
			vy = OBJ->vertices[indv1 * 3 + 1];
			vz = OBJ->vertices[indv1 * 3 + 2];
			//printf("%f %f %f\n", vx, vy, vz);
			vertices[gCount][i * 9 + 0] = vx;
			vertices[gCount][i * 9 + 1] = vy;
			vertices[gCount][i * 9 + 2] = vz;
			//printf("%f %f %f\n",vertices[i*9], vertices[i*9+1], vertices[i*9+2]);
			vx = OBJ->vertices[indv2 * 3];
			vy = OBJ->vertices[indv2 * 3 + 1];
			vz = OBJ->vertices[indv2 * 3 + 2];
			//printf("%f %f %f\n", vx, vy, vz);
			vertices[gCount][i * 9 + 3] = vx;
			vertices[gCount][i * 9 + 4] = vy;
			vertices[gCount][i * 9 + 5] = vz;
			//printf("%f %f %f\n",vertices[i*9+3], vertices[i*9+4], vertices[i*9+5]);
			vx = OBJ->vertices[indv3 * 3];
			vy = OBJ->vertices[indv3 * 3 + 1];
			vz = OBJ->vertices[indv3 * 3 + 2];
			//printf("%f %f %f\n", vx, vy, vz);
			vertices[gCount][i * 9 + 6] = vx;
			vertices[gCount][i * 9 + 7] = vy;
			vertices[gCount][i * 9 + 8] = vz;
			//printf("%f %f %f\n",vertices[i*9+6], vertices[i*9+7], vertices[i*9+8]);
			//printf("\n");

			int indn1 = OBJ->triangles[triangleID].nindices[0];
			int indn2 = OBJ->triangles[triangleID].nindices[1];
			int indn3 = OBJ->triangles[triangleID].nindices[2];
			vnormals[gCount][i * 9] = OBJ->normals[indn1 * 3];
			vnormals[gCount][i * 9 + 1] = OBJ->normals[indn1 * 3 + 1];
			vnormals[gCount][i * 9 + 2] = OBJ->normals[indn1 * 3 + 2];
			vnormals[gCount][i * 9 + 3] = OBJ->normals[indn2 * 3];
			vnormals[gCount][i * 9 + 4] = OBJ->normals[indn2 * 3 + 1];
			vnormals[gCount][i * 9 + 5] = OBJ->normals[indn2 * 3 + 2];
			vnormals[gCount][i * 9 + 6] = OBJ->normals[indn3 * 3];
			vnormals[gCount][i * 9 + 7] = OBJ->normals[indn3 * 3 + 1];
			vnormals[gCount][i * 9 + 8] = OBJ->normals[indn3 * 3 + 2];

			int indt1 = OBJ->triangles[triangleID].tindices[0];
			int indt2 = OBJ->triangles[triangleID].tindices[1];
			int indt3 = OBJ->triangles[triangleID].tindices[2];

			//cout<<OBJ->texcoords[indt1*2]<<" "<<OBJ->texcoords[indt1*2+1]<<" "<<OBJ->texcoords[indt1*2+2]<<" "<<OBJ->texcoords[indt1*2+3]<<" "<<OBJ->texcoords[indt1*2+4]<<" "<<OBJ->texcoords[indt1*2+5]<<endl;
			// TODO: texture coordinates should be aligned by yourself
			vtextures[gCount][i * 9] = OBJ->texcoords[indt1 * 3];
			vtextures[gCount][i * 9 + 1] = OBJ->texcoords[indt1 * 3 + 1];
			vtextures[gCount][i * 9 + 2] = OBJ->texcoords[indt1 * 3 + 2];
			vtextures[gCount][i * 9 + 3] = OBJ->texcoords[indt2 * 3];
			vtextures[gCount][i * 9 + 4] = OBJ->texcoords[indt2 * 3 + 1];
			vtextures[gCount][i * 9 + 5] = OBJ->texcoords[indt2 * 3 + 2];
			vtextures[gCount][i * 9 + 6] = OBJ->texcoords[indt3 * 3];
			vtextures[gCount][i * 9 + 7] = OBJ->texcoords[indt3 * 3 + 1];
			vtextures[gCount][i * 9 + 8] = OBJ->texcoords[indt3 * 3 + 2];

		}
		group = group->next;
		gCount++;
	}

	// custom normalization matrix for each loaded model
	normalization_matrix.identity();
	normalization_matrix = normalization_matrix * myTranslateMatrix(-OBJ->position[0], -OBJ->position[1], -OBJ->position[2]);
	normalization_matrix = normalization_matrix * myScaleMatrix(1 / normalScale, 1 / normalScale, 1 / normalScale);
}

void loadOBJModel(int index)
{
	// load model through glm parser
	OBJ = glmReadOBJ(filename[index]);
	printf("%s\n", filename[index]);
	KAs = new std::vector<float*>();
	KDs = new std::vector<float*>();
	KSs = new std::vector<float*>();
	// uncomment them only if there is no vertex normal info in your .obj file
	// glm will calculate the vertex normals for you
	glmFacetNormals(OBJ);
	glmVertexNormals(OBJ, 90.0);

	// parse texture model and align the vertices
	TextureModel();
}

void setViewPortMatrix()
{
	glViewport(0, 0, screenWidth, screenHeight);
	viewPort.identity();
	float ratio = (float)screenWidth / (float)screenHeight;
	if (ratio >= 1) {
		viewPort = viewPort * myScaleMatrix(1.0f / ratio, 1, 1);
	}
	else {
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

// on window size changed callback
void changeSize(int w, int h)
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
	m[10] = zz * (1 - c) + c;
	m[11] = 0;
	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;

	return m;
}
Matrix4 myRotate_X(float angle)
{
	float c = cosf(angle / 180.0f * M_PI);    // cos
	float s = sinf(angle / 180.0f * M_PI);    // sin


	Matrix4 m;
	m[0] = 1;
	m[1] = 0;
	m[2] = 0;
	m[3] = 0;

	m[4] = 0;
	m[5] = c;
	m[6] = s;
	m[7] = 0;

	m[8] = 0;
	m[9] = -1 * s;
	m[10] = c;
	m[11] = 0;

	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;

	return m;
}
Matrix4 myRotate_Y(float angle)
{
	float c = cosf(angle / 180.0f * M_PI);    // cos
	float s = sinf(angle / 180.0f * M_PI);    // sin


	Matrix4 m;
	m[0] = c;
	m[1] = 0;
	m[2] = -1 * s;
	m[3] = 0;

	m[4] = 0;
	m[5] = 1;
	m[6] = 0;
	m[7] = 0;

	m[8] = s;
	m[9] = 0;
	m[10] = c;
	m[11] = 0;

	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;

	return m;
}
Matrix4 myRotate_Z(float angle)
{
	float c = cosf(angle / 180.0f * M_PI);    // cos
	float s = sinf(angle / 180.0f * M_PI);    // sin


	Matrix4 m;
	m[0] = c;
	m[1] = s;
	m[2] = 0;
	m[3] = 0;

	m[4] = -1 * s;
	m[5] = c;
	m[6] = 0;
	m[7] = 0;

	m[8] = 0;
	m[9] = 0;
	m[10] = 1;
	m[11] = 0;

	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;

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
	return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
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
	n[0] = u[1] * v[2] - u[2] * v[1];
	n[1] = u[2] * v[0] - u[0] * v[2];
	n[2] = u[0] * v[1] - u[1] * v[0];
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
static GLvoid
glmNormalize(GLfloat* v)
{
	GLfloat l;
	l = (GLfloat)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
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
	float E[3] = { eyex, eyey, eyez };
	float L[3] = { cx - eyex, cy - eyey, cz - eyez };
	glmNormalize(L);
	float U[3] = { upx, upy, upz };
	glmNormalize(U);
	float S[3];
	glmCross(L, U, S);
	glmNormalize(S);
	glmCross(S, L, U);

	Matrix4 m;
	m[0] = S[0];
	m[1] = S[1];
	m[2] = S[2];
	m[3] = 0;
	m[4] = U[0];
	m[5] = U[1];
	m[6] = U[2];
	m[7] = 0;
	m[8] = -L[0];
	m[9] = -L[1];
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
	mat[0] = 2 * n / (r - l);
	mat[1] = 0;
	mat[2] = 0;
	mat[3] = 0;

	mat[4] = 0;
	mat[5] = 2 * n / (t - b);
	mat[6] = 0;
	mat[7] = 0;

	mat[8] = (r + l) / (r - l);
	mat[9] = (t + b) / (t - b);
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
	float tangent = tanf(fovY / 2.0f * 180.0f / M_PI); // tangent of half fovY
	float height = front * tangent;         // half height of near plane
	float width = height * aspect;          // half width of near plane

	// params: left, right, bottom, top, near, far
	return myFrustumMatrix(-width, width, -height, height, front, back);
}

Matrix4 myParallelProjectionMatrix(float l, float r, float b, float t, float n, float f)
{
	Matrix4 mat;
	mat[0] = 2 / (r - l);
	mat[1] = 0;
	mat[2] = 0;
	mat[3] = 0;

	mat[4] = 0;
	mat[5] = 2 / (t - b);
	mat[6] = 0;
	mat[7] = 0;

	mat[8] = 0;
	mat[9] = 0;
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
void SetUpLight(Light L, LightLoc m_Light_Locate)
{
	float p[4] = { L.m_Position.x,L.m_Position.y,L.m_Position.z,1.0f };
	float a[4] = { L.ambient.x,L.ambient.y,L.ambient.z,1.0f };
	float d[4] = { L.diffuse.x,L.diffuse.y,L.diffuse.z,1.0f };
	float s[4] = { L.specular.x,L.specular.y,L.specular.z,1.0f };
	float spotd[3] = { L.spotDirection.x,L.spotDirection.y,L.spotDirection.z };

	glUniform4fv(m_Light_Locate.iLightPosition, 1, p);
	glUniform4fv(m_Light_Locate.iLightKA, 1, a);
	glUniform4fv(m_Light_Locate.iLightKD, 1, d);
	glUniform4fv(m_Light_Locate.iLightKS, 1, s);

	glUniform1i(m_Light_Locate.ilighttype, L.light_type);
	glUniform1i(m_Light_Locate.ienable, L.enable);

	float m_CA = 0.1f;
	float m_LA = 4.5f / (float)L.Point_Light_Range;
	float m_QA = 75.0f / ((float)L.Point_Light_Range*(float)L.Point_Light_Range);

	glUniform1f(m_Light_Locate.iconstantAttenuation, m_CA);
	glUniform1f(m_Light_Locate.ilinearAttenuation, m_LA);
	glUniform1f(m_Light_Locate.iquadraticAttenuation, m_QA);


	glUniform3fv(m_Light_Locate.ispotDirection, 1, spotd);

	glUniform1f(m_Light_Locate.ispotCosCutoff, L.m_spotCosCutoff);
	glUniform1f(m_Light_Locate.ispotCutoff, L.m_spotCutoff);
	glUniform1f(m_Light_Locate.ispotExponent, L.m_spotExponent);
}
void renderScene(void)
{
	while (isModelLoading == 1) {
		//printf("waiting\n");
	}
	Matrix4 M;

	glClearColor(0.3f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// aMVP = identity() * M_normalize(Tn*Sn) * M_transformation(S or R or T) * V(lookat) * P(projection)

	M.identity();

	// normalize
	M = M * normalization_matrix;

	// previous rotate matrix
	M = M * preM;

	// mouse rotate
	if (isRBtnDown && isMouseMoving) {
		float temp = sqrt(rotateX * rotateX + rotateY * rotateY);
		if (temp != 0) {
			Matrix4 newM = myRotateMatrix(-temp / 4.0f, rotateY / temp, rotateX / temp, 0.0f);
			// divided by 4 in order to rotate slower			
			M = M * newM;
			preM = preM * newM;
		}
		/*Matrix4 rot_X = myRotate_X(rotateY);
		Matrix4 rot_Y = myRotate_Y(rotateX);
		M = rot_X * M;
		M = rot_Y * M;
		preM = preM * rot_X;
		preM = preM * rot_Y;*/
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
	if (isMouseMoving && isLBtnDown) {
		float ratio = (float)screenWidth / (float)screenHeight;
		float alphaX = (xmax - xmin) / (float)screenWidth;
		float alphaY = (ymax - ymin) / (float)screenHeight;
		if (ratio >= 1) {
			alphaX *= ratio;
		}
		else {
			alphaY /= ratio;
		}
		M = M * myTranslateMatrix(translateX*alphaX, translateY*alphaY, 0);
		preTranslateX += translateX * alphaX;
		preTranslateY += translateY * alphaY;
	}

	Matrix4 modelMatrix;
	// Model matrix ends here
	for (int i = 0; i < 16; i++) {
		modelMatrix[i] = M[i];
	}

	// V (lookat)
	M = M * myLookAtMatrix(eyeVec[0], eyeVec[1], eyeVec[2], eyeVec[0], eyeVec[1], eyeVec[2] - 1.0f, 0, 1, 0);

	// P (project)
	if (projectionMode == PROJECTION_PARA)
	{
		M = M * parallelProjectionMatrix;
	}
	else if (projectionMode == PROJECTION_PERS)
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

	for (int i = 0; i < 16; i++) aMVP[i] = M[i];
	glUniform1i(iLightcount, 3);
	glUniform3fv(iLoceyepos, 1, eyeVec);
	SetUpLight(Dir, m_LightLocate[0]);
	SetUpLight(Pos, m_LightLocate[1]);
	SetUpLight(Spot, m_LightLocate[2]);
	glUniform1i(iLocIsTextureMapping, isTextureMapping);

	double nx, ny, nz;
	nx = M[12];
	ny = M[13];
	nz = M[14];
	//cout << nx << "," << ny << "," <<nz << endl;

	// Matrices
	glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, M.get());
	glUniformMatrix4fv(iLocModelMatrix, 1, GL_FALSE, modelMatrix.get());
	glUniformMatrix4fv(iRotate, 1, GL_FALSE, preM.get());

	GLMgroup* group = OBJ->groups;
	int gCount = 0;
	while (group) {

		KA = KAs->at(gCount);
		KD = KDs->at(gCount);
		KS = KDs->at(gCount);
		float Sns = Shin.at(gCount);

		glUniform4fv(iModelKA, 1, KA);
		glUniform4fv(iModelKD, 1, KD);
		glUniform4fv(iModelKS, 1, KS);
		glUniform1f(iModelShininess, Sns);
		glUniform1i(iPV, PV_flag);
		// enable attributes array
		glEnableVertexAttribArray(iLocPosition);
		glEnableVertexAttribArray(iLocNormal);
		// TODO: texture VertexAttribArray is enabled here		
		glEnableVertexAttribArray(iLocTexCoord);

		// bind attributes array
		glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, vertices[gCount]);
		glVertexAttribPointer(iLocNormal, 3, GL_FLOAT, GL_FALSE, 0, vnormals[gCount]);
		// TODO: bind texture vertex attribute pointer here
		glVertexAttribPointer(iLocTexCoord, 3, GL_FLOAT, GL_FALSE, 0, vtextures[gCount]);
		// texture mag/min filter
		// TODO: texture mag/min filters are defined here
		if (MAG_F) {
			// MAG_F == 1
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		if (MIN_F) {
			// MIN_F == 1
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}


		// texture wrap mode s/t
		// TODO: texture wrap modes are defined here
		if (W_F) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}

		// bind texture material group by group
		// TODO: bind texture here

		// draw arrays
		glDrawArrays(GL_TRIANGLES, 0, group->numtriangles * 3);

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
	printf("Light:             press 1 to turn on/off directional light\n");
	printf("                   press 2 to turn on/off point light\n");
	printf("                   press 3 to turn on/off spot light\n");
	printf("PROJECTION:        press P / p to switch perspective / orthographic projection\n");
	printf("\n");
	printf("TRANSLATION:       press the left button and drag the mouse to translate\n");
	printf("\n");
	printf("ROTATION:          press the R / r to auto-rotate \n");
	printf("\n");
	printf("SCALE:             scroll mouse wheel up and down to scale\n");
	printf("\n");
	printf("MODEL SWITCHING:   press Z / z to switch previous model.\n");
	printf("                   press X / x to switch next model.\n");
	printf("Texture SWITCHING: press T / t to switch wrap mode.\n");
	printf("Render SWITCHING:  press V / v to switch per-pixel/vertex rendering.\n");
	printf("WRAP  SWITCHING:   press W / w to switch wrap mode.\n");
	printf("MAG   SWITCHING:   press M to switch Mag_filter mode.\n");
	printf("MIN   SWITCHING:   press m to switch Min_filter mode.\n");

	printf("\n");
}

void processSpecialKeys(int key, int x, int y) {
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

void processNormalKeys(unsigned char key, int x, int y) {
	int modifier = glutGetModifiers();
	//printf("modifier = %d\n", modifier);
	//printf("key = %d\n", key);
	switch (modifier) {
	case GLUT_ACTIVE_CTRL: {
		switch (key) {
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
	switch (key) {
	case '1':
		Dir.enable = !Dir.enable;
		break;
	case '2':
		Pos.enable = !Pos.enable;
		break;
	case '3':
		Spot.enable = !Spot.enable;
		break;
	case 'v': case 'V':
		PV_flag = !PV_flag;
		break;
	case 'w':case 'W':
		W_F = !W_F;
		if (W_F) {
			printf("Texture wrap mode has been switched to repeat\n");
		}
		else {
			printf("Texture wrap mode has been switched to clamp\n");

		}
		break;
	case 'M':
		MAG_F = !MAG_F;
		if (MAG_F) {
			printf("Texture filter magnify mode has been switched to linear\n");
		}
		else {
			printf("Texture filter magnify mode has been switched to nearest\n");
		}
		break;
	case 'm':
		MIN_F = !MIN_F;
		if (MIN_F) {
			printf("Texture filter minify mode has been switched to linear\n");
		}
		else {
			printf("Texture filter minify mode has been switched to nearest\n");
		}

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
	case 'R': case 'r':
		isAutoRotating = !isAutoRotating;
		break;
	case 'P': case 'p':
		if (projectionMode != PROJECTION_PERS) {
			projectionMode = PROJECTION_PERS;
			printf("perspective projection\n");
		}
		else if (projectionMode != PROJECTION_PARA) {
			projectionMode = PROJECTION_PARA;
			printf("parallel projection\n");
		}
		break;
	case 't': case 'T':
		isTextureMapping = !isTextureMapping;
		break;
	case 27: // esc key
		exit(0);
	}
	glutPostRedisplay();
}

void setShaders() {
	char *vs = NULL, *fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("shader.vert");
	fs = textFileRead("shader.frag");


	const char * ff = fs;
	const char * vv = vs;

	glShaderSource(v, 1, &vv, NULL);
	glShaderSource(f, 1, &ff, NULL);

	free(vs); free(fs);

	glCompileShader(v);
	glCompileShader(f);

	GLint vCompiled;
	glGetShaderiv(v, GL_COMPILE_STATUS, &vCompiled);
	if (!vCompiled) {
		GLint length;
		GLchar* log;

		glGetShaderiv(v, GL_INFO_LOG_LENGTH, &length);

		log = (GLchar*)malloc(length);
		glGetShaderInfoLog(v, length, &length, log);
		fprintf(stderr, "[v] compile log = '%s'\n", log);
		free(log);
	}

	GLint fCompiled;
	glGetShaderiv(f, GL_COMPILE_STATUS, &fCompiled);
	if (!fCompiled) {
		GLint length;
		GLchar* log;

		glGetShaderiv(f, GL_INFO_LOG_LENGTH, &length);

		log = (GLchar*)malloc(length);
		glGetShaderInfoLog(f, length, &length, log);
		fprintf(stderr, "[f] compile log = '%s'\n", log);
		free(log);
	}

	p = glCreateProgram();
	glAttachShader(p, f);
	glAttachShader(p, v);

	glLinkProgram(p);
	GLint linked = 0;
	glGetProgramiv(p, GL_LINK_STATUS, &linked);
	if (linked) {
		glUseProgram(p);
	}
	else {
		GLint length;
		GLchar* log;
		glGetProgramiv(p, GL_INFO_LOG_LENGTH, &length);
		log = (GLchar*)malloc(length);
		glGetProgramInfoLog(p, length, &length, log);
		fprintf(stderr, "link log = '%s'\n", log);
		free(log);
	}

	iLocPosition = glGetAttribLocation(p, "av4position");
	iLocColour = glGetAttribLocation(p, "av3colour");
	iLocTexCoord = glGetAttribLocation(p, "av2texCoord");

	iLocIsTextureMapping = glGetUniformLocation(p, "uiisTextureMapping");
	iLocModelMatrix = glGetUniformLocation(p, "um4modelMatrix");
	iLocNormal = glGetAttribLocation(p, "av3normal");
	iRotate = glGetUniformLocation(p, "um4rotateMatrix");
	iProjection = glGetUniformLocation(p, "Proj");
	iLocMVP = glGetUniformLocation(p, "mvp");
	iLightcount = glGetUniformLocation(p, "Lightcount");
	iModelKA = glGetUniformLocation(p, "Material.ambient");
	iModelKD = glGetUniformLocation(p, "Material.diffuse");
	iModelKS = glGetUniformLocation(p, "Material.specular");
	iModelShininess = glGetUniformLocation(p, "Material.shininess");
	iLoceyepos = glGetUniformLocation(p, "eyepos");
	iPV = glGetUniformLocation(p, "PV_FLAG");
	for (int i = 0; i < 3; i++)
	{
		char* s1 = "LightSource[";
		char s2[256];
		char s3[256];

		sprintf(s2, "%s%d", s1, i);

		sprintf(s3, "%s%s", s2, "].position");
		m_LightLocate[i].iLightPosition = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].ambient");
		m_LightLocate[i].iLightKA = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].diffuse");
		m_LightLocate[i].iLightKD = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].specular");
		m_LightLocate[i].iLightKS = glGetUniformLocation(p, s3);


		sprintf(s3, "%s%s", s2, "].constantAttenuation");
		m_LightLocate[i].iconstantAttenuation = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].linearAttenuation");
		m_LightLocate[i].ilinearAttenuation = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].quadraticAttenuation");
		m_LightLocate[i].iquadraticAttenuation = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].lighttype");
		m_LightLocate[i].ilighttype = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].enable");
		m_LightLocate[i].ienable = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].spotDirection");
		m_LightLocate[i].ispotDirection = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].spotCutoff");
		m_LightLocate[i].ispotCutoff = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].spotExponent");
		m_LightLocate[i].ispotExponent = glGetUniformLocation(p, s3);

		sprintf(s3, "%s%s", s2, "].spotCosCutoff");
		m_LightLocate[i].ispotCosCutoff = glGetUniformLocation(p, s3);

	}

	glUseProgram(p);
}

void processMouse(int who, int state, int x, int y) {
	if (who == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && isLBtnDown == 0)
	{
		rotateStartX = (float)x;
		rotateStartY = (float)y;
		isRBtnDown = 1;
	}
	else if (who == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		isRBtnDown = 0;
	}
	else if (who == GLUT_LEFT_BUTTON && state == GLUT_DOWN && isRBtnDown == 0)
	{
		translateX = x - translateStartX;
		translateY = -(y - translateStartY);
		translateStartX = (float)x;
		translateStartY = (float)y;
		isLBtnDown = 1;
	}
	else if (who == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		isLBtnDown = 0;
	}
	else if (who == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{
		scale = 1.0f;
	}
	else if (who == GLUT_WHEEL_UP && state == GLUT_DOWN)
	{
		if (scale < 2.29) scale *= 1.05f;
	}
	else if (who == GLUT_WHEEL_DOWN && state == GLUT_DOWN)
	{
		if (scale > 0.25) scale /= 1.05f;
	}
}

// callback on mouse drag
void myMotionFunc(int x, int y)
{
	if (isRBtnDown) {
		rotateX = x - rotateStartX;
		rotateY = y - rotateStartY;
		rotateStartX = (float)x;
		rotateStartY = (float)y;
	}
	if (isLBtnDown) {
		translateX = x - translateStartX;
		translateY = -(y - translateStartY);
		translateStartX = (float)x;
		translateStartY = (float)y;
	}
	isMouseMoving = 1;
}
void glPrintContextInfo(bool printExtension)
{
	cout << "GL_VENDOR = " << (const char*)glGetString(GL_VENDOR) << endl;
	cout << "GL_RENDERER = " << (const char*)glGetString(GL_RENDERER) << endl;
	cout << "GL_VERSION = " << (const char*)glGetString(GL_VERSION) << endl;
	cout << "GL_SHADING_LANGUAGE_VERSION = " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	if (printExtension)
	{
		GLint numExt;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
		cout << "GL_EXTENSIONS =" << endl;
		for (GLint i = 0; i < numExt; i++)
		{
			cout << "\t" << (const char*)glGetStringi(GL_EXTENSIONS, i) << endl;
		}
	}
}
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(uiWidth, uiHeight);
	glutCreateWindow("CS550000 CGHW4 TA");
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(processNormalKeys);
	glutMouseFunc(processMouse);
	glutMotionFunc(myMotionFunc);
	glutSpecialFunc(processSpecialKeys);

	glewInit();
	if (glewIsSupported("GL_VERSION_2_0")) {
		printf("Ready for OpenGL 2.0\n");
		showHelp();
	}
	else {
		printf("OpenGL 2.0 not supported\n");
		system("pause");
		exit(123);
	}
	// init
	preTranslateX = preTranslateY = 0;
	scale = 1.0;
	currentModel = 0;
	loadModel(currentModel);
	Dir.diffuse = Vec3(1.0f, 1.0f, 1.0f);

	Dir.m_Position = Vec3(0.0, 0.0, 2.0);

	Dir.ambient = Vec3(1.0f, 1.0f, 1.0f);
	Dir.specular = Vec3(0.1f, 0.1f, 0.1f);
	Dir.spotDirection = Vec3(0.0f, 0.0f, -1.0f);
	Dir.Point_Light_Range = 45;
	Dir.light_type = 0;
	Dir.enable = 0;
	Dir.m_spotExponent = 0.1f;
	Dir.m_spotCutoff = 45.0f;
	Dir.m_spotCosCutoff = 0.9f;

	Pos.diffuse = Vec3(1.0f, 1.0f, 1.0f);

	Pos.m_Position = Vec3(0.0, 0.0, 2.0);

	Pos.ambient = Vec3(0.1f, 0.1f, 0.1f);
	Pos.specular = Vec3(0.1f, 0.1f, 0.1f);
	Pos.spotDirection = Vec3(0.0f, 0.0f, -1.0f);
	Pos.Point_Light_Range = 45;
	Pos.light_type = 1;
	Pos.enable = 0;
	Pos.m_spotExponent = 0.1f;
	Pos.m_spotCutoff = 45.0f;
	Pos.m_spotCosCutoff = 0.9f;

	Spot.diffuse = Vec3(1.0f, 1.0f, 1.0f);

	Spot.m_Position = Vec3(0.0, 0.0, 2.0);

	Spot.ambient = Vec3(0.1f, 0.1f, 0.1f);
	Spot.specular = Vec3(0.1f, 0.1f, 0.1f);
	Spot.spotDirection = Vec3(0.0f, 0.0f, -1.0f);
	Spot.Point_Light_Range = 45;
	Spot.light_type = 2;
	Spot.enable = 0;
	Spot.m_spotExponent = 2.0f;
	Spot.m_spotCutoff = 70.0f;
	Spot.m_spotCosCutoff = 0.9f;

	setShaders();
	glPrintContextInfo(false);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glutMainLoop();

	glmDelete(OBJ);
	return 0;
}

