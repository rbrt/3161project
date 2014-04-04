//TODO: Clean out remaining A2 code
//TODO: Separate faces into component groups and
//		draw properly

// Ignore compiler warnings
#define _CRT_SECURE_NO_WARNINGS

#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>




#define VERTEX_COUNT 6763
#define FACE_COUNT 3640
#define PROP_FACE_COUNT 132

const GLint WINDOW_WIDTH = 800;
const GLint WINDOW_HEIGHT = 600;
const GLint PLANET_COUNT = 9; 
const GLint PLANET_OFFSET = 10;
const GLint SUN_SIZE = 20;
const GLint STAR_COUNT = 3000;
const GLint GRID_SIZE = 5;

const GLfloat TWINKLE_SPEED = .1f;

typedef GLfloat color4f[4];

int texIDs[] = {0,1};
int imageWidth, imageHeight;

GLuint thePlane = 0;
GLuint propeller = 0;

GLint isMovingUp = 0,
	  isMovingDown = 0,
	  isMovingLeft = 0,
	  isMovingRight = 0,
	  isMovingForward = 0,
	  isMovingBackward = 0,
	  isTurningLeft = 0,
	  isTurningRight = 0,
	  isShowingShields = 1,
	  isShowingStars = 1,
	  isShowingCorona = 1,
	  isShowingRings = 1,
	  isInDiscoMode = 0,
	  faceCount = 0,
	  vertexCount = 0,
	  normalCount = 0;

GLfloat theta = 0.0;

GLfloat cameraPosition[] = {0,8.5,5};
GLfloat planePosition[] = {0,7,-2};

GLfloat pointEight = .8f;

color4f yellow = {1.0f,1.0f,0.0f,1.0f};
color4f black = {0.0f,0.0f,0.0f,1.0f};
color4f blue = {0.0f,0.0f,1.0f,1.0f};
color4f purple = {.8f,.8f,0.0f,1.0f};
color4f red = {1.0f,0.0f,0.0f,1.0f};
color4f grey = {0.1f,0.1f,0.1f,1.0f};
color4f white = {1.0f,1.0f,1.0f,1.0f};
color4f orange = {1.0f,.7f,0.0f,1.0f};
color4f green = {0.0f,1.0f,0.0f,1.0f};

GLubyte *seaTexture;
GLubyte *skyTexture;

typedef struct vector3f {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

typedef struct vector3i {
	GLint x;
	GLint y;
	GLint z;
};

typedef struct vector4i {
	GLint x;
	GLint y;
	GLint z;
	GLint w;
};

typedef struct faceContainer {
	GLint groupNumber;
	GLint elementCount;
	GLint elements[30];
};

struct vector3f modelVertices[VERTEX_COUNT];
struct vector3f modelNormals[VERTEX_COUNT];
struct faceContainer modelFaces[FACE_COUNT];
struct vector3f propellerVertices[VERTEX_COUNT];
struct vector3f propellerNormals[VERTEX_COUNT];
struct faceContainer propellerFaces[FACE_COUNT];


GLfloat lerp(GLfloat a, GLfloat b, GLfloat time){
	return a + (b - a) * time;
}

void cameraLerp(GLfloat *position, int plus){
	if (plus){
		*position = lerp(*position, *position + 1, .75);
	}
	else{
		*position = lerp(*position, *position - 1, .75);	
	}
}

// This function taken from the example program provided on the 
// course webpage
GLubyte *loadImage(char* filename){
	FILE *fileID;
	int  maxValue;	
	int  totalPixels;
	char tempChar;
	int i;
	char headerLine[100];
	float RGBScaling;
	int red, green, blue;
	GLubyte *imageData;

	fileID = fopen(filename, "r");

	fscanf(fileID,"%[^\n] ", headerLine);

	if ((headerLine[0] != 'P') || (headerLine[1] != '3'))
	{
		printf("This is not a PPM file!\n"); 
		exit(0);
	}

	printf("This is a PPM file\n");

	fscanf(fileID, "%c", &tempChar);

	while(tempChar == '#') 
	{
		fscanf(fileID, "%[^\n] ", headerLine);

		printf("%s\n", headerLine);
		
		fscanf(fileID, "%c",&tempChar);
	}

	ungetc(tempChar, fileID); 

	fscanf(fileID, "%d %d %d", &imageWidth, &imageHeight, &maxValue);

	printf("%d rows  %d columns  max value= %d\n", imageHeight, imageWidth, maxValue);

	totalPixels = imageWidth * imageHeight;

	imageData = malloc(3 * sizeof(GLuint) * totalPixels);
	RGBScaling = 255.0f / maxValue;


	if (maxValue == 255) 
	{
		for(i = 0; i < totalPixels; i++) 
		{
			fscanf(fileID,"%d %d %d",&red, &green, &blue );

			imageData[3*totalPixels - 3*i - 3] = red;
			imageData[3*totalPixels - 3*i - 2] = green;
			imageData[3*totalPixels - 3*i - 1] = blue;
		}
	}
	else  
	{
		for(i = 0; i < totalPixels; i++) 
		{
			fscanf(fileID,"%d %d %d",&red, &green, &blue );

			imageData[3*totalPixels - 3*i - 3] = red   * RGBScaling;
			imageData[3*totalPixels - 3*i - 2] = green * RGBScaling;
			imageData[3*totalPixels - 3*i - 1] = blue  * RGBScaling;
		}
	}
	fclose(fileID);

	return imageData;
}

void loadTexture(){
	
	seaTexture = loadImage("texturesPPM/sea02.ppm");
	imageHeight = 0;
	imageWidth = 0;
	skyTexture = loadImage("texturesPPM/sky08.ppm");

}

void moveShip(){
	if (isMovingForward == 1){
		cameraLerp(&cameraPosition[2], 0);
	}
	if (isMovingBackward == 1){
		cameraLerp(&cameraPosition[2], 1);
	}
	if (isMovingLeft == 1){
		cameraLerp(&cameraPosition[0], 0);
	}
	if (isMovingRight == 1){
		cameraLerp(&cameraPosition[0], 1);
	}
	if (isMovingUp == 1){
		cameraLerp(&cameraPosition[1], 0);
	}
	if (isMovingDown == 1){
		cameraLerp(&cameraPosition[1], 1);
	}
}

int getColorIndex(groupNumber){
	int colorIndex = 0;
	if (groupNumber < 4){

	}
	else if (groupNumber < 6){
		colorIndex = 1;
	}
	else if (groupNumber < 7){
		colorIndex = 2;
	}
	else if (groupNumber < 8){
		colorIndex = 3;
	}
	else if (groupNumber < 11){
		colorIndex = 4;
	}
	else if (groupNumber < 12){
		colorIndex = 5;
	}
	else if (groupNumber < 14){
		colorIndex = 6;
	}
	else if (groupNumber < 26){
		colorIndex = 7;
	}
	else {
		colorIndex = 8;
	}
	return colorIndex;
}

void drawAxis(){
	GLUquadricObj *quadric;

	glTranslatef(0,0,-50);
	glLineWidth(2);
	glBegin(GL_LINES);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
		glMaterialfv(GL_FRONT, GL_AMBIENT, red);
		glMaterialfv(GL_FRONT, GL_SPECULAR, red);
		glVertex3f(0,0,0);
		glVertex3f(5,0,0);
	glEnd();
	glBegin(GL_LINES);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
		glMaterialfv(GL_FRONT, GL_AMBIENT, green);
		glMaterialfv(GL_FRONT, GL_SPECULAR, green);
		glVertex3f(0,0,0);
		glVertex3f(0,5,0);
	glEnd();
	glBegin(GL_LINES);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
		glMaterialfv(GL_FRONT, GL_AMBIENT, blue);
		glMaterialfv(GL_FRONT, GL_SPECULAR, blue);
		glVertex3f(0,0,0);
		glVertex3f(0,0,5);
	glEnd();

	glMaterialfv(GL_FRONT, GL_DIFFUSE, grey);
	glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
	glMaterialfv(GL_FRONT, GL_SPECULAR, grey);
	
	quadric = gluNewQuadric();
	gluSphere(quadric, .5, 10, 10);
	gluDeleteQuadric(quadric);
}

void drawGrid(){
	int i, j;
	glPushMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1);
	
	glTranslatef(-GRID_SIZE*6.0f, 0.0f, -GRID_SIZE*6.0f);
	for (i = 0; i < 10; i++){
		for (j = 0; j < 10; j++){
			glTranslatef(GRID_SIZE * 1.0f, 0.0f, 0.0f);
			glBegin(GL_QUADS);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
			glMaterialfv(GL_FRONT, GL_AMBIENT, green);
			glNormal3f(0,0,0);
			glVertex3f(0,0,0);
			
			glNormal3f(GRID_SIZE,0,0);
			glVertex3f(GRID_SIZE,0,0);
			
			glVertex3f(GRID_SIZE,0,GRID_SIZE);
			glNormal3f(GRID_SIZE,0,GRID_SIZE);
			
			glNormal3f(0,0,GRID_SIZE);
			glVertex3f(0,0,GRID_SIZE);
			glEnd();
		}
		glTranslatef(-GRID_SIZE*10, 0, 0);
		glTranslatef(0, 0, GRID_SIZE);
	}
	glPopMatrix();
}

void drawCylinderAndDisk(){
	GLUquadricObj *quadric;
	GLfloat fogColor[4] = {1,0,1,1};
	glMaterialfv(GL_FRONT, GL_EMISSION, white);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	glPushMatrix();

	glTranslatef(0, -30, -100);
	glRotatef(270, 1, 0, 0);
	glRotatef(theta/2, 0, 0, 1);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1600, 1200, 0, GL_RGB, GL_UNSIGNED_BYTE, seaTexture);

	quadric = gluNewQuadric();
	//gluQuadricNormals(quadric, GL_SMOOTH);
	
	
	gluQuadricTexture(quadric, GL_TRUE);
	glEnable(GL_FOG);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_MODE, GL_EXP);
	glFogf(GL_FOG_DENSITY, 0.002);
	
	gluDisk(quadric, .1, 300, 20, 20);

	glDisable(GL_FOG);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 896, 385, 0, GL_RGB, GL_UNSIGNED_BYTE, skyTexture);
	

	quadric = gluNewQuadric();
	
	gluCylinder(quadric, 200, 200, 400, 200, 200);
	gluDeleteQuadric(quadric);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glMaterialfv(GL_FRONT, GL_EMISSION, black);

	
}


void drawSea(){
	




	// draw sea here



	
}

void drawPlane() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();
	planePosition[0] = cameraPosition[0];
	planePosition[1] = cameraPosition[1];
	planePosition[2] = cameraPosition[2];
	glTranslatef(planePosition[0], planePosition[1], planePosition[2]);
	glRotatef(270, 0, 1, 0);
	glCallList(thePlane);
	
	glPushMatrix();
	
	glPushMatrix();
	
	glTranslatef(0, 0, -.7);
	
	glCallList(propeller);
	glPopMatrix();


	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glCallList(propeller);
	glPopMatrix();

	glPopMatrix();
}

void setUpPlane() {
	int i = 0;
	int j = 0;
	int groupingCount = -1;
	int isFace = 0;
	char firstChar;
	char *token;

	// Set up a file
	FILE * fileStream;
	// Char array to store
	char string[200];
	fileStream = fopen("cessna.txt", "rt");

	if (fileStream != NULL)
	{
		thePlane = glGenLists(1);
	  	glNewList(thePlane, GL_COMPILE);

		while(fgets(string, 100, fileStream) != NULL)
		{
			firstChar = ' ';
			if(sscanf(string, "v %f %f %f ", &modelVertices[i].x, &modelVertices[i].y, &modelVertices[i].z) == 3) {
				i++;
			} 
			else if(sscanf(string, "n %f %f %f ", &modelNormals[j].x, &modelNormals[j].y, &modelNormals[j].z) == 3) {
				j++;
			} 
			else if(sscanf(string, "%c", &firstChar) == 1) { 
				if(firstChar == 'f') {
					token = strtok(string, " ");
					token = strtok(NULL, " ");
					glBegin(GL_POLYGON);
						glLineWidth(1);
						while(token != NULL && token[0] != 10) {
							
							glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
							if(groupingCount <= 3) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							}
							else if(groupingCount <= 5) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, black);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							} 
							else if(groupingCount <= 6) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, purple);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							} 
							else if(groupingCount <= 7) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							} 
							else if(groupingCount <= 10) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							} 
							else if(groupingCount <= 11) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, black);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							} 
							else if(groupingCount <= 13) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							} 
							else if(groupingCount <= 25) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							} 
							else if(groupingCount <= 32) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							} 
							else {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
								glMaterialfv(GL_FRONT, GL_SPECULAR, white);
							}
							glNormal3f(modelNormals[atoi(token)-1].x, modelNormals[atoi(token)-1].y, modelNormals[atoi(token)-1].z);
							glVertex3f(modelVertices[atoi(token)-1].x, modelVertices[atoi(token)-1].y, modelVertices[atoi(token)-1].z);
							token = strtok(NULL, " ");
						}
					glEnd();
				} 
				else if (firstChar == 'g') {
					// Increase object count
					groupingCount++;
				}
			}
		}
		// End the display list
		glEndList();
	}
	fclose (fileStream);
}

void handleControls(){
	if(isTurningLeft){
		//planePosition[0]+=.2;
	}
	else if (isTurningRight){
		//planePosition[0]-=.2;
	}
}

void myDisplay(){
	int grid = 0;
	GLfloat position[] = {0, 50.0, 0.0, 1};
	handleControls();

	glEnable(GL_LINE_SMOOTH | GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();

	cameraPosition[2] -= .5;

	if (isTurningRight){
		cameraPosition[0] += 2;
		gluLookAt(cameraPosition[0], cameraPosition[1] + 2, cameraPosition[2] + 5,
				  cameraPosition[0] + 2, cameraPosition[1], cameraPosition[2] - 10,
				  0, 1, 0);
	}
	else if (isTurningLeft){
		cameraPosition[0] -= 2;
		gluLookAt(cameraPosition[0], cameraPosition[1] + 2, cameraPosition[2] + 5,
				  cameraPosition[0] - .2, cameraPosition[1], cameraPosition[2] - 10,
				  0, 1, 0);
	}
	else {
		gluLookAt(cameraPosition[0], cameraPosition[1] + 2, cameraPosition[2] + 5,
				  cameraPosition[0], cameraPosition[1], cameraPosition[2] - 10,
				  0, 1, 0);
	}

	glLightfv(GL_LIGHT0, GL_POSITION, position);


	
	drawPlane();

	drawAxis();
	
	if (grid){
		drawGrid();
	}
	else{
		drawCylinderAndDisk();
	}
	drawSea();

	glutSwapBuffers();
}

void keyboardDownFunction(char key, int x, int y){
	switch (key){
		case 'd': isInDiscoMode = isInDiscoMode ? 0 : 1;
				  break;
		case 'r': isShowingRings = isShowingRings ? 0 : 1;
				  break;
		case 's': isShowingStars = isShowingStars ? 0 : 1;
				  break;
		case 'c': isShowingCorona = isShowingCorona ? 0 : 1;
				  break;			  
		case 'k': isShowingShields = isShowingShields ? 0 : 1;
				  break;
		default: break;
	}
}

void specialDownFunction(int key, int x, int y){
	switch (key){
		case GLUT_KEY_PAGE_UP: isMovingForward = 1;
				  break;
		case GLUT_KEY_LEFT: isTurningLeft = 1;
				  break;
		case GLUT_KEY_PAGE_DOWN: isMovingBackward = 1;
				  break;
		case GLUT_KEY_RIGHT: isTurningRight = 1;
				  break;
		case GLUT_KEY_UP: isMovingUp = 1;
				  break;
		case GLUT_KEY_DOWN: isMovingDown = 1;
				  break;
		default: break;
	}

}

void specialUpFunction(int key, int x, int y){
	switch (key){
		case GLUT_KEY_PAGE_UP: isMovingForward = 0;
				  break;
		case GLUT_KEY_LEFT: isTurningLeft = 0;
				  break;
		case GLUT_KEY_PAGE_DOWN: isMovingBackward = 0;
				  break;
		case GLUT_KEY_RIGHT: isTurningRight = 0;
				  break;
		case GLUT_KEY_UP: isMovingUp = 0;
				  break;
		case GLUT_KEY_DOWN: isMovingDown = 0;
				  break;
		default: break;
	}
}


void timerFunction(){
	theta++;

	glutPostRedisplay();
	glutTimerFunc(5, timerFunction, 0);
}

void setUpProp() {
	int i = 0;
	int j = 0;
	int groupingCount = -1;
	int isFace = 0;
	char firstChar;
	char *token;

	FILE *fileStream;
	
	char string[100];
	fileStream = fopen("propeller.txt", "rt");

	if (fileStream != NULL)
	{
		propeller = glGenLists(1);
	  	glNewList(propeller, GL_COMPILE);

		while(fgets(string, 100, fileStream) != NULL)
		{
			firstChar = ' ';
			if(sscanf(string, "v %f %f %f ", &propellerVertices[i].x, &propellerVertices[i].y, &propellerVertices[i].z) == 3) {
				i++;
			} 
			else if(sscanf(string, "n %f %f %f ", &propellerNormals[j].x, &propellerNormals[j].y, &propellerNormals[j].z) == 3) {
				j++;
			} 
			else if(sscanf(string, "%c", &firstChar) == 1) {
				if(firstChar == 'f') {
					// Check for faces
					token = strtok(string, " ");
					// Get next token
					token = strtok(NULL, " ");

					// Draw polygon for this face
					glBegin(GL_POLYGON);
						glLineWidth(1);
						while(token != NULL && token[0] != 10) {
							// Draw the normal and point
							glMaterialf(GL_FRONT, GL_SHININESS, 100.0f);
							if(groupingCount <= 0) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, orange);
								glMaterialfv(GL_FRONT, GL_AMBIENT, orange);
							} else if(groupingCount <= 1) {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
								glMaterialfv(GL_FRONT, GL_AMBIENT, red);
							} else {
								glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
								glMaterialfv(GL_FRONT, GL_AMBIENT, yellow);
							}
							// Get normal and draw color
							glNormal3f(propellerNormals[atoi(token)-1].x, propellerNormals[atoi(token)-1].y, propellerNormals[atoi(token)-1].z);
							glVertex3f(propellerVertices[atoi(token)-1].x, propellerVertices[atoi(token)-1].y, propellerVertices[atoi(token)-1].z);
							// Get next token
							token = strtok(NULL, " ");
						}
					glEnd(); // End drawing of polygon
				} 
				else if (firstChar == 'g') {
					// Increase object count
					groupingCount++;
				}
			}
		}
		// End the display list
		glEndList();
	}
	fclose (fileStream);
}


int main(int argc, char *argv[]){
	GLfloat diffuse[] = {1, 1, 1, 1};
	GLfloat ambient[] = {1, 1, 1, 1};
	GLfloat specular[] = {1, 1, 1, 1};
		GLfloat position[] = {0, 10.0, 0.0, 1};

	GLfloat globalAmbient[] = {.2, .2, .2, 1};	
	srand(time(NULL));
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Flight");
	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(keyboardDownFunction);
	glutSpecialFunc(specialDownFunction);
	glutSpecialUpFunc(specialUpFunction);

	glClearColor(0, 0, 0, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 1, 1000);
	glMatrixMode(GL_MODELVIEW);

	
	glEnable(GL_LIGHTING); 
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	

	glLightfv(GL_LIGHT0, GL_POSITION, position);
	setUpPlane();
	setUpProp();

	loadTexture();

	glutTimerFunc(100, timerFunction, 0);
	
	glutMainLoop();
}