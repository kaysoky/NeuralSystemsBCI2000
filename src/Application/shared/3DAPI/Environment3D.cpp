/*
The "Environment3D" is the driver class for this project.  Most of the drawing
will be taking place in the DrawScreen Function
*/

//---------------------------------------------------------------------------
#include "primObj.cpp"
#include "cameraNlight.cpp"
#include "twoDOverlay.cpp"
#include "twoDText.cpp"
#include "Load3DS.cpp"
//---------------------------------------------------------------------------


/***********************Variable Declaration**********************************/
#define CURSOR_LINES 35         //The number of line need to draw for cicle cursor.
#define BORDER 100               //The length of the dimension
#define FRAMERATE 25            //The frame rate of the GL window

using namespace std;

HGLRC hRC = NULL;                       // Permanent rendering context
HDC hDC = NULL;                         // Private GDI device context
HWND hWnd = NULL;                       // Holds our window handle
HINSTANCE hInstance = NULL;             // Holds the instance of the application

bool keys[256];                         // Array used for the keyboard routine
bool active = true;                     // Window active flag set to true by default
bool fullscreen = true;                 // Fullscreen flag set to fullscreen mode by default
bool lp;		                // L Pressed?
bool light;                             // Is light enabled?
bool    blend;	        	        // Blending OFF/ON?
bool	bp;			        // B Pressed?

/*Process Measurement Variables*/
LARGE_INTEGER   prectime1, prectime2, prectimebase, overhead;
double timeinms;



/*Boundary checking variables  */
GLfloat xPos = 0.0f;
GLfloat xNeg = 0.0f;
GLfloat yPos = 0.0f;
GLfloat yNeg = 0.0f;
GLfloat zPos = 0.0f;
GLfloat zNeg = 0.0f;


//positional variables
GLfloat z = 0.0f;                       //depth
GLfloat y = 0.0f;
GLfloat x = 0.0f;


//font variables			// Base Display List For The Font Set	( ADD )
bool buildfont = false;

//Rotational variables
GLfloat rot;    //variable rotation angle, press R button to rotate object by its plane
GLfloat rotP;   //variable retation angle with respect to a reference point.
bool rotating;  //indicates if the object is still in the poscess of rotating with respect to a reference point.


//debug Variables
bool debug=true;                             //debug bool
FILE *f, *g;                                //debug file

//2D overlay Vairable
RECT    WinDimension;                   //Rectangle that holds the window dimension

//ERROR messoage variable
string globalError;

vector<sphere>          sphereVec(0);
vector<cuboids>         cuboidsVec(0);
vector<threeDText>      threeDTextVec(0);
cameraNLight            camLig;
twoDOverlay             overlay;
vector<twoDText>        twoDTextVec(0);
vector<model3Dclass>    model3dList(0);

/*******************************Function Declaration***************************/
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);   // Declaration for WndProc
GLvoid ReSizeGLScene(GLsizei , GLsizei );               // Resize the GL window
int InitGL();                                           // Initialize GL window
int DrawScreen();                                      // Draw GL window
GLvoid KillGLWindow();                                  // Kill the GL window
bool CreateGLWindow(char* , int , int , int , bool );   // Create the GL window
                                                        // Also calls InitGL()
WINAPI Driver(HINSTANCE , HINSTANCE , LPSTR , int );    // Main program flow

GLvoid BuildSphere();                                   // Builds the sphere primitive
GLvoid BuildCuboid();                                   // Builds the cuboid primitive
GLvoid Build3Dtext();                                   // Builds the 3D text in the 3D environment
GLvoid Build2DOverlay();                                // Builds the 2D overlay
GLvoid Build2Dtext();                                   // Builds the 2D text on the overlay
GLvoid Build2DCursor();                                 // Builds the 2D cursors on the overlay
GLuint BuildFont(string fontStr, GLuint fontID, HFONT oldFont, int i);	// Build our bitmap font                                     // Builds the 3D Texts' font
GLvoid KillFont();                                      // Kill the 3D Texts
GLvoid glPrint(int, GLuint fontID, const char*, ...);                                       // Print the 3D texts to the GL window
GLuint Build2DFont(string , int , HFONT );
GLvoid glDraw2DText(const char *strString, GLuint fontID, GLfloat X, GLfloat Y );
GLvoid Kill2DFont(GLuint );
GLvoid Position2DObject( GLfloat , GLfloat, int );
GLvoid Position2DObject( GLfloat x, GLfloat y, int fontHeight);

//Move the selected object to the new XYZ coordinate by certain speed.
GLvoid moveObj ( int , int,  GLfloat , GLfloat , GLfloat , GLfloat  );
//Rotate the selected object by certain degree with respect to a point's axies
GLvoid rotateObjByPoint(int typeVec, int index, GLfloat rpX, GLfloat rpY, GLfloat rpZ, int speed, GLfloat angXY, GLfloat direction);
//Check for the collision of two objects
bool collide(int objType1, int index1, int objType2, int index2);
//create the texture and save it into the object's "textureData" variable
bool createTexture(string, GLuint, int, int);
//make the object self rotate
GLvoid rotateByMidpoint (int typeIndex, int index, int axis, float degree, int speed);
//collision detection between a cuboid and a border
bool cuboidNborderCollide(int objType2, int index2, int objType1, int index1);
//cross product
GLfloat* CrossProduct(GLfloat V1X, GLfloat V1Y, GLfloat V1Z, GLfloat F1X, GLfloat F1Y, GLfloat F1Z);
//draw the 3ds file
void Draw3ds(model3Dclass &modelClass, int i);
//check the camera change
void checkCamera();

void Draw3ds(model3Dclass &tempModel, int i)
{
try
{

        if(tempModel.isOn)
        {

        t3DModel g_3DModel= tempModel.mod;
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        //coordingate reltiave to the world origin is store at array[12][[13][14]
        GLfloat posAfterCamera[16];
        GLfloat movePointM[16];
        GLfloat rotate1[16];
        GLfloat rotate2[16];
        GLfloat rotate3[16];
        GLfloat rotate4[16];
        GLfloat translatef[16];
        GLfloat rotate5[16];
        GLfloat rotate6[16];
        GLfloat rotate7[16];
        GLfloat selfRot1[16];
        GLfloat selfRot2[16];
        GLfloat selfRot3[16];
        //view                  //aim
	gluLookAt(camLig.getCamViewX()+x, camLig.getCamViewY()+y, camLig.getCamViewZ()+z, camLig.getCamAimX(), camLig.getCamAimY(), camLig.getCamAimZ(), 0.0f, 1.0f, 0.0f);
        //store camera matrix
        glGetFloatv(GL_MODELVIEW_MATRIX,posAfterCamera);

        //check for the boundary and reposition the objects if they are out of bound
        if (xPos != 0.0f && xNeg!=0.0f && yPos!=0.0f && yNeg!=0.0f && zPos!=0.0f && zNeg!=0.0f)    //it's a sphere
        {
                if (tempModel.X > xPos)//sphere's x coord is out of bound
                {
                tempModel.X = xPos;
                }
                if (tempModel.X < xNeg)
                {
                tempModel.X = xNeg;
                }
                if (tempModel.Y > yPos)//y coord is out of bound
                {
                tempModel.Y = yPos;
                }
                if (tempModel.Y < yNeg)
                {
                tempModel.Y = yNeg;
                }
                if (tempModel.Z > zPos)//z coord is out of bound
                {
                tempModel.Z = zPos;
                }
                if (tempModel.Z < zNeg)
                {
                tempModel.Z = zNeg;
                }
        }//check boundary

        //move to the rotation point
        glLoadIdentity();
        glTranslatef(tempModel.rotPointX, tempModel.rotPointY, tempModel.rotPointZ);
        glGetFloatv(GL_MODELVIEW_MATRIX, movePointM);

if(debug)fprintf(f, "IN DRAW 3DS CURRENT OBJECT POSITION IS: X %d, Y %d, Z %d.\n", tempModel.X, tempModel.Y, tempModel.Z);

        GLfloat tempx, tempy, tempz;
        tempx = tempModel.X - tempModel.rotPointX;
        tempy = tempModel.Y - tempModel.rotPointY;
        tempz = tempModel.Z - tempModel.rotPointZ;
if(debug)fprintf(f, "IN DRAW 3DS rot point POSITION IS: X %f, Y %f, Z %f.\n",  tempModel.rotPointX,  tempModel.rotPointY,  tempModel.rotPointZ);

if(debug)fprintf(f, "IN DRAW 3DS TEMP OBJECT POSITION IS: X %f, Y %f, Z %f.\n", tempx, tempy, tempz);

        //distant from the distant point to the object in the xz plane
        GLfloat dist = sqrt(tempx*tempx + tempz * tempz);
        //distant from the distant point to the object in the yz plane
        GLfloat dist2 = sqrt(tempz*tempz + tempy*tempy);
        if (dist == 0) dist = 0.000000001;
        if (dist2 == 0) dist2 = 0.000000001;

        //toMove is the distanct of the object from its rotate point on the x-axis of the object vie.
        GLfloat toMove = sqrt(tempx*tempx + tempy*tempy + tempz*tempz);

        //the angel that is going to rotate the xz plane around the y axis
        GLfloat  ang, angy;

        if (dist == 0)  dist+= 0.0000001;
        if (toMove == 0) toMove += 0.000001;




        //in xz plane, negative z face up 2 quadrane
        tempz = 0-tempz;
        //rotate the object around rotate point's y axis to makes it reach the z coordiante
        ang = asin(tempz/dist);
        //if (tempz<=0)   ang+=180.0f;
        //rotate the object around rotate point's z axis to make it reach the y coordinate
        angy = asin(tempy/toMove);
        //change angle from radian to degrees
        ang = ang*180/M_PI;
        angy = angy*180/M_PI;

        //find out original z
        GLfloat oldTempz = 0 - tempz;
        //if x is negative
        if ( tempx<0)
        {
        ang = asin(oldTempz/dist);
        ang = ang*180/M_PI;
        angy = 0-angy;
        }

        //draw the orbiting object
        //rotate with respect to the reference point
        GLfloat XYang = tempModel.rotPointAngleXY;
        GLfloat rotDirection = tempModel.rotPointAngleYZ;

        //move to the disingated coordiate
        glLoadIdentity();
        glRotatef(ang, 0,1, 0);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate1);

        glLoadIdentity();
        glRotatef(angy, 0,0,1);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate2);

        //rotatae XYang angles around the distant point in rotDirection angle direction
        glLoadIdentity();
        glRotatef(rotDirection,  1,0,0);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate3);

        glLoadIdentity();
        glRotatef(XYang, 0,0, 1);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate4);


/******************Calculating the new object coordinate***********************/
        //total second reotation angle
        GLfloat secRotAng = XYang + angy;
        GLfloat tempBase = toMove*cos((double)secRotAng);
        GLfloat newy = toMove*sin((double)secRotAng);
        GLfloat newx = tempBase*cos((double)ang);
        GLfloat newz = tempBase*sin((double)ang);

//move to the objects' midpoints (coordinate) by calculating the object's relative position
//the z coordinate is 0 because the coordinate alredy rotate for rotation purpose
        glLoadIdentity();
        if (tempx<0) toMove = 0-toMove;
        glTranslatef(toMove, 0, 0);
        glGetFloatv(GL_MODELVIEW_MATRIX, translatef);

//rotate back to original position. So the object wouldn't look tilted
        glLoadIdentity();
        glRotatef((0-XYang), 0,0,1);
        glRotatef((0-rotDirection), 1,0,0);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate5);

        glLoadIdentity();
        glRotatef((0-angy), 0,0, 1);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate6);

        glLoadIdentity();
        glRotatef((0-ang), 0,1,0);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate7);

//rotate the object with respect to its mid point
        if (tempModel.xaxisAngle>=360)
        tempModel.xaxisAngle -= 360;
        if (tempModel.yaxisAngle>=360)
        tempModel.yaxisAngle -= 360;
        if (tempModel.zaxisAngle>=360)
        tempModel.zaxisAngle -= 360;
        //rotate with respect to x axis
        glLoadIdentity();
        glRotatef(tempModel.xaxisAngle, 1.0f, 0.0f, 0.0f);
        glGetFloatv(GL_MODELVIEW_MATRIX, selfRot1);
        //rotate with respect to y axis
        glLoadIdentity();
        glRotatef(tempModel.yaxisAngle, 0.0f, 1.0f, 0.0f);
        glGetFloatv(GL_MODELVIEW_MATRIX, selfRot2);
        //rotate with respect to z axis
        glLoadIdentity();
        glRotatef(tempModel.zaxisAngle, 0.0f, 0.0f, 1.0f);
        glGetFloatv(GL_MODELVIEW_MATRIX, selfRot3);

/****************************ACTURAL DRAWING***************************/
        glLoadIdentity();
        //CAMERAgl
        glMultMatrixf(posAfterCamera);
        //MOVEMENT AND ROTATION
        glMultMatrixf(movePointM);
        glMultMatrixf(rotate1);
        glMultMatrixf(rotate2);
        glMultMatrixf(rotate3);
        glMultMatrixf(rotate4);
        glMultMatrixf(translatef);             
        glMultMatrixf(rotate5);
        glMultMatrixf(rotate6);
        glMultMatrixf(rotate7);
        glMultMatrixf(selfRot1);
        glMultMatrixf(selfRot2);
        glMultMatrixf(selfRot3);



    	// Since we know how many objects our model has, go through each of them.
	for(int i = 0; i < g_3DModel.numOfObjects; i++)
	{
        if(debug)fprintf(f, "IN LOOP FOR 3DS.\n");
		// Make sure we have valid objects just in case. (size() is in the vector class)
		if(g_3DModel.pObject.size() <= 0) break;

		// Get the current object that we are displaying
		t3DObject *pObject = &g_3DModel.pObject[i];
			
		// Check to see if this object has a texture map, if so bind the texture to it.
		if(pObject->bHasTexture)
                {

			// Turn on texture mapping and turn off color
			glEnable(GL_TEXTURE_2D);

			// Reset the color to normal again
			glColor3ub(255, 255, 255);

			// Bind the texture map to the object by it's materialID
			glBindTexture(GL_TEXTURE_2D, g_Texture[pObject->materialID]);
		}
                else
                {

			// Turn off texture mapping and turn on color
			glDisable(GL_TEXTURE_2D);

			// Reset the color to normal again
			glColor3ub(255, 255, 255);
		}

		// This determines if we are in wireframe or normal mode
		glBegin(GL_TRIANGLES);					// Begin drawing with our selected mode (triangles or lines)

			// Go through all of the faces (polygons) of the object and draw them
			for(int j = 0; j < pObject->numOfFaces; j++)
			{
				// Go through each corner of the triangle and draw it.
				for(int whichVertex = 0; whichVertex < 3; whichVertex++)
				{
					// Get the index for each point of the face
					int index = pObject->pFaces[j].vertIndex[whichVertex];
			
					// Give OpenGL the normal for this vertex.
					glNormal3f(pObject->pNormals[ index ].x, pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);

					// If the object has a texture associated with it, give it a texture coordinate.
					if(pObject->bHasTexture) {

						// Make sure there was a UVW map applied to the object or else it won't have tex coords.
						if(pObject->pTexVerts)
                                                {
							glTexCoord2f(pObject->pTexVerts[ index ].x, pObject->pTexVerts[ index ].y);
						}
					} else
                                        {

						// Make sure there is a valid material/color assigned to this object.
						// You should always at least assign a material color to an object,
						// but just in case we want to check the size of the material list.
						// if the size is at least one, and the material ID != -1,
						// then we have a valid material.
						if(g_3DModel.pMaterials.size() && pObject->materialID >= 0) 
						{
							// Get and set the color that the object is, since it must not have a texture
							BYTE *pColor = g_3DModel.pMaterials[pObject->materialID].color;

							// Assign the current color to this model
							glColor3ub(pColor[0], pColor[1], pColor[2]);
						}
					}

					// Pass in the current vertex of the object (Corner of current face)
					glVertex3f(pObject->pVerts[ index ].x, pObject->pVerts[ index ].y, pObject->pVerts[ index ].z);
				}
			}

		glEnd();								// End the drawing
	}  /* for*/
/**************************calculate new position******************************/
GLfloat posAfterRot[16];


glLoadIdentity();
        glMultMatrixf(movePointM);
        glMultMatrixf(rotate1);
        glMultMatrixf(rotate2);
        glMultMatrixf(rotate3);
        glMultMatrixf(rotate4);
        glMultMatrixf(translatef);
        glMultMatrixf(rotate5);
        glMultMatrixf(rotate6);
        glMultMatrixf(rotate7);
        glMultMatrixf(selfRot1);
        glMultMatrixf(selfRot2);
        glMultMatrixf(selfRot3);
        glGetFloatv(GL_MODELVIEW_MATRIX,posAfterRot);
        if(debug) fprintf(f, "posAfterRot x is %f, y is %f, z is %f degrees.\n", posAfterRot[12], posAfterRot[13], posAfterRot[14]);
glLoadIdentity();

model3dList[i].NewX=posAfterRot[12];
model3dList[i].NewY=posAfterRot[13];
model3dList[i].NewZ=posAfterRot[14];
model3dList[i].rotPointAngleXY=0;



        }//if


        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in BuildSphere() function.\n";
                 try
                 {
                        throw Exception("Error in BuildSphere() function");

                 }
                 catch (Exception &exception)
                 {
                         //Application->ShowException(&exception);
                 }
        }//catch
}//draw3ds

/*****************************IMPLEMENTATION***********************************/
/*
*   This function is the helper function for the Biuld2Dtext() function
*/
GLvoid Position2DObject( GLfloat x, GLfloat y, int fontHeight)
{
try{

        //if (debug) fprintf(f, "screen height is %lf font height is %d.\n", WinDimension.top, fontHeight);
        if (debug) fprintf(f, "screen height is %lf n", WinDimension.top);
        if (debug) fprintf(f, " font height is %d.\n", fontHeight);

	glPushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT );

	// Here we use a new projection and modelview matrix to work with.
	glMatrixMode( GL_PROJECTION );						// Set our matrix to our projection matrix
      	glPushMatrix();										// Push on a new matrix to work with
     	glLoadIdentity();									// reset the matrix
      	glMatrixMode( GL_MODELVIEW );						// Set our matrix to our model view matrix
	glPushMatrix();										// Push on a new matrix to work with
	glLoadIdentity();									// Reset that matrix

        glViewport( x , y , 0, 0 );					// Create a new viewport to draw into
        glRasterPos4f(0 ,0, 0, 1 );				// Set the drawing position
	glPopMatrix();						// Pop the current modelview matrix off the stack
	glMatrixMode( GL_PROJECTION );				// Go back into projection mode
	glPopMatrix();										// Pop the projection matrix off the stack

       	glPopAttrib();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();									// This restores our TRANSFORM and VIEWPORT attributes

       // if (debug) fprintf(f, "In position function. Positioning the text with font %d.\n", fontHeight);

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Position2DObject() function.\n";
                 try
                 {
                        throw Exception("Error in Position2DObject() function\n");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch
}//Position2DObject



void Enable2D()
{
try
{
// Get the current viewport dimensions
int viewPort[4]; // Select the projection matrix
// Push the current projection matrix
glGetIntegerv(GL_VIEWPORT, viewPort);
glMatrixMode(GL_PROJECTION);
glPushMatrix(); //save matrix
glLoadIdentity(); // Setup ortho mode
// Select modelview matrix
glOrtho(0, viewPort[2], 0, viewPort[3], 0, 1 );
glMatrixMode(GL_MODELVIEW);//Push the current modelview matrix
glPushMatrix();
glLoadIdentity(); //reset modelview matrix

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Enable2D() function.\n";
                 try
                 {
                        throw Exception("Error in Enable2D()\n");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch
}//Enable2D

// Disables orthographic mode and returns back to
// perspective projection mode for 3D rendering
void Disable2D()
{
try
{

// Select the projection matrix
glMatrixMode(GL_PROJECTION);
// Pop the current projection matrix to return to
// perspective mode, our original projection matrix
glPopMatrix();
glMatrixMode(GL_MODELVIEW);
glPopMatrix();

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Disable2D() function.\n";
                 try
                 {
                        throw Exception("Error in Disable2D() function.\n");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch

}//Disable2D

/*
* Build2DCursor()
* This function draw the circular cursor in the twoDOverlay class
*/
GLvoid Build2DCursor()
{
try
{
        Enable2D();
        if (overlay.getCursorSize()!= 0.0f)
        {
                double  ang;
                GLfloat rad = overlay.getCursorSize();

//              Position2DObject (20.0f, 20.0f, 0);
//if (debug) fprintf (f, "The cursor position is %f, %f. And color is %f, %f, %f\n",  overlay.getCurX(), overlay.getCurY(), overlay.getColorR(), overlay.getColorG(), overlay.getColorB());
                glTranslatef(overlay.getCurX(), overlay.getCurY(), 0.0f);
        glBegin(GL_POLYGON  );
                glColor3f(overlay.getColorR(), overlay.getColorG(), overlay.getColorB());
                for(int i =0;i<=CURSOR_LINES;i++)
                {
                // M_PI defined in cmath.h
                // angle is in radian
                ang = i*2*M_PI/CURSOR_LINES;

                GLfloat x = cos(ang);
                x = x*rad;
                GLfloat y = sin(ang);
                y = y*rad;

                glVertex2f(x, y);

               // if (debug) fprintf(f, "Raidus is %f, and coordiante is (%f, %f).\n", rad,  x, y );
                }//for
        glEnd();
        }//if
        if (debug) fprintf(f, "DONE DRAWING THE CURSOR.\n");


        Disable2D();
        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Build2DCursor() function.\n";
                 try
                 {
                        throw Exception("Error in Build2DCursor() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch

}//Build2DCursor




/*
glDrawText
This function draw the actual text to the screen, on top of the 2D overlay
*/
void glDraw2DText(int index, const char *strString, GLuint fontID, GLfloat X, GLfloat Y, int fontHeight)
{
try
{

	char		strText[256];							// This will hold our text to display
	va_list		argumentPtr;							// This will hold the pointer to the argument list

	// If you have never used a va_list, listen up.  Remember printf()?
	// or sprintf()?  Well, you can add unlimited arguments into the text like:
	// printf("My name is %s and I am %d years old!", strName, age);
	// Well, that is what va_list's do.

	// First we need to check if there was even a string given
	if (strString == NULL)								// Check if a string was given
		return;											// Don't render anything then

	// First we need to parse the string for arguments given
	// To do this we pass in a va_list variable that is a pointer to the list of arguments.
	// Then we pass in the string that holds all of those arguments.
	va_start(argumentPtr, strString);					// Parse the arguments out of the string

	// Then we use a special version of sprintf() that takes a pointer to the argument list.
	// This then does the normal sprintf() functionality.
	vsprintf(strText, strString, argumentPtr);			// Now add the arguments into the full string

	va_end(argumentPtr);								// This resets and frees the pointer to the argument list.

        // position the 2D text
        if (debug) fprintf(f, "The text coordinate is %f, %f, font heiht is %d.\n", X, Y, fontHeight);

        LARGE_INTEGER T1, T2;
                 Position2DObject( X,Y, fontHeight);
        if(debug )
        {
        QueryPerformanceCounter(&T2);
        timeinms= ( (double)T2.QuadPart-(double)T1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
        if(debug)fprintf(g, "\t\t\tPosition Object cost %e ms.\n", timeinms);
        }


    	// Now, before we set the list base, we need to save off the current one.
	glPushAttrib(GL_LIST_BIT);							// This saves the list base information


	// Then we want to set the list base to the font's list base, which should be 1 in our case.
	// That way when we call our display list it will start from the font's lists'.
	glListBase(fontID);							// This sets the lists base
        if (debug) fprintf(f, "fontID is %d.\n", fontID);
	// Now comes the actually rendering.  We pass in the length of the string,
	// then the data types (which are characters so its a UINT), then the actually char array.
	// This will then take the ASCII value of each character and associate it with a bitmap.

if(debug ){QueryPerformanceCounter(&T1);}
	glCallLists(strlen(strText), GL_UNSIGNED_BYTE, strText);
if(debug )
{
        QueryPerformanceCounter(&T2);
        timeinms= ( (double)T2.QuadPart-(double)T1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
        if(debug)fprintf(g, "\t\t\tglCallLists %e ms.\n", timeinms);
}
	glPopAttrib();

									// Return the display list back to it's previous state
        if(debug) fprintf(f, "Done drawing text, %s.\n", strText);
        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in glDraw2DText() function.\n";
                 try
                 {
                        throw Exception("Error in glDraw2DText() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch
}//GLDraw2DText


/*
Build2DFont()
This function build the 2D font that is going to be used in the 2D text
*/
GLuint Build2DFont(string strFontName, int height, HFONT oldFont)	// Build Our Bitmap Font
{
try
{
	GLuint	fontListID ;								// This will hold the base ID for our display list
	HFONT	hFont;										// This will store the handle to our font

	// Here we generate the lists for each character we want to use.
	// This function then returns the base pointer, which will be 1 because
	// we haven't created any other lists.  If we generated another list after
	// this, the base pointer would be at 257 since the last one used was 256 (which is MAX_CHARS)
	fontListID = glGenLists(256);					// Generate the list for the font

	// Now we actually need to create the font.  We use a windows function called:
	// CreateFont() that returns a handle to a font (HFONT).  Our CreateOpenGLFont()
	// function allows us to pass in a name and height.  For simplistic reasons, I left
	// other options out, but feel free to add them to your function (like bold, italic, width..)

	hFont = CreateFont(	                height,			                // Our desired HEIGHT of the font
						0,					// The WIDTH (If we leave this zero it will pick the best width depending on the height)
						0,					// The angle of escapement
						0,					// The angle of orientation
						FW_BOLD,				// The font's weight (We want it bold)
						FALSE,					// Italic - We don't want italic
						FALSE,					// Underline - We don't want it underlined
						FALSE,					// Strikeout - We don't want it strikethrough
						ANSI_CHARSET,				// This is the type of character set
						OUT_TT_PRECIS,				// The Output Precision
						CLIP_DEFAULT_PRECIS,			// The Clipping Precision
						ANTIALIASED_QUALITY,			// The quality of the font - We want anitaliased fonts
						FF_DONTCARE|DEFAULT_PITCH,		// The family and pitch of the font.  We don't care.
						strFontName.c_str());					// The font name (Like "Arial", "Courier", etc...)

	// Now that we have created a new font, we need to select that font into our global HDC.
	// We store the old font so we can select it back in when we are done to avoid memory leaks.
	oldFont = (HFONT)SelectObject(hDC, hFont);

	// This function does the magic.  It takes the current font selected in
	// the hdc and makes bitmaps out of each character.  These are called glyphs.
	// The first parameter is the HDC that holds the font to be used.
	// The second parameters is the ASCII value to start from, which is zero in our case.
	// The third parameters is the ASCII value to end on (255 is the last of the ASCII values so we minus 1 from MAX_CHARS)
	// The last parameter is the base pointer for the display lists being used.  This should be 1.
        if (debug) fprintf (f, "THE FONT LIST ID IS  %d.\n", fontListID);
	wglUseFontBitmaps(hDC, 0,255, fontListID);	// Builds 255 bitmap characters
        SelectObject(hDC, oldFont);				// Selects The Font We Want
	DeleteObject(hFont);					// Delete The Font
	return fontListID;									// Return the ID to the display list to use later
        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Build2DFont() function.\n";
                 try
                 {
                        throw Exception("Error in Build2DFont() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch


}//Build2DFont



/*
Build2Dtext()
This function build the 2D text by using the display list.  The memory for all the
2D texts are in the fontID variable of the twoDText class.  This function will called
its helper function print2Dtext to perform the actual printing of the 2D texts to
the 2D overlay
*/
GLvoid Build2Dtext()
{
try
{
        int textVecLen = twoDTextVec.size();
        if (debug)      fprintf(g, "Building the 2d Text and the txtVecLen is %d.\n", textVecLen);
        for (int i=0; i < textVecLen; i++)
        {
                glLoadIdentity();

                glColor3f(twoDTextVec[i].getColorR(),twoDTextVec[i].getColorG(),twoDTextVec[i].getColorB());
if (debug)      fprintf(f, "Building text color %f, %f, %f.\n", twoDTextVec[i].getColorR(),twoDTextVec[i].getColorG(),twoDTextVec[i].getColorB());
        //Position2DObject( 0,0,0);
        LARGE_INTEGER time1, time2;
        if(debug ) {QueryPerformanceCounter(&time1);}
       
                glDraw2DText(i, twoDTextVec[i].getCaption().c_str(), twoDTextVec[i].getFontID(), twoDTextVec[i].getTextX(), twoDTextVec[i].getTextY(), twoDTextVec[i].getTextSize());
        if(debug )
        {
        QueryPerformanceCounter(&time2);
        timeinms= ( (double)time2.QuadPart-(double)time1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
        fprintf(g, "\t 2D Text Vector, unit %d, took %e ms.\n", i, timeinms);
        }

        }//for

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Build2DText() function.\n";
                 try
                 {
                        throw Exception("Error in Build2DText() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch

}//Build2DText



/*
This function builds the 2D overlay by loading a texture file from the texture
directory and map to the screen.  The size of the texture file is same as the
current screen demension
*/
GLvoid Build2DOverlay()
{
try
{
        glLoadIdentity();               //reset the view
        //set the camera
        gluLookAt(camLig.getCamViewX()+x, camLig.getCamViewY()+y, camLig.getCamViewZ()+z, camLig.getCamAimX(), camLig.getCamAimY(), camLig.getCamAimZ(), 0.0f, 1.0f, 0.0f);

        // Before drawing the scope texture, we need to switch to ortho (2D) mode.  We pass in
	// our screen width and height to use as the right and bottom coordinates of the view port.

        // Switch to our projection matrix so that we can change it to ortho mode, not perspective.
	glMatrixMode(GL_PROJECTION);
	// Push on a new matrix so that we can just pop it off to go back to perspective mode
	glPushMatrix();
	// Reset the current matrix to our identify matrix
	glLoadIdentity();
	//Pass in our 2D ortho screen coordinates.like so (left, right, bottom, top).  The last
	// 2 parameters are the near and far planes.
	glOrtho( WinDimension.left, WinDimension.right, WinDimension.bottom, WinDimension.top, 0, 1 );



        // Switch to model view so that we can render the 2D image
	glMatrixMode(GL_MODELVIEW);
        // Initialize the current model view matrix with the identity matrix
	glLoadIdentity();


        if (debug) fprintf(f, "LOADING THE MASKING AND OVERLAY.\n");
        if (debug) fprintf(f, "Loading the %s.\n", overlay.getOverlayTrans());

        // Load the texture for the transparency file
       if (overlay.getTraTexture())
        {
               glColor4f(1, 1, 1, 1);
                // Disable depth testing (MUST do this to make it work)
        	glDisable(GL_DEPTH_TEST);

                //Enable texture drawing
                glEnable(GL_TEXTURE_2D);
	        // Select our desired depth testing and turn on blending

	        glEnable(GL_BLEND);
                glBlendFunc(GL_DST_COLOR,GL_ZERO);

              	//Bind the mask texture to our new 2D quad
        	glBindTexture(GL_TEXTURE_2D,  overlay.getTransData());

               	// Display a 2D quad with the scope/cross hair mask
	glBegin(GL_QUADS);

		// Notice that when we are in ortho mode, we use glVertex2f() to pass
		// in screen coordinates, not vertices.  This makes it incredibly easy
		// to put up 2D interface art.  It's just like doing 2D graphics.
		// The texture coordinate stay the same regardless.

		// Display the top left point of the 2D image
		glTexCoord2f(0.0f, 1.0f);	glVertex2f(0, 0);

		// Display the bottom left point of the 2D image
		glTexCoord2f(0.0f, 0.0f);	glVertex2f(0, WinDimension.bottom);

		// Display the bottom right point of the 2D image
		glTexCoord2f(1.0f, 0.0f);	glVertex2f(WinDimension.right, WinDimension.bottom);

		// Display the top right point of the 2D image
		glTexCoord2f(1.0f, 1.0f);	glVertex2f(WinDimension.right, 0);


	// Stop drawing
	glEnd();

                if (overlay.getPicTexture())
                {
               	// Turn our blending mode to one to one
        	glBlendFunc(GL_ONE,GL_ONE);

	        // Bind the scope texture to this next 2D quad
        	glBindTexture(GL_TEXTURE_2D,  overlay.getPictureData());

	        // Display a 2D quad with our scope texture
        	glBegin(GL_QUADS);

        		// Display the top left point of the 2D image
	        	glTexCoord2f(0.0f, 1.0f);	glVertex2f(0, 0);

		        // Display the bottom left point of the 2D image
	        	glTexCoord2f(0.0f, 0.0f);	glVertex2f(0, WinDimension.bottom);

        		// Display the bottom right point of the 2D image
		        glTexCoord2f(1.0f, 0.0f);	glVertex2f(WinDimension.right, WinDimension.bottom);

                        // Display the top right point of the 2D image
		        glTexCoord2f(1.0f, 1.0f);	glVertex2f(WinDimension.right, 0);

	        // Stop drawing
        	glEnd();

                // Since we are done, we can now turn OFF blending and turn ON depth testing
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
                glDisable(GL_TEXTURE_2D);
                }//if  overlay has picture

        }//if texture created
        else
        {
                if(debug)
                fprintf(f, "The mask loading failed.\n");
        }

        // Let's set our mode back to perspective 3D mode.  None of this archaic 2D stuff :)
        // Enter into our projection matrix mode
        glMatrixMode( GL_PROJECTION );
        // Pop off the last matrix pushed on when in projection mode (Get rid of ortho mode)
        glPopMatrix();
        // Go back to our model view matrix like normal
        glMatrixMode( GL_MODELVIEW );
        // We should be in the normal 3D perspective mode now
        glPopMatrix();

                }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Build2DOverlay() function.\n";
                 try
                 {
                        throw Exception("Error in Build2DOverlay() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch



}//Build2DOverlay

//Build 3DText
GLvoid Build3Dtext(threeDText textObj, int i)
{
try
{

        //enable material color for the 3D texts
        //glEnable(GL_LIGHT0);								// Enable Default Light (Quick And Dirty)
	//								// Enable Lighting

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity() ;

//set the camera view
gluLookAt(camLig.getCamViewX()+x, camLig.getCamViewY()+y, camLig.getCamViewZ()+z, camLig.getCamAimX(), camLig.getCamAimY(), camLig.getCamAimZ(), 0.0f, 1.0f, 0.0f);

if(debug) fprintf(f, "In build3d Text.\n");

        if(textObj.getStatus())
        {
        //build the font

if(debug) fprintf(f, "building...\n");
//check for the boundary and reposition the objects if they are out of bound
        if (xPos != 0.0f && xNeg!=0.0f && yPos!=0.0f && yNeg!=0.0f && zPos!=0.0f && zNeg!=0.0f)    //boundary has been set
        {
                if (textObj.getOriX() > xPos)//sphere's x coord is out of bound
                {
                textObj.setOrigin(xPos, textObj.getOriY(), textObj.getOriZ());
                }
                if (textObj.getOriX() < xNeg)
                {
                textObj.setOrigin(xNeg, textObj.getOriY(), textObj.getOriZ());
                }
                if (textObj.getOriY() > yPos)//y coord is out of bound
                {
                textObj.setOrigin(textObj.getOriX(), yPos, textObj.getOriZ());
                }
                if (textObj.getOriY() < yNeg)
                {
                textObj.setOrigin(textObj.getOriX(), yNeg, textObj.getOriZ());
                }
                if (textObj.getOriZ() > zPos)//z coord is out of bound
                {
                textObj.setOrigin(textObj.getOriX(), textObj.getOriY(), zPos);
                }
                if (textObj.getOriZ() < zNeg)
                {
                textObj.setOrigin(textObj.getOriX(), textObj.getOriY(), zNeg);
                }
        }//if bound has been set

        //move to origin of the text
        glTranslatef(textObj.getOriX(),textObj.getOriY(), textObj.getOriZ());

        GLfloat dirX =        textObj.getDirX();
        GLfloat dirY =        textObj.getDirY();
        GLfloat dirZ =        textObj.getDirZ();
        GLfloat zPlaneRot;
        GLfloat yPlaneRot;

        GLfloat toDiv =  ( textObj.getOriX()-dirX);
        if (toDiv == 0.0f)       //check for 0
        {
                toDiv = 0.000001f;
                zPlaneRot =  (GLfloat)atan( (double)( (textObj.getOriY()-dirY) / toDiv));
                yPlaneRot =  (GLfloat)atan( (double)( (textObj.getOriZ()-dirZ) / toDiv ));
                dirX = 0.0f;
        }
        else
        {
                zPlaneRot =  (GLfloat)atan( (double)( (textObj.getOriY()-dirY) /toDiv ));
                yPlaneRot =  (GLfloat)atan( (double)( (textObj.getOriZ()-dirZ) /toDiv ));
        }


if (debug) fprintf(f, "font dirY 1and dirZ are %f and %f.\n", dirY, dirZ);
if (debug) fprintf(f, "B4B4BB4B4B4B4: font rotate z by %f degree and y by %f degree.\n", zPlaneRot, yPlaneRot);



        //change to degree
        zPlaneRot = zPlaneRot*180/3.1415926525;
        yPlaneRot = yPlaneRot*180/3.1415926525;

        if(dirX>textObj.getOriX())
        {
                if(dirY>textObj.getOriY())
                {
                        //zplanerot rotates positiove degree unchanged
                }
                if(dirY<textObj.getOriY())
                {
                        //zplanerot rotates negative degree unchanged
                }
                if(dirZ>textObj.getOriZ())
                {
                        //change positive degree to negative
                        yPlaneRot = 0-yPlaneRot;
                }
                if(dirZ<textObj.getOriZ())
                {
                        //change negative degree to positive
                        yPlaneRot = 0-yPlaneRot;
                }
        }
        if(dirX<textObj.getOriX())
        {
                if(dirY>textObj.getOriY())
                {
                         //zplanerot rotates from negative side of the x axis
                        zPlaneRot += 180;
                }
                if(dirY<textObj.getOriY())
                {
                        //zplanerot rotates from negative side of the x axis
                        zPlaneRot += 180;
                }
                if(dirZ>textObj.getOriZ())
                {
                        //negative degree

                }
                if(dirZ<textObj.getOriZ())
                {
                        //positive degree
                        
                }
        }

        //rotate to the direction
        //      rotate with respect to z
        glRotatef(zPlaneRot, 0.0f, 0.0f, 1.0f );
        //      rotate with respect to y
        glRotatef(yPlaneRot, 0.0f, 1.0f, 0.0f );
if (debug) fprintf(f, "font rotate z by %f degree and y by %f degree.\n", zPlaneRot, yPlaneRot);

//transparency and brightness
        GLfloat fontBri = (GLfloat)textObj.getBrightness()/255;
        if (fontBri>= 1)      fontBri=1;
        GLfloat fontTrans = (GLfloat)textObj.getTransparency()/255;
        if (fontTrans>=1)     fontTrans = 1;

        //set the color

        glColor4f(textObj.getColorR()*fontBri,textObj.getColorG()*fontBri, textObj.getColorB()*fontBri, fontTrans);

        GLfloat temp[4];
        glGetFloatv(GL_CURRENT_COLOR, temp);
        //if (debug) fprintf(f, "temp is %f, %f, %f, %f.\n", temp[0], temp[1], temp[2], temp[3]);


        glEnable(GL_BLEND);		// Turn Blending On
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);		// Blending Function For Translucency Based On Source Alpha Value ( NEW )


        //if (debug) fprintf(f, "fontBri is %f and fontTrans is %f", fontBri, fontTrans);

        //resize the font
        glScalef(textObj.getFontSize()/12, textObj.getFontSize()/12,textObj.getFontSize()/12);
        //print the 3D text
        glPrint(i, textObj.getFontID(), textObj.getCaption().c_str());

        //if (debug) fprintf(f, "FFFFFFFFFFFFFFFF IN 3DTEXT, rotated by %f, %f.\n", zPlaneRot, yPlaneRot);

        }//if 3D text is on

        glDisable(GL_BLEND);		// Turn Blending Off
        //End Transparency





        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Build3DText() function.\n";
                 try
                 {
                        throw Exception("Error in Build3DText() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch

}//Build3DText

//Build the font for 3D text
GLuint BuildFont(string fontStr, GLuint fontID, HFONT oldFont, int i)	// Build our bitmap font
{
try
{
	HFONT font;		// Windows font ID

	fontID = glGenLists(256);	// Storage for 256 characters

	font = CreateFont(	0,	                        // Height of font
				0,				// Width of font
				0,				// Angle of escapement
				0,				// Orientation angle
				FW_BOLD,			// Font weight
				FALSE,				// Italic
				FALSE,				// Underline
				FALSE,				// Strikeout
				ANSI_CHARSET,			// Character set identifier
				OUT_TT_PRECIS,			// Output precision
				CLIP_DEFAULT_PRECIS,		// Clipping precision
				ANTIALIASED_QUALITY,		// Output quality
				FF_DONTCARE|DEFAULT_PITCH,	// Family and pitch
				fontStr.c_str());		// Font name

	SelectObject(hDC, font);				// Selects the font we created

	wglUseFontOutlines(	hDC,				// Select the current DC
				0,				// Starting character
				255,				// Number of display lists to build
				fontID,				// Starting display lists
				0.0f,				// Deviation from the true outlines
				0.3f,				// Font thickness in the Z direction
			       	WGL_FONT_POLYGONS,		// Use polygons, not lines
				threeDTextVec[i].gmf);				// Address of buffer to recieve data
        return fontID;
        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in BuildFont() function.\n";
                 try
                 {
                        throw Exception("Error in BuildFont() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch


}//BuildFont



//Kill 3D Font
GLvoid KillFont(GLvoid)						// Delete The Font
{
try
{
        if (threeDTextVec.size() != 0)
        {
        int textSize = threeDTextVec.size();
        for (int i = 0; i<textSize; i++)
        {
                glDeleteLists(threeDTextVec[i].getFontID(), 256);				// Delete All 256 Characters
        }//for
        }
        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in KillFont() function.\n";
                 try
                 {
                        throw Exception("Error in KillFont() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch


}


//Kill 2D Font
GLvoid Kill2DFont()
{
try
{

        if (twoDTextVec.size() != 0)
        {
                int vecLen = twoDTextVec.size();
                for (int i = 0; i< vecLen; i++)
                {

                glDeleteLists(twoDTextVec[i].getFontID(), 256);		        // Free the display list
                }
        }
        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Kill2DFont() function.\n";
                 try
                 {
                        throw Exception("Error in Kill2DFont() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch


}//Kill2DFont


//Print the Font to the GL window
GLvoid glPrint(int i, GLuint fontID, const char *fmt, ...)    // Custom GL "Print" routine
{
try
{
	float length = 0;		// Used to find the length of the text
	char text[256];			// Holds our string
	va_list ap;			// Pointer to list of arguments

	if (fmt == NULL)		// If there's no text
		return;			// Do nothing

	va_start(ap, fmt);		// Parses the string for variables
	    vsprintf(text, fmt, ap);    // And converts symbols to actual numbers
	va_end(ap);			// Results are stored in text

	for (unsigned int loop=0;loop<(strlen(text));loop++)	// Loop to find text length
	{
		length+=threeDTextVec[i].gmf[text[loop]].gmfCellIncX;		// Increase length by each characters width
	}

      //	glTranslatef(-length/2,0.0f,0.0f);			// Center our text on the screen

	glPushAttrib(GL_LIST_BIT);				// Pushes the display list bits
	glListBase(fontID);					// Sets the base character to 0

QueryPerformanceCounter(&prectime1);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws the display list text
QueryPerformanceCounter(&prectime2);
timeinms= ( (double)prectime2.QuadPart-(double)prectime1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if (debug) fprintf(g, "3D Text call list used %e ms\n", timeinms);


	glPopAttrib();						// Pops the display list bits
if (debug) fprintf(f, "FFFFFFFFFFFFFFFF IN glPrint.\n");
        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in glPrint() function.\n";
                 try
                 {
                        throw Exception("Error in glPrint() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch

}//glPrint







/*create the texture and save it into the object's "textureData" variable
ID:           Index of the primitive object vector,
type:         1 = sphere, 2 = cuboid, 3 = boundary, 4 = 3DText, 5 = overlay

type == 5 ==> 2Doverlay and its transparency file
        =>      ID == 1       2Doverlay
        =>      ID == 2       overlay transparency file
*/
bool createTexture(string file , GLuint tData, int ID, int type)
{
        try
        {

if (type == 2 || type == 1 || type == 3 || type == 5)
{
        AUX_RGBImageRec *TextureImage[1];	// Create Storage Space For The Texture
        //TextureImage = NULL;
        memset(TextureImage,0,sizeof(void *)*1);// Set The Pointer To NULL

        if (file.size()==0)             //check to see if the file name is given
        {
        if (debug) fprintf(f, "Texutre file name was not given.\n");
        //then clear out the texture memory and return false
        //if (debug) fprintf(f, "Disabling texture drawing.\n");
        //glDisable(GL_TEXTURE_2D);
        return false;
        }


        TextureImage[0] = auxDIBImageLoad(file.c_str());        //loading the texture file
        if (TextureImage == NULL)    //if the data is NULL
        {
        if (debug) fprintf(f, "Texture file is empty.\n");
        //glDisable(GL_TEXTURE_2D);
        return false;
        }//if

        if (debug) fprintf(f, "Texture %s loaded.\n", file);

        if (type == 5)
        {
                if (ID == 1)
                {

                glGenTextures(1, &overlay.pictureData );
                glBindTexture(GL_TEXTURE_2D, overlay.pictureData);
                }
                if (ID == 2)
                {

                glGenTextures(1, &overlay.transData );
                glBindTexture(GL_TEXTURE_2D, overlay.transData);
                }
        }
        else
        {
                if (type == 1 )    //sphere
                {
                        glGenTextures(1, &sphereVec[ID].textureData );
                        glBindTexture(GL_TEXTURE_2D,  sphereVec[ID].textureData);
                }
                if (type == 2)    //cuboids
                {

                        glGenTextures(1, &cuboidsVec[ID].textureData );
                        glBindTexture(GL_TEXTURE_2D,  cuboidsVec[ID].textureData);
                }
                if (type == 3)    //boundary
                {
                        glGenTextures(1, &cuboidsVec[ID].textureData );
                        glBindTexture(GL_TEXTURE_2D,  cuboidsVec[ID].textureData);
                }
                if (type == 4)    //3DText
                {
                }
        }
 /* */
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data); // ( NEW )

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);

        //free the memory
        if (TextureImage[0])
        {
            if(TextureImage[0]->data)
            {
                free(TextureImage[0]->data);
            }
            free(TextureImage[0]);
        }
        //glEnable(GL_TEXTURE_2D);
        return true;

}//if cuboid
else
{
        return false;
}
        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in createTexture() function.\n";
                 try
                 {
                        throw Exception("Error in createTexture() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch

}//createTexture






/*
* Function Name:cuboidNborderCollide
* Purpose:      A helper fuction for collide().  It checks for the collision
*               between the cuboid and the border.
* Parameter:    border  -- primitive ID of border, should be 3
*               borderIndex -- border's vector ID, should be 0
*               cuboid -- primitive ID of cuboid, should be 2
*               cuboidIndex -- border's vector ID, sbhoud > 0
* Return:       True    -- collision occurs
*               False   -- collision did not occurs
* Implementation:
* Compare all the 8 vertercies of the cuboids against border's plane. If only one
* of the vertex is on the other side of the plane, i.e vertex's x value is bigger
* than right side border's x value, then collision occurs
*/
bool cuboidNborderCollide(int bord, int borderIndex, int cuboid, int cuboidIndex)
{
        cuboids border = cuboidsVec[borderIndex];
        //Declare variable for 6 planes
        bool topTouch = false;
        bool bottomTouch = false;
        bool leftTouch = false;
        bool rightTouch = false;
        bool frontTouch = false;
        bool backTouch = false;
  /*      GLfloat A1, B1, C1, D1;        //top
        GLfloat A2, B2, C2, D2;         //bottom
        GLfloat A3, B3, C3, D3;         //front
        GLfloat A4, B4, C4, D4;         //back
        GLfloat A5, B5, C5, D5;         //left
        GLfloat A6, B6, C6, D6;         //right */
/**************************top plane******************************************/
        GLfloat topY;                  //Top Plane, Y value
        topY = border.getLTFY();       //Left, Top, Front's vertex's Y value
        if ( cuboidsVec[cuboidIndex].getLBFY() >= topY )        topTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTFY() >= topY )        topTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTBY() >= topY )        topTouch = true;
        if ( cuboidsVec[cuboidIndex].getLBBY() >= topY )        topTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBFY() >= topY )        topTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTFY() >= topY )        topTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTBY() >= topY )        topTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBBY() >= topY )        topTouch = true;
/************************bottom plane*****************************************/
        GLfloat bottomY = border.getLBFY();     //Left, Bottom, Front's Y value
        if ( cuboidsVec[cuboidIndex].getLBFY() <= bottomY )        bottomTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTFY() <= bottomY )        bottomTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTBY() <= bottomY )        bottomTouch = true;
        if ( cuboidsVec[cuboidIndex].getLBBY() <= bottomY )        bottomTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBFY() <= bottomY )        bottomTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTFY() <= bottomY )        bottomTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTBY() <= bottomY )        bottomTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBBY() <= bottomY )        bottomTouch = true;
/**********************************LEFT***************************************/
        GLfloat leftX = border.getLBFX();       //Left, Bottom, Front's X value
        if ( cuboidsVec[cuboidIndex].getLBFX() <= leftX )        leftTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTFX() <= leftX )        leftTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTBX() <= leftX )        leftTouch = true;
        if ( cuboidsVec[cuboidIndex].getLBBX() <= leftX )        leftTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBFX() <= leftX )        leftTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTFX() <= leftX )        leftTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTBX() <= leftX )        leftTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBBX() <= leftX )        leftTouch = true;
/***************************RIGHT PLANE****************************************/
        GLfloat rightX = border.getRBFX();       //Right, Bottom, Front's X value
        if ( cuboidsVec[cuboidIndex].getLBFX() >= rightX )        rightTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTFX() >= rightX )        rightTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTBX() >= rightX )        rightTouch = true;
        if ( cuboidsVec[cuboidIndex].getLBBX() >= rightX )        rightTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBFX() >= rightX )        rightTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTFX() >= rightX )        rightTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTBX() >= rightX )        rightTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBBX() >= rightX )        rightTouch = true;
/***************************FRONT PLANE****************************************/
        GLfloat frontZ = border.getRBFZ();       //Right, Bottom, Front's Z value
        if ( cuboidsVec[cuboidIndex].getLBFZ() >= frontZ )        frontTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTFZ() >= frontZ )        frontTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTBZ() >= frontZ )        frontTouch = true;
        if ( cuboidsVec[cuboidIndex].getLBBZ() >= frontZ )        frontTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBFZ() >= frontZ )        frontTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTFZ() >= frontZ )        frontTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTBZ() >= frontZ )        frontTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBBZ() >= frontZ )        frontTouch = true;
/***************************BACK PLANE****************************************/
        GLfloat backZ = border.getRBBZ();       //RIght, Bottom, Back's Z value
        if ( cuboidsVec[cuboidIndex].getLBFZ() <= backZ )        backTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTFZ() <= backZ )        backTouch = true;
        if ( cuboidsVec[cuboidIndex].getLTBZ() <= backZ )        backTouch = true;
        if ( cuboidsVec[cuboidIndex].getLBBZ() <= backZ )        backTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBFZ() <= backZ )        backTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTFZ() <= backZ )        backTouch = true;
        if ( cuboidsVec[cuboidIndex].getRTBZ() <= backZ )        backTouch = true;
        if ( cuboidsVec[cuboidIndex].getRBBZ() <= backZ )        backTouch = true;

if (debug) fprintf(f, "topTouch: %d, bottomTouch: %d, leftTouch: %d, rightTouch: %d, frontTouch: %d, backTouch: %d.\n", topTouch, bottomTouch, leftTouch, rightTouch, frontTouch, backTouch);
        if(topTouch || bottomTouch || leftTouch || rightTouch || frontTouch || backTouch )
        {
                //some vertercies is touching one of the 6 plane
                return true;
        }
        else
        {
                return false;
        }
}

/*
Function Name:  collide
Purpose:        Check for the collision of two objects
Input:  int objType1    --      Type of object
                1       --      sphere
                2       --      cuboids
                3       --      plane, (large cuboids boundary)
        int index1      --      the vector index of the object
        int objType2    --      same as above
        int index2      --      same as above
Return: boolean
        true    --      two objects are collided
        false   --      two objects are not collid
           
*/
bool collide(int objType1, int index1, int objType2, int index2)
{
try
{
        if(debug) fprintf(f, "STARTING COLLIDE.\n");
        //both objects are sphere
        if (objType1 == 1 && objType2 == 1)     
        {
                if(debug) fprintf(f, "SPHERE AND SPHRE COLLISOIN DETECTION.\n");
                if (index1 == index2)           //both input are a single sphere
                        return true;

                GLfloat rad1, rad2;
                rad1 = sphereVec[index1].getSphereRad();
                rad2 = sphereVec[index2].getSphereRad();

                GLfloat ax, ay, az, bx, by, bz; //coordinate for both sphere a and b
                ax = sphereVec[index1].getSphereX();
                ay = sphereVec[index1].getSphereY();
                az = sphereVec[index1].getSphereZ();
                bx = sphereVec[index2].getSphereX();
                by = sphereVec[index2].getSphereY();
                bz = sphereVec[index2].getSphereZ();
                // if xdisttance^2 + ydistance^2 + zdistance^2   >  object1 and object2's distance
                if ( (pow(ax-bx, 2) + pow(ay-by,2) + pow(az-bz,2)) > ((rad1+rad2)*(rad1+rad2)) )
                        return false;
                else
                        return true;
        }

        //one object is sphere another one is cuboids
        if ( (objType1 == 1 && objType2 == 2) || (objType1 == 2 && objType2 == 1) )
        {
                if(debug) fprintf(f, "CUBOID AND SPHERE COLLISOIN DETECTION.\n");

                cuboids compCub;
                sphere compSph;
                //setting the sphere
                if (objType1 == 1)      compSph = sphereVec[index1];
                if (objType2 == 1)      compSph = sphereVec[index2];
                //setting the cuboid
                if (objType1 == 2)      compCub = cuboidsVec[index1];
                if (objType2 == 2)      compCub = cuboidsVec[index2];
                if (debug) fprintf(f, "shere at %f, %f, %f, and its rad is %f.\n",compSph.getSphereX(), compSph.getSphereY(), compSph.getSphereZ(), compSph.getSphereRad());
                if (debug) fprintf(f, "the cuboids at %f, %f, %f with w %f, h %f, d %f.\n", compCub.getCuboidX(), compCub.getCuboidY(), compCub.getCuboidZ(), compCub.getCuboidW(), compCub.getCuboidH(), compCub.getCuboidD() );

                //boolean indicate if the sphere is touching the plane
                bool touchTop = false;
                bool touchBottom = false;
                bool touchLeft = false;
                bool touchRight = false;
                bool touchFront = false;
                bool touchBack = false;
                bool insideCub = false;
                float topSide;
                float bottomSide;
                float leftSide;
                float rightSide;
                float frontSide;
                float backSide;
                float numerator;
                //Declare variable for 6 planes
                GLfloat A1, B1, C1, D1;         //top
                GLfloat A2, B2, C2, D2;         //bottom
                GLfloat A3, B3, C3, D3;         //front
                GLfloat A4, B4, C4, D4;         //back
                GLfloat A5, B5, C5, D5;         //left
                GLfloat A6, B6, C6, D6;         //right

/**************************top plane******************************************/
                GLfloat topX1, topY1, topZ1;    //Left, Top, Front
                GLfloat topX2, topY2, topZ2;    //Left, Top, Back
                GLfloat topX3, topY3, topZ3;    //Right, Top, Back
                topX1 = compCub.getLTFX();
                topY1 = compCub.getLTFY();
                topZ1 = compCub.getLTFZ();
                topX2 = compCub.getLTBX();
                topY2 = compCub.getLTBY();
                topZ2 = compCub.getLTBZ();
                topX3 = compCub.getRTFX();
                topY3 = compCub.getRTFY();
                topZ3 = compCub.getRTFZ();
                if (debug) fprintf(f, "top plane, LTF is %f, %f, %f, LTB is %f, %f, %f, and RTB is %f, %f, %f.\n", topX1, topY1, topZ1, topX2, topY2, topZ2, topX3, topY3, topZ3);
                //plane equation
                //A = y1 (z2 - z3) + y2 (z3 - z1) + y3 (z1 - z2)
                //B = z1 (x2 - x3) + z2 (x3 - x1) + z3 (x1 - x2)
                //C = x1 (y2 - y3) + x2 (y3 - y1) + x3 (y1 - y2)
                //- D = x1 (y2 z3 - y3 z2) + x2 (y3 z1 - y1 z3) + x3 (y1 z2 - y2 z1)
                A1 = topY1 * (topZ2 - topZ3) + topY2 * (topZ3 - topZ1) + topY3 * (topZ1 - topZ2);
                B1 = topZ1 * (topX2 - topX3) + topZ2 * (topX3 - topX1) + topZ3 * (topX1 - topX2);
                C1 = topX1 * (topY2 - topY3) + topX2 * (topY3 - topY1) + topX3 * (topY1 - topY2);
                D1 = (-1)*(topX1)*(topY2*topZ3 - topY3*topZ2) + (-1)*(topX2)*(topY3*topZ1-topY1*topZ3) + (-1)*(topX3)*(topY1*topZ2-topY2*topZ1);
                //distance from sphere to top plane
                float topD;

                
                //check for divide by 0
                numerator = sqrt(A1*A1 + B1*B1 + C1*C1);
                if (numerator == 0)
                {
                numerator+= 0.000001;
                }
                topD =  ( A1*compSph.getSphereX() + B1*compSph.getSphereY() + C1*compSph.getSphereZ() + D1 ) / numerator ;
if (debug) fprintf(f,  "The distance b/t cuboid %d and sphere %d on TOPPLANE is %f.\n", compCub.getElementID(), compSph.getElementID(), topD);
                topSide = topD;
                if (topD < 0)   topD = (float)fabs( (double)topD );

                if (debug) fprintf(f,  "A1: %f, B1: %f, C1: %f, D1: %f.\n", A1, B1, C1, D1);
                if (debug) fprintf(f, "%f, %f.\n", A1*compSph.getSphereX() + B1*compSph.getSphereY() + C1*compSph.getSphereZ() + D1,   sqrt(A1*A1 + B1*B1 + C1*C1) );

                if ( topD <= compSph.getSphereRad() )   //if sphere is touch plane
                {

                        //find out the point where the sphere is touching plane
                        GLfloat pointX, pointY, pointZ;
                        /*Formula:
                        * Point of intersection on the plane =
                        * sphere midpoint(x0, y0, z0) - (A*x0+B*y0+Cz0+D)/(A^2+B^2+C^2)*(A, B, C)
                        * (A,B,C) is the normal of the plane
                        */
                        GLfloat sPI;
                        sPI = (A1*compSph.getSphereX()+B1*compSph.getSphereY()+C1*compSph.getSphereZ()+D1)/(A1*A1+B1*B1+C1*C1);
                        pointX = compSph.getSphereX() - sPI*A1;
                        pointY = compSph.getSphereY() - sPI*B1;
                        pointZ = compSph.getSphereZ() - sPI*C1;
                        if (debug) fprintf(f, "The contact point is: %f, %f, %f.\n", pointX, pointY, pointZ);

                        //deteremine if the contact point is within the rectangel, i.e. cuboid face
                        GLfloat Ver1X, Ver1Y, Ver1Z;            //LTF
                        GLfloat Ver2X, Ver2Y, Ver2Z;            //RTB

                        GLfloat line1x, line1y, line1z;         //origin is Ver1
                        GLfloat line2x, line2y, line2z;         //origin is Ver1
                        GLfloat line3x, line3y, line3z;         //origin is Ver2
                        GLfloat line4x, line4y, line4z;         //origin is Ver2
                        //vector Ver1 to
                        line1x = compCub.getLTBX() - compCub.getLTFX();
                        line1y = compCub.getLTBY() - compCub.getLTFY();
                        line1z = compCub.getLTBZ() - compCub.getLTFZ();

                        line2x = compCub.getRTFX() - compCub.getLTFX();
                        line2y = compCub.getRTFY() - compCub.getLTFY();
                        line2z = compCub.getRTFZ() - compCub.getLTFZ();

                        line3x = compCub.getLTBX() - compCub.getRTBX();
                        line3y = compCub.getLTBY() - compCub.getRTBY();
                        line3z = compCub.getLTBZ() - compCub.getRTBZ();

                        line4x = compCub.getRTFX() - compCub.getRTBX();
                        line4y = compCub.getRTFY() - compCub.getRTBY();
                        line4z = compCub.getRTFZ() - compCub.getRTBZ();
                        //vercotr from the contact point to the two choose vertax
                        GLfloat P1x, P1y, P1z;
                        GLfloat P2x, P2y, P2z;

                        P1x = pointX - compCub.getLTFX();
                        P1y = pointY - compCub.getLTFY();
                        P1z = pointZ - compCub.getLTFZ();

                        P2x = pointX - compCub.getRTBX();
                        P2y = pointY - compCub.getRTBY();
                        P2z = pointZ - compCub.getRTBZ();

                        GLfloat dot1, dot2, dot3, dot4;
                        dot1 = P1x*line1x + P1y*line1y + P1z*line1z;
                        dot2 = P1x*line2x + P1y*line2y + P1z*line2z;
                        dot3 = P2x*line3x + P2y*line3y + P2z*line3z;
                        dot4 = P2x*line4x + P2y*line4y + P2z*line4z;
                        if (debug) fprintf(f, "line1: %f, %f, %f.\n", line1x, line1y, line1z);
                        if (debug) fprintf(f, "line2: %f, %f, %f.\n", line2x, line2y, line2z);
                        if (debug) fprintf(f, "line3: %f, %f, %f.\n", line3x, line3y, line3z);
                        if (debug) fprintf(f, "line4: %f, %f, %f.\n", line4x, line4y, line4z);
                        if (debug) fprintf(f, "P1: %f, %f, %f.\n", P1x, P1y, P1z);
                        if (debug) fprintf(f, "P2: %f, %f, %f.\n", P2x, P2y, P2z);
                        if (debug) fprintf(f, "dot1: %f.\n", dot1);
                        if (debug) fprintf(f, "dot2: %f.\n", dot2);
                        if (debug) fprintf(f, "dot3: %f.\n", dot3);
                        if (debug) fprintf(f, "dot4: %f.\n", dot4);
                        //4 dot products have to be nonegavtive to show the contact point is within the rectangle.
                        if (dot1 >= 0 && dot2 >= 0 && dot3 >= 0 && dot4 >= 0)
                        {
                        touchTop = true;
                        if(debug) fprintf(f,"The touchTop is true.\n");
                        }
                        else
                        {
                        touchTop = false;
                        if(debug) fprintf(f,"The touchTop is false.\n");
                        }
                }
                else
                {
                        touchTop = false;
                        if(debug) fprintf(f,"The touchTop is false.\n");
                }

/************************bottom plane*****************************************/
                GLfloat bottomX1, bottomY1, bottomZ1;    //Left, bottom, Front
                GLfloat bottomX2, bottomY2, bottomZ2;    //Left, bottom, Back
                GLfloat bottomX3, bottomY3, bottomZ3;    //Right, bottom, Back
                bottomX1 = compCub.getLBFX();
                bottomY1 = compCub.getLBFY();
                bottomZ1 = compCub.getLBFZ();
                bottomX2 = compCub.getLBBX();
                bottomY2 = compCub.getLBBY();
                bottomZ2 = compCub.getLBBZ();
                bottomX3 = compCub.getRBFX();
                bottomY3 = compCub.getRBFY();
                bottomZ3 = compCub.getRBFZ();
                if (debug) fprintf(f, "bottom plane, LBF is %f, %f, %f, LBB is %f, %f, %f, and RBB is %f, %f, %f.\n", bottomX1, bottomY1, bottomZ1, bottomX2, bottomY2, bottomZ2, bottomX3, bottomY3, bottomZ3);
                //plane equation
                A2 = bottomY1 * (bottomZ2 - bottomZ3) + bottomY2 * (bottomZ3 - bottomZ1) + bottomY3 * (bottomZ1 - bottomZ2);
                B2 = bottomZ1 * (bottomX2 - bottomX3) + bottomZ2 * (bottomX3 - bottomX1) + bottomZ3 * (bottomX1 - bottomX2);
                C2 = bottomX1 * (bottomY2 - bottomY3) + bottomX2 * (bottomY3 - bottomY1) + bottomX3 * (bottomY1 - bottomY2);
                D2 = (-1)*(bottomX1)*(bottomY2*bottomZ3 - bottomY3*bottomZ2) + (-1)*(bottomX2)*(bottomY3*bottomZ1-bottomY1*bottomZ3) + (-1)*(bottomX3)*(bottomY1*bottomZ2-bottomY2*bottomZ1);
                //distance from sphere to top plane
                float bottomD;
                numerator =  sqrt(A2*A2 + B2*B2 + C2*C2) ;
                if (numerator == 0)
                {
                numerator += 0.000001;
                }
                bottomD = ( A2*compSph.getSphereX() + B2*compSph.getSphereY() + C2*compSph.getSphereZ() + D2 ) /numerator;
if (debug) fprintf(f,  "The distance b/t cuboid %d and sphere %d on BottomPlane is %f.\n", compCub.getElementID(), compSph.getElementID(), bottomD);
                bottomSide = bottomD;
                //get the absolute value of the bottomDistance
                if (bottomD < 0)   bottomD = (float)fabs((double)bottomD);
                if (debug) fprintf(f, "bottomDistance is %f and sphereRad is %f.\n", bottomD, compSph.getSphereRad());
                //if the distance b/t the sphere and the bottom plane is shorter than the sphere radius
                if ( bottomD <= compSph.getSphereRad() )
                {
                        //find out the point where the sphere is touching plane
                        GLfloat pointX, pointY, pointZ;
                        /*Formula:
                        * Point of intersection on the plane =
                        * sphere midpoint(x0, y0, z0) - (A*x0+B*y0+Cz0+D)/(A^2+B^2+C^2)*(A, B, C)
                        * (A,B,C) is the normal of the plane
                        */
                        GLfloat sPI;
                        sPI = (A2*compSph.getSphereX()+B2*compSph.getSphereY()+C2*compSph.getSphereZ()+D2)/(A2*A2+B2*B2+C2*C2);
                        pointX = compSph.getSphereX() - sPI*A2;
                        pointY = compSph.getSphereY() - sPI*B2;
                        pointZ = compSph.getSphereZ() - sPI*C2;
                        if (debug) fprintf(f, "The contact point is: %f, %f, %f.\n", pointX, pointY, pointZ);
                        //deteremine if the contact point is within the rectangel, i.e. cuboid face
                        GLfloat Ver1X, Ver1Y, Ver1Z;            //LBF
                        GLfloat Ver2X, Ver2Y, Ver2Z;            //RBB
                        GLfloat line1x, line1y, line1z;         //origin is Ver1
                        GLfloat line2x, line2y, line2z;         //origin is Ver1
                        GLfloat line3x, line3y, line3z;         //origin is Ver2
                        GLfloat line4x, line4y, line4z;         //origin is Ver2
                        //vector Ver1 to
                        line1x = compCub.getLBBX() - compCub.getLBFX();
                        line1y = compCub.getLBBY() - compCub.getLBFY();
                        line1z = compCub.getLBBZ() - compCub.getLBFZ();

                        line2x = compCub.getRBFX() - compCub.getLBFX();
                        line2y = compCub.getRBFY() - compCub.getLBFY();
                        line2z = compCub.getRBFZ() - compCub.getLBFZ();

                        line3x = compCub.getLBBX() - compCub.getRBBX();
                        line3y = compCub.getLBBY() - compCub.getRBBY();
                        line3z = compCub.getLBBZ() - compCub.getRBBZ();

                        line4x = compCub.getRBFX() - compCub.getRBBX();
                        line4y = compCub.getRBFY() - compCub.getRBBY();
                        line4z = compCub.getRBFZ() - compCub.getRTBZ();
                        //vercotr from the contact point to the two choose vertax
                        GLfloat P1x, P1y, P1z;
                        GLfloat P2x, P2y, P2z;

                        P1x = pointX - compCub.getLBFX();
                        P1y = pointY - compCub.getLBFY();
                        P1z = pointZ - compCub.getLBFZ();

                        P2x = pointX - compCub.getRBBX();
                        P2y = pointY - compCub.getRBBY();
                        P2z = pointZ - compCub.getRBBZ();

                        GLfloat dot1, dot2, dot3, dot4;
                        dot1 = P1x*line1x + P1y*line1y + P1z*line1z;
                        dot2 = P1x*line2x + P1y*line2y + P1z*line2z;
                        dot3 = P2x*line3x + P2y*line3y + P2z*line3z;
                        dot4 = P2x*line4x + P2y*line4y + P2z*line4z;
if (debug) fprintf(f, "line1: %f, %f, %f.\n", line1x, line1y, line1z);
if (debug) fprintf(f, "line2: %f, %f, %f.\n", line2x, line2y, line2z);
if (debug) fprintf(f, "line3: %f, %f, %f.\n", line3x, line3y, line3z);
if (debug) fprintf(f, "line4: %f, %f, %f.\n", line4x, line4y, line4z);
if (debug) fprintf(f, "P1: %f, %f, %f.\n", P1x, P1y, P1z);
if (debug) fprintf(f, "P2: %f, %f, %f.\n", P2x, P2y, P2z);
if (debug) fprintf(f, "dot1: %f.\n", dot1);
if (debug) fprintf(f, "dot2: %f.\n", dot2);
if (debug) fprintf(f, "dot3: %f.\n", dot3);
if (debug) fprintf(f, "dot4: %f.\n", dot4);
                        //4 dot products have to be nonegavtive to show the contact point is within the rectangle.
                        if (dot1 >= 0 && dot2 >= 0 && dot3 >= 0 && dot4 >= 0)
                        {
                        touchBottom = true;
                        if(debug) fprintf(f,"The touchBottom is true.\n");
                        }
                        else
                        {
                        touchBottom = false;
                        if(debug) fprintf(f,"The touchBottom is false.\n");
                        }
                }
                else
                {
                touchBottom = false;
                if(debug) fprintf(f,"The touchBottom is false.\n");
                }

/**********************************LEFT***************************************/
                GLfloat leftX1, leftY1, leftZ1;    //Left, bottom, Front
                GLfloat leftX2, leftY2, leftZ2;    //Left, bottom, Back
                GLfloat leftX3, leftY3, leftZ3;    //Left, top, Back
                leftX1 = compCub.getLBFX();
                leftY1 = compCub.getLBFY();
                leftZ1 = compCub.getLBFZ();
                leftX2 = compCub.getLBBX();
                leftY2 = compCub.getLBBY();
                leftZ2 = compCub.getLBBZ();
                leftX3 = compCub.getLTBX();
                leftY3 = compCub.getLTBY();
                leftZ3 = compCub.getLTBZ();
                if (debug) fprintf(f, "top plane, LBF is %f, %f, %f, LBB is %f, %f, %f, and LTB is %f, %f, %f.\n", leftX1, leftY1, leftZ1, leftX2, leftY2, leftZ2,leftX3, leftY3, leftZ3);
                //plane equation
                A3 = leftY1 * (leftZ2 - leftZ3) + leftY2 * (leftZ3 - leftZ1) + leftY3 * (leftZ1 - leftZ2);
                B3 = leftZ1 * (leftX2 - leftX3) + leftZ2 * (leftX3 - leftX1) + leftZ3 * (leftX1 - leftX2);
                C3 = leftX1 * (leftY2 - leftY3) + leftX2 * (leftY3 - leftY1) + leftX3 * (leftY1 - leftY2);
                D3 = (-1)*(leftX1)*(leftY2*leftZ3 - leftY3*leftZ2) + (-1)*(leftX2)*(leftY3*leftZ1-leftY1*leftZ3) + (-1)*(leftX3)*(leftY1*leftZ2-leftY2*leftZ1);
                //distance from sphere to top plane
                float leftD;
                numerator =  sqrt(A3*A3 + B3*B3 + C3*C3) ;
                if (numerator == 0)
                {
                numerator += 0.000001;
                }
                leftD = ( A3*compSph.getSphereX() + B3*compSph.getSphereY() + C3*compSph.getSphereZ() + D3 ) / numerator ;
if (debug) fprintf(f,  "The distance b/t cuboid %d and sphere %d on LEFTPLANE is %f.\n", compCub.getElementID(), compSph.getElementID(), leftD);

                leftSide = leftD;
                //get the absolute value of the bottomDistance
                if (leftD < 0)   leftD = (float)fabs((double)leftD);
                if (debug) fprintf(f, "leftD is %f and sphereRad is %f.\n", leftD, compSph.getSphereRad());
                //if the distance b/t the sphere and the bottom plane is shorter than the sphere radius
                if ( leftD <= compSph.getSphereRad() )
                {
                        //find out the point where the sphere is touching plane
                        GLfloat pointX, pointY, pointZ;
                        /*Formula:
                        * Point of intersection on the plane =
                        * sphere midpoint(x0, y0, z0) - (A*x0+B*y0+Cz0+D)/(A^2+B^2+C^2)*(A, B, C)
                        * (A,B,C) is the normal of the plane
                        */
                        GLfloat sPI;
                        sPI = (A3*compSph.getSphereX()+B3*compSph.getSphereY()+C3*compSph.getSphereZ()+D3)/(A3*A3+B3*B3+C3*C3);
                        pointX = compSph.getSphereX() - sPI*A3;
                        pointY = compSph.getSphereY() - sPI*B3;
                        pointZ = compSph.getSphereZ() - sPI*C3;
                        if (debug) fprintf(f, "The contact point is: %f, %f, %f.\n", pointX, pointY, pointZ);
                        //deteremine if the contact point is within the rectangel, i.e. cuboid face
                        GLfloat Ver1X, Ver1Y, Ver1Z;            //LBF
                        GLfloat Ver2X, Ver2Y, Ver2Z;            //LTB
                        GLfloat line1x, line1y, line1z;         //origin is Ver1
                        GLfloat line2x, line2y, line2z;         //origin is Ver1
                        GLfloat line3x, line3y, line3z;         //origin is Ver2
                        GLfloat line4x, line4y, line4z;         //origin is Ver2
                        //vector Ver1 to
                        line1x = compCub.getLBBX() - compCub.getLBFX();
                        line1y = compCub.getLBBY() - compCub.getLBFY();
                        line1z = compCub.getLBBZ() - compCub.getLBFZ();

                        line2x = compCub.getLTFX() - compCub.getLBFX();
                        line2y = compCub.getLTFY() - compCub.getLBFY();
                        line2z = compCub.getLTFZ() - compCub.getLBFZ();

                        line3x = compCub.getLBBX() - compCub.getLTBX();
                        line3y = compCub.getLBBY() - compCub.getLTBY();
                        line3z = compCub.getLBBZ() - compCub.getLTBZ();

                        line4x = compCub.getLTFX() - compCub.getLTBX();
                        line4y = compCub.getLTFY() - compCub.getLTBY();
                        line4z = compCub.getLTFZ() - compCub.getLTBZ();
                        //vercotr from the contact point to the two choose vertax
                        GLfloat P1x, P1y, P1z;
                        GLfloat P2x, P2y, P2z;

                        P1x = pointX - compCub.getLBFX();
                        P1y = pointY - compCub.getLBFY();
                        P1z = pointZ - compCub.getLBFZ();

                        P2x = pointX - compCub.getLTBX();
                        P2y = pointY - compCub.getLTBY();
                        P2z = pointZ - compCub.getLTBZ();

                        GLfloat dot1, dot2, dot3, dot4;
                        dot1 = P1x*line1x + P1y*line1y + P1z*line1z;
                        dot2 = P1x*line2x + P1y*line2y + P1z*line2z;
                        dot3 = P2x*line3x + P2y*line3y + P2z*line3z;
                        dot4 = P2x*line4x + P2y*line4y + P2z*line4z;
if (debug) fprintf(f, "line1: %f, %f, %f.\n", line1x, line1y, line1z);
if (debug) fprintf(f, "line2: %f, %f, %f.\n", line2x, line2y, line2z);
if (debug) fprintf(f, "line3: %f, %f, %f.\n", line3x, line3y, line3z);
if (debug) fprintf(f, "line4: %f, %f, %f.\n", line4x, line4y, line4z);
if (debug) fprintf(f, "P1: %f, %f, %f.\n", P1x, P1y, P1z);
if (debug) fprintf(f, "P2: %f, %f, %f.\n", P2x, P2y, P2z);
if (debug) fprintf(f, "dot1: %f.\n", dot1);
if (debug) fprintf(f, "dot2: %f.\n", dot2);
if (debug) fprintf(f, "dot3: %f.\n", dot3);
if (debug) fprintf(f, "dot4: %f.\n", dot4);
                        //4 dot products have to be nonegavtive to show the contact point is within the rectangle.
                        if (dot1 >= 0 && dot2 >= 0 && dot3 >= 0 && dot4 >= 0)
                        {
                        touchLeft = true;
                        if(debug) fprintf(f,"The touchLeft is true.\n");
                        }
                        else
                        {
                        touchLeft = false;
                        if(debug) fprintf(f,"The touchLeft is false.\n");
                        }
                }
                else
                {
                touchLeft = false;
                if(debug) fprintf(f,"The touchLeft is false.\n");
                }
/***************************RIGHT PLANE****************************************/
                GLfloat rightX1, rightY1, rightZ1;    //Right, bottom, Front
                GLfloat rightX2, rightY2, rightZ2;    //Right, bottom, Back
                GLfloat rightX3, rightY3, rightZ3;    //Right, top, Back
                rightX1 = compCub.getRBFX();
                rightY1 = compCub.getRBFY();
                rightZ1 = compCub.getRBFZ();
                rightX2 = compCub.getRBBX();
                rightY2 = compCub.getRBBY();
                rightZ2 = compCub.getRBBZ();
                rightX3 = compCub.getRTBX();
                rightY3 = compCub.getRTBY();
                rightZ3 = compCub.getRTBZ();
                if (debug) fprintf(f, "top plane, RBF is %f, %f, %f, RBB is %f, %f, %f, and RTB is %f, %f, %f.\n", rightX1, rightY1, rightZ1, rightX2, rightY2, rightZ2, rightX3, rightY3, rightZ3);
                //plane equation
                A4 = rightY1 * (rightZ2 - rightZ3) + rightY2 * (rightZ3 - rightZ1) + rightY3 * (rightZ1 - rightZ2);
                B4 = rightZ1 * (rightX2 - rightX3) + rightZ2 * (rightX3 - rightX1) + rightZ3 * (rightX1 - rightX2);
                C4 = rightX1 * (rightY2 - rightY3) + rightX2 * (rightY3 - rightY1) + rightX3 * (rightY1 - rightY2);
                D4 = (-1)*(rightX1)*(rightY2*rightZ3 - rightY3*rightZ2) + (-1)*(rightX2)*(rightY3*rightZ1-rightY1*rightZ3) + (-1)*(rightX3)*(rightY1*rightZ2-rightY2*rightZ1);
                //distance from sphere to top plane
                float rightD;
                numerator =  sqrt(A4*A4 + B4*B4 + C4*C4) ;
                if (numerator == 0)
                {
                numerator += 0.000001;
                }
                rightD = ( A4*compSph.getSphereX() + B4*compSph.getSphereY() + C4*compSph.getSphereZ() + D4 ) / numerator;
if (debug) fprintf(f,  "The distance b/t cuboid %d and sphere %d on RIGHTPLANE is %f.\n", compCub.getElementID(), compSph.getElementID(), rightD);

                rightSide = rightD;
                //get the absolute value of the bottomDistance
                if (rightD < 0)   rightD = (float)fabs((double)rightD);
                if (debug) fprintf(f, "rightD is %f and sphereRad is %f.\n", rightD, compSph.getSphereRad());
                //if the distance b/t the sphere and the bottom plane is shorter than the sphere radius
                if ( rightD <= compSph.getSphereRad() )
                {
                        //find out the point where the sphere is touching plane
                        GLfloat pointX, pointY, pointZ;
                        /*Formula:
                        * Point of intersection on the plane =
                        * sphere midpoint(x0, y0, z0) - (A*x0+B*y0+Cz0+D)/(A^2+B^2+C^2)*(A, B, C)
                        * (A,B,C) is the normal of the plane
                        */
                        GLfloat sPI;
                        sPI = (A4*compSph.getSphereX()+B4*compSph.getSphereY()+C4*compSph.getSphereZ()+D4)/(A4*A4+B4*B4+C4*C4);
                        pointX = compSph.getSphereX() - sPI*A4;
                        pointY = compSph.getSphereY() - sPI*B4;
                        pointZ = compSph.getSphereZ() - sPI*C4;
                        if (debug) fprintf(f, "The contact point is: %f, %f, %f.\n", pointX, pointY, pointZ);
                        //deteremine if the contact point is within the rectangel, i.e. cuboid face
                        GLfloat Ver1X, Ver1Y, Ver1Z;            //RBF
                        GLfloat Ver2X, Ver2Y, Ver2Z;            //RTB
                        GLfloat line1x, line1y, line1z;         //origin is Ver1
                        GLfloat line2x, line2y, line2z;         //origin is Ver1
                        GLfloat line3x, line3y, line3z;         //origin is Ver2
                        GLfloat line4x, line4y, line4z;         //origin is Ver2
                        //vector Ver1 to
                        line1x = compCub.getRBBX() - compCub.getRBFX();
                        line1y = compCub.getRBBY() - compCub.getRBFY();
                        line1z = compCub.getRBBZ() - compCub.getRBFZ();

                        line2x = compCub.getRTFX() - compCub.getRBFX();
                        line2y = compCub.getRTFY() - compCub.getRBFY();
                        line2z = compCub.getRTFZ() - compCub.getRBFZ();

                        line3x = compCub.getRBBX() - compCub.getRTBX();
                        line3y = compCub.getRBBY() - compCub.getRTBY();
                        line3z = compCub.getRBBZ() - compCub.getRTBZ();

                        line4x = compCub.getRTFX() - compCub.getRTBX();
                        line4y = compCub.getRTFY() - compCub.getRTBY();
                        line4z = compCub.getRTFZ() - compCub.getRTBZ();
                        //vercotr from the contact point to the two choose vertax
                        GLfloat P1x, P1y, P1z;
                        GLfloat P2x, P2y, P2z;

                        P1x = pointX - compCub.getRBFX();
                        P1y = pointY - compCub.getRBFY();
                        P1z = pointZ - compCub.getRBFZ();

                        P2x = pointX - compCub.getRTBX();
                        P2y = pointY - compCub.getRTBY();
                        P2z = pointZ - compCub.getRTBZ();

                        GLfloat dot1, dot2, dot3, dot4;
                        dot1 = P1x*line1x + P1y*line1y + P1z*line1z;
                        dot2 = P1x*line2x + P1y*line2y + P1z*line2z;
                        dot3 = P2x*line3x + P2y*line3y + P2z*line3z;
                        dot4 = P2x*line4x + P2y*line4y + P2z*line4z;
if (debug) fprintf(f, "line1: %f, %f, %f.\n", line1x, line1y, line1z);
if (debug) fprintf(f, "line2: %f, %f, %f.\n", line2x, line2y, line2z);
if (debug) fprintf(f, "line3: %f, %f, %f.\n", line3x, line3y, line3z);
if (debug) fprintf(f, "line4: %f, %f, %f.\n", line4x, line4y, line4z);
if (debug) fprintf(f, "P1: %f, %f, %f.\n", P1x, P1y, P1z);
if (debug) fprintf(f, "P2: %f, %f, %f.\n", P2x, P2y, P2z);
if (debug) fprintf(f, "dot1: %f.\n", dot1);
if (debug) fprintf(f, "dot2: %f.\n", dot2);
if (debug) fprintf(f, "dot3: %f.\n", dot3);
if (debug) fprintf(f, "dot4: %f.\n", dot4);
                        //4 dot products have to be nonegavtive to show the contact point is within the rectangle.
                        if (dot1 >= 0 && dot2 >= 0 && dot3 >= 0 && dot4 >= 0)
                        {
                        touchRight = true;
                        if(debug) fprintf(f,"The touchRight is true.\n");
                        }
                        else
                        {
                        touchRight = false;
                        if(debug) fprintf(f,"The touchRight is false.\n");
                        }
                }
                else
                {
                touchRight = false;
                if(debug) fprintf(f,"The touchRight is false.\n");
                }
/***************************FRONT PLANE****************************************/
                GLfloat frontX1, frontY1, frontZ1;    //Left, bottom, Front
                GLfloat frontX2, frontY2, frontZ2;    //Right, bottom, Front
                GLfloat frontX3, frontY3, frontZ3;    //Right, top, Front
                frontX1 = compCub.getLBFX();
                frontY1 = compCub.getLBFY();
                frontZ1 = compCub.getLBFZ();
                frontX2 = compCub.getRBFX();
                frontY2 = compCub.getRBFY();
                frontZ2 = compCub.getRBFZ();
                frontX3 = compCub.getRTFX();
                frontY3 = compCub.getRTFY();
                frontZ3 = compCub.getRTFZ();
                if (debug) fprintf(f, "Front plane, LBF is %f, %f, %f, RBF is %f, %f, %f, and RTF is %f, %f, %f.\n", frontX1, frontY1, frontZ1, frontX2, frontY2, frontZ2, frontX3, frontY3,frontZ3);
                //plane equation
                A5 = frontY1 * (frontZ2 - frontZ3) + frontY2 * (frontZ3 - frontZ1) + frontY3 * (frontZ1 - frontZ2);
                B5 = frontZ1 * (frontX2 - frontX3) + frontZ2 * (frontX3 - frontX1) + frontZ3 * (frontX1 - frontX2);
                C5 = frontX1 * (frontY2 - frontY3) + frontX2 * (frontY3 - frontY1) + frontX3 * (frontY1 - frontY2);
                D5 = (-1)*(frontX1)*(frontY2*frontZ3 - frontY3*frontZ2) + (-1)*(frontX2)*(frontY3*frontZ1-frontY1*frontZ3) + (-1)*(frontX3)*(frontY1*frontZ2-frontY2*frontZ1);
                //distance from sphere to top plane
                float frontD;
                numerator = sqrt(A5*A5 + B5*B5 + C5*C5);
                if (numerator == 0)
                {
                numerator+= 0.000001;
                }
                frontD = ( A5*compSph.getSphereX() + B5*compSph.getSphereY() + C5*compSph.getSphereZ() + D5 ) / numerator ;
if (debug) fprintf(f,  "The distance b/t cuboid %d and sphere %d on FRONTPLANE is %f.\n", compCub.getElementID(), compSph.getElementID(), frontD);

                frontSide = frontD;
                //get the absolute value of the bottomDistance
                if (frontD < 0)   frontD = (float)fabs((double)frontD);
                if (debug) fprintf(f, "frontD is %f and sphereRad is %f.\n", frontD, compSph.getSphereRad());
                //if the distance b/t the sphere and the bottom plane is shorter than the sphere radius
                if ( frontD <= compSph.getSphereRad() )
                {
                        //find out the point where the sphere is touching plane
                        GLfloat pointX, pointY, pointZ;
                        /*Formula:
                        * Point of intersection on the plane =
                        * sphere midpoint(x0, y0, z0) - (A*x0+B*y0+Cz0+D)/(A^2+B^2+C^2)*(A, B, C)
                        * (A,B,C) is the normal of the plane
                        */
                        GLfloat sPI;
                        sPI = (A5*compSph.getSphereX()+B5*compSph.getSphereY()+C5*compSph.getSphereZ()+D5)/(A5*A5+B5*B5+C5*C5);
                        pointX = compSph.getSphereX() - sPI*A5;
                        pointY = compSph.getSphereY() - sPI*B5;
                        pointZ = compSph.getSphereZ() - sPI*C5;
                        if (debug) fprintf(f, "The contact point is: %f, %f, %f.\n", pointX, pointY, pointZ);
                        //deteremine if the contact point is within the rectangel, i.e. cuboid face
                        GLfloat Ver1X, Ver1Y, Ver1Z;            //LBF
                        GLfloat Ver2X, Ver2Y, Ver2Z;            //RTF
                        GLfloat line1x, line1y, line1z;         //origin is Ver1
                        GLfloat line2x, line2y, line2z;         //origin is Ver1
                        GLfloat line3x, line3y, line3z;         //origin is Ver2
                        GLfloat line4x, line4y, line4z;         //origin is Ver2
                        //vector Ver1 to
                        line1x = compCub.getRBFX() - compCub.getLBFX();
                        line1y = compCub.getRBFY() - compCub.getLBFY();
                        line1z = compCub.getRBFZ() - compCub.getLBFZ();

                        line2x = compCub.getLTFX() - compCub.getLBFX();
                        line2y = compCub.getLTFY() - compCub.getLBFY();
                        line2z = compCub.getLTFZ() - compCub.getLBFZ();

                        line3x = compCub.getRBFX() - compCub.getRTFX();
                        line3y = compCub.getRBFY() - compCub.getRTFY();
                        line3z = compCub.getRBFZ() - compCub.getRTFZ();

                        line4x = compCub.getLTFX() - compCub.getRTFX();
                        line4y = compCub.getLTFY() - compCub.getRTFY();
                        line4z = compCub.getLTFZ() - compCub.getRTFZ();
                        //vercotr from the contact point to the two choose vertax
                        GLfloat P1x, P1y, P1z;
                        GLfloat P2x, P2y, P2z;

                        P1x = pointX - compCub.getLBFX();
                        P1y = pointY - compCub.getLBFY();
                        P1z = pointZ - compCub.getLBFZ();

                        P2x = pointX - compCub.getRTFX();
                        P2y = pointY - compCub.getRTFY();
                        P2z = pointZ - compCub.getRTFZ();

                        GLfloat dot1, dot2, dot3, dot4;
                        dot1 = P1x*line1x + P1y*line1y + P1z*line1z;
                        dot2 = P1x*line2x + P1y*line2y + P1z*line2z;
                        dot3 = P2x*line3x + P2y*line3y + P2z*line3z;
                        dot4 = P2x*line4x + P2y*line4y + P2z*line4z;
if (debug) fprintf(f, "line1: %f, %f, %f.\n", line1x, line1y, line1z);
if (debug) fprintf(f, "line2: %f, %f, %f.\n", line2x, line2y, line2z);
if (debug) fprintf(f, "line3: %f, %f, %f.\n", line3x, line3y, line3z);
if (debug) fprintf(f, "line4: %f, %f, %f.\n", line4x, line4y, line4z);
if (debug) fprintf(f, "P1: %f, %f, %f.\n", P1x, P1y, P1z);
if (debug) fprintf(f, "P2: %f, %f, %f.\n", P2x, P2y, P2z);
if (debug) fprintf(f, "dot1: %f.\n", dot1);
if (debug) fprintf(f, "dot2: %f.\n", dot2);
if (debug) fprintf(f, "dot3: %f.\n", dot3);
if (debug) fprintf(f, "dot4: %f.\n", dot4);
                        //4 dot products have to be nonegavtive to show the contact point is within the rectangle.
                        if (dot1 >= 0 && dot2 >= 0 && dot3 >= 0 && dot4 >= 0)
                        {
                        touchFront = true;
                        if(debug) fprintf(f,"The touchFront is true.\n");
                        }
                        else
                        {
                        touchFront = false;
                        if(debug) fprintf(f,"The touchFront is false.\n");
                        }
                }
                else
                {
                touchFront = false;
                if(debug) fprintf(f,"The touchFront is false.\n");
                }

/***************************BACK PLANE****************************************/
                GLfloat backX1, backY1, backZ1;    //Left, bottom, Back
                GLfloat backX2, backY2, backZ2;    //Right, bottom, Back
                GLfloat backX3, backY3, backZ3;    //Right, top, Back
                backX1 = compCub.getLBBX();
                backY1 = compCub.getLBBY();
                backZ1 = compCub.getLBBZ();
                backX2 = compCub.getRBBX();
                backY2 = compCub.getRBBY();
                backZ2 = compCub.getRBBZ();
                backX3 = compCub.getRTBX();
                backY3 = compCub.getRTBY();
                backZ3 = compCub.getRTBZ();
                if (debug) fprintf(f, "Back plane, LBB is %f, %f, %f, RBB is %f, %f, %f, and RTB is %f, %f, %f.\n", backX1, backY1, backZ1, backX2, backY2, backZ2, backX3, backY3, backZ3);
                //plane equation
                A6 = backY1 * (backZ2 - backZ3) + backY2 * (backZ3 - backZ1) + backY3 * (backZ1 - backZ2);
                B6 = backZ1 * (backX2 - backX3) + backZ2 * (backX3 - backX1) + backZ3 * (backX1 - backX2);
                C6 = backX1 * (backY2 - backY3) + backX2 * (backY3 - backY1) + backX3 * (backY1 - backY2);
                D6 = (-1)*(backX1)*(backY2*backZ3 - backY3*backZ2) + (-1)*(backX2)*(backY3*backZ1-backY1*backZ3) + (-1)*(backX3)*(backY1*backZ2-backY2*backZ1);
                //distance from sphere to top plane
                float backD;
                numerator = sqrt(A6*A6 + B6*B6 + C6*C6);
                if (numerator == 0)
                {
                numerator+= 0.000001;
                }
                backD = ( A6*compSph.getSphereX() + B6*compSph.getSphereY() + C6*compSph.getSphereZ() + D6 ) / numerator;
if (debug) fprintf(f,  "The distance b/t cuboid %d and sphere %d on BACKPLANE is %f.\n", compCub.getElementID(), compSph.getElementID(), backD);

                backSide = backD;
                //get the absolute value of the bottomDistance
                if (backD < 0)   backD = (float)fabs((double)backD);
                if (debug) fprintf(f, "backD is %f and sphereRad is %f.\n", backD, compSph.getSphereRad());
                //if the distance b/t the sphere and the bottom plane is shorter than the sphere radius
                if ( backD <= compSph.getSphereRad() )
                {
                        //find out the point where the sphere is touching plane
                        GLfloat pointX, pointY, pointZ;
                        /*Formula:
                        * Point of intersection on the plane =
                        * sphere midpoint(x0, y0, z0) - (A*x0+B*y0+Cz0+D)/(A^2+B^2+C^2)*(A, B, C)
                        * (A,B,C) is the normal of the plane
                        */
                        GLfloat sPI;
                        sPI = (A6*compSph.getSphereX()+B6*compSph.getSphereY()+C6*compSph.getSphereZ()+D6)/(A6*A6+B6*B6+C6*C6);
                        pointX = compSph.getSphereX() - sPI*A6;
                        pointY = compSph.getSphereY() - sPI*B6;
                        pointZ = compSph.getSphereZ() - sPI*C6;
                        if (debug) fprintf(f, "The contact point is: %f, %f, %f.\n", pointX, pointY, pointZ);
                        //deteremine if the contact point is within the rectangel, i.e. cuboid face
                        GLfloat Ver1X, Ver1Y, Ver1Z;            //LBB
                        GLfloat Ver2X, Ver2Y, Ver2Z;            //RTB
                        GLfloat line1x, line1y, line1z;         //origin is Ver1
                        GLfloat line2x, line2y, line2z;         //origin is Ver1
                        GLfloat line3x, line3y, line3z;         //origin is Ver2
                        GLfloat line4x, line4y, line4z;         //origin is Ver2
                        //vector Ver1 to
                        line1x = compCub.getRBBX() - compCub.getLBBX();
                        line1y = compCub.getRBBY() - compCub.getLBBY();
                        line1z = compCub.getRBBZ() - compCub.getLBBZ();

                        line2x = compCub.getLTBX() - compCub.getLBBX();
                        line2y = compCub.getLTBY() - compCub.getLBBY();
                        line2z = compCub.getLTBZ() - compCub.getLBBZ();

                        line3x = compCub.getRBBX() - compCub.getRTBX();
                        line3y = compCub.getRBBY() - compCub.getRTBY();
                        line3z = compCub.getRBBZ() - compCub.getRTBZ();

                        line4x = compCub.getLTBX() - compCub.getRTBX();
                        line4y = compCub.getLTBY() - compCub.getRTBY();
                        line4z = compCub.getLTBZ() - compCub.getRTBZ();
                        //vercotr from the contact point to the two choose vertax
                        GLfloat P1x, P1y, P1z;
                        GLfloat P2x, P2y, P2z;

                        P1x = pointX - compCub.getLBBX();
                        P1y = pointY - compCub.getLBBY();
                        P1z = pointZ - compCub.getLBBZ();

                        P2x = pointX - compCub.getRTBX();
                        P2y = pointY - compCub.getRTBY();
                        P2z = pointZ - compCub.getRTBZ();

                        GLfloat dot1, dot2, dot3, dot4;
                        dot1 = P1x*line1x + P1y*line1y + P1z*line1z;
                        dot2 = P1x*line2x + P1y*line2y + P1z*line2z;
                        dot3 = P2x*line3x + P2y*line3y + P2z*line3z;
                        dot4 = P2x*line4x + P2y*line4y + P2z*line4z;
if (debug) fprintf(f, "line1: %f, %f, %f.\n", line1x, line1y, line1z);
if (debug) fprintf(f, "line2: %f, %f, %f.\n", line2x, line2y, line2z);
if (debug) fprintf(f, "line3: %f, %f, %f.\n", line3x, line3y, line3z);
if (debug) fprintf(f, "line4: %f, %f, %f.\n", line4x, line4y, line4z);
if (debug) fprintf(f, "P1: %f, %f, %f.\n", P1x, P1y, P1z);
if (debug) fprintf(f, "P2: %f, %f, %f.\n", P2x, P2y, P2z);
if (debug) fprintf(f, "dot1: %f.\n", dot1);
if (debug) fprintf(f, "dot2: %f.\n", dot2);
if (debug) fprintf(f, "dot3: %f.\n", dot3);
if (debug) fprintf(f, "dot4: %f.\n", dot4);
                        //4 dot products have to be nonegavtive to show the contact point is within the rectangle.
                        if (dot1 >= 0 && dot2 >= 0 && dot3 >= 0 && dot4 >= 0)
                        {
                        touchBack = true;
                        if(debug) fprintf(f,"The touchBack is true.\n");
                        }
                        else
                        {
                        touchBack = false;
                        if(debug) fprintf(f,"The touchBack is false.\n");
                        }
                }
                else
                {
                touchBack = false;
                if(debug) fprintf(f,"The touchBack is false.\n");
                }


/***********************CHECK INSIDE CUBOIDS***********************************/
                //check for 3 pairs of the oppostie planes see if the sphere midpoint is between the plane
                //insideCub

                //if sphere midpoint is between top and bottom plane
                if(   (topSide>=0 && bottomSide<0) || (topSide<0 && bottomSide>=0 ) )
                {
                        if(debug) fprintf(f, "b/t top and bottom plane.\n");
                        //if between left and right plane
                        if( (leftSide>=0 && rightSide<0) || (leftSide<0 && rightSide>=0) )
                        {
                                if(debug) fprintf(f, "b/t left and right plane.\n");
                                //if between front and back plane
                                if ( (frontSide>=0 && backSide<0) || (frontSide<0 && backSide>=0)   )
                                {
                                        if(debug) fprintf(f, "b/t front and back plane.\n");
                                        insideCub = true;
                                }
                        }
                }
/*******************ONE OBJECT IS SPHERE AND ANOTHER ONE IS BORDER*************/ 
                if( compCub.getPrimitiveID() == 3 || compCub.getElementID() == 0 )//if it is boundary
                {
                        if(debug) fprintf(f, "SPHERE AND BORDER COLLISOIN DETECTION.\n");
                        insideCub = false;
                }

                if (debug) fprintf(f, "topSide: %f, bottomSide: %f, leftSide: %f, rightSide: %f, frontSide: %f, backSide: %f.\n", topSide, bottomSide, leftSide, rightSide, frontSide, backSide);
                if(insideCub)
                {
                if(debug) fprintf(f,"The insideCub is true.\n");
                }
                else
                {
                if(debug) fprintf(f,"The insideCub is false.\n");
                }




                if (touchTop || touchBottom || touchLeft || touchRight || touchFront || touchBack || insideCub)
                {
                        return true;
                }
                else
                {
                        return false;
                }
        }//if one is sphere and other one is cuboids



/************************BOTH OBJECTS ARE CUBOIDS******************************/
        if (objType1 == 2 && objType2 == 2)
        {
        /*******************IF ONE OF THE CUBOID IS THE BORDER*****************/
                if(cuboidsVec[index1].getPrimitiveID()==3 || cuboidsVec[index2].getPrimitiveID()==3)
                {
                        if(debug) fprintf(f, "CUBOID AND BORDER COLLISOIN DETECTION.\n");
                        if(cuboidsVec[index1].getPrimitiveID()==3 )
                        {
                        return cuboidNborderCollide(objType1, index1, objType2, index2);
                        }
                        if(cuboidsVec[index2].getPrimitiveID()==3 )
                        {
                        return cuboidNborderCollide(objType2, index2, objType1, index1);
                        }
                }
        /******************BOTH CUBOIDS ARE REGULAR CUBOIDS********************/
                if(cuboidsVec[index1].getPrimitiveID()==2 || cuboidsVec[index2].getPrimitiveID()==2)
                {
                        if(debug) fprintf(f, "CUBOID AND CUBOID COLLISOIN DETECTION.\n");
                        cuboids box1 = cuboidsVec[index1];
                        cuboids box2 = cuboidsVec[index2];

                        /**************************15 NORMALS******************/
                        //Use N1~N3 to denote box1's normal, ues N4~N6 to denote box2's normal
                        //Use N7~15 to denote pair normals.
                        GLfloat N1X, N1Y, N1Z;
                        GLfloat N2X, N2Y, N2Z;
                        GLfloat N3X, N3Y, N3Z;
                        GLfloat N4X, N4Y, N4Z;
                        GLfloat N5X, N5Y, N5Z;
                        GLfloat N6X, N6Y, N6Z;
                        GLfloat N7X, N7Y, N7Z;
                        GLfloat N8X, N8Y, N8Z;
                        GLfloat N9X, N9Y, N9Z;
                        GLfloat N10X, N10Y, N10Z;
                        GLfloat N11X, N11Y, N11Z;
                        GLfloat N12X, N12Y, N12Z;
                        GLfloat N13X, N13Y, N13Z;
                        GLfloat N14X, N14Y, N14Z;
                        GLfloat N15X, N15Y, N15Z;
                        //Find the 15 normals from the two cuboids.
                        //3 normals are obtain from each cuboids' plane. 2*3=6
                        //9 are obtained by making pair of edge from 2 boxs, 3*3=9
                        //Use V to denote box2's vertex, W to denote box2's vertex
                        //Use E to denote box1's edges, F to denote box2's Edges
                        /**********BOX1*************/
                        GLfloat V1X = box1.getLTFX();   //Left, Top, Front
                        GLfloat V1Y = box1.getLTFY();
                        GLfloat V1Z = box1.getLTFZ();
                        GLfloat V2X = box1.getLBFX();   //Left, Bottom, Front
                        GLfloat V2Y = box1.getLBFY();
                        GLfloat V2Z = box1.getLBFZ();
                        GLfloat V3X = box1.getRTFX();   //Right, Top, Front
                        GLfloat V3Y = box1.getRTFY();
                        GLfloat V3Z = box1.getRTFZ();
                        GLfloat V4X = box1.getLTBX();   //Left, Top, Back
                        GLfloat V4Y = box1.getLTBY();
                        GLfloat V4Z = box1.getLTBZ();
                        /**********BOX2*************/
                        GLfloat W1X = box2.getLTFX();   //Left, Top, Front
                        GLfloat W1Y = box2.getLTFY();
                        GLfloat W1Z = box2.getLTFZ();
                        GLfloat W2X = box2.getLBFX();   //Left, Bottom, Front
                        GLfloat W2Y = box2.getLBFY();
                        GLfloat W2Z = box2.getLBFZ();
                        GLfloat W3X = box2.getRTFX();   //Right, Top, Front
                        GLfloat W3Y = box2.getRTFY();
                        GLfloat W3Z = box2.getRTFZ();
                        GLfloat W4X = box2.getLTBX();   //Left, Top, Back
                        GLfloat W4Y = box2.getLTBY();
                        GLfloat W4Z = box2.getLTBZ();
                        /**********BOX1 EDGES*******/
                        GLfloat E1X, E1Y, E1Z;
                        GLfloat E2X, E2Y, E2Z;
                        GLfloat E3X, E3Y, E3Z;
                        E1X = V2X - V1X;                //Left, Front edge
                        E1Y = V2Y - V1Y;
                        E1Z = V2Z - V1Z;
                        E2X = V3X - V1X;                //Front, Top edge
                        E2Y = V3Y - V1Y;
                        E2Z = V3Z - V1Z;
                        E3X = V4X - V1X;                //Left, Top edge
                        E3Y = V4Y - V1Y;
                        E3Z = V4Z - V1Z;
                        //if(debug) fprintf(f, "x1: %f, y1: %f, z1: %f, x2: %f, y2: %f, z2; %f.\n", E1X, E1Y, E1Z, E2X, E2Y, E2Z);
                        GLfloat *tempN;
                        tempN = CrossProduct(E1X, E1Y, E1Z, E2X, E2Y, E2Z);
                        N1X = tempN[0];
                        N1Y = tempN[1];
                        N1Z = tempN[2];
                        //if(debug) fprintf(f, "normal: %f, %f, %f.\n", N1X, N1Y, N1Z);
                        tempN = CrossProduct(E1X, E1Y, E1Z, E3X, E3Y, E3Z);
                        N2X = tempN[0];
                        N2Y = tempN[1];
                        N2Z = tempN[2];
                        tempN = CrossProduct(E2X, E2Y, E2Z, E3X, E3Y, E3Z);
                        N3X = tempN[0];
                        N3Y = tempN[1];
                        N3Z = tempN[2];
                        /**********BOX2 EDGES*******/
                        GLfloat F1X, F1Y, F1Z;
                        GLfloat F2X, F2Y, F2Z;
                        GLfloat F3X, F3Y, F3Z;
                        F1X = W2X - W1X;                //Left, Front edge
                        F1Y = W2Y - W1Y;
                        F1Z = W2Z - W1Z;
                        F2X = W3X - W1X;                //Front, Top edge
                        F2Y = W3Y - W1Y;
                        F2Z = W3Z - W1Z;
                        F3X = W4X - W1X;                //Left, Top edge
                        F3Y = W4Y - W1Y;
                        F3Z = W4Z - W1Z;
                        tempN = CrossProduct(F1X, F1Y, F1Z, F2X, F2Y, F2Z);
                        N4X = tempN[0];
                        N4Y = tempN[1];
                        N4Z = tempN[2];
                        //if(debug) fprintf(f, "normal: %f, %f, %f.\n", N1X, N1Y, N1Z);
                        tempN = CrossProduct(F1X, F1Y, F1Z, F3X, F3Y, F3Z);
                        N5X = tempN[0];
                        N5Y = tempN[1];
                        N5Z = tempN[2];
                        tempN = CrossProduct(F2X, F2Y, F2Z, F3X, F3Y, F3Z);
                        N6X = tempN[0];
                        N6Y = tempN[1];
                        N6Z = tempN[2];
if(debug) fprintf(f, "E1 is %f, %f, %f.\n", E1X, E1Y, E1Z);
if(debug) fprintf(f, "E2 is %f, %f, %f.\n", E2X, E2Y, E2Z);
if(debug) fprintf(f, "E3 is %f, %f, %f.\n", E3X, E3Y, E3Z);
if(debug) fprintf(f, "F1 is %f, %f, %f.\n", F1X, F1Y, F1Z);
if(debug) fprintf(f, "F2 is %f, %f, %f.\n", F2X, F2Y, F2Z);
if(debug) fprintf(f, "F3 is %f, %f, %f.\n", F3X, F3Y, F3Z);
                        /**********PAIR THE EDGES TO FORM 9 MORE NORMALS*******/
                        /*
                        E1XF1, E1XF2, E1XF3
                        E2XF1, E2XF2, E2XF3
                        E3XF1, E3XF2, E3XF3
                        */
                        tempN = CrossProduct(E1X, E1Y, E1Z, F1X, F1Y, F1Z);
                        N7X = tempN[0];
                        N7Y = tempN[1];
                        N7Z = tempN[2];
                        tempN = CrossProduct(E1X, E1Y, E1Z, F2X, F2Y, F2Z);
                        N8X = tempN[0];
                        N8Y = tempN[1];
                        N8Z = tempN[2];
                        tempN = CrossProduct(E1X, E1Y, E1Z, F3X, F3Y, F3Z);
                        N9X = tempN[0];
                        N9Y = tempN[1];
                        N9Z = tempN[2];
                        tempN = CrossProduct(E2X, E2Y, E2Z, F1X, F1Y, F1Z);
                        N10X = tempN[0];
                        N10Y = tempN[1];
                        N10Z = tempN[2];
                        tempN = CrossProduct(E2X, E2Y, E2Z, F2X, F2Y, F2Z);
                        N11X = tempN[0];
                        N11Y = tempN[1];
                        N11Z = tempN[2];
                        tempN = CrossProduct(E2X, E2Y, E2Z, F3X, F3Y, F3Z);
                        N12X = tempN[0];
                        N12Y = tempN[1];
                        N12Z = tempN[2];
                        tempN = CrossProduct(E3X, E3Y, E3Z, F1X, F1Y, F1Z);
                        N13X = tempN[0];
                        N13Y = tempN[1];
                        N13Z = tempN[2];
                        tempN = CrossProduct(E3X, E3Y, E3Z, F2X, F2Y, F2Z);
                        N14X = tempN[0];
                        N14Y = tempN[1];
                        N14Z = tempN[2];
                        tempN = CrossProduct(E3X, E3Y, E3Z, F3X, F3Y, F3Z);
                        N15X = tempN[0];
                        N15Y = tempN[1];
                        N15Z = tempN[2];
                        //make 3 array, each have array has x,  y, z values
                        //      1 for 15 normal
                        //      1 for 8 array of box 1
                        //      1 for 8 array of box 2
                        //make 4 more array for the min and max of the 2 box projections to the normal
                        GLfloat normArr[15][3];
                        GLfloat box1Ver[8][3];
                        GLfloat box2Ver[8][3];
                        GLfloat box1Min[15];
                        GLfloat box1Max[15];
                        GLfloat box2Min[15];
                        GLfloat box2Max[15];
                        bool collide = false;

                        //BOX 1 VERTEX
                        box1Ver[0][0] = box1.getLBFX(); //left, bottom, front
                        box1Ver[0][1] = box1.getLBFY();
                        box1Ver[0][2] = box1.getLBFZ();
                        box1Ver[1][0] = box1.getLTFX(); //left, top, front
                        box1Ver[1][1] = box1.getLTFY();
                        box1Ver[1][2] = box1.getLTFZ();
                        box1Ver[2][0] = box1.getLTBX(); //left, top, back
                        box1Ver[2][1] = box1.getLTBY();
                        box1Ver[2][2] = box1.getLTBZ();
                        box1Ver[3][0] = box1.getLBBX(); //left, bottom, back
                        box1Ver[3][1] = box1.getLBBY();
                        box1Ver[3][2] = box1.getLBBZ();
                        box1Ver[4][0] = box1.getRBFX(); //right, bottom, front
                        box1Ver[4][1] = box1.getRBFY();
                        box1Ver[4][2] = box1.getRBFZ();
                        box1Ver[5][0] = box1.getRTFX(); //right, top, front
                        box1Ver[5][1] = box1.getRTFY();
                        box1Ver[5][2] = box1.getRTFZ();
                        box1Ver[6][0] = box1.getRTBX(); //right, top, back
                        box1Ver[6][1] = box1.getRTBY();
                        box1Ver[6][2] = box1.getRTBZ();
                        box1Ver[7][0] = box1.getRBBX(); //right, bottom, back
                        box1Ver[7][1] = box1.getRBBY();
                        box1Ver[7][2] = box1.getRBBZ();
                        //BOX 2 VERTEX
                        box2Ver[0][0] = box2.getLBFX(); //left, bottom, front
                        box2Ver[0][1] = box2.getLBFY();
                        box2Ver[0][2] = box2.getLBFZ();
                        box2Ver[1][0] = box2.getLTFX(); //left, top, front
                        box2Ver[1][1] = box2.getLTFY();
                        box2Ver[1][2] = box2.getLTFZ();
                        box2Ver[2][0] = box2.getLTBX(); //left, top, back
                        box2Ver[2][1] = box2.getLTBY();
                        box2Ver[2][2] = box2.getLTBZ();
                        box2Ver[3][0] = box2.getLBBX(); //left, bottom, back
                        box2Ver[3][1] = box2.getLBBY();
                        box2Ver[3][2] = box2.getLBBZ();
                        box2Ver[4][0] = box2.getRBFX(); //right, bottom, front
                        box2Ver[4][1] = box2.getRBFY();
                        box2Ver[4][2] = box2.getRBFZ();
                        box2Ver[5][0] = box2.getRTFX(); //right, top, front
                        box2Ver[5][1] = box2.getRTFY();
                        box2Ver[5][2] = box2.getRTFZ();
                        box2Ver[6][0] = box2.getRTBX(); //right, top, back
                        box2Ver[6][1] = box2.getRTBY();
                        box2Ver[6][2] = box2.getRTBZ();
                        box2Ver[7][0] = box2.getRBBX(); //right, bottom, back
                        box2Ver[7][1] = box2.getRBBY();
                        box2Ver[7][2] = box2.getRBBZ();
                        //NORMALS ARRAY
                        normArr[0][0] = N1X;
                        normArr[0][1] = N1Y;
                        normArr[0][2] = N1Z;
                        normArr[1][0] = N2X;
                        normArr[1][1] = N2Y;
                        normArr[1][2] = N2Z;
                        normArr[2][0] = N3X;
                        normArr[2][1] = N3Y;
                        normArr[2][2] = N3Z;
                        normArr[3][0] = N4X;
                        normArr[3][1] = N4Y;
                        normArr[3][2] = N4Z;
                        normArr[4][0] = N5X;
                        normArr[4][1] = N5Y;
                        normArr[4][2] = N5Z;
                        normArr[5][0] = N6X;
                        normArr[5][1] = N6Y;
                        normArr[5][2] = N6Z;
                        normArr[6][0] = N7X;
                        normArr[6][1] = N7Y;
                        normArr[6][2] = N7Z;
                        normArr[7][0] = N8X;
                        normArr[7][1] = N8Y;
                        normArr[7][2] = N8Z;
                        normArr[8][0] = N9X;
                        normArr[8][1] = N9Y;
                        normArr[8][2] = N9Z;
                        normArr[9][0] = N10X;
                        normArr[9][1] = N10Y;
                        normArr[9][2] = N10Z;
                        normArr[10][0] = N11X;
                        normArr[10][1] = N11Y;
                        normArr[10][2] = N11Z;
                        normArr[11][0] = N12X;
                        normArr[11][1] = N12Y;
                        normArr[11][2] = N12Z;
                        normArr[12][0] = N13X;
                        normArr[12][1] = N13Y;
                        normArr[12][2] = N13Z;
                        normArr[13][0] = N14X;
                        normArr[13][1] = N14Y;
                        normArr[13][2] = N14Z;
                        normArr[14][0] = N15X;
                        normArr[14][1] = N15Y;
                        normArr[14][2] = N15Z;

                        for (int i=0; i<8; i++)
                        {
if (debug) fprintf(f, "box 1 vertex are: %f, %f, %f.\n", box1Ver[i][0],box1Ver[i][1],box1Ver[i][2]);
                        }
                        for (int i=0; i<8; i++)
                        {
if (debug) fprintf(f, "box 2 vertex are: %f, %f, %f.\n", box2Ver[i][0],box2Ver[i][1],box2Ver[i][2]);
                        }

                        for (int i=0; i<15; i++)
                        {
if (debug) fprintf(f, "normal %d is %f, %f, %f.\n", i, normArr[i][0],normArr[i][1],normArr[i][2]);
                        }
if (debug) fprintf(f, "**********************************************************\n");

                        GLfloat box1Dot, box2Dot;
                        box1Dot = 0;
                        box2Dot = 0;

                        //project box one to the 15 normals
                        for (int i=0; i<15; i++)
                        {
                                for(int j=0; j<8; j++)
                                {
box1Dot = box1Ver[j][0]*normArr[i][0]+box1Ver[j][1]*normArr[i][1]+box1Ver[j][2]*normArr[i][2];
//if(debug) fprintf(f, "box1 dot product with normals are: %d, %d,  %f.\n", i, j, box1Dot);
                                if(j==0)//if first loop inside vertex array
                                {
                                        box1Min[i] = box1Dot;
                                        box1Max[i] = box1Dot;
                                }
                                else
                                {
                                        if(box1Dot<box1Min[i])  //if current dot product smaller than mininum dot product
                                        {
                                        box1Min[i] = box1Dot;
                                        }
                                        if(box1Dot>box1Max[i])
                                        {
                                        box1Max[i] = box1Dot;
                                        }
                                }
                                }//for 8 vertex of box1
                        }//for 15 normals

                        //project the box 2 to 15 normals
                        for (int i=0; i<15; i++)
                        {
                                for(int j=0; j<8; j++)
                                {
box2Dot = box2Ver[j][0]*normArr[i][0]+box2Ver[j][1]*normArr[i][1]+box2Ver[j][2]*normArr[i][2];
if(debug) fprintf(f, "box2 dot product with are: %d, %d,  %f.\n", i, j, box2Dot);
                                if(j ==0 )//if first loop
                                {
                                        box2Min[i] = box2Dot;
                                        box2Max[i] = box2Dot;
                                }
                                else
                                {
                                        if(box2Dot<box2Min[i])  //if current dot product smaller than mininum dot product
                                        {
                                        box2Min[i] = box2Dot;
                                        }
                                        if(box2Dot>box2Max[i])
                                        {
                                        box2Max[i] = box2Dot;
                                        }
                                }
if(debug) fprintf(f, "current min and max are: %f, %f.\n", box2Min[i], box2Max[i]);                                                                        
                                }//for 8 vertex of box1
                        }//for 15 normals

                        //compare the 15 min and max from both box
                        for (int i = 0; i<15; i++)
                        {
                                //fprintf(f, "box1Min %d is %f, box1Max is %f.\n", i, box1Min[i], box1Max[i]);
                                fprintf(f, "box2Min %d is %f, box2Max is %f.\n", i, box2Min[i], box2Max[i]);
                                if (box1Min[i]>box2Max[i] || box1Max[i]<box2Min[i])      //if don't overlap
                                {
                                        return false;
                                        break;
                                }
                        }
                        return true;
                }//if both are regular cuboids


        }////if both are cuboids





        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in collide() function.\n";
                 try
                 {
                        throw Exception("Error in collide() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch


}//collides


/*
* Function Name:        CrossProduct
* Purpose:      Take in two vector and take their cross product
* Parameters:   VX, VY, VZ   -- vector 1
*               FX, FY, FZ   -- vector 2
* Return:       toRet           -- array of float store new vector coordinate
*/
GLfloat* CrossProduct(GLfloat VX, GLfloat VY, GLfloat VZ, GLfloat FX, GLfloat FY, GLfloat FZ)
{
        GLfloat toRet[3];

        toRet[0] = VY*FZ - VZ*FY;
        toRet[1] = VZ*FX - VX*FZ;
        toRet[2] = VX*FY - VY*FX;
if(debug) fprintf(f, "corss product of vector %f, %f, %f and vector %f, %f, %f are %f, %f, %f.\n", VX, VY, VZ, FX, FY, FZ, toRet[0], toRet[1], toRet[2]);
        return toRet;
}//CrossProduct


/*
This function build the 3D cuboid
*/
GLvoid BuildCuboid(cuboids cuboidsObj, int i)
{
try
{


GLfloat posAfterCamera[16];
GLfloat movePointM[16];
GLfloat rotate1[16];
GLfloat rotate2[16];
GLfloat rotate3[16];
GLfloat rotate4[16];
GLfloat translatef[16];
GLfloat rotate5[16];
GLfloat rotate6[16];
GLfloat rotate7[16];
GLfloat selfRot1[16];
GLfloat selfRot2[16];
GLfloat selfRot3[16];


        if(  (cuboidsObj.getPrimitiveID()==2 && cuboidsObj.getStatus() )
        ||
        (cuboidsObj.getPrimitiveID()==3 && cuboidsObj.getStatus()) )
        {

                glMatrixMode(GL_MODELVIEW);

//set the camera view
glLoadIdentity();
gluLookAt(camLig.getCamViewX()+x, camLig.getCamViewY()+y, camLig.getCamViewZ()+z, camLig.getCamAimX(), camLig.getCamAimY(), camLig.getCamAimZ(), 0.0f, 1.0f, 0.0f);
glGetFloatv(GL_MODELVIEW_MATRIX,posAfterCamera);


//1.    Set the boundary
//2.    move cuboids inside the boundar
        if (cuboidsObj.getPrimitiveID() == 3) //it is a boundary
        {
        //Boundary checking variables
                xPos = cuboidsObj.getCuboidX()+ (cuboidsObj.getCuboidW()/2 );
                xNeg = cuboidsObj.getCuboidX()- (cuboidsObj.getCuboidW()/2 );
                yPos = cuboidsObj.getCuboidY()+ (cuboidsObj.getCuboidH()/2 );
                yNeg = cuboidsObj.getCuboidY()- (cuboidsObj.getCuboidH()/2 );
                zPos = cuboidsObj.getCuboidZ()+ (cuboidsObj.getCuboidD()/2 );
                zNeg = cuboidsObj.getCuboidZ()- (cuboidsObj.getCuboidD()/2 );
            //    if (debug) fprintf(f, "The boundary varibles: xPos= %f, xNeg= %f, yPos= %f, yNeg= %f, zPos= %f, zNeg %f.\n",             xPos, xNeg, yPos, yNeg, zPos, zNeg);
        }//if
        else    // it is not a boundary
        {
        //check for the boundary of the cuboids
        if (cuboidsVec[i].getPrimitiveID() == 2 && xPos != 0.0f && xNeg!=0.0f && yPos!=0.0f && yNeg!=0.0f && zPos!=0.0f && zNeg!=0.0f)
        {
                if (cuboidsVec[i].getCuboidX() > xPos)// x coord is out of bound
                {
                cuboidsVec[i].setCuboid(xPos, cuboidsObj.getCuboidY(), cuboidsObj.getCuboidZ(), cuboidsObj.getCuboidW(), cuboidsObj.getCuboidH(), cuboidsObj.getCuboidD());
                }
                if (cuboidsVec[i].getCuboidX() < xNeg)
                {
                cuboidsVec[i].setCuboid(xNeg, cuboidsObj.getCuboidY(), cuboidsObj.getCuboidZ(), cuboidsObj.getCuboidW(), cuboidsObj.getCuboidH(), cuboidsObj.getCuboidD());
                }
                if (cuboidsVec[i].getCuboidY() > yPos)//y coord is out of bound
                {
                cuboidsVec[i].setCuboid(cuboidsObj.getCuboidX(), yPos, cuboidsObj.getCuboidZ(), cuboidsObj.getCuboidW(), cuboidsObj.getCuboidH(), cuboidsObj.getCuboidD() );
                }
                if (cuboidsVec[i].getCuboidY() < yNeg)
                {
                cuboidsVec[i].setCuboid(cuboidsObj.getCuboidX(), yNeg, cuboidsObj.getCuboidZ(), cuboidsObj.getCuboidW(), cuboidsObj.getCuboidH(), cuboidsObj.getCuboidD() );
                }
                if (cuboidsVec[i].getCuboidZ() > zPos)//z coord is out of bound
                {
                cuboidsVec[i].setCuboid(cuboidsObj.getCuboidX(), cuboidsObj.getCuboidY(), zPos, cuboidsObj.getCuboidW(), cuboidsObj.getCuboidH(), cuboidsObj.getCuboidD() );
                }
                if (cuboidsVec[i].getCuboidZ() < zNeg)
                {
                cuboidsVec[i].setCuboid(cuboidsObj.getCuboidX(), cuboidsObj.getCuboidY(), zNeg, cuboidsObj.getCuboidW(), cuboidsObj.getCuboidH(), cuboidsObj.getCuboidD() );
                }
        }//if coubids is out of bound

        if(debug)
        {
        fprintf(f, "Currenty CUBOIDS coordinate, X= %f,  Y= %f, Z= %f.\n", cuboidsObj.getCuboidX(),cuboidsObj.getCuboidY(), cuboidsObj.getCuboidZ() );
        fprintf(f, "IN CUBOIDS::::  The boundary varibles: xPos= %f, xNeg= %f, yPos= %f, yNeg= %f, zPos= %f, zNeg %f.\n",             xPos, xNeg, yPos, yNeg, zPos, zNeg);
        }

        }//else  it's not a boundary

//move to the reference point
        glLoadIdentity();
        //draw the rotation point
        glTranslatef(cuboidsObj.getRPX(), cuboidsObj.getRPY(), cuboidsObj.getRPZ());
        glGetFloatv(GL_MODELVIEW_MATRIX, movePointM);



        if (cuboidsObj.getPrimitiveID() != 3) //it is not a boundary
        {
                GLfloat tempx, tempy, tempz;
                tempx = cuboidsObj.getCuboidX() - cuboidsObj.getRPX();
                tempy = cuboidsObj.getCuboidY() - cuboidsObj.getRPY();
                tempz = cuboidsObj.getCuboidZ() - cuboidsObj.getRPZ();
                //distant from the distant point to the object in the xz plane
                GLfloat dist = sqrt(tempx*tempx + tempz * tempz);
                GLfloat dist2 = sqrt(tempz*tempz + tempy*tempy);
                if (dist == 0) dist = 0.000000001;
                if (dist2 == 0) dist2 = 0.000000001;

                //toMove is the distanct of the object from its rotate point on the x-axis of the object vie.
                GLfloat toMove = sqrt(tempx*tempx + tempy*tempy + tempz*tempz);

                if(debug) fprintf(f, "%d      , cub tempz is %f, tempy is %f, tempx is %f.\n",i, tempz, tempy, tempx);

                //the angel that is going to rotate the xz plane around the y axis
                GLfloat  ang, angy;

            /*  if (tempz == 0)
                {
                        ang = 0;
                }
                if (tempz != 0)
                {   */

                if (dist == 0)  dist+= 0.000001;
                if (toMove == 0) toMove += 0.000001;

        if(debug) fprintf(f, "GLOBAL: %s.\n", globalError);
                        tempz=0-tempz;
                        //rotate the object around rotate point's y axis to makes it reach the z coordiante

                        ang = asin(tempz/dist);
                        //rotate the object around rotate point's z axis to make it reach the y coordinate
                        angy = asin(tempy/toMove);
                        //change angle from radian to degrees
                        ang = ang*180/M_PI;
                        angy = angy*180/M_PI;
       if (debug) fprintf(f, "DEBUG angy is %f, ang is %f, tempy is %f,toMove is %f.\n", angy, ang, tempy, toMove);
                        //find out original z
                        GLfloat oldTempz = 0 - tempz;
                        if ( tempx<0)
                        {
                           ang = asin(oldTempz/dist);
                           ang = ang*180/M_PI;
                           angy = 0-angy;
                        }
              //}//else
       if (debug) fprintf(f, "DEBUG::: angy is %f, ang is %f.\n", angy, ang);
                //draw the orbiting object
                //rotate with respect to the reference point
                GLfloat XYang = cuboidsObj.getRotPointAngleXY();
                GLfloat rotDirection = cuboidsObj.getRotPointAngleYZ();

                //rotate 4 times to make the rotation around point effect
                glLoadIdentity();
                glRotatef(ang, 0,1, 0);
                glGetFloatv(GL_MODELVIEW_MATRIX, rotate1);

                glLoadIdentity();
                glRotatef(angy, 0,0,1);
                glGetFloatv(GL_MODELVIEW_MATRIX, rotate2);

                //rotatae XYang angles around the distant point in rotDirection angle direction
                glLoadIdentity();
                glRotatef(rotDirection,  1,0,0);
                glGetFloatv(GL_MODELVIEW_MATRIX, rotate3);

                glLoadIdentity();
                glRotatef(XYang, 0,0, 1);
                glGetFloatv(GL_MODELVIEW_MATRIX, rotate4);

        if(debug) fprintf(f, "cub dist is %f, dist2 is %f, toMove is %f.\n", dist, dist2, toMove);
        if(debug) fprintf(f, "cub the rotate angle is %f, %f, and %f.\n", ang, angy, XYang);
        //rotate XY plane angle with respect to z axis


//total second reotation angle
GLfloat secRotAng = XYang + angy;
GLfloat tempBase = toMove*cos((double)secRotAng);
GLfloat newy = toMove*sin((double)secRotAng);
GLfloat newx = tempBase*cos((double)ang);
GLfloat newz = tempBase*sin((double)ang);

        if(debug) fprintf(f, "CUBE %d: tempz is %f, tempy is %f, tempx is %f.\n",i, tempz, tempy, tempx);
        if(debug) fprintf(f, " dist is %f, dist2 is %f, toMove is %f.\n", dist, dist2, toMove);
        if(debug) fprintf(f, "the rotate angle is %f, %f, and %f.\n", ang, angy, XYang);
        //rotate XY plane angle with respect to z axis

//move to the objects' midpoints (coordinate) by calculating the object's relative position
//the y and z coordinate is 0 because the coordinate alredy rotate for rotation purpose
        glLoadIdentity();
        if (tempx<0) toMove = 0-toMove;
        glTranslatef(toMove, 0, 0);
        glGetFloatv(GL_MODELVIEW_MATRIX, translatef);

        //rotate back to original position. So the object wouldn't look tilted

        glLoadIdentity();
        glRotatef((0-XYang), 0,0,1);
        glRotatef((0-rotDirection), 1,0,0);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate5);

        glLoadIdentity();
        glRotatef((0-angy), 0,0,1);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate6);

        glLoadIdentity();
        glRotatef((0-ang), 0,1, 0);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate7);

//rotate at the current position with respect to axis

        //rotate with respect to x axis
        glLoadIdentity();
        glRotatef(cuboidsObj.getRotAngX(), 1.0f, 0.0f, 0.0f);
        glGetFloatv(GL_MODELVIEW_MATRIX, selfRot1);
        //rotate with respect to y axis
        glLoadIdentity();
        glRotatef(cuboidsObj.getRotAngY(), 0.0f, 1.0f, 0.0f);
        glGetFloatv(GL_MODELVIEW_MATRIX, selfRot2);
        //rotate with respect to z axis
        glLoadIdentity();
        glRotatef(cuboidsObj.getRotAngZ(), 0.0f, 0.0f, 1.0f);
        glGetFloatv(GL_MODELVIEW_MATRIX, selfRot3);
        }//not a boundary
/**********************************ACTURAL DRAWING********************/
        glLoadIdentity();
        //CAMERAgl
        glMultMatrixf(posAfterCamera);
        
        if (cuboidsObj.getPrimitiveID()!=3) //not boundary
        {
        //MOVEMENT AND ROTATION
        glMultMatrixf(movePointM);
        glMultMatrixf(rotate1);
        glMultMatrixf(rotate2);
        glMultMatrixf(rotate3);
        glMultMatrixf(rotate4);
        glMultMatrixf(translatef);              
        glMultMatrixf(rotate5);
        glMultMatrixf(rotate6);
        glMultMatrixf(rotate7);
        glMultMatrixf(selfRot1);
        glMultMatrixf(selfRot2);
        glMultMatrixf(selfRot3);
        }



//load texture from the primObj class texture data  
        if (cuboidsObj.getHasTexture() )       //check to see if texture date loaded successfully
        {
                if (debug) fprintf(f, "cuboids texture loaded.\n");
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, cuboidsObj.getTextureData() );
        }
        else
        {
                if (debug) fprintf(f, "texture loaded failed.\n");
        }


//Set the dimension of the cubiods to temp variables
        if (debug) fprintf(f, "Primitive ID is %d.\n", cuboidsObj.getPrimitiveID());
        GLfloat w, d, h;
        w = cuboidsObj.getCuboidW();     //width: x
        d = cuboidsObj.getCuboidD();     //depth: z
        h = cuboidsObj.getCuboidH();     //height: y



/*transparency  */
        GLfloat cubBri = (GLfloat)cuboidsObj.getBrightness()/255;
        if (cubBri>= 1)      cubBri=1;
        GLfloat cubTrans = (GLfloat)cuboidsObj.getTransparency()/255;
        if (cubTrans>=1)     cubTrans = 1;

        glEnable(GL_BLEND);             // Turn Blending On/
        glColor4f(cuboidsObj.getColorR()*cubBri,cuboidsObj.getColorG()*cubBri,cuboidsObj.getColorB()*cubBri, cubTrans);
  	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Blending Function For Translucency Based On Source Alpha Value ( NEW )


//Draw the cuboids
        glBegin(GL_QUADS);
                // Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);					// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-(w/2), -(h/2),  d/2);	// Point 1 (Front)
		glTexCoord2f(1.0f, 0.0f);
                glVertex3f( (w/2), -(h/2),  d/2);	// Point 2 (Front)
		glTexCoord2f(1.0f, 1.0f);
                glVertex3f( w/2,  h/2,  d/2);	// Point 3 (Front)
		glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-(w/2),  h/2,  d/2);	// Point 4 (Front)
		// Back Face
		glNormal3f( 0.0f, 0.0f,-1.0f);					// Normal Pointing Away From Viewer
		glTexCoord2f(1.0f, 0.0f);
                glVertex3f(-w/2, -h/2, -d/2);	// Point 1 (Back)
		glTexCoord2f(1.0f, 1.0f);
                glVertex3f(-w/2,  h/2, -d/2);	// Point 2 (Back)
		glTexCoord2f(0.0f, 1.0f);
                glVertex3f( w/2,  h/2, -d/2);	// Point 3 (Back)
		glTexCoord2f(0.0f, 0.0f);
                glVertex3f( w/2, -h/2, -d/2);	// Point 4 (Back)
		// Top Face
		glNormal3f( 0.0f, 1.0f, 0.0f);					// Normal Pointing Up
		glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-w/2,  h/2, -d/2);	// Point 1 (Top)
		glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-w/2,  h/2,  d/2);	// Point 2 (Top)
		glTexCoord2f(1.0f, 0.0f);
                glVertex3f( w/2,  h/2,  d/2);	// Point 3 (Top)
		glTexCoord2f(1.0f, 1.0f);
                glVertex3f( w/2,  h/2, -d/2);	// Point 4 (Top)
		// Bottom Face
		glNormal3f( 0.0f,-1.0f, 0.0f);					// Normal Pointing Down
		glTexCoord2f(1.0f, 1.0f);
                glVertex3f(-w/2, -h/2, -d/2);	// Point 1 (Bottom)
		glTexCoord2f(0.0f, 1.0f);
                glVertex3f( w/2, -h/2, -d/2);	// Point 2 (Bottom)
		glTexCoord2f(0.0f, 0.0f);
                glVertex3f( w/2, -h/2,  d/2);	// Point 3 (Bottom)
		glTexCoord2f(1.0f, 0.0f);
                glVertex3f(-w/2, -h/2,  d/2);	// Point 4 (Bottom)
		// Right face
		glNormal3f( 1.0f, 0.0f, 0.0f);					// Normal Pointing Right
		glTexCoord2f(1.0f, 0.0f);
                glVertex3f( w/2, -h/2, -d/2);	// Point 1 (Right)
		glTexCoord2f(1.0f, 1.0f);
                glVertex3f( w/2,  h/2, -d/2);	// Point 2 (Right)
		glTexCoord2f(0.0f, 1.0f);
                glVertex3f( w/2,  h/2,  d/2);	// Point 3 (Right)
		glTexCoord2f(0.0f, 0.0f);
                glVertex3f( w/2, -h/2,  d/2);	// Point 4 (Right)
		// Left Face
		glNormal3f(-1.0f, 0.0f, 0.0f);					// Normal Pointing Left
		glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-w/2, -h/2, -d/2);	// Point 1 (Left)
		glTexCoord2f(1.0f, 0.0f);
                glVertex3f(-w/2, -h/2,  d/2);	// Point 2 (Left)
		glTexCoord2f(1.0f, 1.0f);
                glVertex3f(-w/2,  h/2,  d/2);	// Point 3 (Left)
		glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-w/2,  h/2, -d/2);	// Point 4 (Left)
	glEnd();
        								// Done Drawing Quads

        glDisable(GL_BLEND);		// Turn Blending off
        glDisable(GL_TEXTURE_2D);       // Turn the 2D texture off
  	//glEnable(GL_DEPTH_TEST);	// Turn Depth Testing Off

/***************************ACTUAL DRAWING END***************************************/


/*****************************FINDING THE NEW MIDPOINT***************************/
if (cuboidsObj.getPrimitiveID() != 3)
{
//posoition after the rotation
GLfloat posAfterRot[16];

glLoadIdentity();
        glMultMatrixf(movePointM);
        glMultMatrixf(rotate1);
        glMultMatrixf(rotate2);
        glMultMatrixf(rotate3);
        glMultMatrixf(rotate4);
        glMultMatrixf(translatef);
        glMultMatrixf(rotate5);
        glMultMatrixf(rotate6);
        glMultMatrixf(rotate7);
        glMultMatrixf(selfRot1);        //self rotate aroudn X axi
        glMultMatrixf(selfRot2);        //self rotate around Y axi
        glMultMatrixf(selfRot3);        //self rotate around z axi
        glGetFloatv(GL_MODELVIEW_MATRIX,posAfterRot);
        if(debug) fprintf(f, "posAfterRot x is %f, y is %f, z is %f degrees.\n", posAfterRot[12], posAfterRot[13], posAfterRot[14]);
glLoadIdentity();
cuboidsVec[i].setcoordNewX( posAfterRot[12] );
cuboidsVec[i].setcoordNewY( posAfterRot[13] );
cuboidsVec[i].setcoordNewZ( posAfterRot[14] );
//cuboidsVec[i].setCuboid(posAfterRot[12]-posAfterCamera[12],posAfterRot[13]-posAfterCamera[13],posAfterRot[14]-posAfterCamera[14], cuboidsObj.getCuboidW(), cuboidsObj.getCuboidH(), cuboidsObj.getCuboidD());
cuboidsVec[i].setRotPointAngleXY(0);
//cuboidsVec[i].setRotPointAngleYZ(0);
/*********************************FINDING THE NEW MIDPOINT ENDS*************************/
        GLfloat newMidX, newMidY, newMidZ;
        newMidX = posAfterRot[12];
        newMidY = posAfterRot[13];
        newMidZ = posAfterRot[14];

        //Calculate default position if no rotate angle
        if (cuboidsObj.getRotAngX() == 0 && cuboidsObj.getRotAngY() == 0 && cuboidsObj.getRotAngX() == 0 )
        {
                cuboidsVec[i].setVertex(newMidX, newMidY, newMidZ);
                if (debug)
                {
                fprintf(f, "Cuboid %d New midpoint is %f, %f, %f\n", i, newMidX, newMidY, newMidZ );
                fprintf(f, "Cuboid %d 's W, H, D is %f, %f, %f\n", i, cuboidsVec[i].getCuboidW(), cuboidsVec[i].getCuboidH(), cuboidsVec[i].getCuboidD() );
                fprintf(f, "Left, Bottom, Front is: %f, %f, %f.\n", cuboidsVec[i].getLBFX(), cuboidsVec[i].getLBFY(), cuboidsVec[i].getLBFZ());
                fprintf(f, "Left, Bottom, Back is: %f, %f, %f.\n", cuboidsVec[i].getLBBX(), cuboidsVec[i].getLBBY(), cuboidsVec[i].getLBBZ());
                fprintf(f, "Left, Top, Front is: %f, %f, %f.\n", cuboidsVec[i].getLTFX(), cuboidsVec[i].getLTFY(), cuboidsVec[i].getLTFZ());
                fprintf(f, "Left, Top, Back is: %f, %f, %f.\n", cuboidsVec[i].getLTBX(), cuboidsVec[i].getLTBY(), cuboidsVec[i].getLTBZ());
                fprintf(f, "Right, Bottom, Front is: %f, %f, %f.\n", cuboidsVec[i].getRBFX(), cuboidsVec[i].getRBFY(), cuboidsVec[i].getRBFZ());
                fprintf(f, "Right, Bottom, Back is: %f, %f, %f.\n", cuboidsVec[i].getRBBX(), cuboidsVec[i].getRBBY(), cuboidsVec[i].getRBFZ());
                fprintf(f, "Right, Top, Front is: %f, %f, %f.\n", cuboidsVec[i].getRTFX(), cuboidsVec[i].getRTFY(), cuboidsVec[i].getRTFZ());
                fprintf(f, "Right, Top, Back is: %f, %f, %f.\n", cuboidsVec[i].getRTBX(), cuboidsVec[i].getRTBY(), cuboidsVec[i].getRTBZ());
                }
        }
        else
        {
       
        //Calculate the vertext when the self rotation is applied
        GLfloat newVertex[16];
        GLfloat moveLTF[16];
        GLfloat moveLBF[16];
        GLfloat moveLBB[16];
        GLfloat moveLTB[16];
        GLfloat moveRTF[16];
        GLfloat moveRBF[16];
        GLfloat moveRBB[16];
        GLfloat moveRTB[16];
        //Getting the movement matrix

glLoadIdentity();
        //-w/2, +H/2, +D/2      //Left, Top, Front
        glTranslatef(0-cuboidsVec[i].getCuboidW()/2,cuboidsVec[i].getCuboidH()/2, cuboidsVec[i].getCuboidD()/2);
        glGetFloatv(GL_MODELVIEW_MATRIX,moveLTF);
glLoadIdentity();

glLoadIdentity();
        //-w/2, -H/2, +D/2      //Left, Bottom, Front
        glTranslatef(0-cuboidsVec[i].getCuboidW()/2, 0-cuboidsVec[i].getCuboidH()/2, cuboidsVec[i].getCuboidD()/2);
        glGetFloatv(GL_MODELVIEW_MATRIX,moveLBF);
glLoadIdentity();

glLoadIdentity();
        //-w/2, -H/2, -D/2      //Left, Bottom, Back
        glTranslatef(0-cuboidsVec[i].getCuboidW()/2, 0-cuboidsVec[i].getCuboidH()/2, 0-cuboidsVec[i].getCuboidD()/2);
        glGetFloatv(GL_MODELVIEW_MATRIX,moveLBB);
glLoadIdentity();

glLoadIdentity();
        //-w/2, +H/2, -D/2      //Left, Top, Back
        glTranslatef(0-cuboidsVec[i].getCuboidW()/2, cuboidsVec[i].getCuboidH()/2, 0-cuboidsVec[i].getCuboidD()/2);
        glGetFloatv(GL_MODELVIEW_MATRIX,moveLTB);
glLoadIdentity();

glLoadIdentity();
        //w/2, +H/2, +D/2      //Right, Top, Front
        glTranslatef(cuboidsVec[i].getCuboidW()/2,cuboidsVec[i].getCuboidH()/2, cuboidsVec[i].getCuboidD()/2);
        glGetFloatv(GL_MODELVIEW_MATRIX,moveRTF);
glLoadIdentity();

glLoadIdentity();
        //w/2, -H/2, +D/2      //Right, Bottom, Front
        glTranslatef(cuboidsVec[i].getCuboidW()/2, 0-cuboidsVec[i].getCuboidH()/2, cuboidsVec[i].getCuboidD()/2);
        glGetFloatv(GL_MODELVIEW_MATRIX,moveRBF);
glLoadIdentity();

glLoadIdentity();
        //w/2, -H/2, -D/2      //Right, Bottom, Back
        glTranslatef(cuboidsVec[i].getCuboidW()/2, 0-cuboidsVec[i].getCuboidH()/2, 0-cuboidsVec[i].getCuboidD()/2);
        glGetFloatv(GL_MODELVIEW_MATRIX,moveRBB);
glLoadIdentity();

glLoadIdentity();
        //w/2, +H/2, -D/2      //Right, Top, Back
        glTranslatef(cuboidsVec[i].getCuboidW()/2, cuboidsVec[i].getCuboidH()/2, 0-cuboidsVec[i].getCuboidD()/2);
        glGetFloatv(GL_MODELVIEW_MATRIX,moveRTB);
glLoadIdentity();


        fprintf(f, "Cuboid %d New midpoint is %f, %f, %f\n", i, newMidX, newMidY, newMidZ );
        fprintf(f, "Cuboid %d 's W, H, D is %f, %f, %f\n", i, cuboidsVec[i].getCuboidW(), cuboidsVec[i].getCuboidH(), cuboidsVec[i].getCuboidD() );

glLoadIdentity();
        glMultMatrixf(movePointM);
        glMultMatrixf(rotate1);
        glMultMatrixf(rotate2);
        glMultMatrixf(rotate3);
        glMultMatrixf(rotate4);
        glMultMatrixf(translatef);
        glMultMatrixf(rotate5);
        glMultMatrixf(rotate6);
        glMultMatrixf(rotate7);
        glMultMatrixf(selfRot1);        //self rotate aroudn X axi
        glMultMatrixf(selfRot2);        //self rotate around Y axi
        glMultMatrixf(selfRot3);        //self rotate around z axi
                //Get the Left, Top, Front vertex
                glPushMatrix();         //save the new midpoint
                glMultMatrixf(moveLTF);
                glGetFloatv(GL_MODELVIEW_MATRIX,newVertex);
                cuboidsVec[i].setLTFX(newVertex[12]);
                cuboidsVec[i].setLTFY(newVertex[13]);
                cuboidsVec[i].setLTFZ(newVertex[14]);
                if (debug) fprintf(f, "The new, Left, Top, Front is: %f, %f, %f.\n",cuboidsVec[i].getLTFX(), cuboidsVec[i].getLTFY(), cuboidsVec[i].getLTFZ() );
                glPopMatrix();
                //Get the Left, Bottom, Front vertex
                glPushMatrix();
                glMultMatrixf(moveLBF);
                glGetFloatv(GL_MODELVIEW_MATRIX,newVertex);
                cuboidsVec[i].setLBFX(newVertex[12]);
                cuboidsVec[i].setLBFY(newVertex[13]);
                cuboidsVec[i].setLBFZ(newVertex[14]);
                if (debug) fprintf(f, "The new, Left, Bottom, Front is: %f, %f, %f.\n",cuboidsVec[i].getLBFX(), cuboidsVec[i].getLBFY(), cuboidsVec[i].getLBFZ() );
                glPopMatrix();
                //Get the Left, Bottom, Back vertex
                glPushMatrix();         //save the new midpoint
                glMultMatrixf(moveLBB);
                glGetFloatv(GL_MODELVIEW_MATRIX,newVertex);
                cuboidsVec[i].setLBBX(newVertex[12]);
                cuboidsVec[i].setLBBY(newVertex[13]);
                cuboidsVec[i].setLBBZ(newVertex[14]);
                if (debug) fprintf(f, "The new, Left, Bottom, Back is: %f, %f, %f.\n",cuboidsVec[i].getLBBX(), cuboidsVec[i].getLBBY(), cuboidsVec[i].getLBBZ() );
                glPopMatrix();
                //Get the Left, Top, Back vertex
                glPushMatrix();
                glMultMatrixf(moveLTB);
                glGetFloatv(GL_MODELVIEW_MATRIX,newVertex);
                cuboidsVec[i].setLTBX(newVertex[12]);
                cuboidsVec[i].setLTBY(newVertex[13]);
                cuboidsVec[i].setLTBZ(newVertex[14]);
                if (debug) fprintf(f, "The new, Left, Top, Back is: %f, %f, %f.\n",cuboidsVec[i].getLTBX(), cuboidsVec[i].getLTBY(), cuboidsVec[i].getLTBZ() );
                glPopMatrix();
                //Get the Right, Top, Front vertex
                glPushMatrix();         //save the new midpoint
                glMultMatrixf(moveRTF);
                glGetFloatv(GL_MODELVIEW_MATRIX,newVertex);
                cuboidsVec[i].setRTFX(newVertex[12]);
                cuboidsVec[i].setRTFY(newVertex[13]);
                cuboidsVec[i].setRTFZ(newVertex[14]);
                if (debug) fprintf(f, "The new, Right, Top, Front is: %f, %f, %f.\n",cuboidsVec[i].getRTFX(), cuboidsVec[i].getRTFY(), cuboidsVec[i].getRTFZ() );
                glPopMatrix();
                //Get the Right, Bottom, Front vertex
                glPushMatrix();
                glMultMatrixf(moveRBF);
                glGetFloatv(GL_MODELVIEW_MATRIX,newVertex);
                cuboidsVec[i].setRBFX(newVertex[12]);
                cuboidsVec[i].setRBFY(newVertex[13]);
                cuboidsVec[i].setRBFZ(newVertex[14]);
                if (debug) fprintf(f, "The new, Right, Bottom, Front is: %f, %f, %f.\n",cuboidsVec[i].getRBFX(), cuboidsVec[i].getRBFY(), cuboidsVec[i].getRBFZ() );
                glPopMatrix();
                //Get the Right, Bottom, Back vertex
                glPushMatrix();         //save the new midpoint
                glMultMatrixf(moveRBB);
                glGetFloatv(GL_MODELVIEW_MATRIX,newVertex);
                cuboidsVec[i].setRBBX(newVertex[12]);
                cuboidsVec[i].setRBBY(newVertex[13]);
                cuboidsVec[i].setRBBZ(newVertex[14]);
                if (debug) fprintf(f, "The new, Right, Bottom, Back is: %f, %f, %f.\n",cuboidsVec[i].getRBBX(), cuboidsVec[i].getRBBY(), cuboidsVec[i].getRBBZ() );
                glPopMatrix();
                //Get the Right, Top, Back vertex
                glPushMatrix();
                glMultMatrixf(moveRTB);
                glGetFloatv(GL_MODELVIEW_MATRIX,newVertex);
                cuboidsVec[i].setRTBX(newVertex[12]);
                cuboidsVec[i].setRTBY(newVertex[13]);
                cuboidsVec[i].setRTBZ(newVertex[14]);
                if (debug) fprintf(f, "The new, Right, Top, Back is: %f, %f, %f.\n",cuboidsVec[i].getRTBX(), cuboidsVec[i].getRTBY(), cuboidsVec[i].getRTBZ() );
                glPopMatrix();
glLoadIdentity();
        }//else.  There is self rotatoin applied
/************************CALCULATE NEW VERTICES**************************************/

 //testing
  /*  glLoadIdentity();
    gluLookAt(camLig.getCamViewX()+x, camLig.getCamViewY()+y, camLig.getCamViewZ()+z, camLig.getCamAimX(), camLig.getCamAimY(), camLig.getCamAimZ(), 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
                glColor4f(1, 0,0, 0);
                // Front Face
		glVertex3f(cuboidsVec[2].getLBFX()-.1, cuboidsVec[2].getLBFY(), cuboidsVec[2].getLBFZ());	// Point 1 (Front)   LBF
                glVertex3f(cuboidsVec[2].getRBFX()-.1, cuboidsVec[2].getRBFY(), cuboidsVec[2].getRBFZ());	// Point 2 (Front)   RBF
                glVertex3f(cuboidsVec[2].getRTFX()-.1, cuboidsVec[2].getRTFY(), cuboidsVec[2].getRTFZ());	        // Point 3 (Front)   RTF
                glVertex3f(cuboidsVec[2].getLTFX()-.1, cuboidsVec[2].getLTFY(), cuboidsVec[2].getLTFZ());	        // Point 4 (Front)   LTF
                //glVertex3f(cuboidsVec[2].getLBFX(), cuboidsVec[2].getLBFY(), cuboidsVec[2].getLBFZ());	// Point 1 (Front)   LBF
                glColor4f(1, 1,0, 0);
		// Back Face
		glVertex3f(cuboidsVec[2].getLBBX(), cuboidsVec[2].getLBBY(), cuboidsVec[2].getLBBZ());	// Point 1 (Front)   LBB
                glVertex3f(cuboidsVec[2].getLTBX(), cuboidsVec[2].getLTBY(), cuboidsVec[2].getLTBZ());	// Point 2 (Front)   LTB
                glVertex3f(cuboidsVec[2].getRTBX(), cuboidsVec[2].getRTBY(), cuboidsVec[2].getRTBZ());  // Point 3 (Front)   RTB
                glVertex3f(cuboidsVec[2].getRBBX(), cuboidsVec[2].getRBBY(), cuboidsVec[2].getRBBZ());	// Point 4 (Front)   RBB
                //glVertex3f(cuboidsVec[2].getLBBX(), cuboidsVec[2].getLBBY(), cuboidsVec[2].getLBBZ());	// Point 1 (Front)   LBB
                glColor4f(1, 0,1, 0);
		// Top Face
		glVertex3f(cuboidsVec[2].getLTBX(), cuboidsVec[2].getLTBY(), cuboidsVec[2].getLTBZ());	// Point 1 (Front)   LTB
                glVertex3f(cuboidsVec[2].getLTFX(), cuboidsVec[2].getLTFY(), cuboidsVec[2].getLTFZ());	// Point 2 (Front)   LTF
                glVertex3f(cuboidsVec[2].getRTFX(), cuboidsVec[2].getRTFY(), cuboidsVec[2].getRTFZ());  // Point 3 (Front)   RTF
                glVertex3f(cuboidsVec[2].getRTBX(), cuboidsVec[2].getRTBY(), cuboidsVec[2].getRTBZ());	// Point 4 (Front)   RTB
                //glVertex3f(cuboidsVec[2].getLTBX(), cuboidsVec[2].getLTBY(), cuboidsVec[2].getLTBZ());	// Point 1 (Front)   LTB

                // Bottom Face
		glVertex3f(cuboidsVec[2].getLBBX(), cuboidsVec[2].getLBBY(), cuboidsVec[2].getLBBZ());	// Point 1 (Front)   LBB
                glVertex3f(cuboidsVec[2].getRBBX(), cuboidsVec[2].getRBBY(), cuboidsVec[2].getRBBZ());	// Point 2 (Front)   RBB
                glVertex3f(cuboidsVec[2].getRBFX(), cuboidsVec[2].getRBFY(), cuboidsVec[2].getRBFZ());  // Point 3 (Front)   RBF
                glVertex3f(cuboidsVec[2].getLBFX(), cuboidsVec[2].getLBFY(), cuboidsVec[2].getLBFZ());	// Point 4 (Front)   LBF
                //glVertex3f(cuboidsVec[2].getLBBX(), cuboidsVec[2].getLBBY(), cuboidsVec[2].getLBBZ());	// Point 1 (Front)   LBB

		// Right face
	        glVertex3f(cuboidsVec[2].getRBBX(), cuboidsVec[2].getRBBY(), cuboidsVec[2].getRBBZ());	// Point 1 (Front)   RBB
                glVertex3f(cuboidsVec[2].getRTBX(), cuboidsVec[2].getRTBY(), cuboidsVec[2].getRTBZ());	// Point 2 (Front)   RTB
                glVertex3f(cuboidsVec[2].getRTFX(), cuboidsVec[2].getRTFY(), cuboidsVec[2].getRTFZ());  // Point 3 (Front)   RTF
                glVertex3f(cuboidsVec[2].getRBFX(), cuboidsVec[2].getRBFY(), cuboidsVec[2].getRBFZ());	// Point 4 (Front)   RBF
               // glVertex3f(cuboidsVec[2].getRBBX(), cuboidsVec[2].getRBBY(), cuboidsVec[2].getRBBZ());	// Point 1 (Front)   RBB

        	// Left Face
	        glVertex3f(cuboidsVec[2].getLBBX(), cuboidsVec[2].getLBBY(), cuboidsVec[2].getLBBZ());	// Point 1 (Front)   LBB
                glVertex3f(cuboidsVec[2].getLBFX(), cuboidsVec[2].getLBFY(), cuboidsVec[2].getLBFZ());	// Point 2 (Front)   LBF
                glVertex3f(cuboidsVec[2].getLTFX(), cuboidsVec[2].getLTFY(), cuboidsVec[2].getLTFZ());  // Point 3 (Front)   LTF
                glVertex3f(cuboidsVec[2].getLTBX(), cuboidsVec[2].getLTBY(), cuboidsVec[2].getLTBZ());	// Point 4 (Front)   LTB
                //glVertex3f(cuboidsVec[2].getLBBX(), cuboidsVec[2].getLBBY(), cuboidsVec[2].getLBBZ());	// Point 1 (Front)   LBB

	glEnd();
                */
/*************************CALCULATE NEW VERTICES END******************************/



}//if it is not bound
else    //it is a bound, i.e, midpoint is always 0,0,0
{
        cuboidsVec[i].setVertex(0,0,0);
}


}//if it is a cube

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in BuildCuboid() function.\n";
                 try
                 {
                        throw Exception("Error in BuildCuboid() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch


}//BuildCuboid




/*
This function build the 3D sphere
*/
GLvoid BuildSphere(sphere sphereObj, int i)
{
try
{
        GLUquadricObj	*q;
        q = gluNewQuadric();             // Create A New Quadratic
        gluQuadricNormals(q, GL_SMOOTH);        // Generate Smooth Normals For The Quad
   	gluQuadricTexture(q, GL_TRUE);          // Enable Texture Coords For The Quad

        if (sphereObj.getPrimitiveID()==1 && sphereObj.getStatus())
        //if the object in the vector is a sphere and is on
        {


                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
//set the camera view
gluLookAt(camLig.getCamViewX()+x, camLig.getCamViewY()+y, camLig.getCamViewZ()+z, camLig.getCamAimX(), camLig.getCamAimY(), camLig.getCamAimZ(), 0.0f, 1.0f, 0.0f);

//coordingate reltiave to the world origin is store at array[12][[13][14]
GLfloat posAfterCamera[16];
GLfloat movePointM[16];
GLfloat rotate1[16];
GLfloat rotate2[16];
GLfloat rotate3[16];
GLfloat rotate4[16];
GLfloat translatef[16];
GLfloat rotate5[16];
GLfloat rotate6[16];
GLfloat rotate7[16];
GLfloat selfRot1[16];
GLfloat selfRot2[16];
GLfloat selfRot3[16];

glGetFloatv(GL_MODELVIEW_MATRIX,posAfterCamera);
if (  debug  ) fprintf(f, "\n\n\n\n\n\n\n\n\n\n00000000Current position is: x=%f, y=%f, z=%f s=%f.\n\n\n\n\n\n\n\n\n", posAfterCamera[12], posAfterCamera[13], posAfterCamera[14], posAfterCamera[15]);



                




//check for the boundary and reposition the objects if they are out of bound
if (sphereObj.getPrimitiveID() == 1 && xPos != 0.0f && xNeg!=0.0f && yPos!=0.0f && yNeg!=0.0f && zPos!=0.0f && zNeg!=0.0f)    //it's a sphere
        {
                if (sphereObj.getSphereX() > xPos)//sphere's x coord is out of bound
                {
                sphereVec[i].setSphere(xPos, sphereObj.getSphereY(), sphereObj.getSphereZ(), sphereObj.getSphereRad());
                }
                if (sphereObj.getSphereX() < xNeg)
                {
                sphereVec[i].setSphere(xNeg, sphereObj.getSphereY(), sphereObj.getSphereZ(), sphereObj.getSphereRad());
                }
                if (sphereObj.getSphereY() > yPos)//y coord is out of bound
                {
                sphereVec[i].setSphere(sphereObj.getSphereX(), yPos, sphereObj.getSphereZ(), sphereObj.getSphereRad() );
                }
                if (sphereObj.getSphereY() < yNeg)
                {
                sphereVec[i].setSphere(sphereObj.getSphereX(), yNeg, sphereObj.getSphereZ(), sphereObj.getSphereRad() );
                }
                if (sphereObj.getSphereZ() > zPos)//z coord is out of bound
                {
                sphereVec[i].setSphere(sphereObj.getSphereX(), sphereObj.getSphereY(), zPos, sphereObj.getSphereRad() );
                }
                if (sphereObj.getSphereZ() < zNeg)
                {
                sphereVec[i].setSphere(sphereObj.getSphereX(), sphereObj.getSphereY(), zNeg, sphereObj.getSphereRad() );
                }
        }//sphere
        if(debug)
        {
        //fprintf(f, "Currenty Sphere coordinate, X= %f,  Y= %f, Z= %f.\n", sphereObj.getSphereX(),sphereObj.getSphereY(), sphereObj.getSphereZ() );
        //fprintf(f, "IN SPHERE::::  The boundary varibles: xPos= %f, xNeg= %f, yPos= %f, yNeg= %f, zPos= %f, zNeg %f.\n",             xPos, xNeg, yPos, yNeg, zPos, zNeg);
        }


//move to the reference point
        //draw the rotation point
        glLoadIdentity();
        glTranslatef(sphereObj.getRPX(), sphereObj.getRPY(), sphereObj.getRPZ());
        glGetFloatv(GL_MODELVIEW_MATRIX, movePointM);

                GLfloat tempx, tempy, tempz;
                tempx = sphereObj.getSphereX() - sphereObj.getRPX();
                tempy = sphereObj.getSphereY() - sphereObj.getRPY();
                tempz = sphereObj.getSphereZ() - sphereObj.getRPZ();
                //distant from the distant point to the object in the xz plane
                GLfloat dist = sqrt(tempx*tempx + tempz * tempz);
                GLfloat dist2 = sqrt(tempz*tempz + tempy*tempy);
                if (dist == 0) dist = 0.000000001;
                if (dist2 == 0) dist2 = 0.000000001;

                //toMove is the distanct of the object from its rotate point on the x-axis of the object vie.
                GLfloat toMove = sqrt(tempx*tempx + tempy*tempy + tempz*tempz);

                //the angel that is going to rotate the xz plane around the y axis
                GLfloat  ang, angy;

                if (tempz == 0)
                {
                        ang = 0;
                }
                else
                {
                        //in xz plane, negative z face up 2 quadrane
                        tempz = 0-tempz;
                        //rotate the object around rotate point's y axis to makes it reach the z coordiante
                        ang = asin(tempz/dist);
                }
                        if (dist == 0)  dist+= 0.000001;
                        if (toMove == 0) toMove += 0.000001;

                if(debug) fprintf(f, "GLOBAL: %s.\n", globalError);
     if(debug) fprintf(f, "tempz is %f, tempy is %f, tempx is %f.\n", tempz, tempy, tempx);
        if(debug) fprintf(f, " dist is %f, dist2 is %f, toMove is %f.\n", dist, dist2, toMove);
if(debug && (((tempz/dist)>1 || (tempz/dist)<-1  ) ||  (tempy/toMove>1 || tempy/toMove<-1 )) )
{
        fprintf(f, "DOMAIN ERROR.\n");
}

                        //if (tempz<=0)   ang+=180.0f;
                        //rotate the object around rotate point's z axis to make it reach the y coordinate
                        angy = asin(tempy/toMove);
                        //change angle from radian to degrees
                        ang = ang*180/M_PI;
                        angy = angy*180/M_PI;

                        //find out original z
                        GLfloat oldTempz = 0 - tempz;
                        //if x is negative
                        if ( tempx<0)
                        {
                           ang = asin(oldTempz/dist);
                           ang = ang*180/M_PI;
                           angy = 0-angy;
                        }

          

                //draw the orbiting object
                //rotate with respect to the reference point
                GLfloat XYang = sphereObj.getRotPointAngleXY();
                GLfloat rotDirection = sphereObj.getRotPointAngleYZ();


                //move to the disingated coordiate
                glLoadIdentity();
                glRotatef(ang, 0,1, 0);
                glGetFloatv(GL_MODELVIEW_MATRIX, rotate1);

                glLoadIdentity();
                glRotatef(angy, 0,0,1);
                glGetFloatv(GL_MODELVIEW_MATRIX, rotate2);

                //rotatae XYang angles around the distant point in rotDirection angle direction
                glLoadIdentity();
                glRotatef(rotDirection,  1,0,0);
                glGetFloatv(GL_MODELVIEW_MATRIX, rotate3);

                glLoadIdentity();
                glRotatef(XYang, 0,0, 1);
                glGetFloatv(GL_MODELVIEW_MATRIX, rotate4);
/******************Calculating the new object coordinate***********************/
//total second reotation angle
GLfloat secRotAng = XYang + angy;
GLfloat tempBase = toMove*cos((double)secRotAng);
GLfloat newy = toMove*sin((double)secRotAng);
GLfloat newx = tempBase*cos((double)ang);
GLfloat newz = tempBase*sin((double)ang);




//move to the objects' midpoints (coordinate) by calculating the object's relative position
//the z coordinate is 0 because the coordinate alredy rotate for rotation purpose
        glLoadIdentity();
        if (tempx<0) toMove = 0-toMove;
        glTranslatef(toMove, 0, 0);
        glGetFloatv(GL_MODELVIEW_MATRIX, translatef);

//rotate back to original position. So the object wouldn't look tilted
        glLoadIdentity();
        glRotatef((0-XYang), 0,0,1);
        glRotatef((0-rotDirection), 1,0,0);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate5);

        glLoadIdentity();
        glRotatef((0-angy), 0,0, 1);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate6);

        glLoadIdentity();
        glRotatef((0-ang), 0,1,0);
        glGetFloatv(GL_MODELVIEW_MATRIX, rotate7);


//rotate the object with respect to its mid point
        if (sphereObj.getRotAngX()>=360)
        sphereObj.setRotAngX(sphereObj.getRotAngX()-360);
        if (sphereObj.getRotAngY()>=360)
        sphereObj.setRotAngY(sphereObj.getRotAngY()-360);
        if (sphereObj.getRotAngZ()>=360)
        sphereObj.setRotAngZ(sphereObj.getRotAngZ()-360);
                //rotate with respect to x axis
                glLoadIdentity();
                glRotatef(sphereObj.getRotAngX(), 1.0f, 0.0f, 0.0f);
                glGetFloatv(GL_MODELVIEW_MATRIX, selfRot1);
                //rotate with respect to y axis
                glLoadIdentity();
                glRotatef(sphereObj.getRotAngY(), 0.0f, 1.0f, 0.0f);
                glGetFloatv(GL_MODELVIEW_MATRIX, selfRot2);
                //rotate with respect to z axis
                glLoadIdentity();
                glRotatef(sphereObj.getRotAngZ(), 0.0f, 0.0f, 1.0f);
                glGetFloatv(GL_MODELVIEW_MATRIX, selfRot3);




if(debug) fprintf(f, "rotate itself x is %f, y is %f, z is %f degrees.\n", sphereObj.getRotAngX(), sphereObj.getRotAngY(), sphereObj.getRotAngZ());



/****************************ACTURAL DRAWING***************************/
        glLoadIdentity();
        //CAMERAgl
        glMultMatrixf(posAfterCamera);
        //MOVEMENT AND ROTATION
        glMultMatrixf(movePointM);
        glMultMatrixf(rotate1);
        glMultMatrixf(rotate2);
        glMultMatrixf(rotate3);
        glMultMatrixf(rotate4);
        glMultMatrixf(translatef);             
        glMultMatrixf(rotate5);
        glMultMatrixf(rotate6);
        glMultMatrixf(rotate7);
        glMultMatrixf(selfRot1);
        glMultMatrixf(selfRot2);
        glMultMatrixf(selfRot3);
//LOAD TEXTURE
                if (sphereObj.getTextureData() != NULL )       //check to see if texture date loaded successfully
                {
                        if (debug) fprintf(f, "printing texture to the SPHERE.\n");
                        glEnable(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D, sphereObj.getTextureData() );
                }
                else
                {
                        glDisable(GL_TEXTURE_2D);
                        if (debug) fprintf(f, "SPHERE has no texture.\n");
                }

                //transparency and brightness
                GLfloat sphereBri = (GLfloat)sphereObj.getBrightness()/255;
                if (sphereBri>= 1)      sphereBri=1;
                GLfloat sphereTrans = (GLfloat)sphereObj.getTransparency()/255;
                if (sphereTrans>=1)     sphereTrans = 1;

    	        glColor4f(sphereObj.getColorR()*sphereBri, sphereObj.getColorG()*sphereBri, sphereObj.getColorB()*sphereBri, sphereTrans);
                glEnable(GL_BLEND);		// Turn Blending On
	        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Blending Function For Translucency Based On Source Alpha Value ( NEW )

                //if (debug) fprintf(f, "sphereBri is %f and sphereTrans is %f, sphere color is %f, %f, %f", sphereBri, sphereTrans, sphereObj.getColorR(), sphereObj.getColorG(), sphereObj.getColorB());

        if(debug ) {QueryPerformanceCounter(&prectime1);}

              gluSphere(q, sphereObj.getSphereRad(), 100, 100);// Draw  Sphere

        if(debug)
        {
        QueryPerformanceCounter(&prectime2);
        timeinms= ( (double)prectime2.QuadPart-(double)prectime1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
        fprintf(g, "%e\n", timeinms);
        }
                
                //disable the transparency
                glDisable(GL_BLEND);		// Turn Blending Off
                glDisable(GL_TEXTURE_2D);       // Turn the 2D texture off


/****************************ACTURAL DRAWING END **********************/

/*GLfloat afMatrix[16];
glGetFloatv(GL_MODELVIEW_MATRIX,afMatrix);
        /******TEST POSITION SHAPE***
        glLoadIdentity();
        glTranslatef(afMatrix[12], afMatrix[13], afMatrix[14]);
        glBegin(GL_QUADS);
                glVertex3f(0,0,0);
                glVertex3f(0,1,0);
                glVertex3f(1,1,0);
                glVertex3f(1,0,0);
        glEnd();
       */ /******TEST POSITION SHAPE END*****/

//posoition after the rotation
GLfloat posAfterRot[16];


glLoadIdentity();
        glMultMatrixf(movePointM);
        glMultMatrixf(rotate1);
        glMultMatrixf(rotate2);
        glMultMatrixf(rotate3);
        glMultMatrixf(rotate4);
        glMultMatrixf(translatef);
        glMultMatrixf(rotate5);
        glMultMatrixf(rotate6);
        glMultMatrixf(rotate7);
        glMultMatrixf(selfRot1);
        glMultMatrixf(selfRot2);
        glMultMatrixf(selfRot3);
        glGetFloatv(GL_MODELVIEW_MATRIX,posAfterRot);
        if(debug) fprintf(f, "posAfterRot x is %f, y is %f, z is %f degrees.\n", posAfterRot[12], posAfterRot[13], posAfterRot[14]);
glLoadIdentity();

sphereVec[i].setcoordNewX(posAfterRot[12]);
sphereVec[i].setcoordNewY(posAfterRot[13]);
sphereVec[i].setcoordNewZ(posAfterRot[14]);
sphereVec[i].setRotPointAngleXY(0);
//sphereVec[i].setRotPointAngleYZ(0);
        }//if it is a sphere

        gluDeleteQuadric(q);    //free the memory


        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in BuildSphere() function.\n";
                 try
                 {
                        throw Exception("Error in BuildSphere() function");

                 }
                 catch (Exception &exception)
                 {
                         //Application->ShowException(&exception);
                 }
        }//catch


}//BuildSphere



/*
Function Name:  rotateByMidpoint
Rotate the object with respect to its midpoint's axis
@Parameter
        @typeIndex:
                1       --      Sphere
                2       --      Cuboids
        @index          --      The index of the object on the object vector
        @axis:
                1       --      x axis
                2       --      y axis
                3       --      z axis
        @degree         --      the degrees that object going to rotate
        @speed          --      # of frame it takes the object to rotate
                                minimum is 1, maximum is 100
*/
GLvoid rotateByMidpoint (int typeIndex, int index, int axis, float degree, int speed)
{
try
{
        //Time measurement for debug prupose
        LARGE_INTEGER   S1, S2, prectimebase, overhead, aa, bb;
        QueryPerformanceCounter(&S1);
        QueryPerformanceCounter(&S2);
        overhead.QuadPart = prectime2.QuadPart - prectime1.QuadPart;
        QueryPerformanceFrequency(&prectimebase);
        double timeinms;

        
        if (speed<1)    speed =1;
        if (speed>100)  speed =100;
        if (typeIndex == 1)     //sphere
        {
        for (int i=1; i<=speed; i++)
        {
                //calcualting the incremented new x, y, z coordinate
                GLfloat angle = degree/speed;

                //set the rotation degree, i.e. how many degrees would the object rotatoin around the point.
                if (axis == 1)
                {
                sphereVec[index].setRotAngX( sphereVec[index].getRotAngX()+angle);
                }
                else if (axis == 2)
                {
                sphereVec[index].setRotAngY( sphereVec[index].getRotAngY()+angle);
                }
                else if (axis == 3)
                {
                sphereVec[index].setRotAngZ( sphereVec[index].getRotAngZ()+angle);
                }
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/FRAMERATE;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}
        }//for
        }//if it is a sphere

        if (typeIndex == 2)     //cuboids
        {
        for (int i=1; i<=speed; i++)
        {
                //calcualting the incremented new x, y, z coordinate
                GLfloat angle = degree/speed;

                //set the rotation degree, i.e. how many degrees would the object rotatoin around the point.
                if (axis == 1)
                {
                cuboidsVec[index].setRotAngX( cuboidsVec[index].getRotAngX()+angle);
                }
                else if (axis == 2)
                {
                cuboidsVec[index].setRotAngY( cuboidsVec[index].getRotAngY()+angle);
                }
                else if (axis == 3)
                {
                cuboidsVec[index].setRotAngZ( cuboidsVec[index].getRotAngZ()+angle);
                }
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/FRAMERATE;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}
        }//for
        }//if it is a cuboids

        if (typeIndex == 3)     //3ds file object
        {
        for (int i=1; i<=speed; i++)
        {
                //calcualting the incremented new x, y, z coordinate
                GLfloat angle = degree/speed;

                //set the rotation degree, i.e. how many degrees would the object rotatoin around the point.
                if (axis == 1)
                {
                model3dList[index].xaxisAngle += angle;
                }
                else if (axis == 2)
                {
                model3dList[index].yaxisAngle += angle;
                }
                else if (axis == 3)
                {
                model3dList[index].zaxisAngle += angle;
                }
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/FRAMERATE;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}
        }//for
        }//if it is a cuboids


                }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in BuildSphere() function.\n";
                 try
                 {
                        throw Exception("Error in BuildSphere() function");

                 }
                 catch (Exception &exception)
                 {
                         //Application->ShowException(&exception);
                 }
        }//catch

}//rotateByMidpoint


/*
Function Name:  rotateObjByPoint
Rotate the object by the specific degree with respect to a distant point
         @typeVec       --      The inter indicates what's the object type
                1       --      Sphere
                2       --      cuboids
         @index                 The object index in its appropriate object vector
         @rpX           --       reference poing X coordiante
         @rpY           --      reference poing Y coordinate
         @rpZ           --       reference poing Z coordinate
         @speed         --      the speed that object would move toward its new location.
                        speed range is 1 to 10, 1 is fastest...100 is slowest
                        1 implies movement will complete in 1 frame
                        100 implies movement will complete in 50 frames
         @angXY         --      the angle that the object rotate around the obejct's z axis,
                                this indicates how much the object rotate aroudn the point.
         @direction     --      the angle that the object rotate around the x axis to set the rotation direction,
                                this indicates which path the object is rotate around the point.
Purpose&Notes:
        This function rotate the object with respect to a distant point.
        When the function ended, the object will store it's newest position.

*/
GLvoid rotateObjByPoint(int typeVec, int index, GLfloat rpX, GLfloat rpY, GLfloat rpZ, int speed, GLfloat angXY, GLfloat direction)
{

try
{
        //Time measurement for debug prupose
        LARGE_INTEGER   S1, S2, prectimebase, overhead, aa, bb;
        QueryPerformanceCounter(&S1);
        QueryPerformanceCounter(&S2);
        overhead.QuadPart = prectime2.QuadPart - prectime1.QuadPart;
        QueryPerformanceFrequency(&prectimebase);
        double timeinms;
        
        if (speed > 100) speed = 100;
        if (speed < 1 ) speed = 1;

        if(typeVec == 1)
        {
                for (int i=1; i<=speed; i++)
                {       //MODIFICATION NEEDED
                        //calcualting the incremented new x, y, z coordinate
                        GLfloat angle = angXY/speed*i;

if (debug) fprintf(f, "BEFORE THE ROTATE CALLED, THE OLD COORDINATE OF SPHERE %d IS %f, %f, %f. \n", index, sphereVec[index].getSphereX(), sphereVec[index].getSphereY(), sphereVec[index].getSphereZ());
if (debug) fprintf(f, "Rotate direction is %f, rotate angle is %f, rotate point is, %f,%f,%f.\n", direction, angle, rpX, rpY, rpZ);

                        //set the rotation degree, i.e. how many degrees would the object rotatoin around the point.
                        sphereVec[index].setRotPointAngleXY(angle);
                        //set the rotational direction
                        sphereVec[index].setRotPointAngleYZ(direction);
                        sphereVec[index].setRotPointPosition(rpX, rpY, rpZ);
                        //sphereVec[index].setSphere(tempx, tempy, tempz, sphereVec[index].getSphereRad());
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/FRAMERATE;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}
              }//for
sphereVec[index].setSphere(sphereVec[index].getcoordNewX(), sphereVec[index].getcoordNewY(), sphereVec[index].getcoordNewZ(), sphereVec[index].getSphereRad() );
sphereVec[index].setRotPointAngleXY(0);                        //reset the rotation degree

          }//sphere rotate
          if (typeVec == 2)       //cuboids
          {
                GLfloat oldX, oldY, oldZ;
                oldX = cuboidsVec[index].getCuboidX();
                oldY = cuboidsVec[index].getCuboidY();
                oldZ = cuboidsVec[index].getCuboidZ();
if (debug) fprintf(f, "BEFORE THE ROTATE CALLED, THE OLD COORDINATE OF SPHERE %d IS %f, %f, %f. \n", index,oldX, oldY, oldZ);
                for (int i=1; i<=speed; i++)
                {
                        //calcualting the incremented new x, y, z coordinate
                        GLfloat angle = angXY/speed*i;
                        cuboidsVec[index].setRotPointAngleXY(angle);
                        cuboidsVec[index].setRotPointAngleYZ(direction);
                        cuboidsVec[index].setRotPointPosition(rpX, rpY, rpZ);
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/FRAMERATE;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}
                }//for
cuboidsVec[index].setCuboid(cuboidsVec[index].getcoordNewX(), cuboidsVec[index].getcoordNewY(), cuboidsVec[index].getcoordNewZ(), cuboidsVec[index].getCuboidW(), cuboidsVec[index].getCuboidH(), cuboidsVec[index].getCuboidD()  );
cuboidsVec[index].setRotPointAngleXY(0);


        }//if cuboids
          if (typeVec == 3)       //3ds file object
          {
                GLfloat oldX, oldY, oldZ;
                oldX = model3dList[index].X;
                oldY = model3dList[index].Y;
                oldZ = model3dList[index].Z;
                for (int i=1; i<=speed; i++)
                {
                        //calcualting the incremented new x, y, z coordinate
                        GLfloat angle = angXY/speed*i;
                        model3dList[index].rotPointAngleXY=angle;
                        model3dList[index].rotPointAngleYZ=direction;
                        model3dList[index].rotPointX=rpX;
                        model3dList[index].rotPointY=rpY;
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/FRAMERATE;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}
                }//for
model3dList[index].X=model3dList[index].NewX;
model3dList[index].Y=model3dList[index].NewY;
model3dList[index].Z=model3dList[index].NewZ;
model3dList[index].rotPointAngleXY=0;


        }//if cuboids

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in rotateObjByPoint() function.\n";
                 try
                 {
                        throw Exception("Error in rotateObjByPoint() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch


}//rotateObjByPoint





void checkCamera()
{
        if (keys[VK_PRIOR])
        {
	        z -= 0.5f;
	}
	if (keys[VK_NEXT])
	{
		z += 0.5f;
	}
        if (keys[VK_LEFT])
        {
                x -= 0.5f;
        }
        if (keys[VK_RIGHT])
        {
                x += 0.5f;
        }
        if (keys[VK_UP])
        {
                y += 0.5f;
        }
        if (keys[VK_DOWN])
        {
                y -= 0.5f;
        }
}//checkCamera

/*
Function Name:  moveObj
Move the selected object to the new XYZ coordinate by certain speed.
        @typeVec   --   The inter indicates what's the object type
                1  --   Sphere
                2  --   Cuboids
                3  --   3ds file object
        @inidex    --   The object index in its appropriate object vector
        @newX      --   new X coordiante
        @newY      --   new Y coordinate
        @newZ      --   new Z coordinate
        @speed     --   the speed that object would move toward its new location.
                        speed range is 1 to 10, 1 is fastest...50 is slowest
                        1 implies movement will complete in 1 frame
                        50 implies movement will complete in 50 frames
*/
GLvoid moveObj (int typeVec, int index, GLfloat newX, GLfloat newY, GLfloat newZ, GLfloat speed )
{

try
{
        if (speed > 50) speed = 50;
        if (speed < 1 ) speed = 1;

        //Time measurement for debug prupose
        LARGE_INTEGER   S1, S2, prectimebase, overhead, aa, bb;
        QueryPerformanceCounter(&S1);
        QueryPerformanceCounter(&S2);
        overhead.QuadPart = prectime2.QuadPart - prectime1.QuadPart;
        QueryPerformanceFrequency(&prectimebase);
        double timeinms;

        if (typeVec == 1)       //sphere
        {
                GLfloat oldX, oldY, oldZ;
                GLfloat tempx, tempy, tempz;
                oldX = sphereVec[index].getSphereX();
                oldY = sphereVec[index].getSphereY();
                oldZ = sphereVec[index].getSphereZ();
                for (int i=1; i<=speed; i++)
                {

                        //difference in x / speed * i + oldX
                        tempx = ( newX - oldX )/speed * i + oldX;
                        tempy = ( newY - oldY )/speed * i + oldY;
                        tempz = ( newZ - oldZ )/speed * i + oldZ;
                        sphereVec[index].setSphere(tempx, tempy, tempz, sphereVec[index].getSphereRad());

checkCamera();
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/FRAMERATE;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}
                }//for
        }
        if (typeVec == 2)       //cuboids
        {
                GLfloat oldX, oldY, oldZ;
                GLfloat tempx, tempy, tempz;
                oldX = cuboidsVec[index].getCuboidX();
                oldY = cuboidsVec[index].getCuboidY();
                oldZ = cuboidsVec[index].getCuboidZ();
                for (int i=1; i<=speed; i++)
                {
                        //difference in x / speed * i + oldX
                        tempx = ( newX -  oldX)/speed * i + oldX;
                        tempy = ( newY -  oldY)/speed * i + oldY;
                        tempz = ( newZ -  oldZ)/speed * i + oldZ;
                        cuboidsVec[index].setCuboid(tempx, tempy, tempz, cuboidsVec[index].getCuboidW(), cuboidsVec[index].getCuboidH(), cuboidsVec[index].getCuboidD());
                        if(debug) fprintf(f, "The current position of the moving cuboids are: %f, %f, %f.\n", tempx, tempy, tempz);
checkCamera();
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/FRAMERATE;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}
                }//for
        }
        if (typeVec == 3)       //3ds file object
        {
                GLfloat oldX, oldY, oldZ;
                GLfloat tempx, tempy, tempz;
                oldX = model3dList[index].X;
                oldY = model3dList[index].Y;
                oldZ = model3dList[index].Z;
               // speed = 15;
                for (int i=1; i<=speed; i++)
                {
                        //difference in x / speed * i + oldX
                        tempx = ( newX -  oldX)/speed * i + oldX;
                        tempy = ( newY -  oldY)/speed * i + oldY;
                        tempz = ( newZ -  oldZ)/speed * i + oldZ;
                        model3dList[index].X = tempx;
                        model3dList[index].Y = tempy;
                        model3dList[index].Z = tempz;
                        if(debug)fprintf(f, "CURRENT OBJECT POSITION IS: X- %f, Y- %f, Z- %f.\n", tempx, tempy, tempz);
checkCamera();
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/FRAMERATE;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}
                }//for
        }//move 3ds file object

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in moveObj() function.\n";
                 try
                 {
                        throw Exception("Error in moveObj() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch


}//moveObj



/*
The function Resize and initialize the GL window
*/
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
try
{
        if (height == 0)                        // Prevent a divide by zero by
        {
                height = 1;                     // Making height equal One
        }

        glViewport(0, 0, width, height);        // Reset the current viewport

        glMatrixMode(GL_PROJECTION);            // Select the projection matrix
	glLoadIdentity();                       // Reset the projection matrix

	// Calculate the aspect ratio of the window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,1000.0f);

	glMatrixMode(GL_MODELVIEW);             // Select the modelview matrix
	glLoadIdentity();                       // Reset the modelview matrix

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in ReSizeGLScene() function.\n";
                 try
                 {
                        throw Exception("Error in ReSizeGLScene() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch

}//ReSizeGLScreen


/*
This function does the initialization of the OpenGL screen
*/
int InitGL()
{
try
{
     // glEnable(GL_TEXTURE_2D);                // Enalbe texture mapping
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer

       	glShadeModel(GL_SMOOTH);                // Enable smooth shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);   // Black background
	glClearDepth(1.0f);                     // Depth buffer setup
	glEnable(GL_DEPTH_TEST);                // Enables depth testing
        glEnable(GL_TEXTURE_2D);                // Enable 2D texture
	glDepthFunc(GL_LEQUAL);                 // The type of depth testing to do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Really nice perspective calculations
        glShadeModel(GL_SMOOTH);

        
        //set up the light
        //white ambient light background
        if (debug)       fprintf(f, "getting light.\n");
        GLfloat abmBri = (GLfloat)camLig.getAmbLightBri()/255;
        GLfloat ligBri = (GLfloat)camLig.getLightBri()/255;
        if (abmBri >= 1)
                abmBri = 1.0f;
        if (ligBri >=1 )
                ligBri = 1.0f;
        if (debug)      fprintf(f, "abmBri is %f and ligBri is %f.\n", abmBri, ligBri);
        GLfloat abmLight[] ={abmBri*camLig.getLightColorX(), abmBri*camLig.getLightColorY(), abmBri*camLig.getLightColorZ(),  1.0f};
        GLfloat spec[] = {ligBri*camLig.getLightColorX(), ligBri*camLig.getLightColorY(), ligBri*camLig.getLightColorZ(), 1.0f};
        GLfloat lightPosition[] = {camLig.getLightX(), camLig.getLightY(),  camLig.getLightZ(), 0.0f};


        glLightfv(GL_LIGHT0, GL_AMBIENT, abmLight);		// Setup The Ambient Light
        glLightfv(GL_LIGHT0, GL_DIFFUSE, spec);		        // Setup The Specular Light
        glLightfv(GL_LIGHT0, GL_SPECULAR, spec);		// Setup The Specular Light
        glLightfv(GL_LIGHT0, GL_POSITION,lightPosition);	// Position The Lig

        //shade added
        glMaterialfv(GL_FRONT, GL_DIFFUSE, spec);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
        glMaterialfv(GL_FRONT, GL_AMBIENT, abmLight);
        glMaterialf(GL_FRONT, GL_SHININESS, 128.0);
        //added

        glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
	glEnable(GL_LIGHT0);                                    // Enable light one
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL );
        

        
        GetClientRect(hWnd, &WinDimension);					// Assign the windows rectangle to a global RECT
    
        //build the font
        int tSize = threeDTextVec.size();
        for (int i=0; i<tSize; i++)
        {
        GLuint temp = BuildFont(threeDTextVec[i].getFont(), threeDTextVec[i].getFontID(), threeDTextVec[i].getOldFont(), i);
        threeDTextVec[i].setFontID(temp);
        }


   
       //Sphere
        if (sphereVec.size() != 0 )
        {
                int tempSize = sphereVec.size();
                for (int i=0; i<tempSize; i++)
                {
                if(sphereVec[i].getHasTexture())
                {
                        if (createTexture( sphereVec[i].getTexture(), sphereVec[i].getTextureData(), i, sphereVec[i].getPrimitiveID() ) )
                        {       sphereVec[i].setHasTexture(true);  }
                        else
                        {       sphereVec[i].setHasTexture(false); }
                }//if
                }//for
        }

           //Cuboids
        if (cuboidsVec.size() != 0)
        {
                int tempSize = cuboidsVec.size();
                for (int i=0;  i<tempSize; i++)
                {
                if (cuboidsVec[i].getHasTexture())
                {
                        if(  createTexture( cuboidsVec[i].getTexture(), cuboidsVec[i].getTextureData(), i, cuboidsVec[i].getPrimitiveID() ) )
                        {       cuboidsVec[i].setHasTexture(true);   }
                        else
                        {       cuboidsVec[i].setHasTexture(false);    }
                }//if
                }//for
        }


        //Overlay
        if (overlay.getStatus() )
        {
                if(overlay.getPicTexture())
                {
                createTexture(overlay.getOverlayPicture(), overlay.getPictureData(), 1, 5 );
                }
                if(overlay.getTraTexture())
                {
                createTexture(overlay.getOverlayTrans(), overlay.getTransData(), 2, 5 );
                }
        }

        if(twoDTextVec.size() != 0)
        {
                int counter = twoDTextVec.size();
                for (int i=0; i<counter; i++)
                {
                if (debug) fprintf(f, "BUILDING THE FONT FOR 2D TEXT, font height is %d.\n", twoDTextVec[i].getTextSize());
                if (debug) fprintf(f, "B4 The fontID is %d.\n", twoDTextVec[i].getFontID());
twoDTextVec[i].setFontID( Build2DFont( twoDTextVec[i].getTextFont(), twoDTextVec[i].getTextSize(), twoDTextVec[i].getOldFont() ) );
if(i==0)twoDTextVec[i].setFontID( Build2DFont( twoDTextVec[i].getTextFont(), twoDTextVec[i].getTextSize(), twoDTextVec[i].getOldFont() ) );

                if (debug) fprintf(f, "After The fontID is %d.\n", twoDTextVec[i].getFontID());
                }
        }//Initialze 2D text display list

        //LOAD 3DS FILE
        if (model3dList.size() != 0)
        {
                int s = model3dList.size();
                for (int i=0; i<s; i++)
                {
                        t3DModel& rvModel = model3dList[i].mod;
                        FILE_NAME = model3dList[i].filename3ds;
                        Load3dsFile(rvModel);
                }//for
        }




      /*  /*   */
        return true;

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in InitGL() function.\n";
                 try
                 {
                        throw Exception("Error in InitGL() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch
}//InitGL


/*
The is the primary function that does all the drawing in the OpenGL window
*/
int DrawScreen(GLvoid)
{
        try
        {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear screen and depth buffer
	glLoadIdentity();       // Reset the current modelview matrix



// calculate the current time
QueryPerformanceCounter(&prectime1);
QueryPerformanceCounter(&prectime2);
overhead.QuadPart = prectime2.QuadPart - prectime1.QuadPart;
QueryPerformanceFrequency(&prectimebase);
timeinms= ( (double)prectime2.QuadPart-(double)prectime1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;

    //draw the cuboids

        int Csize = cuboidsVec.size();
        for (int i=0; i<Csize; i++)
        {
        if(debug) {QueryPerformanceCounter(&prectime1);}
                BuildCuboid(cuboidsVec[i], i);
        if(debug)
        {
        QueryPerformanceCounter(&prectime2);
        timeinms= ( (double)prectime2.QuadPart-(double)prectime1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
        fprintf(g, "%e\n", timeinms);
        }
        }


        //draw the sphere
        fprintf(f, "drawing the sphere.\n");
        int Ssize = sphereVec.size();
        if(debug) fprintf(f, "The Ssize is %d.\n", Ssize);
        for (int i=0; i<Ssize; i++)
        {
                if(debug) QueryPerformanceCounter(&prectime1);
                BuildSphere(sphereVec[i], i);
                if(debug)
                {
                QueryPerformanceCounter(&prectime2);
                timeinms= ( (double)prectime2.QuadPart-(double)prectime1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
                fprintf(g, "%e\n", timeinms);
                }
        }
  /*  */


        //draw the 3D text
        int threeSize = threeDTextVec.size();
        if(debug) fprintf(f, "The threeSize is %d.\n", threeSize);
        for (int i=0; i<threeSize; i++)
        {
        if(debug) QueryPerformanceCounter(&prectime1);
                if(debug) fprintf(f, "threeDTextVec[i] is %d.\n", threeDTextVec[i].getStatus());
                Build3Dtext(threeDTextVec[i], i);
        if(debug)
        {
        QueryPerformanceCounter(&prectime2);
        timeinms= ( (double)prectime2.QuadPart-(double)prectime1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
        fprintf(g, "%e\n", timeinms);
        }
        }

        //draw the 3ds file
        int modelSize = model3dList.size();
        for(int i=0; i<modelSize; i++)
        {
                if(debug) fprintf(f, "In draw screen,  %d time draw 3ds file.\n", i);

                model3Dclass& temp = model3dList[i];
       if(debug)fprintf(f, "IN DRAW screen CURRENT OBJECT POSITION IS: X- %f, Y- %f, Z- %f.\n", model3dList[i].X, model3dList[i].Y, model3dList[i].Z);

                if (model3dList[i].isOn)
                {
                Draw3ds(temp, i);
                }
        }//for

if(debug) QueryPerformanceCounter(&prectime1);
        if (overlay.getStatus())
            Build2DOverlay();
if(debug)
{
QueryPerformanceCounter(&prectime2);
timeinms= ( (double)prectime2.QuadPart-(double)prectime1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
fprintf(g, "%e\n", timeinms);
}

if(debug) QueryPerformanceCounter(&prectime1);
        if (overlay.getCursorSize() > 0)
        Build2DCursor();
if(debug)
{
QueryPerformanceCounter(&prectime2);
timeinms= ( (double)prectime2.QuadPart-(double)prectime1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
fprintf(g, "%e\n", timeinms);
}



if(debug) QueryPerformanceCounter(&prectime1);
        //build the 2D text on top of the overlay
        Build2Dtext();
if(debug)
{
QueryPerformanceCounter(&prectime2);
timeinms= ( (double)prectime2.QuadPart-(double)prectime1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
fprintf(g, "%e\n", timeinms);
}

/**/
        



        
        if (debug) fprintf(f, "LOOPING IN THE DRAW SCREEEN.\n");



        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();
        return false;
        }
        catch (...)
        {
                globalError = "Unknown error occur in DrawScreen() function.\n";
                 try
                 {
                        throw Exception("Error in DrawScreen() function.");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
	return true;            // Everything went OK
}//DrqwScreen



/*
This function properly kill the openGL window
*/
GLvoid KillGLWindow(GLvoid)
{
try
{

	if (fullscreen)         // Are we in fullscreen mode?
	{
		ChangeDisplaySettings(NULL,0);  // If so switch back to the desktop
		ShowCursor(true);               // Show mouse pointer
	}

	if (hRC)        // Do we have a rendering context?
	{
		if (!wglMakeCurrent(NULL,NULL))         // Are we able to release the DC and RC contexts?
		{
			MessageBox(NULL,"Release of DC and RC failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))             // Are we able to delete the RC?
		{
			MessageBox(NULL,"Release rendering context failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;             // Set RC to NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))        // Are we able to release the DC
	{
		MessageBox(NULL,"Release device context failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC = NULL;             // Set DC to NULL
	}

	if (hWnd && !DestroyWindow(hWnd))       // Are we able to destroy the window?
	{
		MessageBox(NULL,"Could not release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;            // Set hWnd to NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))       // Are we able to unregister class
	{
		MessageBox(NULL,"Could not unregister class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;       // Set hInstance to NULL
	}

        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in KillGLWindow() function.\n";
                 try
                 {
                        throw Exception("Error in KillGLWindow() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch
}//KillGLWindow


/*	This Code Creates Our OpenGL Window.  Parameters Are:
 *	title			- Title To Appear At The Top Of The Window
 *	width			- Width Of The GL Window Or Fullscreen Mode
 *	height			- Height Of The GL Window Or Fullscreen Mode
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)
 *      positionX               - X position of the window
 *      positionY               - Y position of the window
 *	fullscreenflag	- Use Fullscreen Mode (true) Or Windowed Mode (false)
 */
bool CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag, int positionX, int positionY)
{
try
{
	GLuint		PixelFormat;		// Holds the results after searching for a match
	WNDCLASS	wc;		        // Windows class structure
	DWORD		dwExStyle;              // Window extended style
	DWORD		dwStyle;                // Window style
	RECT		WindowRect;             // Grabs rctangle upper left / lower right values
	WindowRect.left = (long)0;              // Set left value to 0
	WindowRect.right = (long)width;		// Set right value to requested width
	WindowRect.top = (long)0;               // Set top value to 0
	WindowRect.bottom = (long)height;       // Set bottom value to requested height

	fullscreen = fullscreenflag;              // Set the global fullscreen flag

	hInstance               = GetModuleHandle(NULL);		// Grab an instance for our window
	wc.style                = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;   // Redraw on size, and own DC for window
	wc.lpfnWndProc          = (WNDPROC) WndProc;			// WndProc handles messages
	wc.cbClsExtra           = 0;					// No extra window data
	wc.cbWndExtra           = 0;					// No extra window data
	wc.hInstance            = hInstance;				// Set the Instance
	wc.hIcon                = LoadIcon(NULL, IDI_WINLOGO);		// Load the default icon
	wc.hCursor              = LoadCursor(NULL, IDC_ARROW);		// Load the arrow pointer
	wc.hbrBackground        = NULL;					// No background required for GL
	wc.lpszMenuName		= NULL;					// We don't want a menu
	wc.lpszClassName	= "OpenGL";				// Set the class name

	if (!RegisterClass(&wc))					// Attempt to register the window class
	{
		MessageBox(NULL,"Failed to register the window class.","Error",MB_OK|MB_ICONEXCLAMATION);

		return false;   // Return false
	}

	if (fullscreen)         // Attempt fullscreen mode?
	{
		DEVMODE dmScreenSettings;                                       // Device mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	        // Makes sure memory's cleared
		dmScreenSettings.dmSize         = sizeof(dmScreenSettings);     // Size of the devmode structure
		dmScreenSettings.dmPelsWidth	= width;                        // Selected screen width
		dmScreenSettings.dmPelsHeight	= height;                       // Selected screen height
		dmScreenSettings.dmBitsPerPel	= bits;	                        // Selected bits per pixel
		dmScreenSettings.dmFields       = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try to set selected mode and get results. NOTE: CDS_FULLSCREEN gets rid of start bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If the mode fails, offer two options. Quit or use windowed mode.
			if (MessageBox(NULL,"The requested fullscreen mode is not supported by\nyour video card. Use windowed mode instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = false;       // Windowed mode selected. Fullscreen = false
			}
			else
			{
				// Pop up a message box letting user know the program is closing.
				MessageBox(NULL,"Program will now close.","ERROR",MB_OK|MB_ICONSTOP);
				return false;           // Return false
			}
		}
	}

	if (fullscreen)                         // Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;    // Window extended style
		dwStyle = WS_POPUP;		// Windows style
		ShowCursor(true);		// Hide mouse pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;           // Window extended style
		dwStyle = WS_OVERLAPPEDWINDOW;                            // Windows style
	}

	AdjustWindowRectEx(&WindowRect,dwStyle,false,dwExStyle);        // Adjust window to true requested size

	// Create the window
	if (!(hWnd = CreateWindowEx(dwExStyle,          // Extended Style For The Window
                "OpenGL",				// Class name
		title,					// Window title
		dwStyle |				// Defined window style
		WS_CLIPSIBLINGS |			// Required window style
		WS_CLIPCHILDREN,			// Required window style
		positionX, positionY,			// Window position
		WindowRect.right-WindowRect.left,	// Calculate window width
		WindowRect.bottom-WindowRect.top,	// Calculate window height
		NULL,					// No parent window
		NULL,					// No menu
		hInstance,				// Instance
		NULL)))					// Dont pass anything to WM_CREATE
	{
		KillGLWindow();                         // Reset the display
		MessageBox(NULL,"Window creation error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;                           // Return false
	}

	static	PIXELFORMATDESCRIPTOR pfd =             // pfd tells windows how we want things to be
	{
		sizeof(PIXELFORMATDESCRIPTOR),          // Size of this pixel format descriptor
		1,					// Version number
		PFD_DRAW_TO_WINDOW |			// Format must support window
		PFD_SUPPORT_OPENGL |			// Format must support OpenGL
		PFD_DOUBLEBUFFER,			// Must support double buffering
		PFD_TYPE_RGBA,				// Request an RGBA format
		bits,					// Select our color depth
		0, 0, 0, 0, 0, 0,			// Color bits ignored
		0,					// No alpha buffer
		0,					// Shift bit ignored
		0,					// No accumulation buffer
		0, 0, 0, 0,				// Accumulation bits ignored
		16,					// 16Bit Z-Buffer (Depth buffer)
		0,					// No stencil buffer
		0,					// No auxiliary buffer
		PFD_MAIN_PLANE,				// Main drawing layer
		0,					// Reserved
		0, 0, 0					// Layer masks ignored
	};

	if (!(hDC = GetDC(hWnd)))         // Did we get a device context?
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't create a GL device context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return false
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC,&pfd)))	// Did windows find a matching pixel format?
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't find a suitable pixelformat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return false
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))       // Are we able to set the pixel format?
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't set the pixelformat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return false
	}

	if (!(hRC = wglCreateContext(hDC)))               // Are we able to get a rendering context?
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't create a GL rendering context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return false
	}

	if(!wglMakeCurrent(hDC,hRC))    // Try to activate the rendering context
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't activate the GL rendering context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return false
	}

	ShowWindow(hWnd,SW_SHOW);       // Show the window
	SetForegroundWindow(hWnd);      // Slightly higher priority
	SetFocus(hWnd);                 // Sets keyboard focus to the window
	ReSizeGLScene(width, height);   // Set up our perspective GL screen

	if (!InitGL())                  // Initialize our newly created GL window
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Initialization failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return false
	}
        else                            //window created successfully
        {
                //set the window dimension
                GetClientRect(hWnd, &WinDimension);
        }
	return true;                    // Success

}//try

        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in CreateGLWindow() function.\n";
                try
                {
                       throw Exception("Error in CreateGLWindow() function");
                }
                catch (Exception &exception)
                {
                        Application->ShowException(&exception);
                }
        }//catch
}//CreateGLWindow

 /*  */
LRESULT CALLBACK WndProc(HWND hWnd,     // Handle for this window
                        UINT uMsg,      // Message for this window
			WPARAM wParam,  // Additional message information
			LPARAM lParam)  // Additional message information
{
	switch (uMsg)                           // Check for windows messages
	{
		case WM_ACTIVATE:               // Watch for window activate message
		{
			if (!HIWORD(wParam))    // Check minimization state
			{
				active = true;  // Program is active
			}
			else
			{
				active = false; // Program is no longer active
			}

			return 0;               // Return to the message loop
		}

		case WM_SYSCOMMAND:             // Intercept system commands
		{
			switch (wParam)         // Check system calls
			{
				case SC_SCREENSAVE:     // Screensaver trying to start?
				case SC_MONITORPOWER:	// Monitor trying to enter powersave?
				return 0;       // Prevent from happening
			}
			break;                  // Exit
		}

		case WM_CLOSE:                  // Did we receive a close message?
		{
			PostQuitMessage(0);     // Send a quit message
			return 0;               // Jump back
		}

		case WM_KEYDOWN:                // Is a key being held down?
		{
			keys[wParam] = true;    // If so, mark it as true
			return 0;               // Jump back
		}

		case WM_KEYUP:                  // Has a key been released?
		{
			keys[wParam] = false;   // If so, mark it as false
			return 0;               // Jump back
		}

		case WM_SIZE:                   // Resize the OpenGL window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord = Width, HiWord = Height
			return 0;               // Jump back
		}
	}

	// Pass all unhandled messages to DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);

}


          












