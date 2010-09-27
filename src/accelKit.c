/*
    'accelKit'
    Copyright (C) 2010  Nikos Kastellanos

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
    e-mail: nkastellanos@gmail.com
*/



// ============================================================================
//	Includes
// ============================================================================

#include <stdio.h>
#include <stdlib.h>				
#include <GL/glut.h>
#include <AR/config.h>
#include <AR/video.h>
#include <AR/param.h>			
#include <AR/ar.h>
#include <AR/gsub_lite.h>

#include <wingdi.h>
#include "resource.h"

#include "WindowGrab.h"
#include "WServer.h"


// ============================================================================
//	Constants
// ============================================================================

#define VIEW_SCALEFACTOR		0.025		// 1.0 ARToolKit unit becomes 0.025 of my OpenGL units.
#define VIEW_DISTANCE_MIN		0.1			// Objects closer to the camera than this will not be displayed.
#define VIEW_DISTANCE_MAX		100.0		// Objects further away from the camera than this will not be displayed.


//WEBCAMMODE  
#define WCM_VERTICAL 0
#define WCM_HORIZONTAL 1

// ============================================================================
//	Global variables
// ============================================================================

// Preferences.
static int prefWindowed = TRUE;
static int prefWidth = 640;					// Fullscreen mode width.
static int prefHeight = 480;				// Fullscreen mode height.
static int prefDepth = 32;					// Fullscreen mode bit depth.
static int prefRefresh = 0;					// Fullscreen mode refresh rate. Set to 0 to use default rate.

// Image acquisition.
static ARUint8		*gARTImage = NULL;

// Marker detection.
static int			gARTThreshhold = 100;
static long			gCallCountMarkerDetect = 0;

// Transformation matrix retrieval.
static double		gPatt_width     = 80.0;	// Per-marker, but we are using only 1 marker.
static double		gPatt_centre[2] = {0.0, 0.0}; // Per-marker, but we are using only 1 marker.
static double		gPatt_trans[3][4];		// Per-marker, but we are using only 1 marker.
static int			gPatt_found = FALSE;	// Per-marker, but we are using only 1 marker.
static int			gPatt_id;				// Per-marker, but we are using only 1 marker.

// Drawing.
static ARParam		gARTCparam;
static ARGL_CONTEXT_SETTINGS_REF gArglSettings = NULL;


//textures
HBITMAP bmpA = 0;
HBITMAP bmpB = 0;
GLuint texture[2]; //define 2 textures


int webcamMode = WCM_VERTICAL;


// ============================================================================
//	Functions
// ============================================================================

void LoadGLTexture(HBITMAP bitmap) // Load Bitmaps And Convert To Textures
{
 // store bitmap data in a vector
 unsigned char data[256*479*3];
 unsigned char buff;
 int i;
 HDC hDC;
 HWND hWnd = 0;
 int res;

 // setup 24 bits bitmap structure
 BITMAPINFO info;
 BITMAPINFOHEADER header;
 header.biSize = sizeof(BITMAPINFOHEADER);
 header.biWidth = 256; 
 header.biHeight = 479; 
 header.biPlanes = 1; 
 header.biBitCount = 24;
 header.biCompression = BI_RGB;
 header.biSizeImage = 0;
 header.biClrUsed = 0; 
 header.biClrImportant = 0; 
 info.bmiHeader = header;
 info.bmiColors->rgbRed = NULL;
 info.bmiColors->rgbGreen = NULL;
 info.bmiColors->rgbBlue = NULL;
 info.bmiColors->rgbReserved = NULL;


 // store bitmap data in a vector
 hDC = GetDC(hWnd);
 res = GetDIBits(hDC, bitmap, 0, 479, &data, &info, DIB_RGB_COLORS);
 res = ReleaseDC(hWnd,hDC);


 // convert from BGR to RGB
 for(i=0; i<256*479; i++)
 {
	buff = data[i*3];
	if(i>=3)
	{
	  data[i*3] = data[i*3+2];
	  data[i*3+2] = buff;
	}
 }

  //glGenTextures(1, &texture[0]); // Create The Texture
  // Build Texture
  {
  // Typical Texture Generation Using Data From The Bitmap
   glBindTexture(GL_TEXTURE_2D, texture[0]);
   // Generate The Texture 
   glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 479, 0, GL_RGB, GL_UNSIGNED_BYTE, &data);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  }

}




// Something to look at, draw a rotating colour cube.
static void DrawWP7(void)
{
	BITMAPINFOHEADER   bmih_out;
	HDC                inDC, outDC;

	BITMAP bmp;
	HBITMAP            old_bitmap;
	HBITMAP image;
	void               *pixels=NULL;
	float w=256,h=256;
	int texh=0, texw=0;
	int ret;

	//paint hLocalBitmap on texture 
	HBITMAP wp7bmp = wgCapture(bmpA);
	if(wp7bmp!=NULL)
	{
		/*
		// get the dimensions of the passed bitmap
		GetObject(hLocalBitmap, sizeof(BITMAP), &bmp);
	    h = (float)bmp.bmHeight;
		w = (float)bmp.bmWidth;

		// figure out what the texture dimensions have to be in order to encompass the entire image
		texh = next_po2((int)h);
		texw = next_po2((int)w);


 	  // create a DIB big enough to hold the original image and all of it's padding
         bmih_out.biSize                  = sizeof(bmih_out);
         bmih_out.biWidth                 = texw;
         bmih_out.biHeight                = texh;
         bmih_out.biPlanes                = 1;
         bmih_out.biBitCount              = 32;
         bmih_out.biCompression   = BI_RGB;
         bmih_out.biSizeImage     = 0;
         bmih_out.biXPelsPerMeter = 0;
         bmih_out.biYPelsPerMeter = 0;
         bmih_out.biClrUsed       = 0;
         bmih_out.biClrImportant  = 0;
 
		image = CreateDIBSection(NULL, (BITMAPINFO*)&bmih_out, DIB_RGB_COLORS, &(pixels), NULL, 0);


		// select the bitmap resource into input DC
         inDC  = CreateCompatibleDC(NULL);
         SelectObject(inDC, hLocalBitmap);
 
         // select the new DIB section into the output DC
         outDC = CreateCompatibleDC(NULL);
         old_bitmap = (HBITMAP)SelectObject(outDC, image);
 
         // copy the bitmap into the new DIB section and then unselect the DIB section from the outDC
         ret = BitBlt(outDC, 0,texh-(int)h, (int)w, (int)h, inDC, 0, 0, SRCCOPY);
         SelectObject(outDC, old_bitmap);
 
         DeleteDC(inDC);
         DeleteDC(outDC);
 
         // create the texture
        glGenTextures(1, &texture[0]);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

		if (image!=NULL) DeleteObject(image);
*/
		
	}
		
	
    LoadGLTexture(bmpA);

	glPushMatrix(); // Save world coordinate system.
	glTranslatef(0.0, 0.0, 0.0); // Place base of cube on marker surface.
	
	// Enable texturing and select first texture
    glColor3f(1.0f,1.0f,1.0f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,texture[0]);

	

	// Draw the top face
     glBegin(GL_QUADS);
     glTexCoord2f(0.0f,1.0f); glVertex3f(-1.5f, 2.43f, 0.0f);
     glTexCoord2f(1.0f,1.0f); glVertex3f( 1.5f, 2.43f, 0.0f);
     glTexCoord2f(1.0f,0.0f); glVertex3f( 1.5f,-2.90f, 0.0f);
     glTexCoord2f(0.0f,0.0f); glVertex3f(-1.5f,-2.90f, 0.0f);
     glEnd();


	glPopMatrix();	// Restore world coordinate system.

}

BOOL is_po2(int a)
{
     if (a <= 0) return FALSE; 
     return ((a & -a) == a);
 }
 
 int next_po2(int a)
 {
         int i; 
         if (is_po2(a)) return a; 
         for (i=1; i<=sizeof(int)*8; i++) 
		 {
                 a >>= 1;
                 if (a == 0)  break;
         } 
         return (1 << i);
 }



static int setupCamera(const char *cparam_name, char *vconf, ARParam *cparam)
{	
    ARParam			wparam;
	int				xsize, ysize;

    // Open the video path.
    if (arVideoOpen(vconf) < 0) {
    	fprintf(stderr, "setupCamera(): Unable to open connection to camera.\n");
    	return (FALSE);
	}
	
    // Find the size of the window.
    if (arVideoInqSize(&xsize, &ysize) < 0) return (FALSE);
    fprintf(stdout, "Camera image size (x,y) = (%d,%d)\n", xsize, ysize);
	
	// Load the camera parameters, resize for the window and init.
    if (arParamLoad(cparam_name, 1, &wparam) < 0) {
		fprintf(stderr, "setupCamera(): Error loading parameter file %s for camera.\n", cparam_name);
        return (FALSE);
    }
    arParamChangeSize(&wparam, xsize, ysize, cparam);
    fprintf(stdout, "*** Camera Parameter ***\n");
    arParamDisp(cparam);
	
    arInitCparam(cparam);

	if (arVideoCapStart() != 0) {
    	fprintf(stderr, "setupCamera(): Unable to begin camera data capture.\n");
		return (FALSE);		
	}
	
	return (TRUE);
}

static int setupMarker(const char *patt_name, int *patt_id)
{
	
    if((*patt_id = arLoadPatt(patt_name)) < 0) {
        fprintf(stderr, "setupMarker(): pattern load error !!\n");
        return (FALSE);
    }
	
	return (TRUE);
}

// Report state of ARToolKit global variables arFittingMode,
// arImageProcMode, arglDrawMode, arTemplateMatchingMode, arMatchingPCAMode.
static void debugReportMode(void)
{
	if(arFittingMode == AR_FITTING_TO_INPUT ) {
		fprintf(stderr, "FittingMode (Z): INPUT IMAGE\n");
	} else {
		fprintf(stderr, "FittingMode (Z): COMPENSATED IMAGE\n");
	}
	
	if( arImageProcMode == AR_IMAGE_PROC_IN_FULL ) {
		fprintf(stderr, "ProcMode (X)   : FULL IMAGE\n");
	} else {
		fprintf(stderr, "ProcMode (X)   : HALF IMAGE\n");
	}
	
	if (arglDrawModeGet(gArglSettings) == AR_DRAW_BY_GL_DRAW_PIXELS) {
		fprintf(stderr, "DrawMode (C)   : GL_DRAW_PIXELS\n");
	} else if (arglTexmapModeGet(gArglSettings) == AR_DRAW_TEXTURE_FULL_IMAGE) {
		fprintf(stderr, "DrawMode (C)   : TEXTURE MAPPING (FULL RESOLUTION)\n");
	} else {
		fprintf(stderr, "DrawMode (C)   : TEXTURE MAPPING (HALF RESOLUTION)\n");
	}
		
	if( arTemplateMatchingMode == AR_TEMPLATE_MATCHING_COLOR ) {
		fprintf(stderr, "TemplateMatchingMode (M)   : Color Template\n");
	} else {
		fprintf(stderr, "TemplateMatchingMode (M)   : BW Template\n");
	}
	
	if( arMatchingPCAMode == AR_MATCHING_WITHOUT_PCA ) {
		fprintf(stderr, "MatchingPCAMode (P)   : Without PCA\n");
	} else {
		fprintf(stderr, "MatchingPCAMode (P)   : With PCA\n");
	}

	fprintf(stderr, "\n");
	
	
}

static void Quit(void)
{
	arglCleanup(gArglSettings);
	arVideoCapStop();
	arVideoClose();
	exit(0);
}

static void Keyboard(unsigned char key, int x, int y)
{
	int mode;
	switch (key) 
	{
		case 0x1B:						// Quit.
		case 'Q':
		case 'q':
			Quit();
			break;
			break;
		case 'C':
		case 'c':
			mode = arglDrawModeGet(gArglSettings);
			if (mode == AR_DRAW_BY_GL_DRAW_PIXELS) 
			{
				arglDrawModeSet(gArglSettings, AR_DRAW_BY_TEXTURE_MAPPING);
				arglTexmapModeSet(gArglSettings, AR_DRAW_TEXTURE_FULL_IMAGE);
			} 
			else 
			{
				mode = arglTexmapModeGet(gArglSettings);
				if (mode == AR_DRAW_TEXTURE_FULL_IMAGE)	arglTexmapModeSet(gArglSettings, AR_DRAW_TEXTURE_HALF_IMAGE);
				else arglDrawModeSet(gArglSettings, AR_DRAW_BY_GL_DRAW_PIXELS);
			}
			fprintf(stderr, "*** Camera - %f (frame/sec)\n", (double)gCallCountMarkerDetect/arUtilTimer());
			gCallCountMarkerDetect = 0;
			arUtilTimerReset();
			debugReportMode();
			break;
		case 'D':
		case 'd':
			arDebug = !arDebug;
			break;
		case 'O':
		case 'o':
			webcamMode = (webcamMode+1)%2;
			fprintf(stderr, "Webcam mode: %f (frame/sec)\n", (webcamMode==WCM_VERTICAL)?"Vertical":"Horizontal");
			break;
		case '?':
		case '/':
			printf("Keys:\n");
			printf(" q or [esc]    Quit demo.\n");
			printf(" c             Change arglDrawMode and arglTexmapMode.\n");
			printf(" d             Activate / deactivate debug mode.\n");
			printf(" ? or /        Show this help.\n");
			printf("\nAdditionally, the ARVideo library supplied the following help text:\n");
			arVideoDispOption();
			break;
		default:
			break;
	}
}

static void Idle(void)
{
	static int ms_prev;
	int ms;
	float s_elapsed;
	ARUint8 *image;

	ARMarkerInfo    *marker_info;					// Pointer to array holding the details of detected markers.
    int             marker_num;						// Count of number of markers detected.
    int             j, k;
	
	// Find out how long since Idle() last ran.
	ms = glutGet(GLUT_ELAPSED_TIME);
	s_elapsed = (float)(ms - ms_prev) * 0.001;
	if (s_elapsed < 0.01f) return; // Don't update more often than 100 Hz.
	ms_prev = ms;
	

	// Grab a video frame.
	if ((image = arVideoGetImage()) != NULL) 
	{
		gARTImage = image;	// Save the fetched image.
		gPatt_found = FALSE;	// Invalidate any previous detected markers.
		
		gCallCountMarkerDetect++; // Increment ARToolKit FPS counter.
		
		// Detect the markers in the video frame.
		if (arDetectMarker(gARTImage, gARTThreshhold, &marker_info, &marker_num) < 0) {
			exit(-1);
		}
		
		// Check through the marker_info array for highest confidence
		// visible marker matching our preferred pattern.
		k = -1;
		for (j = 0; j < marker_num; j++) {
			if (marker_info[j].id == gPatt_id) {
				if (k == -1) k = j; // First marker detected.
				else if(marker_info[j].cf > marker_info[k].cf) k = j; // Higher confidence marker detected.
			}
		}
		
		if (k != -1) {
			// Get the transformation between the marker and the real camera into gPatt_trans.
			arGetTransMat(&(marker_info[k]), gPatt_centre, gPatt_width, gPatt_trans);
			gPatt_found = TRUE;
		}
		
		// Tell GLUT the display has changed.
		glutPostRedisplay();
	}
}

//
//	This function is called on events when the visibility of the
//	GLUT window changes (including when it first becomes visible).
//
static void Visibility(int visible)
{
	if (visible == GLUT_VISIBLE) 
	{
		glutIdleFunc(Idle);
	} 
	else 
	{
		glutIdleFunc(NULL);
	}
}

//
//	This function is called when the
//	GLUT window is resized.
//
static void Reshape(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Call through to anyone else who needs to know about window sizing here.
}

//
// This function is called when the window needs redrawing.
//
static void Display(void)
{
    GLdouble p[16];
	GLdouble m[16];
	
	// Select correct buffer for this context.
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers for new frame.
	
	arglDispImage(gARTImage, &gARTCparam, 1.0, gArglSettings);	// zoom = 1.0.
	arVideoCapNext();
	gARTImage = NULL; // Image data is no longer valid after calling arVideoCapNext().
	
	if (gPatt_found) 
	{
		// Projection transformation.
		arglCameraFrustumRH(&gARTCparam, VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX, p);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(p);
		glMatrixMode(GL_MODELVIEW);
		
		// Viewing transformation.
		glLoadIdentity();
		// Lighting and geometry that moves with the camera should go here.
		// (I.e. must be specified before viewing transformations.)
		//none
	

		switch(webcamMode==WCM_VERTICAL)
		{
		case WCM_VERTICAL:
			wsoutx = gPatt_trans[1][0];
			wsouty = gPatt_trans[1][1];
			wsoutz = gPatt_trans[1][2];
			break;
		case WCM_HORIZONTAL:
			
			break;


		}
		

		// ARToolKit supplied distance in millimetres, but I want OpenGL to work in my units.
		arglCameraViewRH(gPatt_trans, m, VIEW_SCALEFACTOR);
		glLoadMatrixd(m);

		// All other lighting and geometry goes here.
		DrawWP7();

	} // gPatt_found
	
	// Any 2D overlays go here.
	//none
	
	glutSwapBuffers();
}

HBITMAP LoadBmp()
{
	// load bitmap from resource file
	HBITMAP bitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));
	return bitmap;
}








int main(int argc, char** argv)
{
	char glutGamemode[32];
	const char *cparam_name = "Data/camera_para.dat";
	//
	// Camera configuration.
	//
#ifdef _WIN32
	char			*vconf = "Data\\WDM_camera_acc.xml";
#else
	char			*vconf = "";
#endif
	const char *patt_name  = "Data/patt.accel";
	
	

	// ----------------------------------------------------------------------------
	// Library inits.
	//
	glutInit(&argc, argv);


	// ----------------------------------------------------------------------------
	// start our web server
	//
	WServerStart(88);

	// ----------------------------------------------------------------------------
	// Hardware setup.
	//	
	if (!setupCamera(cparam_name, vconf, &gARTCparam)) {
		fprintf(stderr, "main(): Unable to set up AR camera.\n");
		exit(-1);
	}
	if (!setupMarker(patt_name, &gPatt_id)) {
		fprintf(stderr, "main(): Unable to set up AR marker.\n");
		exit(-1);
	}
	
	// ----------------------------------------------------------------------------
	// Library setup.
	//

	// Set up GL context(s) for OpenGL to draw into.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	if (!prefWindowed) {
		if (prefRefresh) sprintf(glutGamemode, "%ix%i:%i@%i", prefWidth, prefHeight, prefDepth, prefRefresh);
		else sprintf(glutGamemode, "%ix%i:%i", prefWidth, prefHeight, prefDepth);
		glutGameModeString(glutGamemode);
		glutEnterGameMode();
	} else {
		glutInitWindowSize(prefWidth, prefHeight);
		glutCreateWindow("accelKit");
	}

	// Setup argl library for current context.
	if ((gArglSettings = arglSetupForCurrentContext()) == NULL) {
		fprintf(stderr, "main(): arglSetupForCurrentContext() returned error.\n");
		exit(-1);
	}
	debugReportMode();
	glEnable(GL_DEPTH_TEST);
	arUtilTimerReset();
		
	// Register GLUT event-handling callbacks.
	// NB: Idle() is registered by Visibility.
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutVisibilityFunc(Visibility);
	glutKeyboardFunc(Keyboard);
	
	bmpA = LoadBmp();
	glGenTextures(1, &texture[0]); // Create The Texture
    LoadGLTexture(bmpA);
	
	
	//wgStart("XDE_SkinWindow");
	wgStart("XDE_LCDWindow");
	


	glutMainLoop();

	glDeleteTextures( 1, &texture[0] );
	DeleteObject(bmpA);

	return (0);
}




