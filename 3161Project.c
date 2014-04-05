#define _CRT_SECURE_NO_WARNINGS
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

//Necessary defines
#define VERTEX_COUNT 6763
#define FACE_COUNT 3640
#define PROP_FACE_COUNT 132
#define MOUNTAIN_MESH_DENSITY 21
#define MOUNTAIN_COUNT 4

// Set up constants
const GLint WINDOW_WIDTH = 800;
const GLint WINDOW_HEIGHT = 600;
const GLint GRID_SIZE = 5;

// New type to hold color information
typedef GLfloat color4f[4];

// For laoding textures
int imageWidth,
	imageHeight;

// The heightmaps for each mountain
GLfloat heightMap[MOUNTAIN_COUNT][MOUNTAIN_MESH_DENSITY][MOUNTAIN_MESH_DENSITY];

float moveSpeed = 0.01f,	// how fast the plane is moving
	  turningAngle = 0.0f,	// The angle at which the plane is turning
	  turningIncrement = 0,	// the amount turning angle has been incremented
	  currentPlaneAngle = 0,// the current angle of the plane
	  planeTilt = 0,		// How much to tilt the plane
	  groundHeight = -10;

// Display lists for the plane and propellers
GLuint planeDisplayList = 0;
GLuint propellerDisplayList = 0;

// Flag variables which determine whether or not to display certain features
GLint isMovingUp = 0,
	  isMovingDown = 0,
	  isMovingLeft = 0,
	  isMovingRight = 0,
	  isMovingForward = 0,
	  isMovingBackward = 0,
	  isTurningLeft = 0,
	  isTurningRight = 0,
	  isDrawingSolid = 0,
	  isSpeedingUp = 0,
	  isSlowingDown = 0,
	  isShowingFog = 0,
	  isFullscreen = 0,
	  isBarrelRolling = 0,
	  isShowingTexture = 0,
	  isShowingMountainTexture = 0,
	  isShowingMountains = 0,
	  isInDiscoMode = 0;

// How much change has occured
GLfloat theta = 0.0;

// The positon of the plane and camera to start
GLfloat cameraPosition[] = {0,8.5,5};
GLfloat planePosition[] = {0,7,-2};

// color definitions 
color4f yellow = {1.0f,1.0f,0.0f,1.0f};
color4f black = {0.0f,0.0f,0.0f,1.0f};
color4f blue = {0.0f,0.0f,1.0f,1.0f};
color4f purple = {.8f,.8f,0.0f,1.0f};
color4f red = {1.0f,0.0f,0.0f,1.0f};
color4f grey = {0.1f,0.1f,0.1f,1.0f};
color4f white = {1.0f,1.0f,1.0f,1.0f};
color4f orange = {1.0f,.7f,0.0f,1.0f};
color4f green = {0.0f,1.0f,0.0f,1.0f};

// The sea, sky, and mountain textures as arrays of unsigned bytes
GLubyte *seaTexture;
GLubyte *skyTexture;
GLubyte *mountainTexture;

// struct which holds 3 floats
typedef struct vector3f {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

//struct which holds 3 ints
typedef struct vector3i {
	GLint x;
	GLint y;
	GLint z;
};

// struct which holds 4 ints
typedef struct vector4i {
	GLint x;
	GLint y;
	GLint z;
	GLint w;
};

// vertices and normals for the plane and propeller
struct vector3f modelVertices[VERTEX_COUNT];
struct vector3f modelNormals[VERTEX_COUNT];
struct vector3f propellerVertices[VERTEX_COUNT];
struct vector3f propellerNormals[VERTEX_COUNT];

// Returns log base 2 of a number, since log2 is not part fo standard c math implementation
double log2( double n )  
{  
    return log( n ) / log( 2 );  
}

// linear interpolation function
GLfloat lerp(GLfloat a, GLfloat b, GLfloat time){
	return a + (b - a) * time;
}

// linear interpolation on camera function
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

	fscanf(fileID, "%c", &tempChar);

	while(tempChar == '#') 
	{
		fscanf(fileID, "%[^\n] ", headerLine);

		printf("%s\n", headerLine);
		
		fscanf(fileID, "%c",&tempChar);
	}

	ungetc(tempChar, fileID); 

	fscanf(fileID, "%d %d %d", &imageWidth, &imageHeight, &maxValue);

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

// Function to load all three textures required for the scene
void loadTexture(){
	
	seaTexture = loadImage("texturesPPM/sea02.ppm");
	imageHeight = 0;
	imageWidth = 0;
	skyTexture = loadImage("texturesPPM/sky08.ppm");
	imageHeight = 0;
	imageWidth = 0;
	mountainTexture = loadImage("texturesPPM/mount03.ppm");

}

// Draws the three axis lines and the grey sphere at the center
void drawAxis(){
	GLUquadricObj *quadric;

	// Place it in front of the camera
	glTranslatef(0,0,-50);
	glLineWidth(2);
	
	// Draw the red x line
	glBegin(GL_LINES);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
	glMaterialfv(GL_FRONT, GL_AMBIENT, red);
	glMaterialfv(GL_FRONT, GL_SPECULAR, red);
	glVertex3f(0,0,0);
	glVertex3f(5,0,0);
	glEnd();
	
	// draw the green y line
	glBegin(GL_LINES);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
	glMaterialfv(GL_FRONT, GL_AMBIENT, green);
	glMaterialfv(GL_FRONT, GL_SPECULAR, green);
	glVertex3f(0,0,0);
	glVertex3f(0,5,0);
	glEnd();
	
	// draw the blue z line
	glBegin(GL_LINES);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
	glMaterialfv(GL_FRONT, GL_AMBIENT, blue);
	glMaterialfv(GL_FRONT, GL_SPECULAR, blue);
	glVertex3f(0,0,0);
	glVertex3f(0,0,5);
	glEnd();

	// Set the material for the sphere
	glMaterialfv(GL_FRONT, GL_DIFFUSE, grey);
	glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
	glMaterialfv(GL_FRONT, GL_SPECULAR, grey);
	
	// Draw the sphere
	quadric = gluNewQuadric();
	gluSphere(quadric, .5, 10, 10);
	gluDeleteQuadric(quadric);
}

// Function to alter a height map. Allows for different values for each mountain to be randomly 
// generated each time
void alterHeightMap(int mid, float alteration, int mapIndex){
	int i = 0,
		j = 0,
		k = 0,
		debug = 0;

	GLint density = MOUNTAIN_MESH_DENSITY;
	srand(glutGet(GLUT_ELAPSED_TIME));

	// print out info if we are debugging
	if (debug){
		for (i = 0; i < density; i++){
			for (k = 0; k < density; k++){
				printf("%1.2f ", heightMap[mapIndex][i][k]);
			}
			printf("\n");
		}

		printf("---------------\n");
	}
	
	// apply the alteration to a row of cooridinates on the mountain
	// left side
	for (j = 0; j < density - 2*mid; j++){
		heightMap[mapIndex][mid][mid + j] += sin(j)*((rand()%10) / 10.0)*alteration;
	}

	// far side
	for (j = 1; j < density - 2*mid; j++){
		heightMap[mapIndex][mid + j][mid] += sin(j)*((rand()%10) / 10.0)*alteration;
	}

	// right side
	for (j = 1; j < density - 2*mid; j++){
		heightMap[mapIndex][density-mid-1][mid + j] += cos(j)*((rand()%10) / 10.0)*alteration;
	}

	// close side
	for (j = 1; j < density - 2*mid-1; j++){
		heightMap[mapIndex][mid + j][density-mid-1] += sin(j)*((rand()%10) / 10.0)*alteration;
	}

	if (debug){
		for (i = 0; i < density; i++){
			for (k = 0; k < density; k++){
				printf("%1.2f ", heightMap[mapIndex][i][k]);
			}
			printf("\n");
		}
	}
}

// Fucntion to recursively alter the heights of a mountain by dividing it in two, and continuing to do so
// until several levels of recursion have occurred and the mountain has been suitably altered.
void recursivelyDivideMountains(int division, int mapIndex){
	if (division <= 0 ){
		return;
	}
	else{
		// Calculate row to deform, then deform each row
		int index = MOUNTAIN_MESH_DENSITY / division;
		alterHeightMap(index, division, mapIndex);
		alterHeightMap(division, division, mapIndex);
	}

	// Call self
	recursivelyDivideMountains(division-1, mapIndex);
}


// Function to initialize each mountain
void initMountains(){
	int k = 0,
		i = 0,
		j = 0,
		density = MOUNTAIN_MESH_DENSITY;

	// Calculate the number of division by getting the log base 2 of density
	GLint divisions = (int)floor(log2(density));

	//Initialize each mountain with a different seed for rand()
	for (k = 0; k < MOUNTAIN_COUNT; k++){
		for (i = 0; i < density; i++){
			for (j = 0; j < density; j++){
				// Creates a default height map for each mountain in the shape of a pyramid, 
				//which is later deformed with the recursive alteration function
				if (j > density / 2){
					heightMap[k][i][j] = density - j - 1;	
				}
				else{
					heightMap[k][i][j] = j;
				}
				if (i < density / 2){
					if (heightMap[k][i][j] > i){
						heightMap[k][i][j] = i;
					}
				}
				else{
					if (heightMap[k][i][j] > density - i - 1){
						heightMap[k][i][j] = density - i - 1;
					}	
				}
				
			}
		}

		// Always make the middle point large
		heightMap[k][i][j] *= 1.5;

		// Commence altering each height map
		recursivelyDivideMountains(divisions, k);
	}
}

// Function to draw each mountain
void drawMountains(){
	int i, j, m;
	GLfloat spacing = 1;
	GLint density = MOUNTAIN_MESH_DENSITY;

	glPushMatrix();
	// Set the drawing mode based on the flag
	if (isDrawingSolid){
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glLineWidth(1);
	
	// Move to a reasonable position
	glTranslatef(-10, groundHeight, -40);
	
	// Draw the mountain texture if the flag is set
	if (isShowingMountainTexture){
		glShadeModel(GL_SMOOTH);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1104, 1280, 0, GL_RGB, GL_UNSIGNED_BYTE, mountainTexture);
		glEnable(GL_TEXTURE_2D);
	}
	// Draw each mountain
	for (m = 0; m < MOUNTAIN_COUNT; m++){
		// Translate based on which mountain we are designing so that the 
		// mountains appear in different places. Also scale them differently
		// so that we have different sizes
		if (m == 0){
			glTranslatef(-30, 0, - 80);	
			glScalef(2,2,2);
		}
		else if (m == 1){
			glTranslatef(100, 0, 10);
			glScalef(.5,4,.5);
		}
		else if (m == 2){
			glTranslatef(-110, 0, 50);
			glScalef(4,1,4);
		}
		else if (m == 3){
			glTranslatef(120, 0, 0);
			glScalef(2,4,2);
		}

		// Begin new quads and draw the mountain
		glBegin(GL_QUADS);
		for (i = 0; i < density-1; i++){
			for (j = 0; j < density-1; j++){
				color4f mountainWhite;
				color4f mountainGreen;
				// color the mountain as a function of height
				mountainWhite[0] = (heightMap[m][i][j] / (.6 * density/2));
				mountainWhite[1] = (heightMap[m][i][j]  / (.6 * density/2));
				mountainWhite[2] = (heightMap[m][i][j]  / (.6 * density/2));
				mountainWhite[3] = 1;

				
				mountainGreen[0] = 0;
				mountainGreen[1] = (heightMap[m][i][j] / (.6 * density/2));
				mountainGreen[2] = 0;
				mountainGreen[3] = 1;

				if (heightMap[m][i][j] > .6 * density/2){
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mountainWhite);
					glMaterialfv(GL_FRONT, GL_AMBIENT, mountainWhite);
				}
				else{
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mountainGreen);
					glMaterialfv(GL_FRONT, GL_AMBIENT, mountainGreen);
				}
				// Set texture coords and make the mountains dance if the flag is set
				// Repeat this for each of the four corners of each quad
				glTexCoord2f(i,j);
				if (isInDiscoMode){
					if (sin(theta*.01) > 0){
						glNormal3f(i,sin(theta*.01)*heightMap[m][i][j],j);
						glVertex3f(i,sin(theta*.01)*heightMap[m][i][j],j);
					}
					else{
						glNormal3f(i,-sin(theta*.01)*heightMap[m][i][j],j);
						glVertex3f(i,-sin(theta*.01)*heightMap[m][i][j],j);
					}
				}
				else{
					glNormal3f(i,heightMap[m][i][j],j);
					glVertex3f(i,heightMap[m][i][j],j);	
				}
				if (heightMap[m][i][j+1] > .6 * density/2){
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mountainWhite);
					glMaterialfv(GL_FRONT, GL_AMBIENT, mountainWhite);
				}
				else{
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mountainGreen);
					glMaterialfv(GL_FRONT, GL_AMBIENT, mountainGreen);
				}
				glTexCoord2f(i,j+1);
				if (isInDiscoMode){
					if (sin(theta*.01) > 0){
						glNormal3f(i,sin(theta*.01)*heightMap[m][i][j+1],j+1);
						glVertex3f(i,sin(theta*.01)*heightMap[m][i][j+1],j+1);	
					}
					else{
						glNormal3f(i,-sin(theta*.01)*heightMap[m][i][j+1],j+1);
						glVertex3f(i,-sin(theta*.01)*heightMap[m][i][j+1],j+1);	
					}
				}
				else{
					glNormal3f(i,heightMap[m][i][j+1],j+1);
					glVertex3f(i,heightMap[m][i][j+1],j+1);
				}

				if (heightMap[m][i+1][j+1] > .6 * density/2){
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mountainWhite);
					glMaterialfv(GL_FRONT, GL_AMBIENT, mountainWhite);
				}
				else{
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mountainGreen);
					glMaterialfv(GL_FRONT, GL_AMBIENT, mountainGreen);
				}
				glTexCoord2f(i+1,j+1);

				if (isInDiscoMode){
					if (sin(theta*.01) > 0){
						glVertex3f(i+1,sin(theta*.01)*heightMap[m][i+1][j+1],j+1);
						glNormal3f(i+1,sin(theta*.01)*heightMap[m][i+1][j+1],j+1);
					}
					else{
						glVertex3f(i+1,-sin(theta*.01)*heightMap[m][i+1][j+1],j+1);
						glNormal3f(i+1,-sin(theta*.01)*heightMap[m][i+1][j+1],j+1);
					}
				}
				else{
					glVertex3f(i+1,heightMap[m][i+1][j+1],j+1);
					glNormal3f(i+1,heightMap[m][i+1][j+1],j+1);	
				}

				if (heightMap[m][i+1][j] > .6 * density/2){
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mountainWhite);
					glMaterialfv(GL_FRONT, GL_AMBIENT, mountainWhite);
				}
				else{
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mountainGreen);
					glMaterialfv(GL_FRONT, GL_AMBIENT, mountainGreen);
				}
				glTexCoord2f(i+1,j);
				if (isInDiscoMode){
					if (sin(theta*.01) > 0){
						glNormal3f(i+1,sin(theta*.01)*heightMap[m][i+1][j],j);
						glVertex3f(i+1,sin(theta*.01)*heightMap[m][i+1][j],j);
					}
					else{
						glNormal3f(i+1,-sin(theta*.01)*heightMap[m][i+1][j],j);
						glVertex3f(i+1,-sin(theta*.01)*heightMap[m][i+1][j],j);
					}
				}
				else{
					glNormal3f(i+1,heightMap[m][i+1][j],j);
					glVertex3f(i+1,heightMap[m][i+1][j],j);	
				}

			}
		}
		glEnd();

		// Undo the scaling that was done to resize each individual mountain
		if (m == 0){
			glScalef(.5,.5,.5);
		}
		else if (m == 1){
			glScalef(2,.25,2);
		}
		else if (m == 2){
			glScalef(.25,1,.25);
		}
		else if (m == 3){
			glScalef(.5,.25,.5);
		}
	}

	// Turn off texture mode and pop the matrix so we dont affect future 
	// transformations
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

// A function to draw a 100 x 100 grid to use as a frame of
// reference, and as described in the assignment
void drawGrid(){
	int i, j;
	glPushMatrix();
	// Set the draw mode based on the current state of the flag
	if (isDrawingSolid){
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glLineWidth(1);
	
	// Move the grid so that it will appear centered and in
	// front of the camera
	glTranslatef(-GRID_SIZE*6.0f, 0.0f, -GRID_SIZE*6.0f);
	for (i = 0; i < 10; i++){
		for (j = 0; j < 10; j++){
			// Draw each portion of the grid
			glTranslatef(GRID_SIZE * 1.0f, 0.0f, 0.0f);
			glBegin(GL_QUADS);
			// Set materials
			glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
			glMaterialfv(GL_FRONT, GL_AMBIENT, green);

			// Set vertices and normals
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
		// Translate so the next row is in the correct position
		glTranslatef(-GRID_SIZE*10, 0, 0);
		glTranslatef(0, 0, GRID_SIZE);
	}
	glPopMatrix();
}


// A function to draw the cylinder and disk. It will also texture them if 
// the flag is set
void drawCylinderAndDisk(){
	// Create new quadric and set fog color
	GLUquadricObj *quadric;
	GLfloat fogColor[4] = {1,0,1,1};
	
	// Set the polygon mode and apply textures if
	// the correct flag is set
	if (isDrawingSolid == 0){
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (isDrawingSolid){
		glMaterialfv(GL_FRONT, GL_EMISSION, white);
		glEnable(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	}
	glPushMatrix();

	// translate so the ocean is at "ground height" and rotate so that
	// the cylinder appears correctly
	glTranslatef(0, groundHeight, -100);
	glRotatef(270, 1, 0, 0);

	// Rotate the disc to get funky water effects
	glRotatef(theta/4, 0, 0, 1);

	// Set up and enable seas texture if flag is set
	if (isDrawingSolid){
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1600, 1200, 0, GL_RGB, GL_UNSIGNED_BYTE, seaTexture);
	}
	
	quadric = gluNewQuadric();
	
	if (isDrawingSolid){
		gluQuadricTexture(quadric, GL_TRUE);
	}
	
	// Draw the fog is the flag is true
	if (isShowingFog){
		glEnable(GL_FOG);
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_MODE, GL_EXP);
		glFogf(GL_FOG_DENSITY, 0.002);
	}
	
	// draw the disk
	gluDisk(quadric, .1, 300, 20, 20);

	// disable fog
	if (isShowingFog){
		glDisable(GL_FOG);
	}

	// set up the texture for the sky
	if (isDrawingSolid){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 896, 385, 0, GL_RGB, GL_UNSIGNED_BYTE, skyTexture);
	}
	
	// reinitialize the quadric object
	quadric = gluNewQuadric();
	
	// Draw the cylinder, free the memory, and disable texture mode
	gluCylinder(quadric, 200, 200, 150, 100, 100);
	gluDeleteQuadric(quadric);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	// Disable emissive material
	glMaterialfv(GL_FRONT, GL_EMISSION, black);

	
}

// Function to draw the plane
void drawPlane() {
	// Rotate the plane based on the mouse position
	float rotation = (currentPlaneAngle * 100 / glutGet(GLUT_WINDOW_WIDTH) * .6) - 30;
	// Set correct polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();

	// Move the plane to the contents of planePosition, and rotate it
	// so that it faces the correct way in front of the camera
	glTranslatef(planePosition[0], planePosition[1], planePosition[2]);
	glRotatef(-turningAngle, 0, 1, 0);
	glRotatef(270, 0, 1, 0);
	
	// Clamp rotation, and display the plane tilted to the correct side
	if (rotation <= 30 && rotation >= -30){
		glRotatef(-rotation , 1, 0, 0);	
	}
	else if (rotation > 30){
		glRotatef(-30, 1, 0, 0);
	}
	else if (rotation < -30){
		glRotatef(30, 1, 0, 0);
	}

	// Tilt the plane up or down if it is currently moving 
	// up or down, and lerp it back to 0 if not
	if (isMovingUp){
		if (planeTilt > -15){
			planeTilt = lerp(planeTilt, -15, .2);
			glRotatef(planeTilt, 0, 0, 1);
		}
	}
	else if (isMovingDown){
		if (planeTilt < 15){
			planeTilt = lerp(planeTilt, 15, .2);
			glRotatef(planeTilt, 0, 0, 1);
		}
	}
	else{
		planeTilt = lerp(planeTilt, 0.1f, .2);
		glRotatef(planeTilt, 0, 0, 1);
	}
	
	// DO A BARREL ROLL
	if (isBarrelRolling){
		glRotatef(theta*15,1,0,0);
	}

	// draw the plane
	glCallList(planeDisplayList);
	
	// Setup the propellers in front of the plane at the
	// right positions and draw them
	glPushMatrix();
	glTranslatef(0,0,0);
	glCallList(propellerDisplayList);
	glPopMatrix();
	
	// Draw the second propeller
	glPushMatrix();
	glTranslatef(0, 0, -.7);
	glCallList(propellerDisplayList);
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
		planeDisplayList = glGenLists(1);
	  	glNewList(planeDisplayList, GL_COMPILE);

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

void myDisplay(){
	int grid = 0;
	GLfloat position[] = {0, 50.0, 0.0, 1};

	glEnable(GL_LINE_SMOOTH | GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	turningAngle += turningIncrement;

	glLoadIdentity();

	if (isSpeedingUp){
		moveSpeed += .01;
		if (moveSpeed > .3){
			moveSpeed = .3;
		}
	}
	else if (isSlowingDown){
		moveSpeed -= .01;
		if (moveSpeed < 0.01){
			moveSpeed = 0.01;
		}
	}

	if (isMovingUp){
		planePosition[1]+=.1;
	}
	else if (isMovingDown){
		planePosition[1]-=.1;
	}
	
	planePosition[0] += sin(turningAngle * 3.1415/180.0f) * moveSpeed;
	planePosition[2] -= cos(turningAngle * 3.1415/180.0f) * moveSpeed;


	gluLookAt(planePosition[0] + sin(turningAngle * (3.1415/180.0f)) * -5, planePosition[1] + 2, planePosition[2] - cos(turningAngle * (3.1415/180.0f)) * -5,
			  planePosition[0], planePosition[1], planePosition[2],
			  0, 1, 0);
	

	glLightfv(GL_LIGHT0, GL_POSITION, position);


	
	drawPlane();

	if (isShowingMountains){
		drawMountains();
	}

	if (isShowingTexture){
		drawCylinderAndDisk();
	}
	else{
		drawAxis();
		drawGrid();
	}
	
	glutSwapBuffers();
}

void alternateFullscreenAndWindowed(int fullscreen){
	if (isFullscreen){
		glutPositionWindow(0,0);
        glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
	}
	else{
		glutFullScreen();
	}
	isFullscreen = isFullscreen ? 0 : 1;
}

void keyboardDownFunction(char key, int x, int y){
	switch (key){
		case 'd': isInDiscoMode = isInDiscoMode ? 0 : 1;
				  break;
		case 'w': isDrawingSolid = isDrawingSolid ? 0 : 1;
				  break;
		case 'f': alternateFullscreenAndWindowed(isFullscreen);
				  break;
		case 'q': exit(1);
				  break;
		case 's': isShowingTexture = isShowingTexture ? 0 : 1;
				  break;
		case 'b': isShowingFog = isShowingFog ? 0 : 1;
				   break;
		case 'p': isBarrelRolling = isBarrelRolling ? 0 : 1;
				  break;
		case 't': isShowingMountains = isShowingMountains ? 0 : 1;
				  break;
		case 'm': isShowingMountainTexture = isShowingMountainTexture ? 0 : 1;
				  break;
		default: break;
	}
}

void specialDownFunction(int key, int x, int y){
	switch (key){
		case GLUT_KEY_PAGE_UP: isSpeedingUp = 1;
				  break;
		case GLUT_KEY_LEFT: isTurningLeft = 1;
				  break;
		case GLUT_KEY_PAGE_DOWN: isSlowingDown = 1;
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
		case GLUT_KEY_PAGE_UP: isSpeedingUp = 0;
				  break;
		case GLUT_KEY_LEFT: isTurningLeft = 0;
				  break;
		case GLUT_KEY_PAGE_DOWN: isSlowingDown = 0;
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
		propellerDisplayList = glGenLists(1);
	  	glNewList(propellerDisplayList, GL_COMPILE);

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

void mouseFunction(int x, int y){
	int centerCoord;

	currentPlaneAngle = x;
	centerCoord = glutGet(GLUT_WINDOW_WIDTH) / 2;

	if (x < centerCoord){
		turningIncrement = -(fabsf((centerCoord - x)) * 100 / glutGet(GLUT_WINDOW_WIDTH) / 2) * .0100;
	}
	else if (x > centerCoord){
		turningIncrement = ((fabsf(centerCoord - x)) * 100 / glutGet(GLUT_WINDOW_WIDTH) / 2) * .0100;
	}
	else{

	}

}

void resizeFunction(int width, int height){
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)width/(float)height, 1, 1000);
	glMatrixMode(GL_MODELVIEW);
}

void printControls(){
	printf("Controls:\n");
	printf("b - toggle fog\n");
	printf("w - wireframe/solid drawing\n");
	printf("f - toggle fullscreen/windowed\n");
	printf("s - toggle grid or sea/sky\n");
	printf("p - enter BARREL ROLL mode\n");
	printf("d - DANCING MOUNTAINS");
	printf("m - toggle mountains\n");
	printf("page up - increase speed\n");
	printf("page down - decrease speed\n");
	printf("up arrow - move up\n");
	printf("down arrow - move down\n");
	printf("mouse - steer plane\n");
	printf("t - toggle mountain textures\n");

}


int main(int argc, char *argv[]){
	GLfloat diffuse[] = {1, 1, 1, 1};
	GLfloat ambient[] = {1, 1, 1, 1};
	GLfloat specular[] = {1, 1, 1, 1};
		GLfloat position[] = {0, 10.0, 0.0, 1};

	GLfloat globalAmbient[] = {.2, .2, .2, 1};	
	srand(time(NULL));
	printControls();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Flight");
	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(keyboardDownFunction);
	glutSpecialFunc(specialDownFunction);
	glutSpecialUpFunc(specialUpFunction);
	glutReshapeFunc(resizeFunction);
	glutPassiveMotionFunc(mouseFunction);

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

	initMountains();

	loadTexture();

	glutTimerFunc(100, timerFunction, 0);
	
	glutMainLoop();
}