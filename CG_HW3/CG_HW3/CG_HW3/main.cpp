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

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

// Shader attributes
GLint iLocPosition;
GLint iLocNormal;
GLint iLocMVP;

GLint iLocMDiffuse, iLocMAmbient, iLocMSpecular, iLocMShininess;
GLint iLocLDAmbient,iLocLDPosition;

char filename[] = "NormalModels/Low/boxN.obj";
GLMmodel* OBJ;
GLfloat* vertices;
GLfloat* normals;

Matrix4 N;

struct LightSourceParameters {
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float position[4];
	float halfVector[4];
	float spotDirection[4];
	float spotExponent;
	float spotCutoff; // (range: [0.0,90.0], 180.0)
	float spotCosCutoff; // (range: [1.0,0.0],-1.0)
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
}typedef LightSource;
LightSource lightsource[3];

void traverseColorModel()
{
	int i;
	
	GLfloat maxVal[3];
	GLfloat minVal[3];

	// TODO:
	//// You should traverse the vertices and the colors of each triangle, and 
	//// then normalize the model to unit size by using transformation matrices. 
	//// i.e. Each vertex should be bounded in [-1, 1], which will fit the camera clipping window.


	// number of triangles
	vertices = (GLfloat*)malloc(sizeof(GLfloat)*OBJ->numtriangles*9);
	normals = (GLfloat*)malloc(sizeof(GLfloat)*OBJ->numtriangles*9);

	float max_x = OBJ->vertices[3];
	float max_y = OBJ->vertices[4];
	float max_z = OBJ->vertices[5];
	float min_x = OBJ->vertices[3];
	float min_y = OBJ->vertices[4];
	float min_z = OBJ->vertices[5];

	for(i=0; i<(int)OBJ->numtriangles; i++)
	{
		// the index of each vertex
		int indv1 = OBJ->triangles[i].vindices[0];
		int indv2 = OBJ->triangles[i].vindices[1];
		int indv3 = OBJ->triangles[i].vindices[2];

		// the index of each color
		int indc1 = indv1;
		int indc2 = indv2;
		int indc3 = indv3;

		// vertices
		
		vertices[i*9 + 0] = OBJ->vertices[indv1*3+0];
		vertices[i*9 + 1] = OBJ->vertices[indv1*3+1];
		vertices[i*9 + 2] = OBJ->vertices[indv1*3+2];
		if(vertices[i*9 + 0] > max_x) max_x = vertices[i*9 + 0];
		if(vertices[i*9 + 1] > max_y) max_y = vertices[i*9 + 1]; 
		if(vertices[i*9 + 2] > max_z) max_z = vertices[i*9 + 2];
		if(vertices[i*9 + 0] < min_x) min_x = vertices[i*9 + 0];
		if(vertices[i*9 + 1] < min_y) min_y = vertices[i*9 + 1];
		if(vertices[i*9 + 2] < min_z) min_z = vertices[i*9 + 2];

		vertices[i*9 + 3] = OBJ->vertices[indv2*3+0];
		vertices[i*9 + 4] = OBJ->vertices[indv2*3+1];
		vertices[i*9 + 5] = OBJ->vertices[indv2*3+2];
		if(vertices[i*9 + 3] > max_x) max_x = vertices[i*9 + 3];
		if(vertices[i*9 + 4] > max_y) max_y = vertices[i*9 + 4];
		if(vertices[i*9 + 5] > max_z) max_z = vertices[i*9 + 5];
		if(vertices[i*9 + 3] < min_x) min_x = vertices[i*9 + 3];
		if(vertices[i*9 + 4] < min_y) min_y = vertices[i*9 + 4];
		if(vertices[i*9 + 5] < min_z) min_z = vertices[i*9 + 5];
	
		vertices[i*9 + 6] = OBJ->vertices[indv3*3+0];
		vertices[i*9 + 7] = OBJ->vertices[indv3*3+1];
		vertices[i*9 + 8] = OBJ->vertices[indv3*3+2];
		if(vertices[i*9 + 6] > max_x) max_x = vertices[i*9 + 6];
		if(vertices[i*9 + 7] > max_y) max_y = vertices[i*9 + 7];
		if(vertices[i*9 + 8] > max_z) max_z = vertices[i*9 + 8];
		if(vertices[i*9 + 6] < min_x) min_x = vertices[i*9 + 6];
		if(vertices[i*9 + 7] < min_y) min_y = vertices[i*9 + 7];
		if(vertices[i*9 + 8] < min_z) min_z = vertices[i*9 + 8];

		// colors

		normals[i*9 + 0] = OBJ->normals[indv1*3+0];
		normals[i*9 + 1] = OBJ->normals[indv1*3+1];
		normals[i*9 + 2] = OBJ->normals[indv1*3+2];

		normals[i*9 + 3] = OBJ->normals[indv2*3+0];
		normals[i*9 + 4] = OBJ->normals[indv2*3+1];
		normals[i*9 + 5] = OBJ->normals[indv2*3+2];

		normals[i*9 + 6] = OBJ->normals[indv3*3+0];
		normals[i*9 + 7] = OBJ->normals[indv3*3+1];
		normals[i*9 + 8] = OBJ->normals[indv3*3+2];


	}
	float normalize_scale = max(max(abs(max_x-min_x),abs(max_y-min_y)),abs(max_z-min_z));

	Matrix4 S,T;
	S.identity();
	T.identity();
	S[0] = 2/normalize_scale;
	S[5] = 2/normalize_scale;;
	S[10] = 2/normalize_scale;
	T[3] = -(min_x+max_x)/2;
	T[7] = -(min_y+max_y)/2;
	T[11] = -(min_z+max_z)/2;

	N = S*T;

}

void loadOBJModel()
{
	// read an obj model here
	if(OBJ != NULL){
		free(OBJ);
	}
	OBJ = glmReadOBJ(filename);
	printf("%s\n", filename);

	glmFacetNormals(OBJ);
	glmVertexNormals(OBJ, 90.0);

	// traverse the color model
	traverseColorModel();
}

void onIdle()
{
	glutPostRedisplay();
}

void onDisplay(void)
{
	// clear canvas
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(iLocPosition);
	glEnableVertexAttribArray(iLocNormal);

	// organize the arrays
	static GLfloat triangle_normals[] = {
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -1.0f
	};

	static GLfloat triangle_vertex[] = {
		 1.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};

	static GLfloat ambient[]={0.500000, 0.500000, 0.500000};
	static GLfloat diffuse[]={0.784314, 0.470588, 0.752941};
	static GLfloat specular[]={1.000000,1.000000, 1.000000};
	static GLfloat shininess = 1.000000;

	// TODO:
	//// Please define the model transformation matrix, viewing transformation matrix, 
	//// projection transformation matrix

	//MVP
	Matrix4 T;
	Matrix4 S;
	Matrix4 R;

	Matrix4 M = Matrix4(
						1, 0, 0, 0, 
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
	Matrix4 V = Matrix4(
						1, 0, 0, 0, 
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
	Matrix4 P = Matrix4(
						1, 0, 0, 0, 
						0, 1, 0, 0,
						0, 0, -1, 0,
						0, 0, 0, 1);

	Matrix4 MVP = P*V*M;

	GLfloat mvp[16];
	// row-major ---> column-major
	mvp[0] = MVP[0];  mvp[4] = MVP[1];   mvp[8]  = MVP[2];    mvp[12] = MVP[3];  
	mvp[1] = MVP[4];  mvp[5] = MVP[5];   mvp[9]  = MVP[6];    mvp[13] = MVP[7];  
	mvp[2] = MVP[8];  mvp[6] = MVP[9];   mvp[10] = MVP[10];   mvp[14] = MVP[11];  
	mvp[3] = MVP[12]; mvp[7] = MVP[13];  mvp[11] = MVP[14];   mvp[15] = MVP[15];


	//pass model material value to the shader
	glUniform4fv(iLocMAmbient, 1, ambient);
	glUniform4fv(iLocMDiffuse, 1, diffuse);
	glUniform4fv(iLocMSpecular, 1, specular);
	glUniform1f(iLocMShininess, shininess);

	// bind array pointers to shader
	glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, triangle_vertex);
	glVertexAttribPointer(iLocNormal, 3, GL_FLOAT, GL_FALSE, 0, triangle_normals);
	
	// bind uniform matrix to shader
	glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, mvp);

	// draw the array we just bound
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glutSwapBuffers();
}

void showShaderCompileStatus(GLuint shader, GLint *shaderCompiled)
{
	glGetShaderiv(shader, GL_COMPILE_STATUS, shaderCompiled);
	if(GL_FALSE == (*shaderCompiled))
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character.
		GLchar *errorLog = (GLchar*) malloc(sizeof(GLchar) * maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		fprintf(stderr, "%s", errorLog);

		glDeleteShader(shader);
		free(errorLog);
	}
}
void setLightingSource(){

	lightsource[0].position[0] = 0;
	lightsource[0].position[1] = 0;
	lightsource[0].position[2] = -1;
	lightsource[0].position[3] = 1;
	lightsource[0].ambient[0] = 0.5;
	lightsource[0].ambient[1] = 0.5;
	lightsource[0].ambient[2] = 0.5;
	lightsource[0].ambient[3] = 1;
	//To Do : Setup your own lighting source parameters;

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
	if(!vShaderCompiled) system("pause"), exit(123);

	// compile fragment shader
	glCompileShader(f);
	GLint fShaderCompiled;
	showShaderCompileStatus(f, &fShaderCompiled);
	if(!fShaderCompiled) system("pause"), exit(456);

	p = glCreateProgram();

	// bind shader
	glAttachShader(p, f);
	glAttachShader(p, v);

	// link program
	glLinkProgram(p);

	iLocPosition = glGetAttribLocation (p, "av4position");
	iLocNormal = glGetAttribLocation(p, "av3normal");
	iLocMVP		 = glGetUniformLocation(p, "mvp");

	iLocMDiffuse = glGetUniformLocation(p, "Material.diffuse");
	iLocMAmbient = glGetUniformLocation(p, "Material.ambient");
	iLocMSpecular = glGetUniformLocation(p, "Material.specular");
	iLocMShininess = glGetUniformLocation(p, "Material.shininess");

	iLocLDAmbient = glGetUniformLocation(p, "LightSource[0].ambient");
	iLocLDPosition = glGetUniformLocation(p, "LightSource[0].position");
	

	glUseProgram(p);

	glUniform4fv(iLocLDAmbient, 1, lightsource[0].ambient);
	glUniform4fv(iLocLDPosition, 1, lightsource[0].position);

}


void onMouse(int who, int state, int x, int y)
{
	printf("%18s(): (%d, %d) ", __FUNCTION__, x, y);

	switch(who)
	{
		case GLUT_LEFT_BUTTON:   printf("left button   "); break;
		case GLUT_MIDDLE_BUTTON: printf("middle button "); break;
		case GLUT_RIGHT_BUTTON:  printf("right button  "); break; 
		case GLUT_WHEEL_UP:      printf("wheel up      "); break;
		case GLUT_WHEEL_DOWN:    printf("wheel down    "); break;
		default:                 printf("0x%02X          ", who); break;
	}

	switch(state)
	{
		case GLUT_DOWN: printf("start "); break;
		case GLUT_UP:   printf("end   "); break;
	}

	printf("\n");
}

void onMouseMotion(int x, int y)
{
	printf("%18s(): (%d, %d) mouse move\n", __FUNCTION__, x, y);
}

void onKeyboard(unsigned char key, int x, int y) 
{
	printf("%18s(): (%d, %d) key: %c(0x%02X) ", __FUNCTION__, x, y, key, key);
	switch(key) 
	{
		case GLUT_KEY_ESC: /* the Esc key */ 
			exit(0); 
			break;
	}
	printf("\n");
}

void onKeyboardSpecial(int key, int x, int y){
	printf("%18s(): (%d, %d) ", __FUNCTION__, x, y);
	switch(key)
	{
		case GLUT_KEY_LEFT:
			printf("key: LEFT ARROW");
			break;
			
		case GLUT_KEY_RIGHT:
			printf("key: RIGHT ARROW");
			break;

		default:
			printf("key: 0x%02X      ", key);
			break;
	}
	printf("\n");
}


void onWindowReshape(int width, int height)
{
	printf("%18s(): %dx%d\n", __FUNCTION__, width, height);
}

int main(int argc, char **argv) 
{
	// glut init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	// create window
	glutInitWindowPosition(500, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("10420 CS550000 CG HW2 TA");

	glewInit();
	if(glewIsSupported("GL_VERSION_2_0")){
		printf("Ready for OpenGL 2.0\n");
	}else{
		printf("OpenGL 2.0 not supported\n");
		system("pause");
		exit(1);
	}

	// load obj models through glm
	loadOBJModel();

	// register glut callback functions
	glutDisplayFunc (onDisplay);
	glutIdleFunc    (onIdle);
	glutKeyboardFunc(onKeyboard);
	glutSpecialFunc (onKeyboardSpecial);
	glutMouseFunc   (onMouse);
	glutMotionFunc  (onMouseMotion);
	glutReshapeFunc (onWindowReshape);

	//set up lighting parameters
	setLightingSource();

	// set up shaders here
	setShaders();
	
	glEnable(GL_DEPTH_TEST);

	// main loop
	glutMainLoop();

	// free
	glmDelete(OBJ);

	return 0;
}

