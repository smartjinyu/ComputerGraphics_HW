#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <freeglut/glut.h>
#include "textfile.h"
#include "glm.h"

#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "freeglut.lib")

#ifndef GLUT_WHEEL_UP
# define GLUT_WHEEL_UP   0x0003
# define GLUT_WHEEL_DOWN 0x0004
#endif

#ifndef GLUT_KEY_ESC
# define GLUT_KEY_ESC 0x001B
#endif

#ifndef GLUT_KEY_h
# define GLUT_KEY_h 0x0068
#endif

#ifndef GLUT_KEY_z
# define GLUT_KEY_z 0x007A
#endif

#ifndef GLUT_KEY_x
# define GLUT_KEY_x 0x0078
#endif

#ifndef GLUT_KEY_w
# define GLUT_KEY_w 0x0077
#endif

#ifndef GLUT_KEY_c
# define GLUT_KEY_c 0x0063
#endif

#ifndef GLUT_KEY_s
# define GLUT_KEY_s 0x0073
#endif



#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

// Shader attributes
GLint iLocPosition;
GLint iLocColor;
GLint iMode;


#define numOfModels 12
char filename[numOfModels][100] = {"ColorModels/armadillo12KC.obj",
									"ColorModels/brain18KC.obj",
									"ColorModels/Dino20KC.obj",
									"ColorModels/dragon10KC.obj",
									"ColorModels/elephant16KC.obj",
									"ColorModels/happy10KC.obj",
									"ColorModels/hippo23KC.obj",
									"ColorModels/igea17KC.obj",
									"ColorModels/lion12KC.obj",
									"ColorModels/maxplanck20KC.obj",
									"ColorModels/lucy25KC.obj",
									"ColorModels/texturedknot11KC.obj" };
int modelIndex = 0;
bool solidMode = true;


int colorMode = 0;
/*
* colorMode = 0: normal (R,G,B)
* colorMode = 1: only Red (R,0,0)
* colorMode = 2: only Green (0,G,0)
* colorMode = 2: only Blue (0,0,B)
*/


GLMmodel* OBJ;
GLfloat* vertices;
GLfloat* colors;

void traverseColorModel()
{
	int i;

	GLfloat minVal[3],maxVal[3];

	// the array of vertices have 3*numvertices members, vertices[0] [1] [2] are x,y,z of the first vertice
	// initialize the min/max value
	// notice that index of vertice begins from 1, so the valid index range is [1,OBJ->numvertices]
	maxVal[0] = OBJ->vertices[3+0];
	maxVal[1] = OBJ->vertices[3+1];
	maxVal[2] = OBJ->vertices[3+2];
	
	minVal[0] = OBJ->vertices[3+0];
	minVal[1] = OBJ->vertices[3+1];
	minVal[2] = OBJ->vertices[3+2];

	// get the max/min value
	for (i = 2; i <= (int)OBJ->numvertices; i++) {
		maxVal[0] = max(maxVal[0], OBJ->vertices[i * 3 + 0]);
		maxVal[1] = max(maxVal[1], OBJ->vertices[i * 3 + 1]);
		maxVal[2] = max(maxVal[2], OBJ->vertices[i * 3 + 2]);
		minVal[0] = min(minVal[0], OBJ->vertices[i * 3 + 0]);
		minVal[1] = min(minVal[1], OBJ->vertices[i * 3 + 1]);
		minVal[2] = min(minVal[2], OBJ->vertices[i * 3 + 2]);
	}

	//printf("max = %f,%f,%f\n", maxVal[0], maxVal[1], maxVal[2]);
	//printf("min = %f,%f,%f\n", minVal[0], minVal[1], minVal[2]);

	// The center position of the model 
	OBJ->position[0] = (maxVal[0] + minVal[0]) / 2.0;
	OBJ->position[1] = (maxVal[1] + minVal[1]) / 2.0;
	OBJ->position[2] = (maxVal[2] + minVal[2]) / 2.0;

	// transform the center to (0,0)
	for (i = 1; i <= (int)OBJ->numvertices; i++) {
		OBJ->vertices[i * 3 + 0] -= OBJ->position[0];
		OBJ->vertices[i * 3 + 1] -= OBJ->position[1];
		OBJ->vertices[i * 3 + 2] -= OBJ->position[2];
	}

	// then fill the array vertices and colors
	vertices = (GLfloat*)malloc(9 * OBJ->numtriangles * sizeof(GLfloat));
	colors = (GLfloat*)malloc(9 * OBJ->numtriangles * sizeof(GLfloat));

	//GLfloat r = sqrt(
	//	(maxVal[0] - minVal[0])*(maxVal[0] - minVal[0]) + 
	//	(maxVal[1] - minVal[1])*(maxVal[1] - minVal[1]) + 
	//	(maxVal[2] - minVal[2])*(maxVal[2] - minVal[2])
	//)/2.0;
	GLfloat r = max(maxVal[0] - minVal[0], max(maxVal[1] - minVal[1], maxVal[2] - minVal[2]))/2.0; // use this to fit the bound of box
	int j = 0;
	for (i = 0, j = 0; i < (int)OBJ->numtriangles; i++, j += 9)
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
		GLfloat vx, vy, vz;
		vx = OBJ->vertices[indv1*3+0];
		vy = OBJ->vertices[indv1*3+1];
		vz = OBJ->vertices[indv1*3+2];
		// scale to [-1,1]
		vertices[j] = vx / r;
		vertices[j + 1] = vy / r;
		vertices[j + 2] = vz / r;

		vx = OBJ->vertices[indv2*3+0];
		vy = OBJ->vertices[indv2*3+1];
		vz = OBJ->vertices[indv2*3+2];
		vertices[j + 3] = vx / r;
		vertices[j + 4] = vy / r;
		vertices[j + 5] = vz / r;


		vx = OBJ->vertices[indv3*3+0];
		vy = OBJ->vertices[indv3*3+1];
		vz = OBJ->vertices[indv3*3+2];
		vertices[j + 6] = vx / r;
		vertices[j + 7] = vy / r;
		vertices[j + 8] = vz / r;


		// what do we need to do about color?
		// colors
		GLfloat c1, c2, c3;
		c1 = OBJ->colors[indv1*3+0];
		c2 = OBJ->colors[indv1*3+1];
		c3 = OBJ->colors[indv1*3+2];
		colors[j] = c1;
		colors[j + 1] = c2;
		colors[j + 2] = c3;

		c1 = OBJ->colors[indv2*3+0];
		c2 = OBJ->colors[indv2*3+1];
		c3 = OBJ->colors[indv2*3+2];
		colors[j + 3] = c1;
		colors[j + 4] = c2;
		colors[j + 5] = c3;

		c1 = OBJ->colors[indv3*3+0];
		c2 = OBJ->colors[indv3*3+1];
		c3 = OBJ->colors[indv3*3+2];
		colors[j + 6] = c1;
		colors[j + 7] = c2;
		colors[j + 8] = c3;

	}


}

void loadOBJModel()
{
	// read an obj model here
	if(OBJ != NULL){
		free(OBJ);
	}
	OBJ = glmReadOBJ(filename[modelIndex]);
	printf("%s\n", filename[modelIndex]);

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
	glEnableVertexAttribArray(iLocColor);

	// bind array pointers to shader
	glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(iLocColor, 3, GL_FLOAT, GL_FALSE, 0, colors);



	// draw the array we just bound
	glDrawArrays(GL_TRIANGLES , 0, 3 * (OBJ->numtriangles));

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

void setShaders()
{
	GLuint v, f, p;
	char *vs = NULL; // vertex shader
	char *fs = NULL; // fragment shader

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("shader.vert");
	fs = textFileRead("shader.frag");

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

	// get parameters from shader
	glUseProgram(p);

	iLocPosition = glGetAttribLocation(p, "av4position");
	iLocColor = glGetAttribLocation(p, "av3color");
	iMode = glGetUniformLocation(p, "fmode");
	glUniform1i(iMode, colorMode);



}


void onMouse(int who, int state, int x, int y)
{
	//printf("%18s(): (%d, %d) ", __FUNCTION__, x, y);

	//switch(who)
	//{
	//	case GLUT_LEFT_BUTTON:   printf("left button   "); break;
	//	case GLUT_MIDDLE_BUTTON: printf("middle button "); break;
	//	case GLUT_RIGHT_BUTTON:  printf("right button  "); break; 
	//	case GLUT_WHEEL_UP:      printf("wheel up      "); break;
	//	case GLUT_WHEEL_DOWN:    printf("wheel down    "); break;
	//	default:                 printf("0x%02X          ", who); break;
	//}

	//switch(state)
	//{
	//	case GLUT_DOWN: printf("start "); break;
	//	case GLUT_UP:   printf("end   "); break;
	//}

	//printf("\n");
}

void onMouseMotion(int x, int y)
{
	//printf("%18s(): (%d, %d) mouse move\n", __FUNCTION__, x, y);
}

void onKeyboard(unsigned char key, int x, int y) 
{
	//printf("%18s(): (%d, %d) key: %c(0x%02X) ", __FUNCTION__, x, y, key, key);
	switch(key) 
	{
		case GLUT_KEY_ESC: /* the Esc key */ 
			exit(0); 
			break;
		case GLUT_KEY_h:
			// show help menu
			//printf("Key h clicked\n");
			printf("----------Help Menu----------\n\n");
			printf("h: show help menu\n");
			printf("w: switch between solid : wired rendering mode\n");
			printf("z : move to previous model\n");
			printf("x : move to next model\n");
			printf("c : color filter function\n");
			printf("s : show author information.\n\n");
			printf("----------Help Menu----------\n");
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
		case GLUT_KEY_w:
			// switch solid/wireframe mode
			solidMode = !solidMode;
			if (solidMode) {
				printf("change to solid mode\n");
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			else {
				printf("change to wireframe mode\n");
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			break;
		case GLUT_KEY_c:
			// color filter function
			printf("change color filter\n");
			colorMode = (colorMode + 1) % 4;
			setShaders();
			break;
		case GLUT_KEY_s:
			// author information
			//printf("Key c clicked\n");
			break;





	}
	//printf("\n");
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
	// GLUT_DOUBLE means double buffering (drawing to a background buffer while another buffer is displayed)

	// create window
	glutInitWindowPosition(500, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("10420 CS550000 CG HW1 Computer Graphics");

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

	// set up shaders here
	setShaders();
	
	glEnable(GL_DEPTH_TEST);

	// main loop
	glutMainLoop();

	// free
	glmDelete(OBJ);

	return 0;
}

