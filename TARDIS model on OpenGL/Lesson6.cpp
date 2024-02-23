/*
 *		This Code Was Created By Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing The Base Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 *		Conversion to Visual Studio.NET done by GRANT JAMES(ZEUS)
 */

#include <windows.h>		// Header File For Windows
#include <mmsystem.h>
#include <math.h>			// Math Library Header File
#include <stdio.h>			// Header File For Standard Input/Output
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <gl\glaux.h>		// Header File For The Glaux Library
#pragma comment(lib, "winmm.lib")

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active=TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default
bool	blend;				// Blending ON/OFF
bool	bp;					// B Pressed?
bool	fp;					// F Pressed?

const float piover180 = 0.0174532925f;
float heading;
float xpos;
float zpos;


GLfloat	xrot;				// X Rotation ( NEW )
GLfloat	yrot;				// Y Rotation ( NEW )
GLfloat	zloc=-65.0f;				// Z Rotation ( NEW )
GLfloat walkbias = 0;
GLfloat walkbiasangle = 0;
GLfloat lookupdown = 0.0f;
GLfloat	z = 0.0f;				// Depth Into The Screen

GLuint	filter;				// Which Filter To Use
GLuint	texture[1];			// Storage For One Texture ( NEW )


typedef struct tagVERTEX
{
	float x, y, z;
	float u, v;
} VERTEX;

typedef struct tagTRIANGLE
{
	VERTEX vertex[3];
} TRIANGLE;

typedef struct tagSECTOR
{
	int numtriangles;
	TRIANGLE* triangle;
} SECTOR;

SECTOR sector1;				// Our Model Goes Here:

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc


AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;										// If Load Failed Return NULL
}

int LoadGLTextures()									// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;									// Status Indicator

	AUX_RGBImageRec *TextureImage[1];					// Create Storage Space For The Texture

	memset(TextureImage,0,sizeof(void *)*1);           	// Set The Pointer To NULL

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if (TextureImage[0]=LoadBMP("Data/NeHe2.bmp"))
	{
		Status=TRUE;									// Set The Status To TRUE

		glGenTextures(1, &texture[0]);					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}

	if (TextureImage[0])									// If Texture Exists
	{
		if (TextureImage[0]->data)							// If Texture Image Exists
		{
			free(TextureImage[0]->data);					// Free The Texture Image Memory
		}

		free(TextureImage[0]);								// Free The Image Structure
	}

	return Status;										// Return The Status
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{

	if (!LoadGLTextures())								// Jump To Texture Loading Routine ( NEW )
	{
		return FALSE;									// If Texture Didn't Load Return FALSE
	}

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping ( NEW )
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	return TRUE;										// Initialization Went OK
}
void handleKeyboardInput(unsigned char key, int x, int y) {
	switch (key) {
	case VK_DOWN:
		PlaySound(TEXT("Data/landing.wav"), NULL, SND_FILENAME | SND_ASYNC);
		break;
		// handle other keys
	
	case VK_UP:
		PlaySound(TEXT("Data/takeoff.wav"), NULL, SND_FILENAME | SND_ASYNC);
		break;
		// handle other keys
}
}
int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View
	glTranslatef(0.0f,0.0f,zloc);

	glRotatef(xrot,1.0f,0.0f,0.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);


	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glBegin(GL_QUADS);
	// uuumid Front wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(-8.0f, 17.0f, 8.0f); // Bottom-left
	glTexCoord2f(0.285f, 0.502f); glVertex3f(8.0f, 17.0f, 8.0f); // Bottom-right
	glTexCoord2f(0.285f, 0.551f); glVertex3f(8.0f, 19.0f, 8.0f); // Top-right
	glTexCoord2f(0.0f, 0.551f); glVertex3f(-8.0f, 19.0f, 8.0f); // Top-left

	// uuumid Back wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(-8.0f, 17.0f, -8.0f); // Bottom-left
	glTexCoord2f(0.0f, 0.551f); glVertex3f(-8.0f, 19.0f, -8.0f); // Top-left
	glTexCoord2f(0.285f, 0.551f); glVertex3f(8.0f, 19.0f, -8.0f); // Top-right
	glTexCoord2f(0.285f, 0.502f); glVertex3f(8.0f, 17.0f, -8.0f); // Bottom-right

	// uuumid Left wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(-8.0f, 17.0f, -8.0f); // Bottom-front
	glTexCoord2f(0.285f, 0.502f); glVertex3f(-8.0f, 17.0f, 8.0f); // Bottom-back
	glTexCoord2f(0.285f, 0.551f); glVertex3f(-8.0f, 19.0f, 8.0f); // Top-back
	glTexCoord2f(0.0f, 0.551f); glVertex3f(-8.0f, 19.0f, -8.0f); // Top-front

	// uuumid Right wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(8.0f, 17.0f, -8.0f); // Bottom-back
	glTexCoord2f(0.285f, 0.502f); glVertex3f(8.0f, 17.0f, 8.0f); // Bottom-front
	glTexCoord2f(0.285f, 0.551f); glVertex3f(8.0f, 19.0f, 8.0f); // Top-front
	glTexCoord2f(0.0f, 0.551f); glVertex3f(8.0f, 19.0f, -8.0f); // Top-back

	// u2midBottom wall
	glTexCoord2f(0.0f, 0.666f); glVertex3f(-8.5f, 17.0f, -8.5f); // Front-left
	glTexCoord2f(0.334f, 0.666f); glVertex3f(8.5f, 17.0f, -8.5f); // Front-right
	glTexCoord2f(0.334f, 1.0f); glVertex3f(8.5f, 17.0f, 8.5f); // Back-right
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-8.5f, 17.0f, 8.5f); // Back-left

	// uumid Front wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(-8.5f, 15.0f, 8.5f); // Bottom-left
	glTexCoord2f(0.285f, 0.502f); glVertex3f(8.5f, 15.0f, 8.5f); // Bottom-right
	glTexCoord2f(0.285f, 0.551f); glVertex3f(8.5f, 17.0f, 8.5f); // Top-right
	glTexCoord2f(0.0f, 0.551f); glVertex3f(-8.5f, 17.0f, 8.5f); // Top-left

	// uumid Back wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(-8.5f, 15.0f, -8.5f); // Bottom-left
	glTexCoord2f(0.0f, 0.551f); glVertex3f(-8.5f, 17.0f, -8.5f); // Top-left
	glTexCoord2f(0.285f, 0.551f); glVertex3f(8.5f, 17.0f, -8.5f); // Top-right
	glTexCoord2f(0.285f, 0.502f); glVertex3f(8.5f, 15.0f, -8.5f); // Bottom-right

	// uumid Left wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(-8.5f, 15.0f, -8.5f); // Bottom-front
	glTexCoord2f(0.285f, 0.502f); glVertex3f(-8.5f, 15.0f, 8.5f); // Bottom-back
	glTexCoord2f(0.285f, 0.551f); glVertex3f(-8.5f, 17.0f, 8.5f); // Top-back
	glTexCoord2f(0.0f, 0.551f); glVertex3f(-8.5f, 17.0f, -8.5f); // Top-front

	// uumid Right wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(8.5f, 15.0f, -8.5f); // Bottom-back
	glTexCoord2f(0.285f, 0.502f); glVertex3f(8.5f, 15.0f, 8.5f); // Bottom-front
	glTexCoord2f(0.285f, 0.551f); glVertex3f(8.5f, 17.0f, 8.5f); // Top-front
	glTexCoord2f(0.0f, 0.551f); glVertex3f(8.5f, 17.0f, -8.5f); // Top-back

	// umid Front wall
	glTexCoord2f(0.0f, 0.659f); glVertex3f(-10.5f, 15.0f, 10.5f); // Bottom-left
	glTexCoord2f(0.440f, 0.659f); glVertex3f(10.5f, 15.0f, 10.5f); // Bottom-right
	glTexCoord2f(0.440f, 0.614f); glVertex3f(10.5f, 13.0f, 10.5f); // Top-right
	glTexCoord2f(0.0f, 0.614f); glVertex3f(-10.5f, 13.0f, 10.5f); // Top-left

	// umid Back wall
	glTexCoord2f(0.440f, 0.659f); glVertex3f(-10.5f, 15.0f, -10.5f); // Bottom-left
	glTexCoord2f(0.440f, 0.614f); glVertex3f(-10.5f, 13.0f, -10.5f); // Top-left
	glTexCoord2f(0.0f, 0.614f); glVertex3f(10.5f, 13.0f, -10.5f); // Top-right
	glTexCoord2f(0.0f, 0.659f); glVertex3f(10.5f, 15.0f, -10.5f); // Bottom-right

	// umid Left wall
	glTexCoord2f(0.0f, 0.659f); glVertex3f(-10.5f, 15.0f, -10.5f); // Bottom-front
	glTexCoord2f(0.440f, 0.659f); glVertex3f(-10.5f, 15.0f, 10.5f); // Bottom-back
	glTexCoord2f(0.440f, 0.614f); glVertex3f(-10.5f, 13.0f, 10.5f); // Top-back
	glTexCoord2f(0.0f, 0.614f); glVertex3f(-10.5f, 13.0f, -10.5f); // Top-front

	// umid Right wall
	glTexCoord2f(0.440f, 0.659f); glVertex3f(10.5f, 15.0f, -10.5f); // Bottom-back
	glTexCoord2f(0.0f, 0.659f); glVertex3f(10.5f, 15.0f, 10.5f); // Bottom-front
	glTexCoord2f(0.0f, 0.614f); glVertex3f(10.5f, 13.0f, 10.5f); // Top-front
	glTexCoord2f(0.440f, 0.614f); glVertex3f(10.5f, 13.0f, -10.5f); // Top-back

	// umidTop wall
	glTexCoord2f(0.0f, 0.666f); glVertex3f(-10.5f, 15.0f, -10.5f); // Front-left
	glTexCoord2f(0.334f, 0.666f); glVertex3f(10.5f, 15.0f, -10.5f); // Front-right
	glTexCoord2f(0.334f, 1.0f); glVertex3f(10.5f, 15.0f, 10.5f); // Back-right
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.5f, 15.0f, 10.5f); // Back-left

	// midBottom wall
	glTexCoord2f(0.0f, 0.666f); glVertex3f(-10.5f, 13.0f, -10.5f); // Front-left
	glTexCoord2f(0.334f, 0.666f); glVertex3f(10.5f, 13.0f, -10.5f); // Front-right
	glTexCoord2f(0.334f, 1.0f); glVertex3f(10.5f, 13.0f, 10.5f); // Back-right
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.5f, 13.0f, 10.5f); // Back-left

	// mid Front wall
	glTexCoord2f(0.286f, 0.0f); glVertex3f(-10.0f, -17.0f, 10.0f); // Bottom-left
	glTexCoord2f(0.571f, 0.0f); glVertex3f(10.0f, -17.0f, 10.0f); // Bottom-right
	glTexCoord2f(0.571f, 0.5f); glVertex3f(10.0f, 13.0f, 10.0f); // Top-right
	glTexCoord2f(0.286f, 0.5f); glVertex3f(-10.0f, 13.0f, 10.0f); // Top-left

	// mid Back wall
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, -17.0f, -10.0f); // Bottom-left
	glTexCoord2f(0.0f, 0.5f); glVertex3f(-10.0f, 13.0f, -10.0f); // Top-left
	glTexCoord2f(0.286f, 0.5f); glVertex3f(10.0f, 13.0f, -10.0f); // Top-right
	glTexCoord2f(0.286f, 0.0f); glVertex3f(10.0f, -17.0f, -10.0f); // Bottom-right

	// mid Left wall
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, -17.0f, -10.0f); // Bottom-front
	glTexCoord2f(0.286f, 0.0f); glVertex3f(-10.0f, -17.0f, 10.0f); // Bottom-back
	glTexCoord2f(0.286f, 0.5f); glVertex3f(-10.0f, 13.0f, 10.0f); // Top-back
	glTexCoord2f(0.0f, 0.5f); glVertex3f(-10.0f, 13.0f, -10.0f); // Top-front

	// mid Right wall
	glTexCoord2f(0.0f, 0.0f); glVertex3f(10.0f, -17.0f, -10.0f); // Bottom-back
	glTexCoord2f(0.286f, 0.0f); glVertex3f(10.0f, -17.0f, 10.0f); // Bottom-front
	glTexCoord2f(0.286f, 0.5f); glVertex3f(10.0f, 13.0f, 10.0f); // Top-front
	glTexCoord2f(0.0f, 0.5f); glVertex3f(10.0f, 13.0f, -10.0f); // Top-back

	// bottomFront wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(-11.0f, -19.0f, 11.0f); // Bottom-left
	glTexCoord2f(0.285f, 0.502f); glVertex3f(11.0f, -19.0f, 11.0f); // Bottom-right
	glTexCoord2f(0.285f, 0.551f); glVertex3f(11.0f, -17.0f, 11.0f); // Top-right
	glTexCoord2f(0.0f, 0.551f); glVertex3f(-11.0f, -17.0f, 11.0f); // Top-left

	// bottomBack wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(-11.0f, -19.0f, -11.0f); // Bottom-left
	glTexCoord2f(0.0f, 0.551f); glVertex3f(-11.0f, -17.0f, -11.0f); // Top-left
	glTexCoord2f(0.285f, 0.551f); glVertex3f(11.0f, -17.0f, -11.0f); // Top-right
	glTexCoord2f(0.285f, 0.502f); glVertex3f(11.0f, -19.0f, -11.0f); // Bottom-right

	// bottomLeft wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(-11.0f, -19.0f, -11.0f); // Bottom-front
	glTexCoord2f(0.285f, 0.502f); glVertex3f(-11.0f, -19.0f, 11.0f); // Bottom-back
	glTexCoord2f(0.285f, 0.551f); glVertex3f(-11.0f, -17.0f, 11.0f); // Top-back
	glTexCoord2f(0.0f, 0.551f); glVertex3f(-11.0f, -17.0f, -11.0f); // Top-front

	// bottom Right wall
	glTexCoord2f(0.0f, 0.502f); glVertex3f(11.0f, -19.0f, -11.0f); // Bottom-back
	glTexCoord2f(0.285f, 0.502f); glVertex3f(11.0f, -19.0f, 11.0f); // Bottom-front
	glTexCoord2f(0.285f, 0.551f); glVertex3f(11.0f, -17.0f, 11.0f); // Top-front
	glTexCoord2f(0.0f, 0.551f); glVertex3f(11.0f, -17.0f, -11.0f); // Top-back

	// Top wall
	glTexCoord2f(0.055f, 0.721f); glVertex3f(-8.0f, 19.0f, -8.0f); // Front-left
	glTexCoord2f(0.279f, 0.721f); glVertex3f(-8.0f, 19.0f, 8.0f); // Front-right
	glTexCoord2f(0.279f, 0.941f); glVertex3f(8.0f, 19.0f, 8.0f); // Back-right
	glTexCoord2f(0.055f, 0.941f); glVertex3f(8.0f, 19.0f, -8.0f); // Back-left

	// uBottom wall
	glTexCoord2f(0.0f, 0.666f); glVertex3f(-11.0f, -17.0f, -11.0f); // Front-left
	glTexCoord2f(0.334f, 0.666f); glVertex3f(11.0f, -17.0f, -11.0f); // Front-right
	glTexCoord2f(0.334f, 1.0f); glVertex3f(11.0f, -17.0f, 11.0f); // Back-right
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-11.0f, -17.0f, 11.0f); // Back-left

	// Bottom wall
	glTexCoord2f(0.055f, 0.721f); glVertex3f(-11.0f, -19.0f, -11.0f); // Front-left
	glTexCoord2f(0.279f, 0.721f); glVertex3f(11.0f, -19.0f, -11.0f); // Front-right
	glTexCoord2f(0.279f, 0.941f); glVertex3f(11.0f, -19.0f, 11.0f); // Back-right
	glTexCoord2f(0.055f, 0.941f); glVertex3f(-11.0f, -19.0f, 11.0f); // Back-left
	glEnd();

	
	glBegin(GL_QUADS);
	// Front wall
	glTexCoord2f(0.746f, 0.584f); glVertex3f(-3.728f, -3.0f, 9.0f);
	glTexCoord2f(1.0f, 0.584f); glVertex3f(3.728f, -3.0f, 9.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(3.728f, 3.0f, 9.0f);
	glTexCoord2f(0.746f, 1.0f); glVertex3f(-3.728f, 3.0f, 9.0f);
	// Back wall
	glTexCoord2f(0.748f, 0.0f); glVertex3f(-3.728f, -3.0f, -9.0f);
	glTexCoord2f(0.748f, 0.404f); glVertex3f(-3.728f, 3.0f, -9.0f);
	glTexCoord2f(1.0f, 0.404f); glVertex3f(3.728f, 3.0f, -9.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(3.728f, -3.0f, -9.0f);
	// left wall
	glTexCoord2f(1.0f, 0.578f); glVertex3f(-9.0f, -3.0f, 3.728f); // Bottom - front
	glTexCoord2f(1.0f, 0.408f); glVertex3f(-9.0f, -3.0f, -3.728f);// Bottom-back
	glTexCoord2f(0.699f, 0.408f); glVertex3f(-9.0f, 3.0f, -3.728f); // Top-back
	glTexCoord2f(0.699f, 0.578f); glVertex3f(-9.0f, 3.0f, 3.728f); // Top-front
	// right wall
	glTexCoord2f(1.0f, 0.408f); glVertex3f(9.0f, -3.0f, -3.728f); // Bottom-back
	glTexCoord2f(1.0f, 0.578f); glVertex3f(9.0f, -3.0f, 3.728f); // Bottom-front
	glTexCoord2f(0.699f, 0.578f); glVertex3f(9.0f, 3.0f, 3.728f); // Top-front
	glTexCoord2f(0.699f, 0.408f); glVertex3f(9.0f, 3.0f, -3.728f); // Top-back
	// front2 wall
	glTexCoord2f(1.0f, 0.408f); glVertex3f(3.728f, -3.0f, 9.0f); // Bottom-left
	glTexCoord2f(1.0f, 0.578f); glVertex3f(9.0f, -3.0f, 3.728f); // Bottom-right
	glTexCoord2f(0.699f, 0.578f); glVertex3f(9.0f, 3.0f, 3.728f); // Top-right
	glTexCoord2f(0.699f, 0.408f); glVertex3f(3.728f, 3.0f, 9.0f); // Top-left
	// back2 wall
	glTexCoord2f(1.0f, 0.408f); glVertex3f(-9.0f, -3.0f, -3.728f); // Bottom-left
	glTexCoord2f(0.699f, 0.408f); glVertex3f(-9.0f, 3.0f, -3.728f); // Top-left
	glTexCoord2f(0.699f, 0.578f); glVertex3f(-3.728f, 3.0f, -9.0f); // Top-right
	glTexCoord2f(1.0f, 0.578f); glVertex3f(-3.728f, -3.0f, -9.0f); // Bottom-right
	//right2 wall
	glTexCoord2f(1.0f, 0.408f); glVertex3f(-3.728f, -3.0f, 9.0f); // Bottom-front
	glTexCoord2f(1.0f, 0.578f); glVertex3f(-9.0f, -3.0f, 3.728f); // Bottom-back
	glTexCoord2f(0.699f, 0.578f); glVertex3f(-9.0f, 3.0f, 3.728f); // Top-back
	glTexCoord2f(0.699f, 0.408f); glVertex3f(-3.728f, 3.0f, 9.0f); // Top-front

	// lef2 wall
	glTexCoord2f(1.0f, 0.578f); glVertex3f(3.728f, -3.0f, -9.0f); // Bottom-back
	glTexCoord2f(1.0f, 0.408f); glVertex3f(9.0f, -3.0f, -3.728f); // Bottom-front
	glTexCoord2f(0.699f, 0.408f); glVertex3f(9.0f, 3.0f, -3.728f); // Top-front
	glTexCoord2f(0.699f, 0.578f); glVertex3f(3.728f, 3.0f, -9.0f); // Top-back

	// Bottom wall
	glTexCoord2f(0.336f, 0.724f); glVertex3f(-9.0f, -3.0f, -9.0f); // Front-left
	glTexCoord2f(0.336f, 1.0f); glVertex3f(9.0f, -3.0f, -9.0f); // Front-right
	glTexCoord2f(0.625f, 1.0f); glVertex3f(9.0f, -3.0f, 9.0f); // Back-right
	glTexCoord2f(0.625f, 0.724f); glVertex3f(-9.0f, -3.0f, 9.0f); // Back-left

	// Top wall
	glTexCoord2f(0.574f, 0.0f); glVertex3f(-9.0f, 3.0f, -9.0f); // Front-left
	glTexCoord2f(0.574f, 0.166f); glVertex3f(9.0f, 3.0f, -9.0f); // Front-right
	glTexCoord2f(0.746f, 0.166f); glVertex3f(9.0f, 3.0f, 9.0f); // Back-right
	glTexCoord2f(0.746f, 0.0f); glVertex3f(-9.0f, 3.0f, 9.0f); // Back-left

	glEnd();
	

	return TRUE;										// Keep Going
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active=TRUE;						// Program Is Active
			}
			else
			{
				active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}

	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	// Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;							// Windowed Mode
	}

	// Create Our OpenGL Window
	if (!CreateGLWindow("NeHe's Texture Mapping Tutorial",640,480,16,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done = TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
				if (keys['B'] && !bp)
				{
					bp = TRUE;
					blend = !blend;
					if (!blend)
					{
						glDisable(GL_BLEND);
						glEnable(GL_DEPTH_TEST);
					}
					else
					{
						glEnable(GL_BLEND);
						glDisable(GL_DEPTH_TEST);
					}
				}
				if (!keys['B'])
				{
					bp = FALSE;
				}

				if (keys['F'] && !fp)
				{
					fp = TRUE;
					filter += 1;
					if (filter > 2)
					{
						filter = 0;
					}
				}
				if (!keys['F'])
				{
					fp = FALSE;
				}

				if (keys[VK_PRIOR])
				{
					z -= 0.02f;
				}

				if (keys[VK_NEXT])
				{
					z += 0.02f;
				}

				if (keys[VK_UP])
				{
					PlaySound(TEXT("Data/takeoff.wav"), NULL, SND_FILENAME | SND_ASYNC);
					heading = 0.0f;
					zloc = heading;
				}

				if (keys[VK_DOWN])
				{
					PlaySound(TEXT("Data/landing.wav"), NULL, SND_FILENAME | SND_ASYNC);
					heading = -65.0f;
					zloc = heading;
				}

				if (keys[VK_RIGHT])
				{
					heading -= 1.0f;
					yrot = heading;
				}

				if (keys[VK_LEFT])
				{
					heading += 1.0f;
					yrot = heading;
				}

				if (keys[VK_PRIOR])
				{
					lookupdown -= 1.0f;
				}

				if (keys[VK_NEXT])
				{
					lookupdown += 1.0f;
				}

				if (keys[VK_F1])						// Is F1 Being Pressed?
				{
					keys[VK_F1] = FALSE;					// If So Make Key FALSE
					KillGLWindow();						// Kill Our Current Window
					fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
					// Recreate Our OpenGL Window
					if (!CreateGLWindow("Lionel Brits & NeHe's 3D World Tutorial", 640, 480, 16, fullscreen))
					{
						return 0;						// Quit If Window Was Not Created
					}
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();										// Kill The Window
	return (msg.wParam);								// Exit The Program
}
