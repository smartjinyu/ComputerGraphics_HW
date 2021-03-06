#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <freeglut/glut.h>
#include "textfile.h"
#include "glm.h"

#include "Matrices.h"

#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "freeglut.lib")

#ifndef GLUT_WHEEL_UP
# define GLUT_WHEEL_UP   0x0003
# define GLUT_WHEEL_DOWN 0x0004
#endif

#ifndef GLUT_KEY_ESC
# define GLUT_KEY_ESC 0x001B
#endif

#ifndef GLUT_KEY_z
# define GLUT_KEY_z 0x007A
#endif

#ifndef GLUT_KEY_x
# define GLUT_KEY_x 0x0078
#endif

#ifndef GLUT_KEY_h
# define GLUT_KEY_h 0x0068
#endif

#ifndef GLUT_KEY_q
# define GLUT_KEY_q 0x0071
#endif

#ifndef GLUT_KEY_w
# define GLUT_KEY_w 0x0077
#endif

#ifndef GLUT_KEY_e
# define GLUT_KEY_e 0x0065
#endif

#ifndef GLUT_KEY_a
# define GLUT_KEY_a 0x0061
#endif

#ifndef GLUT_KEY_s
# define GLUT_KEY_s 0x0073
#endif

#ifndef GLUT_KEY_d
# define GLUT_KEY_d 0x0064
#endif

#ifndef GLUT_KEY_r
# define GLUT_KEY_r 0x0072
#endif

#ifndef GLUT_KEY_v
# define GLUT_KEY_v 0x0076
#endif

#ifndef GLUT_KEY_b
# define GLUT_KEY_b 0x0062
#endif

#ifndef GLUT_KEY_c
# define GLUT_KEY_c 0x0063
#endif

#ifndef GLUT_KEY_f
# define GLUT_KEY_f 0x0066
#endif


#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif


// Shader attributes
GLint iLocPosition;
GLint iLocNormal;
GLint iLocMVP;

GLint iLocMDiffuse, iLocMAmbient, iLocMSpecular, iLocMShininess;
GLint iLocAmbientOn, iLocDiffuseOn, iLocSpecularOn;
GLint iLocDirectionalOn, iLocPointOn, iLocSpotOn;
GLint iLocNormalTransform, iLocModelTransform, iLocViewTransform;
GLint iLocEyePosition;
GLint iLocPointPosition, iLocSpotPosition, iLocSpotExponent, iLocSpotCutoff, iLocSpotCosCutoff;
GLint iLocPerPixelLighting;

#define numOfModels 5
char filename[numOfModels][256] = { "NormalModels/High/dragon10KN.obj",
"NormalModels/Medium/bunny5KN.obj",
"NormalModels/Low/sphereN.obj",
"NormalModels/High/happy10KN.obj",
"NormalModels/High/brain18KN.obj", };

int modelIndex = 0;

GLMmodel* OBJ;
GLfloat* vertices; // index from 0 - 9 * group->numtriangles-1 is group 1's vertices, and so on
GLfloat* normals; // index from 0 - 9 * group->numtriangles-1 is group 1's normals, and so on

Matrix4 N;

float xmin = -1.0, xmax = 1.0, ymin = -1.0, ymax = 1.0, znear = 1.0, zfar = 3.0; // zfar/znear should be positive
Vector3 eyePos = Vector3(0, 0, 2);
Vector3 centerPos = Vector3(0, 0, 0);
Vector3 upVec = Vector3(0, 1, 0);// in fact, this vector should be called as P1P3

int ambientOn = 1, diffuseOn = 1, specularOn = 1;
int directionalOn = 0, pointOn = 0, spotOn = 0;
/*
spotOn = 0, no spot light
spotOn = 1, normal spot light
spotOn = 2, directional spot light
spotOn = 3, point spot light
*/
int perPixelOn = 0; // 1: enable per pixel lighting

int autoRotateMode = 0;
float rotateSpeed = 300.0; // it is the reciprocal of actual rotate speed

int windowHeight, windowWidth;

#define numOfLightSources 4
struct LightSourceParameters {
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float position[4];
	float halfVector[4];
	float spotDirection[3];
	float spotExponent;
	float spotCutoff; // (range: [0.0,90.0], 180.0)
	float spotCosCutoff; // (range: [1.0,0.0],-1.0)
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
}typedef LightSource;
LightSource lightsource[numOfLightSources];


Matrix4 getPerpectiveMatrix() {
	// Zfar > Znear > 0
	// in OpenGL api
	// Xmax = Right, Xmin = Left
	// Ymax = Top, Ymin = Bottom
	// Znear = -Near, Zfar = -far
	return Matrix4(
		2.0*znear / (xmax - xmin), 0, (xmax + xmin) / (xmin - xmax), 0,
		0, 2.0*znear / (ymax - ymin), (ymax + ymin) / (ymin - ymax), 0,
		0, 0, (zfar + znear) / (znear - zfar), (2.0*zfar*znear) / (znear - zfar),
		0, 0, -1, 0
	);
}

Matrix4 getViewTransMatrix() {
	/*
	use global parameter eyePos, centerPos, upVec to compute Viewing Transformation Matrix
	*/
	Vector3 P1P2 = centerPos - eyePos;
	Vector3 P1P3 = upVec;
	Vector3 Rz = P1P2.normalize();
	Vector3 Rx = P1P2.cross(P1P3).normalize();
	Vector3 Ry = Rx.cross(Rz).normalize();

	Matrix4 Rv = Matrix4(
		Rx[0], Rx[1], Rx[2], 0, // new X axis
		Ry[0], Ry[1], Ry[2], 0, // new up vector
		-Rz[0], -Rz[1], -Rz[2], 0,// new direction/forward vector
		0, 0, 0, 1);
	Matrix4 Rt = Matrix4(
		1, 0, 0, -eyePos[0],
		0, 1, 0, -eyePos[1],
		0, 0, 1, -eyePos[2],
		0, 0, 0, 1);
	//std::cout <<"Rv"<< Rv << std::endl;
	//std::cout <<"Rt"<< Rt << std::endl;
	//upVec = Ry;
	return Rv*Rt;
}


void traverseColorModel()
{
	int i;

	GLfloat maxVal[3];
	GLfloat minVal[3];


	// number of triangles
	vertices = (GLfloat*)malloc(sizeof(GLfloat)*OBJ->numtriangles * 9);
	normals = (GLfloat*)malloc(sizeof(GLfloat)*OBJ->numtriangles * 9);

	float max_x = OBJ->vertices[3];
	float max_y = OBJ->vertices[4];
	float max_z = OBJ->vertices[5];
	float min_x = OBJ->vertices[3];
	float min_y = OBJ->vertices[4];
	float min_z = OBJ->vertices[5];

	GLMgroup* group = OBJ->groups;
	int offsetIndex = 0;
	// index from 0 - 9 * group->numtriangles-1 is group 1's vertices and normals, and so on
	while (group) {
		for (int i = 0; i < group->numtriangles; i++) {
			int triangleID = group->triangles[i];
			// the index of each vertex
			int indv1 = OBJ->triangles[triangleID].vindices[0];
			int indv2 = OBJ->triangles[triangleID].vindices[1];
			int indv3 = OBJ->triangles[triangleID].vindices[2];

			// the index of each normal
			int indn1 = OBJ->triangles[triangleID].nindices[0];
			int indn2 = OBJ->triangles[triangleID].nindices[1];
			int indn3 = OBJ->triangles[triangleID].nindices[2];


			// vertices

			vertices[offsetIndex + i * 9 + 0] = OBJ->vertices[indv1 * 3 + 0];
			vertices[offsetIndex + i * 9 + 1] = OBJ->vertices[indv1 * 3 + 1];
			vertices[offsetIndex + i * 9 + 2] = OBJ->vertices[indv1 * 3 + 2];
			if (vertices[offsetIndex + i * 9 + 0] > max_x) max_x = vertices[offsetIndex + i * 9 + 0];
			if (vertices[offsetIndex + i * 9 + 1] > max_y) max_y = vertices[offsetIndex + i * 9 + 1];
			if (vertices[offsetIndex + i * 9 + 2] > max_z) max_z = vertices[offsetIndex + i * 9 + 2];
			if (vertices[offsetIndex + i * 9 + 0] < min_x) min_x = vertices[offsetIndex + i * 9 + 0];
			if (vertices[offsetIndex + i * 9 + 1] < min_y) min_y = vertices[offsetIndex + i * 9 + 1];
			if (vertices[offsetIndex + i * 9 + 2] < min_z) min_z = vertices[offsetIndex + i * 9 + 2];

			vertices[offsetIndex + i * 9 + 3] = OBJ->vertices[indv2 * 3 + 0];
			vertices[offsetIndex + i * 9 + 4] = OBJ->vertices[indv2 * 3 + 1];
			vertices[offsetIndex + i * 9 + 5] = OBJ->vertices[indv2 * 3 + 2];
			if (vertices[offsetIndex + i * 9 + 3] > max_x) max_x = vertices[offsetIndex + i * 9 + 3];
			if (vertices[offsetIndex + i * 9 + 4] > max_y) max_y = vertices[offsetIndex + i * 9 + 4];
			if (vertices[offsetIndex + i * 9 + 5] > max_z) max_z = vertices[offsetIndex + i * 9 + 5];
			if (vertices[offsetIndex + i * 9 + 3] < min_x) min_x = vertices[offsetIndex + i * 9 + 3];
			if (vertices[offsetIndex + i * 9 + 4] < min_y) min_y = vertices[offsetIndex + i * 9 + 4];
			if (vertices[offsetIndex + i * 9 + 5] < min_z) min_z = vertices[offsetIndex + i * 9 + 5];

			vertices[offsetIndex + i * 9 + 6] = OBJ->vertices[indv3 * 3 + 0];
			vertices[offsetIndex + i * 9 + 7] = OBJ->vertices[indv3 * 3 + 1];
			vertices[offsetIndex + i * 9 + 8] = OBJ->vertices[indv3 * 3 + 2];
			if (vertices[offsetIndex + i * 9 + 6] > max_x) max_x = vertices[offsetIndex + i * 9 + 6];
			if (vertices[offsetIndex + i * 9 + 7] > max_y) max_y = vertices[offsetIndex + i * 9 + 7];
			if (vertices[offsetIndex + i * 9 + 8] > max_z) max_z = vertices[offsetIndex + i * 9 + 8];
			if (vertices[offsetIndex + i * 9 + 6] < min_x) min_x = vertices[offsetIndex + i * 9 + 6];
			if (vertices[offsetIndex + i * 9 + 7] < min_y) min_y = vertices[offsetIndex + i * 9 + 7];
			if (vertices[offsetIndex + i * 9 + 8] < min_z) min_z = vertices[offsetIndex + i * 9 + 8];

			// colors

			normals[offsetIndex + i * 9 + 0] = OBJ->normals[indn1 * 3 + 0];
			normals[offsetIndex + i * 9 + 1] = OBJ->normals[indn1 * 3 + 1];
			normals[offsetIndex + i * 9 + 2] = OBJ->normals[indn1 * 3 + 2];

			normals[offsetIndex + i * 9 + 3] = OBJ->normals[indn2 * 3 + 0];
			normals[offsetIndex + i * 9 + 4] = OBJ->normals[indn2 * 3 + 1];
			normals[offsetIndex + i * 9 + 5] = OBJ->normals[indn2 * 3 + 2];

			normals[offsetIndex + i * 9 + 6] = OBJ->normals[indn3 * 3 + 0];
			normals[offsetIndex + i * 9 + 7] = OBJ->normals[indn3 * 3 + 1];
			normals[offsetIndex + i * 9 + 8] = OBJ->normals[indn3 * 3 + 2];

		}
		offsetIndex += 9 * group->numtriangles;
		group = group->next;
		// printf("offsetIndex = %d\n", offsetIndex);
	}



	float normalize_scale = max(max(abs(max_x - min_x), abs(max_y - min_y)), abs(max_z - min_z));

	Matrix4 S, T;
	S.identity();
	T.identity();
	S[0] = 2 / normalize_scale;
	S[5] = 2 / normalize_scale;;
	S[10] = 2 / normalize_scale;
	T[3] = -(min_x + max_x) / 2;
	T[7] = -(min_y + max_y) / 2;
	T[11] = -(min_z + max_z) / 2;

	N = S*T;

}

void loadOBJModel()
{
	// read an obj model here
	if (OBJ != NULL) {
		free(OBJ);
	}
	OBJ = glmReadOBJ(filename[modelIndex]);
	printf("%s\n", filename[modelIndex]);

	glmFacetNormals(OBJ);
	glmVertexNormals(OBJ, 90.0);

	// traverse the color model
	traverseColorModel();
}

void onIdle()
{
	glutPostRedisplay();
}

void passMatrixToShader(GLint iLoc, Matrix4 matrix) {
	// pass 4x4 matrix to shader, row-major --> column major
	GLfloat MATRIX[16];
	// row-major ---> column-major
	MATRIX[0] = matrix[0];  MATRIX[4] = matrix[1];   MATRIX[8] = matrix[2];    MATRIX[12] = matrix[3];
	MATRIX[1] = matrix[4];  MATRIX[5] = matrix[5];   MATRIX[9] = matrix[6];    MATRIX[13] = matrix[7];
	MATRIX[2] = matrix[8];  MATRIX[6] = matrix[9];   MATRIX[10] = matrix[10];   MATRIX[14] = matrix[11];
	MATRIX[3] = matrix[12]; MATRIX[7] = matrix[13];  MATRIX[11] = matrix[14];   MATRIX[15] = matrix[15];
	glUniformMatrix4fv(iLoc, 1, GL_FALSE, MATRIX);
}

void passVector3ToShader(GLint iLoc, Vector3 vector) {
	GLfloat vec[3];
	vec[0] = vector[0];
	vec[1] = vector[1];
	vec[2] = vector[2];
	glUniform3fv(iLoc, 1, vec);
}



void onDisplay(void)
{
	// clear canvas
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(iLocPosition); // av4position
	glEnableVertexAttribArray(iLocNormal); // av3normal

	//MVP
	Matrix4 T;
	Matrix4 S;
	Matrix4 R;
	if (autoRotateMode == 0) {
		R = Matrix4(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}
	else {
		float t = glutGet(GLUT_ELAPSED_TIME);
		float offsetTime = t / rotateSpeed;
		R = Matrix4(
			cos(offsetTime), 0, sin(offsetTime), 0,
			0, 1, 0, 0,
			-sin(offsetTime), 0, cos(offsetTime), 0,
			0, 0, 0, 1);
		// rotate around
	}
	Matrix4 M = R*N;
	Matrix4 V = getViewTransMatrix();
	Matrix4 P = getPerpectiveMatrix();

	Matrix4 MVP = P*V*M;
	/*
	GLfloat mvp[16];
	// row-major ---> column-major
	mvp[0] = MVP[0];  mvp[4] = MVP[1];   mvp[8] = MVP[2];    mvp[12] = MVP[3];
	mvp[1] = MVP[4];  mvp[5] = MVP[5];   mvp[9] = MVP[6];    mvp[13] = MVP[7];
	mvp[2] = MVP[8];  mvp[6] = MVP[9];   mvp[10] = MVP[10];   mvp[14] = MVP[11];
	mvp[3] = MVP[12]; mvp[7] = MVP[13];  mvp[11] = MVP[14];   mvp[15] = MVP[15];
	*/

	// pass lightsource and effect on/off info to shader
	glUniform1i(iLocAmbientOn, ambientOn);
	glUniform1i(iLocDiffuseOn, diffuseOn);
	glUniform1i(iLocSpecularOn, specularOn);
	glUniform1i(iLocDirectionalOn, directionalOn);
	glUniform1i(iLocPointOn, pointOn);
	glUniform1i(iLocSpotOn, spotOn);

	glUniform1i(iLocPerPixelLighting, perPixelOn);

	// matrix parameters
	passMatrixToShader(iLocMVP, MVP);
	passMatrixToShader(iLocNormalTransform, N);
	passMatrixToShader(iLocViewTransform, V);
	passMatrixToShader(iLocModelTransform, M);

	passVector3ToShader(iLocEyePosition, eyePos);

	// light source parameters which may change
	glUniform4fv(iLocPointPosition, 1, lightsource[2].position);
	glUniform4fv(iLocSpotPosition, 1, lightsource[3].position);
	glUniform1f(iLocSpotExponent, lightsource[3].spotExponent);
	glUniform1f(iLocSpotCutoff, lightsource[3].spotCutoff);
	glUniform1f(iLocSpotCosCutoff, lightsource[3].spotCosCutoff);

	//pass model material vertices and normals value to the shader
	GLMgroup* group = OBJ->groups;
	int offsetIndex = 0;
	while (group) {
		glUniform4fv(iLocMAmbient, 1, OBJ->materials[group->material].ambient); // Material.ambient
		glUniform4fv(iLocMDiffuse, 1, OBJ->materials[group->material].diffuse); // Material.diffuse
		glUniform4fv(iLocMSpecular, 1, OBJ->materials[group->material].specular); // Material.specular
		glUniform1f(iLocMShininess, OBJ->materials[group->material].shininess); // Material.shininess
		//printf("shininess = %f\n", OBJ->materials[group->material].shininess);
		glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, vertices + offsetIndex);
		glVertexAttribPointer(iLocNormal, 3, GL_FLOAT, GL_FALSE, 0, normals + offsetIndex);
		glDrawArrays(GL_TRIANGLES, 0, 3 * (group->numtriangles));
		offsetIndex += 9 * group->numtriangles;
		group = group->next;
	}

	glutSwapBuffers();
}

void showShaderCompileStatus(GLuint shader, GLint *shaderCompiled)
{
	glGetShaderiv(shader, GL_COMPILE_STATUS, shaderCompiled);
	if (GL_FALSE == (*shaderCompiled))
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character.
		GLchar *errorLog = (GLchar*)malloc(sizeof(GLchar) * maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		fprintf(stderr, "%s", errorLog);

		glDeleteShader(shader);
		free(errorLog);
	}
}
void setLightingSource() {
	float PLRange = 70.0; // todo
	// 0: Ambient
	lightsource[0].position[0] = 0;
	lightsource[0].position[1] = 0;
	lightsource[0].position[2] = -1;
	lightsource[0].position[3] = 1;
	lightsource[0].ambient[0] = 0.75;
	lightsource[0].ambient[1] = 0.75;
	lightsource[0].ambient[2] = 0.75;
	lightsource[0].ambient[3] = 1;

	// 1: directional light
	lightsource[1].position[0] = 0;
	lightsource[1].position[1] = 1;
	lightsource[1].position[2] = 1;
	lightsource[1].position[3] = 1;
	lightsource[1].ambient[0] = 0;
	lightsource[1].ambient[1] = 0;
	lightsource[1].ambient[2] = 0;
	lightsource[1].ambient[3] = 1;
	lightsource[1].diffuse[0] = 0.8;
	lightsource[1].diffuse[1] = 0.8;
	lightsource[1].diffuse[2] = 0.8;
	lightsource[1].diffuse[3] = 1;
	lightsource[1].specular[0] = 0.8;
	lightsource[1].specular[1] = 0.8;
	lightsource[1].specular[2] = 0.8;
	lightsource[1].specular[3] = 1;
	lightsource[1].constantAttenuation = 1;
	lightsource[1].linearAttenuation = 4.5 / PLRange;
	lightsource[1].quadraticAttenuation = 75 / (PLRange*PLRange);

	// 2: point light
	lightsource[2].position[0] = 1;
	lightsource[2].position[1] = 2;
	lightsource[2].position[2] = 0;
	lightsource[2].position[3] = 1;
	lightsource[2].ambient[0] = 0;
	lightsource[2].ambient[1] = 0;
	lightsource[2].ambient[2] = 0;
	lightsource[2].ambient[3] = 1;
	lightsource[2].diffuse[0] = 0.8;
	lightsource[2].diffuse[1] = 0.8;
	lightsource[2].diffuse[2] = 0.8;
	lightsource[2].diffuse[3] = 1;
	lightsource[2].specular[0] = 0.8;
	lightsource[2].specular[1] = 0.8;
	lightsource[2].specular[2] = 0.8;
	lightsource[2].specular[3] = 1;
	lightsource[2].constantAttenuation = 1;
	lightsource[2].linearAttenuation = 4.5 / PLRange;
	lightsource[2].quadraticAttenuation = 75 / (PLRange*PLRange);

	// 3: spot light
	lightsource[3].position[0] = 0;
	lightsource[3].position[1] = 0;
	lightsource[3].position[2] = 1;
	lightsource[3].position[3] = 1;
	lightsource[3].ambient[0] = 0;
	lightsource[3].ambient[1] = 0;
	lightsource[3].ambient[2] = 0;
	lightsource[3].ambient[3] = 1;
	lightsource[3].diffuse[0] = 0.8;
	lightsource[3].diffuse[1] = 0.8;
	lightsource[3].diffuse[2] = 0.8;
	lightsource[3].diffuse[3] = 1;
	lightsource[3].specular[0] = 0.8;
	lightsource[3].specular[1] = 0.8;
	lightsource[3].specular[2] = 0.8;
	lightsource[3].specular[3] = 1;
	lightsource[3].spotDirection[0] = 0;
	lightsource[3].spotDirection[1] = 0;
	lightsource[3].spotDirection[2] = -2;
	lightsource[3].spotExponent = 0.5;
	lightsource[3].spotCutoff = 45;
	lightsource[3].spotCosCutoff = 0.99; // 1/12 pi
	lightsource[3].constantAttenuation = 1;
	lightsource[3].linearAttenuation = 4.5 / PLRange;
	lightsource[3].quadraticAttenuation = 75 / (PLRange*PLRange);

}

void setShaders()
{
	GLuint v, f, p;
	char *vs = NULL;
	char *fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("sample.vert");
	fs = textFileRead("sample.frag");

	glShaderSource(v, 1, (const GLchar**)&vs, NULL);
	glShaderSource(f, 1, (const GLchar**)&fs, NULL);

	free(vs);
	free(fs);

	// compile vertex shader
	glCompileShader(v);
	GLint vShaderCompiled;
	showShaderCompileStatus(v, &vShaderCompiled);
	if (!vShaderCompiled) system("pause"), exit(123);

	// compile fragment shader
	glCompileShader(f);
	GLint fShaderCompiled;
	showShaderCompileStatus(f, &fShaderCompiled);
	if (!fShaderCompiled) system("pause"), exit(456);

	p = glCreateProgram();

	// bind shader
	glAttachShader(p, f);
	glAttachShader(p, v);

	// link program
	glLinkProgram(p);

	iLocPosition = glGetAttribLocation(p, "av4position");
	iLocNormal = glGetAttribLocation(p, "av3normal");
	iLocMVP = glGetUniformLocation(p, "mvp");

	iLocMDiffuse = glGetUniformLocation(p, "Material.diffuse");
	iLocMAmbient = glGetUniformLocation(p, "Material.ambient");
	iLocMSpecular = glGetUniformLocation(p, "Material.specular");
	iLocMShininess = glGetUniformLocation(p, "Material.shininess");

	iLocAmbientOn = glGetUniformLocation(p, "ambientOn");
	iLocDiffuseOn = glGetUniformLocation(p, "diffuseOn");
	iLocSpecularOn = glGetUniformLocation(p, "specularOn");
	iLocDirectionalOn = glGetUniformLocation(p, "directionalOn");
	iLocPointOn = glGetUniformLocation(p, "pointOn");
	iLocSpotOn = glGetUniformLocation(p, "spotOn");
	iLocNormalTransform = glGetUniformLocation(p, "NormalTransMatrix");
	iLocModelTransform = glGetUniformLocation(p, "ModelTransMatrix");
	iLocViewTransform = glGetUniformLocation(p, "ViewTransMatrix");
	iLocEyePosition = glGetUniformLocation(p, "eyePos");
	iLocPerPixelLighting = glGetUniformLocation(p, "perPixelOn");

	iLocPointPosition = glGetUniformLocation(p, "LightSource[2].position");
	iLocSpotPosition = glGetUniformLocation(p, "LightSource[3].position");
	iLocSpotExponent = glGetUniformLocation(p, "LightSource[3].spotExponent");
	iLocSpotCutoff = glGetUniformLocation(p, "LightSource[3].spotCutoff");
	iLocSpotCosCutoff = glGetUniformLocation(p, "LightSource[3].spotCosCutoff");

	glUseProgram(p);

	// pass lightsource parameters to shader
	setLightingSource();

	glUniform4fv(glGetUniformLocation(p, "LightSource[0].position"), 1, lightsource[0].position);
	glUniform4fv(glGetUniformLocation(p, "LightSource[0].ambient"), 1, lightsource[0].ambient);

	glUniform4fv(glGetUniformLocation(p, "LightSource[1].position"), 1, lightsource[1].position);
	glUniform4fv(glGetUniformLocation(p, "LightSource[1].ambient"), 1, lightsource[1].ambient);
	glUniform4fv(glGetUniformLocation(p, "LightSource[1].diffuse"), 1, lightsource[1].diffuse);
	glUniform4fv(glGetUniformLocation(p, "LightSource[1].specular"), 1, lightsource[1].specular);
	glUniform1f(glGetUniformLocation(p, "LightSource[1].constantAttenuation"), lightsource[1].constantAttenuation);
	glUniform1f(glGetUniformLocation(p, "LightSource[1].linearAttenuation"), lightsource[1].linearAttenuation);
	glUniform1f(glGetUniformLocation(p, "LightSource[1].quadraticAttenuation"), lightsource[1].quadraticAttenuation);

	glUniform4fv(iLocPointPosition, 1, lightsource[2].position);
	glUniform4fv(glGetUniformLocation(p, "LightSource[2].ambient"), 1, lightsource[2].ambient);
	glUniform4fv(glGetUniformLocation(p, "LightSource[2].diffuse"), 1, lightsource[2].diffuse);
	glUniform4fv(glGetUniformLocation(p, "LightSource[2].specular"), 1, lightsource[2].specular);
	glUniform1f(glGetUniformLocation(p, "LightSource[2].constantAttenuation"), lightsource[2].constantAttenuation);
	glUniform1f(glGetUniformLocation(p, "LightSource[2].linearAttenuation"), lightsource[2].linearAttenuation);
	glUniform1f(glGetUniformLocation(p, "LightSource[2].quadraticAttenuation"), lightsource[2].quadraticAttenuation);

	glUniform4fv(iLocSpotPosition, 1, lightsource[3].position);
	glUniform4fv(glGetUniformLocation(p, "LightSource[3].ambient"), 1, lightsource[3].ambient);
	glUniform4fv(glGetUniformLocation(p, "LightSource[3].diffuse"), 1, lightsource[3].diffuse);
	glUniform4fv(glGetUniformLocation(p, "LightSource[3].specular"), 1, lightsource[3].specular);
	glUniform3fv(glGetUniformLocation(p, "LightSource[3].spotDirection"), 1, lightsource[3].spotDirection);
	glUniform1f(glGetUniformLocation(p, "LightSource[3].constantAttenuation"), lightsource[2].constantAttenuation);
	glUniform1f(glGetUniformLocation(p, "LightSource[3].linearAttenuation"), lightsource[2].linearAttenuation);
	glUniform1f(glGetUniformLocation(p, "LightSource[3].quadraticAttenuation"), lightsource[2].quadraticAttenuation);
	glUniform1f(iLocSpotExponent, lightsource[3].spotExponent);
	glUniform1f(iLocSpotCutoff, lightsource[3].spotCutoff);
	glUniform1f(iLocSpotCosCutoff, lightsource[3].spotCosCutoff);


}


void printStatus() {
	// print current status of lighting
	printf("Directional light = %s, Point light = %s, Spot light = %s\n",
		directionalOn ? "ON" : "OFF", pointOn ? "ON" : "OFF", spotOn ? "ON" : "OFF");
	printf("Ambient = %s, Diffuse = %s, Specular = %s\n",
		ambientOn ? "ON" : "OFF", diffuseOn ? "ON" : "OFF", specularOn ? "ON" : "OFF");
	printf("\n");
}

void onMouse(int who, int state, int x, int y)
{
	//printf("%18s(): (%d, %d) \n", __FUNCTION__, x, y);

	switch (who)
	{
	case GLUT_LEFT_BUTTON: {
		//printf("left button   "); 
		if (spotOn == 1) {
			lightsource[3].spotExponent += 0.1;
			printf("Turn spotExponent up\n");
		}
		break;
	}
	case GLUT_MIDDLE_BUTTON:
		//printf("middle button "); 
		break;
	case GLUT_RIGHT_BUTTON:
		//printf("right button  "); 
		if (spotOn == 1) {
			lightsource[3].spotExponent -= 0.1;
			printf("Turn spotExponent down\n");
		}

		break;
	case GLUT_WHEEL_UP:
		if (spotOn == 1) {
			lightsource[3].spotCosCutoff -= 0.003;
			printf("Turn CUT_OFF_ANGLE up\n");
		}
		//printf("wheel up      "); 
		break;
	case GLUT_WHEEL_DOWN:
		//printf("wheel down    "); 
		if (spotOn == 1) {
			lightsource[3].spotCosCutoff += 0.003;
			printf("Turn CUT_OFF_ANGLE down\n");
		}

		break;
	default:
		//printf("0x%02X          ", who); 
		break;
	}

	switch (state)
	{
	case GLUT_DOWN:
		//printf("start "); 
		break;
	case GLUT_UP:
		//printf("end   "); 
		break;
	}

	//printf("\n");
}

void onMouseMotion(int x, int y)
{
	//printf("%18s(): (%d, %d) mouse move\n", __FUNCTION__, x, y);
}

void onPassiveMouseMotion(int x, int y) {
	// move the position of spot light
	if (spotOn != 0) {
		float wx = (float)x*2.0 / (float)windowWidth - 1.0;
		float wy = -(float)y*2.0 / (float)windowHeight + 1.0;
		// printf("new x = %f, y= %f\n", wx, wy);
		lightsource[3].position[0] = wx;
		lightsource[3].position[1] = wy;
	}

}

void onKeyboard(unsigned char key, int x, int y)
{
	//printf("%18s(): (%d, %d) key: %c(0x%02X) ", __FUNCTION__, x, y, key, key);
	switch (key)
	{
	case GLUT_KEY_ESC: /* the Esc key */
		exit(0);
		break;
	case GLUT_KEY_z:
		// switch to the previous model
		modelIndex--;
		if (modelIndex < 0) {
			modelIndex += numOfModels;
		}
		loadOBJModel();
		printf("switch to previous model\n");
		break;
	case GLUT_KEY_x:
		// switch to the next model
		printf("switch to next model\n");
		modelIndex++;
		if (modelIndex >= numOfModels) {
			modelIndex -= numOfModels;
		}
		loadOBJModel();
		break;

	case GLUT_KEY_h:
		// show help menu
		printf("----------Help Menu----------\n");
		printf("press 'q' 'w' 'e' to toggle the light source\n");
		printf("press 'a' 's' 'd' to toggle the light arrtibute\n");
		printf("press 'z' 'x' to change model\n");
		printf("press 'r' to toggle auto rotation\n");
		printf("press 'v' 'b' to change the rotatation speed\n");
		printf("press 'c' to change the spot light into another type\n");
		printf("press 'f' to toggle per-pixel rendering\n");
		printf("use arrow buttom to move the point light\n");
		printf("hover mouse to move the spot light\n");
		printf("click mouse to tune EXP\n");
		printf("scroll mouse to tune CUT_OFF_ANGLE\n");
		printf("----------Help Menu----------\n");
		break;
	case GLUT_KEY_q:
		directionalOn = (directionalOn + 1) % 2;
		printf("Turn %s directional light\n", directionalOn ? "ON" : "OFF");
		printStatus();
		break;
	case GLUT_KEY_w:
		pointOn = (pointOn + 1) % 2;
		printf("Turn %s point light\n", pointOn ? "ON" : "OFF");
		printStatus();
		break;
	case GLUT_KEY_e:
		if (spotOn == 0) {
			spotOn = 1;
		}
		else {
			spotOn = 0;
		}
		printf("Turn %s spot light\n", spotOn ? "ON" : "OFF");
		printStatus();
		break;
	case GLUT_KEY_a:
		ambientOn = (ambientOn + 1) % 2;
		printf("Turn %s ambient effect\n", ambientOn ? "ON" : "OFF");
		printStatus();
		break;
	case GLUT_KEY_s:
		diffuseOn = (diffuseOn + 1) % 2;
		printf("Turn %s diffuse effect\n", diffuseOn ? "ON" : "OFF");
		printStatus();
		break;
	case GLUT_KEY_d:
		specularOn = (specularOn + 1) % 2;
		printf("Turn %s specular effect\n", specularOn ? "ON" : "OFF");
		printStatus();
		break;
	case GLUT_KEY_r:
		autoRotateMode = (autoRotateMode + 1) % 2;
		printf("Turn %s auto rotate\n", autoRotateMode ? "ON" : "OFF");
		break;
	case GLUT_KEY_v:
		if (autoRotateMode == 1) {
			if (rotateSpeed > 50.0) {
				printf("Speed up the auto rotation\n");
				rotateSpeed -= 50.0;
			}
			else {
				printf("Please do not try to rotate too fast\n");
			}
		}
		break;
	case GLUT_KEY_b:
		if (autoRotateMode == 1) {
			rotateSpeed += 50.0;
			printf("Slow down the auot rotation\n");
		}
		break;
	case GLUT_KEY_c:
		switch (spotOn) {
		case 1:
			spotOn = 2;
			printf("change spot light into directional mode\n");
			break;
		case 2:
			spotOn = 3;
			printf("change spot light into point mode\n");
			break;
		case 3:
			spotOn = 1;
			printf("change spot light into normal spotlight mode\n");
			break;
		}
		break;
	case GLUT_KEY_f:
		perPixelOn = (perPixelOn + 1) % 2;
		if (perPixelOn == 0) {
			printf("switch to vertex lighting\n");
		}
		else {
			printf("switch to per pixel lighting\n");
		}
		break;
	}
	//printf("\n");
}

void onKeyboardSpecial(int key, int x, int y) {
	//printf("%18s(): (%d, %d) ", __FUNCTION__, x, y);
	switch (key)
	{
	case GLUT_KEY_LEFT:
		//printf("key: LEFT ARROW");
		if (pointOn == 1) {
			lightsource[2].position[0] -= 0.5;
		}
		break;

	case GLUT_KEY_RIGHT:
		//printf("key: RIGHT ARROW");
		if (pointOn == 1) {
			lightsource[2].position[0] += 0.5;
		}
		break;
	case GLUT_KEY_UP:
		if (pointOn == 1) {
			lightsource[2].position[1] += 0.5;
		}
		break;
	case GLUT_KEY_DOWN:
		if (pointOn == 1) {
			lightsource[2].position[1] -= 0.5;
		}
		break;
	default:
		//printf("key: 0x%02X      ", key);
		break;
	}
	//printf("\n");
}


void onWindowReshape(int width, int height)
{
	windowHeight = height;
	windowWidth = width;
	printf("%18s(): %dx%d\n", __FUNCTION__, width, height);
}

int main(int argc, char **argv)
{
	// glut init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	// create window
	windowHeight = windowWidth = 800;
	glutInitWindowPosition(500, 100);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("10420 CS550000 CG HW3 X1052165 Yuchun Jin");

	glewInit();
	if (glewIsSupported("GL_VERSION_2_0")) {
		printf("Ready for OpenGL 2.0\n");
	}
	else {
		printf("OpenGL 2.0 not supported\n");
		system("pause");
		exit(1);
	}

	// load obj models through glm
	loadOBJModel();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// register glut callback functions
	glutDisplayFunc(onDisplay);
	glutIdleFunc(onIdle);
	glutKeyboardFunc(onKeyboard);
	glutSpecialFunc(onKeyboardSpecial);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMouseMotion);
	glutPassiveMotionFunc(onPassiveMouseMotion);
	glutReshapeFunc(onWindowReshape);

	// set up lighting parameters
	// setLightingSource(); 
	// call in setShaders();

	// set up shaders here
	setShaders();

	glEnable(GL_DEPTH_TEST);

	// main loop
	glutMainLoop();

	// free
	glmDelete(OBJ);

	return 0;
}

