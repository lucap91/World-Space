/*
 *		This Code Was Created By Lionel Brits & Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing The Base Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 */


#include <windows.h>		// Header File For Windows
#include <math.h>			// Math Library Header File
#include <stdio.h>			// Header File For Standard Input/Output
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <gl\glaux.h>		// Header File For The Glaux Library
#include "lesson31.h"

#include "MilkshapeModel.h"	// Header File For Milkshape File

#pragma comment( lib, "opengl32.lib" )								// Search For OpenGL32.lib While Linking ( NEW )
#pragma comment( lib, "glu32.lib" )									// Search For GLu32.lib While Linking    ( NEW )
#pragma comment( lib, "glaux.lib" )		

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

Model *pModel = NULL;	// Holds The Model Data
Model *pModelsx = NULL;
Model *pModeldx = NULL;
Model *pModelsu = NULL;
Model *pModelgiu = NULL;
Model *pModelsinistragiu = NULL;
Model *pModeldestragiu = NULL;
Model *pModelsinistrasu = NULL;
Model *pModeldestrasu = NULL;


const float piover180 = 0.0174532925f;
float heading;
float xpos;
float zpos;
float ypos;

float heading2=0.0f;
float oldheading2 = heading2;
float oldheading = heading;// oldheading serve per ruotare il modello tenendo traccia del cambiamento di heading

GLfloat	yrot=0.0f;				// Y Rotation
GLfloat walkbias = 0;
GLfloat walkbiasangle = 0;
GLfloat lookupdown = 0.0f;
GLfloat	z=0.0f;				// Depth Into The Screen

GLuint	filter;				// Which Filter To Use
GLuint	texture[4];			// Storage For 3 Textures


GLfloat	xxrot;				// X Rotation ( NEW )
GLfloat	yyrot;				// Y Rotation ( NEW )
GLfloat	zzrot;				// Z Rotation ( NEW )


LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

void readstr(FILE *f,char *string)
{
	do
	{
		fgets(string, 255, f);
	} while ((string[0] == '/') || (string[0] == '\n'));
	return;
}

AUX_RGBImageRec *LoadBMP(const char *Filename)                // Loads A Bitmap Image
{
        FILE *File=NULL;                                // File Handle

        if (!Filename)                                  // Make Sure A Filename Was Given
        {
                return NULL;                            // If Not Return NULL
        }

        File=fopen(Filename,"r");                       // Check To See If The File Exists

        if (File)                                       // Does The File Exist?
        {
                fclose(File);                           // Close The Handle
                return auxDIBImageLoad(Filename);       // Load The Bitmap And Return A Pointer
        }
        return NULL;                                    // If Load Failed Return NULL
}

int LoadGLTextures()									// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;									// Status Indicator

	AUX_RGBImageRec *TextureImage[4];					// Create Storage Space For The Texture

	memset(TextureImage,0,sizeof(void *)*2);           	// Set The Pointer To NULL

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit

  TextureImage[0]=LoadBMP("Data/earth.bmp");
  TextureImage[1]=LoadBMP("Data/night.bmp");
  TextureImage[2]=LoadBMP("Data/mars.bmp");
  TextureImage[3]=LoadBMP("Data/Jupiter.bmp");


  for(int i=0; i < 4; i++) {
	  if ( TextureImage[i] )
	  {
		Status=TRUE;									// Set The Status To TRUE

		glGenTextures(1, &texture[i]);					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[i]->sizeX,
        TextureImage[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE,
        TextureImage[i]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	  }

	  if (TextureImage[i])									// If Texture Exists
	  {
		if (TextureImage[i]->data)							// If Texture Image Exists
		{
			free(TextureImage[i]->data);					// Free The Texture Image Memory
		}

		free(TextureImage[i]);								// Free The Image Structure
	  }
  }

	return Status;										// Return The Status
}
GLuint LoadGLTexture( const char *filename )						// Load Bitmaps And Convert To Textures
{
	AUX_RGBImageRec *pImage;										// Create Storage Space For The Texture
	GLuint texture = 0;												// Texture ID

	pImage = LoadBMP( filename );									// Loads The Bitmap Specified By filename

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if ( pImage != NULL && pImage->data != NULL )					// If Texture Image Exists
	{
		glGenTextures(1, &texture);									// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, pImage->sizeX, pImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, pImage->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		free(pImage->data);											// Free The Texture Image Memory
		free(pImage);												// Free The Image Structure
	//	MessageBox(NULL,filename,"caricata texture",MB_OK);
	}
	else MessageBox(NULL,filename,"Errore caricamento texture",MB_OK);

	return texture;													// Return The Status
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
	if (!LoadGLTextures())								// Jump To Texture Loading Routine
	{
		return FALSE;									// If Texture Didn't Load Return FALSE
	}
	pModel->reloadTextures();
	pModelsx->reloadTextures();
	pModeldx->reloadTextures();
	pModelsu->reloadTextures();
	pModelgiu->reloadTextures();
	pModeldestrasu->reloadTextures();
	pModeldestragiu->reloadTextures();
	pModelsinistrasu->reloadTextures();
	pModelsinistragiu->reloadTextures();

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Set The Blending Function For Translucency
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// This Will Clear The Background Color To Black
	glClearDepth(1.0f);									// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);								// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glShadeModel(GL_SMOOTH);							// Enables Smooth Color Shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View

	//GLfloat x_m, y_m, z_m, u_m, v_m;
	GLfloat xtrans = -xpos;
	GLfloat ztrans = -zpos;
	GLfloat ytrans = -ypos;
	GLfloat sceneroty = 360.0f - yrot;

	glTranslatef(0,-3,-3);
	glRotatef(30,1,0,0);
	glTranslatef(0,-2,1);

	glTranslatef(0.0,-4.5,-13);
	glRotatef(lookupdown,1.0f,0,0);
	if((heading==oldheading) && (heading2==oldheading2))//se non sto muovendo in su/giu/sx/dx carico modello normale
	pModel->draw();	//da lesson31
	
	

	if((heading>oldheading)&&(heading2==oldheading2))//modello sinistra
	{
		pModelsx->draw();
		oldheading=heading;
	}
	else if((heading<oldheading)&&(heading2==oldheading2))//modello destra
	{
		pModeldx->draw();
		oldheading=heading;
	}
			else if((heading2<oldheading2)&&(heading>oldheading))//modello sinistra giu
			{
				pModelsinistragiu->draw();
				oldheading2=heading2;
				oldheading=heading;
			}
			else if((heading2>oldheading2)&&(heading<oldheading))//modello destra su
				{
					pModeldestrasu->draw();
					oldheading2=heading2;
					oldheading=heading;
				}


	if((heading2>oldheading2)&&(heading==oldheading))//modello su
	{
		pModelsu->draw();
		oldheading2=heading2;
	}
	else if((heading2<oldheading2)&&(heading==oldheading))//modello giu
		{
			pModelgiu->draw();
			oldheading2=heading2;
		}
		else if((heading2>oldheading2)&&(heading>oldheading))//modello sinistra su
			{
				pModelsinistrasu->draw();
				oldheading2=heading2;
				oldheading=heading;
			}
			else if((heading2<oldheading2)&&(heading<oldheading))//modello destra giu
				{
					pModeldestragiu->draw();
					oldheading2=heading2;
					oldheading=heading;
				}
	
	glRotatef(sceneroty,0,1.0f,0);
	

	glTranslatef(xtrans, ytrans, ztrans);//togliendo questo lo sfondo rimane fermo
	

	//SFONDO SFERA++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	glBindTexture(GL_TEXTURE_2D, texture[1]);//TEXTURE SFONDO
	
	GLUquadricObj *quadObj3 = gluNewQuadric();
	gluQuadricTexture(quadObj3, GLU_TRUE);
	float raggio3 = 80.0;
	float slices3 = 400.0;
	gluSphere(quadObj3, raggio3, slices3, 10);
	
	//SFONDO CUBO ALTERNATIVO//////////////////////////////////////////////////////////////////////////////////////
	/*glBegin(GL_QUADS);
		// Front Face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);//glTexCoord definisce i vertici di dove mettere la texture, sono in 2D
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		// Back Face
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		// Top Face
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		// Bottom Face
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		// Right face
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		// Left Face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	  glEnd();
	  */
	//FINESFONDO CUBO ALTERNATIVO///////////////////////////////////////////////////////////////////////////////////////////

	
//SFERA TERRA+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	glBindTexture(GL_TEXTURE_2D, texture[0]);//TEXTURE TERRA
	glTranslatef(xtrans+7, ytrans+1, ztrans-5);

	//ruota la TERRA 
	//glRotatef(xxrot,1.0f,0.0f,0.0f);  
	//glRotatef(yyrot,0.0f,1.0f,0.0f);
	//glRotatef(zzrot,0.0f,0.0f,1.0f);
	

	GLUquadricObj *quadObj = gluNewQuadric();
	gluQuadricTexture(quadObj, GLU_TRUE);
	float raggio = 1.0;
	float slices = 400.0;
	gluSphere(quadObj, raggio, slices, 10);
	glEnd();
	
	xxrot+=0.3f;
	yyrot+=0.2f;
	zzrot+=0.4f;

//FINE SFERA TERRA+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

// SFERA MARTE++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	glBindTexture(GL_TEXTURE_2D, texture[2]);//TEXTURE MARTE
	glTranslatef( 4, 5, ztrans+6);
	
	GLUquadricObj *quadObj4 = gluNewQuadric();
	gluQuadricTexture(quadObj4, GLU_TRUE);
	float raggio4 = 2.0;
	float slices4 = 400.0;
	gluSphere(quadObj4, raggio4, slices4, 10);


	//SFERA LUNA+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	glBindTexture(GL_TEXTURE_2D, texture[3]);//TEXTURE LUNA
	glTranslatef(-5, -1, ztrans);

	GLUquadricObj *quadObj5 = gluNewQuadric();
	gluQuadricTexture(quadObj5, GLU_TRUE);
	float raggio5 = 1.0;
	float slices5 = 400.0;
	gluSphere(quadObj5, raggio5, slices5, 10);
	//FINE SFERA LUNA+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	return true;
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

	pModel = new MilkshapeModel();									// Memory To Hold The Model
	if ( pModel->loadModelData( "data/luca_patti.ms3d" ) == false )		// Loads The Model And Checks For Errors
	{
		MessageBox( NULL, "Couldn't load the model data\\luca_patti.ms3d", "Error", MB_OK | MB_ICONERROR );
		return 0;													// If Model Didn't Load Quit
	}

	pModelsx = new MilkshapeModel();									// Memory To Hold The Model
	if ( pModelsx->loadModelData( "data/sinistra.ms3d" ) == false )		// Loads The Model And Checks For Errors
	{
		MessageBox( NULL, "Couldn't load the model data\\sinistra.ms3d", "Error", MB_OK | MB_ICONERROR );
		return 0;													// If Model Didn't Load Quit
	}

	pModeldx = new MilkshapeModel();									// Memory To Hold The Model
	if ( pModeldx->loadModelData( "data/destra.ms3d" ) == false )		// Loads The Model And Checks For Errors
	{
		MessageBox( NULL, "Couldn't load the model data\\destra.ms3d", "Error", MB_OK | MB_ICONERROR );
		return 0;													// If Model Didn't Load Quit
	}

	pModelsu = new MilkshapeModel();									// Memory To Hold The Model
	if ( pModelsu->loadModelData( "data/su.ms3d" ) == false )		// Loads The Model And Checks For Errors
	{
		MessageBox( NULL, "Couldn't load the model data\\su.ms3d", "Error", MB_OK | MB_ICONERROR );
		return 0;													// If Model Didn't Load Quit
	}

	pModelgiu = new MilkshapeModel();									// Memory To Hold The Model
	if ( pModelgiu->loadModelData( "data/giu.ms3d" ) == false )		// Loads The Model And Checks For Errors
	{
		MessageBox( NULL, "Couldn't load the model data\\giu.ms3d", "Error", MB_OK | MB_ICONERROR );
		return 0;													// If Model Didn't Load Quit
	}
	
	pModelsinistragiu = new MilkshapeModel();									// Memory To Hold The Model
	if ( pModelsinistragiu->loadModelData( "data/sinistragiu.ms3d" ) == false )		// Loads The Model And Checks For Errors
	{
		MessageBox( NULL, "Couldn't load the model data\\sinistragiu.ms3d", "Error", MB_OK | MB_ICONERROR );
		return 0;													// If Model Didn't Load Quit
	}
	
	pModeldestragiu = new MilkshapeModel();									// Memory To Hold The Model
	if ( pModeldestragiu->loadModelData( "data/destragiu.ms3d" ) == false )		// Loads The Model And Checks For Errors
	{
		MessageBox( NULL, "Couldn't load the model data\\destragiu.ms3d", "Error", MB_OK | MB_ICONERROR );
		return 0;													// If Model Didn't Load Quit
	}

	pModelsinistrasu = new MilkshapeModel();									// Memory To Hold The Model
	if ( pModelsinistrasu->loadModelData( "data/sinistrasu.ms3d" ) == false )		// Loads The Model And Checks For Errors
	{
		MessageBox( NULL, "Couldn't load the model data\\sinistrasu.ms3d", "Error", MB_OK | MB_ICONERROR );
		return 0;													// If Model Didn't Load Quit
	}

	pModeldestrasu = new MilkshapeModel();									// Memory To Hold The Model
	if ( pModeldestrasu->loadModelData( "data/destrasu.ms3d" ) == false )		// Loads The Model And Checks For Errors
	{
		MessageBox( NULL, "Couldn't load the model data\\destrasu.ms3d", "Error", MB_OK | MB_ICONERROR );
		return 0;													// If Model Didn't Load Quit
	}

	// Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;							// Windowed Mode
	}

	// Create Our OpenGL Window
	if (!CreateGLWindow("Luca Patti & Fabio Vannucci - World space",640,480,16,fullscreen))
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
				done=TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
				if (keys['B'] && !bp)
				{
					bp=TRUE;
					blend=!blend;
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
					bp=FALSE;
				}

				if (keys['F'] && !fp)
				{
					fp=TRUE;
					filter+=1;
					if (filter>2)
					{
						filter=0;
					}
				}
				if (!keys['F'])
				{
					fp=FALSE;
				}

				if (keys[VK_PRIOR])
				{
					z-=0.02f;
					
				}

				if (keys[VK_NEXT])
				{
					z+=0.02f;
				}

				if (keys[VK_UP])
				{

					xpos -= (float)sin(heading*piover180) * 0.05f;
					zpos -= (float)cos(heading*piover180) * 0.05f;
					
						walkbiasangle = 0.0f;
				
					walkbias = (float)sin(walkbiasangle * piover180)/20.0f;
				}

				if (keys['E'])
				{
					heading2 += 1.0f;//serve solo per caricare il modello che va in su o giu, non serve ad altro questa variabile
					ypos -= (float)sin(heading*piover180) * 0.05f;
					zpos -= (float)cos(heading*piover180) * 0.00001f;

					walkbiasangle = 0.0f;
				
					walkbias = (float)sin(walkbiasangle * piover180)/20.0f;
				}

				if (keys['D'])
				{
					heading2 -= 1.0f;//serve solo per caricare il modello che va in su o giu, non serve ad altro questa variabile
					ypos += (float)sin(heading*piover180) * 0.05f;
					zpos += (float)cos(heading*piover180) * 0.00001f;
					
					walkbiasangle = 0.0f;
				
					walkbias = (float)sin(walkbiasangle * piover180)/20.0f;
				}

				if (keys[VK_DOWN])
				{
					xpos += (float)sin(heading*piover180) * 0.05f;
					zpos += (float)cos(heading*piover180) * 0.05f;
					
					walkbiasangle = 0.0f;
					walkbias = (float)sin(walkbiasangle * piover180)/20.0f;
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

				if (keys['W'])
				{
					lookupdown-= 1.0f;
				}

				if (keys['S'])
				{
					lookupdown+= 1.0f;
				}

				if (keys[VK_F1])						// Is F1 Being Pressed?
				{
					keys[VK_F1]=FALSE;					// If So Make Key FALSE
					KillGLWindow();						// Kill Our Current Window
					fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
					// Recreate Our OpenGL Window
					if (!CreateGLWindow("Luca Patt & Fabio Vannucci - World space",640,480,16,fullscreen))
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




