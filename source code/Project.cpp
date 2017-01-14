/*
 *  Project: An Interactive 3D Maze Game 
 *  Project.cpp
 * 
 * -----------------------------------------------------------------------------
 *  Student Information
 * -----------------------------------------------------------------------------
 *  Student Name: Hongyuan Jin
 *  Student ID: 1409853G-I011-0046
 *  E-mail: jacelynfish@outlook.com
 *  Major: Computer Technology / Electronic Information / Software Technology
 *  Year: 3
 * ------------------------------------------------------------------------------
 */
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "Bitmap.h"

#define M_PI 3.141592654

// ============ Global variables =======================
// Maze information
#define MAX_MAZESIZE 20
static int _mapx, _mapz; // Size of the maze	
static int _map[MAX_MAZESIZE][MAX_MAZESIZE];
int _initpos[2];         // Initial position of the player

static GLfloat _wallHeight = 1.0; // Height of the wall
static GLfloat _wallScale = 2.0;  // Scale of the width of the wall

// Camera setting
GLfloat _viewangle = 45.0; // Angle of view
GLfloat _viewdepth = 20.0; // View depth

//lighting and material settings
GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[] = { (GLfloat)168/255.0,(GLfloat)115/255.0,
						(GLfloat)107/255.0,0.5 };
GLfloat mat_diffuse[] = { 0.2, 0.2, 0.2, 1.0 };

GLfloat ambient[] = { (GLfloat)168 / 255.0,(GLfloat)115 / 255.0,(GLfloat)107 / 255.0, 0.5 };
GLfloat diffuse[] = { 1, (GLfloat)175 / 255.0, (GLfloat)162 / 255.0, 1.0 };
GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat position[] = { 3.0,3.0,3.0,0.0 };

//door position
int doorI, doorJ;

//player size
GLfloat body_x = 0.3, body_y = 0.25, body_z = 0.3;
GLfloat head_x = 0.175, head_y = 0.22, head_z = 0.25;
GLfloat feet_x = 0.1, feet_y = 0.2, feet_z = 0.1;

//collision detection helpers
int preValidi = 0, preValidj = 0;

//texture settings
int groundTexHeight, groundTexWidth, wallTexHeight, wallTexWidth, 
	tailTexWidth, tailTexHeight, doorTexHeight, doorTexWidth;
GLubyte * groundTex, * wallTex, * tailTex,* doorTex;
GLuint texNames[4];


// Define the player information structure
typedef struct _playerInfo {
   GLfloat degree;  // Object orientation
   GLfloat forward, spin;
   GLfloat pos[3];	// User position
   GLfloat mySize;	// User radial size
   GLfloat forwardStepSize;	// Step size
   GLfloat spinStepSize;	// Rotate step size
} playerInfo;

playerInfo _player;

int _drawmode = 0;

void init();
void initplayer();

// Capture the BMP file
GLubyte* TextureLoadBitmap(char *filename, int *w, int *h) // Bitmap file to load
{
   BITMAPINFO *info; // Bitmap information
   void       *bits; // Bitmap pixel bits
   GLubyte    *rgb;  // Bitmap RGB pixels

   // Try loading the bitmap and converting it to RGB...
   bits = LoadDIBitmap(filename, &info);
   
   if (bits==NULL) 
	  return (NULL);
   
   rgb = ConvertRGB(info, bits);
   
   if (rgb == NULL) {
      free(info);
      free(bits);
   }

   *w = info->bmiHeader.biWidth;
   *h = info->bmiHeader.biHeight;

   // Free the bitmap and RGB images, then return 0 (no errors).
   
   free(info);
   free(bits);
   return (rgb);
}

void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(_viewangle, (GLfloat) w / (GLfloat) h, 0.8, _viewdepth);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

//====== Drawing functions ===============
void DrawGround()
{
	glEnable(GL_TEXTURE_2D);

   glPushMatrix();
   
   //draw exit door here
	glPushMatrix();
		glTranslatef(doorI*_wallScale, 0, doorJ*_wallScale);

		glBindTexture(GL_TEXTURE_2D, texNames[3]);
		glTranslatef(_wallScale, 0, 0);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
			glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 0.0, _wallScale);
			glTexCoord2f(1.0, 1.0); glVertex3f(0.0,_wallHeight, _wallScale);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.0, _wallHeight, 0.0);
		glEnd();
	glPopMatrix();

   // Draw the ground here

    glColor3f(1.0, 1.0, 1.0);
	glTranslatef(_wallScale * _mapx / 2.0, 0.0, _wallScale * _mapz / 2.0);
	glScalef(_wallScale * _mapx, 1.0, _wallScale * _mapz);
	glBindTexture(GL_TEXTURE_2D, texNames[1]);
      glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 0.0, 0.5);
		glTexCoord2f(1.0, 1.0); glVertex3f(0.5, 0.0, 0.5);
		glTexCoord2f(1.0, 0.0); glVertex3f(0.5, 0.0, -0.5);
		glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, 0.0, -0.5);
      glEnd();
   glPopMatrix();

   glDisable(GL_TEXTURE_2D);
}

void DrawWalls()
{
   // Draw the maze's walls 
	
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, texNames[0]);

	int i, j;
	//iterate throught the map array and find those whose values are 1.
	for (i = 0; i < MAX_MAZESIZE; i++) {
		for (j = 0; j < MAX_MAZESIZE; j++) {
			if (_map[i][j] == 1) {
				glPushMatrix();
				glTranslatef(i*_wallScale, 0, j*_wallScale);

				//wall 1
				glBegin(GL_QUADS);
					glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
					glTexCoord2f(1.0, 0.0); glVertex3f(_wallScale, 0.0, 0.0);
					glTexCoord2f(1.0, 1.0); glVertex3f(_wallScale, _wallHeight, 0.0);
					glTexCoord2f(0.0, 1.0); glVertex3f(0.0, _wallHeight, 0.0);
				glEnd();

				//wall 2
				glPushMatrix();
				glRotatef(-90, 0, 1, 0);
				glBegin(GL_QUADS);
					glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
					glTexCoord2f(1.0, 0.0); glVertex3f(_wallScale, 0.0, 0.0);
					glTexCoord2f(1.0, 1.0); glVertex3f(_wallScale, _wallHeight, 0.0);
					glTexCoord2f(0.0, 1.0); glVertex3f(0.0, _wallHeight, 0.0);
				glEnd();
				glPopMatrix();

				//wall 3
				glPushMatrix();
				glTranslatef(_wallScale, 0, 0);
				glRotatef(-90, 0, 1, 0);
				glBegin(GL_QUADS);
					glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
					glTexCoord2f(1.0, 0.0); glVertex3f(_wallScale, 0.0, 0.0);
					glTexCoord2f(1.0, 1.0); glVertex3f(_wallScale, _wallHeight, 0.0);
					glTexCoord2f(0.0, 1.0); glVertex3f(0.0, _wallHeight, 0.0);
				glEnd();
				glPopMatrix();
				
				//wall 4
				glPushMatrix();
				glTranslatef(0, 0, _wallScale);
				glBegin(GL_QUADS);
					glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
					glTexCoord2f(1.0, 0.0); glVertex3f(_wallScale, 0.0, 0.0);
					glTexCoord2f(1.0, 1.0); glVertex3f(_wallScale, _wallHeight, 0.0);
					glTexCoord2f(0.0, 1.0); glVertex3f(0.0, _wallHeight, 0.0);
				glEnd();
				glPopMatrix();

				
				glPopMatrix();
			}
		}
	}

	glDisable(GL_TEXTURE_2D);

}

void DrawPlayer()
{
	// Draw your player here
	glPushMatrix();
	glTranslatef(_player.pos[0], _player.pos[1], _player.pos[2]);

	//fix the postion of the play to avoid rotating with the view
	glRotatef(_player.degree - 120,0,1,0);

	glEnable(GL_LIGHTING);

	//set the lighting and material of the player
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHT0);
	
	//body
	glPushMatrix();
	glTranslatef(0, 0.1, 0);

		//draw the tail of the pig here
		glPushMatrix();
		glTranslatef(-body_x, -body_y/2,-body_z/4);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texNames[2]);
		glBegin(GL_QUADS);
			glTexCoord2d(0, 0); glVertex3f(0, 0.1, 0.1);
			glTexCoord2d(1, 0); glVertex3f(0, 0.1, 0.2);
			glTexCoord2d(1, 1); glVertex3f(0, 0.2, 0.2);
			glTexCoord2d(0, 1); glVertex3f(0, 0.2, 0.1);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	glScalef(0.3, 0.25, 0.3);
	glutSolidCube(1);
	glPopMatrix();
	
	//head
	glPushMatrix();
	glTranslatef(body_x / 2 + head_x / 2, 0.2, 0);
	glScalef(head_x, head_y, head_z);
	glutSolidCube(1);
	glPopMatrix();

	//limbs
	glPushMatrix();
	glTranslatef(0, -0.1, 0);

	glPushMatrix();
	glTranslatef(body_x / 2 - feet_x / 2, 0, body_z / 2 - feet_z / 2);
	glScalef(feet_x, feet_y, feet_z);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-(body_x / 2 - feet_x / 2) , 0, body_z / 2 - feet_z / 2);
	glScalef(feet_x, feet_y, feet_z);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-(body_x / 2 - feet_x / 2), 0, -(body_z / 2 - feet_z / 2));
	glScalef(feet_x, feet_y, feet_z);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(body_x / 2 - feet_x / 2, 0, -(body_z / 2 - feet_z / 2));
	glScalef(feet_x, feet_y, feet_z);
	glutSolidCube(1);
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();
	glDisable(GL_LIGHTING);
}

// For debugging collision detection
void DrawSphere()
{
   glPushMatrix();
	  glTranslatef(_player.pos[0], _player.pos[1], _player.pos[2]);
	  //glColor3f(0.0, 0.0, 0.0); 
	  glutWireSphere(_player.mySize, 15, 15);
   glPopMatrix();

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	
	//create fog effect
	glEnable(GL_FOG);
	{
		GLfloat fogColor[4] = { 0.8, 0.8, 0.8, 0.1 };

		GLint fogMode = GL_EXP;
		glFogi(GL_FOG_MODE, fogMode);
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_DENSITY, 0.05);
		glHint(GL_FOG_HINT, GL_DONT_CARE);
		glFogf(GL_FOG_START, _player.pos[0]);
		glFogf(GL_FOG_END, _wallScale * _mapx);
	}

	//sky color
	glClearColor(0.2, 0.8, 1,1.0);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
	  gluLookAt(_player.pos[0] - 2.0 * sin(_player.degree * M_PI / 180.0), // eye
		        _player.pos[1] + 0.25, 
				_player.pos[2] - 2.0 * cos(_player.degree* M_PI / 180.0), 
				_player.pos[0], // at
				_player.pos[1],
				_player.pos[2],
				0.0, 1.0, 0.0); // up
 	  DrawGround();
  	  DrawWalls();
	  
	  glEnable(GL_LIGHTING);
	  if (_drawmode == 0)
		 DrawPlayer();
	  else
		 DrawSphere();
   glPopMatrix();

   glutSwapBuffers();
}

void checkcollide()
{
   float dx, dz;
   // Check collision of walls here

   //calculate the current block
   int curI = _player.pos[0] / _wallScale;
   int curJ = _player.pos[2] / _wallScale;
   if (_map[curI][curJ] != 1) {
	   
	   //show victory info
	   if (_map[preValidi][preValidj] == 3 && _player.pos[0] >= (doorI + 1) * _wallScale) {
		   printf("victory!\n");
	   }
	   // if the current block is not a wall block
	   // Update the current position
		dx = _player.forward * sin((_player.degree) * M_PI / 180.0);
		dz = _player.forward * cos((_player.degree) * M_PI / 180.0);

	   _player.pos[0] += dx;
	   _player.pos[2] += dz;
	   
	   //store the previous valid block information
	   preValidi = curI;
	   preValidj = curJ;
   }
   else {
	   //the current block is a wall block
	   //replace the player to the center of the previous valid block
	   _player.pos[0] = preValidi * _wallScale + _wallScale / 2;
	   _player.pos[2] = preValidj * _wallScale + _wallScale / 2;
   }

   
   
}

void move(void)
{
	if (_player.spin != 0.0) {
		_player.degree += _player.spin;
		if (_player.degree > 360.0) {
			_player.degree -= 360.0;
		}
		else if (_player.degree < -360.0) {
			_player.degree += 360.0;
		}
	}

   if (_player.forward != 0.0) {
	  checkcollide();
   }
   glutPostRedisplay();
}

void keyboard(unsigned char key,int x, int y)
{
	
   switch (key) {
	  case 's':
	  case 'S':
	     // Change to use sphere for the object
		 _drawmode++;
		 _drawmode %= 2;
		 break;
	 
      case 27:
	     exit(0);
   }
}

// Please read this function carefully, you can learn how to 
// make use the mouse buttons to control the Test Object/Player
void mouse(int button, int state, int x, int y)
{
   static int buttonhold = 0;
   if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
	  if (buttonhold >= 2) {
	     // Stop forward and turn right
		 _player.forward = 0.0;
		 _player.spin = -_player.spinStepSize; // Turn right
	  }
	  else
		 _player.spin = 0.0; // Stop turn left
	  buttonhold--;
   }

   if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
	  if (buttonhold >= 2) {
		 // Stop forward and turn left
		 _player.forward = 0.0;
		 _player.spin = _player.spinStepSize; // Turn left
	  }
	  else
	 	 _player.spin = 0.0; // Stop turn right
	  buttonhold--;
   }

   if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_UP)) {
	  _player.forward = 0.0;
	  _player.spin = 0.0;
   }

   if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
	  if (buttonhold > 0) {
		 _player.forward = _player.forwardStepSize;
		 _player.spin = 0.0;
	  }
	  else
		 _player.spin = _player.spinStepSize; // Turn left
	  buttonhold++;
   }

   if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
	  if (buttonhold > 0) {
	     _player.forward = _player.forwardStepSize;
		 _player.spin = 0.0;
	  }
	  else
		 _player.spin = -_player.spinStepSize; // Turn right
	  buttonhold++;
   }

   if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN)) {
	   _player.forward = _player.forwardStepSize;
   }
}

void initplayer()
{
   // Initilaize the player
   // You may try change the values as you like
   _player.degree = 0.0; // User orientation
   _player.mySize = 0.2; // User radial size
   _player.forward = 0.0;
   _player.forwardStepSize = 0.005; // Step size
   _player.spin = 0.0;
   _player.spinStepSize = 0.1; // Rotate step size

   // Init the player's position based on the postion on the map
   _player.pos[0] = _initpos[0] * _wallScale + _wallScale / 2.0;
   _player.pos[1] = _player.mySize;
   _player.pos[2] = _initpos[1] * _wallScale + _wallScale / 2.0;
}

void setTexParam() {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void init()
{
	//create texture objects here
	glGenTextures(4, texNames);

	wallTex = TextureLoadBitmap("wall.bmp", &wallTexWidth, &wallTexHeight);
	glBindTexture(GL_TEXTURE_2D, texNames[0]);
	setTexParam();
	glTexImage2D(GL_TEXTURE_2D, 0, 3, wallTexWidth, wallTexHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, wallTex);

	glBindTexture(GL_TEXTURE_2D, texNames[1]);
	groundTex = TextureLoadBitmap("grass_ground.bmp", &groundTexWidth, &groundTexHeight);
	setTexParam();
	glTexImage2D(GL_TEXTURE_2D, 0, 3, groundTexWidth, groundTexHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, groundTex);

	tailTex = TextureLoadBitmap("tail.bmp", &tailTexWidth, &tailTexHeight);
	glBindTexture(GL_TEXTURE_2D, texNames[2]);
	setTexParam();
	glTexImage2D(GL_TEXTURE_2D, 0, 3, tailTexWidth, tailTexHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, tailTex);

	doorTex = TextureLoadBitmap("door.bmp", &doorTexWidth, &doorTexHeight);
	glBindTexture(GL_TEXTURE_2D, texNames[3]);
	setTexParam();
	glTexImage2D(GL_TEXTURE_2D, 0, 3, doorTexWidth, doorTexHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, doorTex);
	
   initplayer();

}

// Read in the maze map
int readmap(char* filename) 
{
   FILE* fp;
   int i, j;
   char tmp[MAX_MAZESIZE];

   fp = fopen(filename,"r");

   if (fp) {
      fscanf(fp, "%d", &_mapx);
	  fscanf(fp, "%d", &_mapz);
	  for (j = 0; j < _mapz; j++) {
 	     fscanf(fp, "%s", tmp);
	     for (i = 0; i < _mapx; i++) {
			_map[i][j] = tmp[i] - '0';
			if (_map[i][j] == 2) {
			   // Save the initial position
			   _initpos[0] = i;
			   _initpos[1] = j;
			}
			if (_map[i][j] == 3) {
				//save the door position
				doorI = i;
				doorJ = j;
			}
			printf("%d", _map[i][j]);
		 }
 		 printf("\n");
	  }
	  fclose(fp);
   }
   else {
	  printf("Error Reading Map file!\n");
	  return 0;
   }
   return 1;
}

void main(int argc,char **argv)
{
   if (argc >= 2) {
	  srand(time(NULL));
	  if (readmap(argv[1]) == 0 )
	  	 exit(0);
	  
	  glutInit(&argc, argv);
	  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE |
		  GLUT_DEPTH);
	  glutInitWindowSize(400, 300);
	  glutInitWindowPosition(250, 250);
	  if (glutCreateWindow("An Interactive 3D Maze Game (Skeleton)") == GL_FALSE)
	     exit(-1);
	  init();
	  glutDisplayFunc(display);
	  glutReshapeFunc(reshape);
	  glutKeyboardFunc(keyboard);
	  glutMouseFunc(mouse);
	  glutIdleFunc(move);
      glutMainLoop();
   } 
   else
 	  printf("Usage %s <mapfile>\n", argv[0]);
}

