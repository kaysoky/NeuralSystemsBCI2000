//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Usr.h"
#include "UParameter.h"



//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TUser *User;

/*shidong starts*/
string borderTexture;
string targetTexture;
string cursorTexture;
int numTarg;            //number of total targets

/*shidong ends*/

//---------------------------------------------------------------------------
__fastcall TUser::TUser(TComponent* Owner)
        : TForm(Owner)
{


}
//--------------------------------------------------------------
_fastcall TUser::~TUser()
{
/*shidong starts*/

if(a)fclose(a);
if(a)fclose(f);
if(a)fclose(g);

/*shidong ends*/
User->Close();
       
}
//---------------------------------------------------------------------------

void SetUsr( PARAMLIST *plist, STATELIST *slist )
{
        char line[512];

        strcpy(line,"UsrTask int WinXpos= 400 0 0 1 // User Window X location");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"UsrTask int WinYpos= 5 0 0 1 // User Window Y location");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"UsrTask int WinWidth= 512 0 0 1 // User Window Width");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"UsrTask int WinHeight= 512 0 0 1 // User Window Height");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"UsrTask int CursorSize= 25 0 0 1 // User Window Cursor Size");
        plist->AddParameter2List(line,strlen(line));
}

//------------------------------------------------------------------------

void __fastcall TUser:: Initialize(PARAMLIST *plist, STATELIST *slist, AnsiString border, AnsiString target, AnsiString cursor, int totalTarg)
{

       Wx=  atoi(plist->GetParamPtr("WinXpos")->GetValue());
       Wy=  atoi(plist->GetParamPtr("WinYpos")->GetValue());
       Wxl= atoi(plist->GetParamPtr("WinWidth")->GetValue());
       Wyl= atoi(plist->GetParamPtr("WinHeight")->GetValue());
       CursorSize= atoi(plist->GetParamPtr("CursorSize")->GetValue());
       Cursor->Brush->Color= clBlack;
       
       User->ClientWidth=  Wxl;
       User->ClientHeight= Wyl;
       User->Left=         Wx;
       User->Top=          Wy;
       Cursor->Height=   CursorSize;
       Cursor->Width=    CursorSize;

       tT->Font->Height=-Wyl*3/4;
       Canvas->Font=tT->Font;
       tT->Left=abs(Wxl/2-Canvas->TextWidth(tT->Caption)/2);
       tT->Top=abs(tT->Font->Height/8);

       tO->Font->Height=-Wyl*3/4;
       Canvas->Font=tO->Font;
       tO->Left=abs(Wxl/2-Canvas->TextWidth(tO->Caption)/2);
       tO->Top=abs(tO->Font->Height/8);

       HalfCursorSize= CursorSize / 2;

       limit_top= 0; //CursorSize/2;
       limit_bottom= Wyl; //  - CursorSize/2;
       limit_left= 0; // HalfCursorSize;  //  0; CursorSize/2;
       limit_right= Wxl; // - HalfCursorSize; // - CursorSize/2;


       /*shidong starts*/      //User->Show();

        borderTexture = (border.c_str());
        targetTexture = (target.c_str());;
        cursorTexture = (cursor.c_str());;
     /*   */
       if(a)   {}
        else
        {        a = fopen("TaskLog.txt", "w");         }

        if(f)   {}
        else
        {       f = fopen("APIlog.txt" , "w");          }

        if(g)   {}
        else
        {       g = fopen("APIprocess.txt", "w");       }        

        if (a==NULL )bcierr << "Could not open TaskLog.txt for writing" << std::endl;
        if (f==NULL )bcierr << "Could not open APIlog.txt for writing" << std::endl;
        if (g==NULL )bcierr << "Could not open APIprocess.txt for writing" << std::endl;
      debug = true;


        if(debug) fprintf(a, "In Usr->Initialize.\n");
        numTarg=totalTarg;              //set total number of targets

        /*shidong ends*/

}
//----------------------------------------------------------------------------
//
//factors (x and y) to transform to normalized areas
//  ( 0x7fff ) y to bottom-top and  x to right-left
//
void TUser::Scale( float x, float y )
{
        if( x != 0.0 ) scalex= Wxl / x;  // was (limit_right-limit_left) / x;
        else scalex= 1.0;
        if( y != 0.0 ) scaley= Wyl / y;  // was (limit_bottom-limit_top) / y;
        else scaley= 1.0;
}

//-----------------------------------------------------------------------------
void TUser::GetLimits(float *right, float *left, float *top, float *bottom )
{
        *(right) = limit_right;
        *(left)  = limit_left;
        *(top)   = limit_top;
        *(bottom)= limit_bottom;
}

//-----------------------------------------------------------------------------
void TUser::GetSize( float *right, float *left, float *top, float *bottom )
{
        *(right)= Wxl;
        *(left)= 0;
        *(top)= 0;
        *(bottom)= Wyl;
}


//----------------------------------------------------------------------------
void TUser::setCameraLight(
                int CameraX,
                int CameraY,
                int CameraZ,
                int CameraAimX,
                int CameraAimY,
                int CameraAimZ,
                int LightSourceX,
                int LightSourceY,
                int LightSourceZ,
                int LightSourceColorR,
                int LightSourceColorG,
                int LightSourceColorB,
                int LightSourceIntensity)
{
                /*      Camera Setting    */

 if(debug)fprintf(a, "THEY ARE %d, %d, %d,%d, %d, %d,%d, %d, %d,%d, %d, %d,%d\n",CameraX,CameraY,CameraZ,CameraAimX,CameraAimY,CameraAimZ,LightSourceX,LightSourceY,LightSourceZ,LightSourceColorR,LightSourceColorB,LightSourceColorG,LightSourceIntensity );

                camLig.setCamAim((float)CameraAimX/32767*BORDER, (float)CameraAimY/32767*BORDER, (float)CameraAimZ/32767*BORDER);
                camLig.setCamViewPoint((float)CameraX/32767*BORDER, (float)CameraY/32767*BORDER, (float)CameraZ/32767*BORDER);
                camLig.setLight((float)LightSourceX/32767*BORDER, (float)LightSourceY/32767*BORDER, (float)LightSourceZ/32767*BORDER);
                camLig.setLightColor((float)LightSourceColorR/32767*BORDER, (float)LightSourceColorG/32767*BORDER, (float)LightSourceColorB/32767*BORDER);
                camLig.setLightBri((float)LightSourceIntensity);
                camLig.setAmbLightBri((float)LightSourceIntensity);
                //reset the camera control
                x = 0;
                y = 0;
                z = 0;

}








//----------------------------------------------------------------------------
void TUser::calculateCursorColor(float curPos, AnsiString frontColor, AnsiString backColor)
{
        /*
        R.G.B value ranges from 0~1.0, 0~1.0, 0~1.0
        max is 1,1,1 min is 0,0,0
        there for the range of the RGB value is 0 ~ 1000
         */
        AnsiString Fred, Fgreen, Fblue;
        AnsiString Bred, Bgreen, Bblue;

        Fred    = "0x" + frontColor.SubString(3,2);
        Fgreen  = "0x" + frontColor.SubString(5,2);
        Fblue   = "0x" + frontColor.SubString(7,2);
        Bred    = "0x" + backColor.SubString(3,2);
        Bgreen  = "0x" + backColor.SubString(5,2);
        Bblue   = "0x" + backColor.SubString(7,2);

if(debug)fprintf(a, "TaskLog->calculateCursorColro: FrontColor is %s and backcolor is %s.\n", frontColor, backColor);
if(debug)fprintf(a, "TaskLog->calculateCursorColor: fR, fB, fG, bR, bB, bG are %s, %s, %s, %s, %s, %s. \n", Fred, Fgreen, Fblue, Bred, Bgreen, Bblue);

        int Fr, Fg, Fb, Br, Bg, Bb;
        Fr = Fred.ToIntDef(0);
        Fg = Fgreen.ToIntDef(0);
        Fb = Fblue.ToIntDef(0);
        Br = Bred.ToIntDef(0);
        Bg = Bgreen.ToIntDef(0);
        Bb = Bblue.ToIntDef(0);

if(debug)fprintf(a, "TaskLog->calculateCursorColor: Fr, Fg, Fb, Br, Bg, Bb are %d, %d, %d, %d, %d, %d. \n", Fr, Fg, Fb, Br, Bg, Bb);

        int Rrange, Grange, Brange;     //the RGB range between front and back color
        Rrange = abs(Fr - Br);
        Grange = abs(Fg - Bg);
        Brange = abs(Fb - Bb);

        float curZpos = User->posZ;     //current cursor position
        curZpos += BORDER;              //make it all positive number
        float zPercent = curZpos / BORDER;      //current cursor position relativ to Border

if(debug)fprintf(a, "TaskLog->calculateCursorColor: cursor position is %f and zpercent is %f.\n", User->posZ,zPercent); 

        float RtoAdd, GtoAdd, BtoAdd;     //the RGB value that current cursor should have
        RtoAdd = Rrange * zPercent;
        GtoAdd = Grange * zPercent;
        BtoAdd = Brange * zPercent;

if(debug)fprintf(a, "TaskLog->calculateCursorColor: Rrange, Grange, Brange, RtoAdd, GtoAdd, BtoAdd are %d, %d, %d, %f, %f, %f. \n", Rrange, Grange, Brange, RtoAdd, GtoAdd, BtoAdd);

        //RGB value of the cursor should add to the lower range side(front or back) RGB value.
        if (Fr >= Br)
        {
                User->zR = (Br+RtoAdd)/255;
        }
        else
        {
                User->zR = (Fr+RtoAdd)/255;
        }

        if (Fg >= Bg)
        {
                User->zG = (Bg+GtoAdd)/255;
        }
        else
        {
                User->zG = (Fg+GtoAdd)/255;
        }

        if (Fb >= Bb)
        {
                User->zB = (Bb+BtoAdd)/255;
        }
        else
        {
                User->zB = (Fb+BtoAdd)/255;
        }


if(debug)fprintf(a, "TaskLog->calculateCursorColor: AFTER MODIFICATION User->zR is %f and User->zB is %f and User->zG is %f\n", User->zR, User->zB, User->zG);

        
         /*
        int colorValueF = (int)(1000*(float)fColorf);                 //default value 1000 -- white
        int colorValueB = (int)(1000*(float)bColorf);                 //default value 0 -- black
        if(debug)fprintf(a, "TaskLog->calculateCursorColor: colorValueF is %d and colorValueB is %d. \n", colorValueF, colorValueB);

        if (colorValueF == 1000) colorValueF = 999;
        if (colorValueB == 1000) colorValueB = 999;

        if(debug)fprintf(a, "TaskLog->calculateCursorColor: colorValueF is %d and colorValueB is %d. \n", colorValueF, colorValueB);

        int fR, fB, fG, bR, bB, bG;
        fR = colorValueF/100;
        fB = colorValueF/10;    fB = fB%10;
        fG = colorValueF%10;

        bR = colorValueB/100;
        bB = colorValueB/10;    bB = bB%10;
        bG = colorValueB%10;



if(debug)fprintf(a, "TaskLog->calculateCursorColor: fR, fB, fG, bR, bB, bG are %d, %d, %d, %d, %d, %d. \n", fR, fB, fG, bR, bB, bG);

        float curZpos = User->posZ;
        curZpos += BORDER;    //make it all positive number
        float zPercent = curZpos / BORDER;
if(debug)fprintf(a, "TaskLog->calculateCursorColor: curZpos %f. \n", curZpos);

        int colorRange = colorValueB - colorValueF;
if(debug)fprintf(a, "TaskLog->calculateCursorColor: colorRange %d. \n", colorRange);

        if (colorRange < 0) colorRange = 0-colorRange;
        int zColorValue = colorRange * zPercent;

if(debug)fprintf(a, "TaskLog->calculateCursorColor: zColorValue are %d. \n", zColorValue);

        int zR1, zB1, zG1;
        zR1 = zColorValue/100;
        zB1 = zColorValue/10;  zB1 = zB1 % 10;
        zG1 = zColorValue%10;


if(debug)fprintf(a, "TaskLog->calculateCursorColor: User->zR is %f and User->zB is %f and User->zG is %f\n", User->zR, User->zB, User->zG);

        User->zR = fR + zR1;
        User->zB = fB + zB1;
        User->zG = fG + zG1;

        User->zR = User->zR/10;
        User->zB = User->zB/10;
        User->zG = User->zG/10;


if(debug)fprintf(a, "TaskLog->calculateCursorColor: zR1 is %d and zB1 is %d and zG1 is %d\n", zR1, zB1, zG1);
if(debug)fprintf(a, "TaskLog->calculateCursorColor: User->zR is %f and User->zB is %f and User->zG is %f\n", User->zR, User->zB, User->zG);

        if (User->zB > 1)
        {
                User->zB -= 1;
                User->zR += 0.1;
        }
        if (User->zG > 1)
        {
                User->zG -= 1;
                User->zB += 0.1;
        }
if(debug)fprintf(a, "TaskLog->calculateCursorColor: AFTER MODIFICATION User->zR is %f and User->zB is %f and User->zG is %f\n", User->zR, User->zB, User->zG);
               */
}         


//----------------------------------------------------------------------------
void TUser::PutCursor(float *x, float *y, TColor color )
{
     /*                                       // what about x y reversal ???    */
        if( *y <= limit_top )    *y= limit_top;
        if( *y >= limit_bottom ) *y= limit_bottom;
        if( *x <= limit_left )   *x= limit_left;
        if( *x >= limit_right )  *x= limit_right;      

        Cursor->Top=  *y - HalfCursorSize;
        Cursor->Left= *x - HalfCursorSize;
        Cursor->Brush->Color= color;

        /*shidong starts*/
        if(debug)fprintf(a, "TaskLog->PutCursor: xpos is %f and ypos is %f.\n", *x, *y);

        float tempX = *x;
        float tempY = *y;

        tempX = tempX-256;
        tempY = 256-tempY;

        float cX, cY;
        cX = tempX/512*BORDER;
        cY = tempY/512*BORDER;

        if(*x == 256)  cX = 0;
        if(*y == 256)  cY = 0;

       if(debug)fprintf(a, "TaskLog->PutCursor: front color is %s and back color is %s.\n", cursorColorF, cursorColorB);

        if( cY <= -BORDER/2 )    cY = -BORDER/2;
        if( cY >= BORDER/2 ) cY = BORDER/2 ;
        if( cX <= -BORDER/2 )   cX= -BORDER/2;
        if( cX >= BORDER/2 )  cX= BORDER/2 ;
 
        calculateCursorColor(sphereVec[0].getSphereZ(), cursorColorF, cursorColorB);
        sphereVec[0].setColor(User->zR, User->zG, User->zB);

        /*z - 25 to make best view */
        sphereVec[0].setSphere(cX, cY, User->posZ, sphereVec[0].getSphereRad());
  if(debug)fprintf(a, "TaskLog->PutCursor: cX is %f and cY is %f and Z is %f.\n", cX, cY, sphereVec[0].getSphereZ());


        if (color != clBlack)                   //if curosor is not hiding
        {
        sphereVec[0].setStatus(true);          //enable cursor
        }
        else
        {
        sphereVec[0].setStatus(false);          //enable cursor
        User->posZ = startZ;
        }
        /*shidong ends*/
}


//----------------------------------------------------------------------------

/*shidong starts*/
/*
*       Function Name:  putT()
*       Parameter:      Tstate  --      boolean indicates whether the 3D screen is suspend or active
*       Return:         void
*/
/*shidong ends*/
void TUser::PutT(bool Tstate)
{
        if (Tstate)
        {
                Clear();
                tT->Visible=true;
                tO->Visible=false;
                /*shidong starts*/
                suspend();      //show the suspend 3D Screen
                glDisable(GL_LIGHTING);
                /*shidong ends*/
        }
        else
        {
                tT->Visible=false;
                /*shidong starts*/    
                resume();           //show the active 3D screen
                glEnable(GL_LIGHTING);
                /*shidong ends*/
        }
}

//----------------------------------------------------------------------------

void TUser::PutO( bool Tstate )
{
        if(Tstate)
        {
                Clear();
                tO->Visible=true;
        }
        else
                tO->Visible=false;
}                        

//----------------------------------------------------------------------------

void TUser::PutTarget(float x, float y, float sizex, float sizey, TColor color, int target )
{
        Target->Top=  y;
        Target->Left= x;
        Target->Height= sizey;
        Target->Width = sizex;
        Target->Brush->Color= color;
        /*shidong starts*/
        //color: clYellow       --      when target is hit by cursor
        //       clBlack        --      when target should be hided.
        if(color == clYellow)
        {
                cuboidsVec[target].setColor(0,1,1);
                if(sphereVec.size() != 0)
                {
                        if(debug)fprintf(a, "Collision testing, sphere's paramters are: %f, %f, %f, %f.\n", sphereVec[0].getSphereX(), sphereVec[0].getSphereY(),sphereVec[0].getSphereZ(), sphereVec[0].getSphereRad());
                }
                else
                {
                        if(debug)fprintf(a, "Sphere size is %d.\n", sphereVec.size());
                }
                if(cuboidsVec.size() != 0)
                {
                        if(debug)fprintf(a, "Collision testing, Cuboid's paramters are: cube %d, %f, %f, %f, %f, %f, %f.\n", target, cuboidsVec[target].getCuboidX(), cuboidsVec[target].getCuboidY(), cuboidsVec[target].getCuboidZ(), cuboidsVec[target].getCuboidW(), cuboidsVec[target].getCuboidH(), cuboidsVec[target].getCuboidD());
                }
                else
                {
                        if(debug)fprintf(a, "Cuboids size is %d.\n", cuboidsVec.size());
                }
        }
        if(color == clBlack)
        {
                cuboidsVec[target].setStatus(false);
        }
        if(color == clRed)
        {
                for (int i=0; i<=cuboidsVec.size(); i++)
                {
                cuboidsVec[i].setStatus(false);
                cuboidsVec[i].setColor(1,0,0);
                }
                cuboidsVec[0].setColor(1,1,1);
                cuboidsVec[target].setStatus(true); 
        }
        /*shidong ends*/
}

//-----------------------------------------------------------------------
void TUser::Clear( void )
{
        Target->Brush->Color= clBlack;
        Cursor->Brush->Color= clBlack;
        Refresh();
}

//--------------------------------------------------------------------
// Random # generator ran1() from Press et. al.
//   - Builder Random # function not good!

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

float TUser::ran1( long *idum )
{
        int j;
        long k;
        static long iy= 0;
        static long iv[NTAB];
        float temp;

        if(*idum <= 0 || !iy )
        {
                if(-(*idum) <1) *idum=1;
                else *idum= -(*idum);
                for(j=NTAB+7;j>=0;j--)
                {
                        k= (*idum)/IQ;
                        *idum= IA*(*idum-k*IQ)-IR*k;
                        if(*idum<0) *idum += IM;
                        if( j < NTAB ) iv[j]= *idum;
                 }
                 iy= iv[0];
         }
         k= (*idum)/IQ;
         *idum= IA*(*idum-k*IQ)-IR*k;
         if( *idum < 0 ) *idum += IM;
         j= iy/NDIV;
         iy= iv[j];
         iv[j]= *idum;
         if((temp=AM*iy) > RNMX) return RNMX;
         else return temp;
}

//----------------------------------------------------------------------------


/*
 * Function:    StartGLthread(void* null)
 * Parameter:   Null Pointer
 * Purpose:     This function crates a thread for openGL window.
 * Return:      N/A
 */
void startGLthread(void *null)
{

try
{
                //if (debug) fprintf(a, "In thread.\n");


                
                threeDText text;
                text.setCap("T");
                text.setColor(0.0f, 1.0f, 0.0f);
                text.setBrightness(255);
                text.setTransparency(255);
                text.setOrigin(-5,-5, BORDER/1.3);
                text.setDirection(0,-5,BORDER/1.3);
                text.setFontSize(270);
                text.setFont("Times New Roman");
                text.setStatus(true);
                

                threeDTextVec.push_back(text);
                threeDTextVec.back().setElementID(threeDTextVec.size()-1);
              //  if (debug) fprintf(a, "3DText: %d.\n", threeDTextVec.back().getElementID());







        MSG msg;                // Windows message structure
        bool done = false;      // bool variable to exit loop


        fullscreen = false;       // Windowed mode

	// Create our OpenGL window
	if (!CreateGLWindow("D3Box 3D Environment",User->WinHeight,User->WinWidth,16,fullscreen, User->WinXpos, User->WinYpos))
	{
		return;               // Quit if window was not created
	}

        //Time measurement for debug prupose
        LARGE_INTEGER   S1, S2, prectimebase, overhead, aa, bb;
        QueryPerformanceCounter(&S1);
        QueryPerformanceCounter(&S2);
        overhead.QuadPart = prectime2.QuadPart - prectime1.QuadPart;
        QueryPerformanceFrequency(&prectimebase);
        double timeinms;


        //D3 box variables
        bool show=true;
        int showCub;
        bool hitTarget = false;
        bool firstRun = true;
        User->posZ = 0;
        bool posZadd = false;           //boolean that use to track mouse button press
        bool posZminus = false;
        bool hp;
        bool light1;


        

        while(!done)                    // Loop that runs while done = false
	{
                if(firstRun)
                {
                        cuboidsVec[0].setStatus(false);
                }


		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is there a message waiting?
		{
			if (msg.message == WM_QUIT)             // Have we received a quit message?
			{
        		done = true;                    // If so done = true
                        //Free all the memories
                        KillFont();
                        Kill2DFont();
                        sphereVec.clear();						// Destroy The Font
                        cuboidsVec.clear();
                        threeDTextVec.clear();
			}
			else                                    // If not, deal with window messages
			{
				TranslateMessage(&msg);         // Translate the message
				DispatchMessage(&msg);          // Dispatch the message
			}

                        if (msg.message == WM_LBUTTONDOWN)      // if left button mouse is pressed
                        {
                                posZadd = true;
                        }
                        if (msg.message == WM_LBUTTONUP)
                        {
                                posZadd = false;
                        }
                        if (posZadd)
                        {
                                User->posZ += 0.2;
                                if(sphereVec.size() != 0)
                                sphereVec[0].setSphere(sphereVec[0].getSphereX(), sphereVec[0].getSphereY(), User->posZ, sphereVec[0].getSphereRad() );
                        }



                        if (msg.message == WM_RBUTTONDOWN)      // if right button mouse is pressed
                        {
                                posZminus = true;
                        }
                        if (msg.message == WM_RBUTTONUP)
                        {
                                posZminus = false;
                        }
                        if (posZminus)
                        {
                                User->posZ -= 0.2;
                                if(sphereVec.size() != 0)
                                sphereVec[0].setSphere(sphereVec[0].getSphereX(), sphereVec[0].getSphereY(), User->posZ, sphereVec[0].getSphereRad() );
                        }

                        if(sphereVec.size() != 0)
                        sphereVec[0].setSphere(sphereVec[0].getSphereX(), sphereVec[0].getSphereY(), User->posZ, sphereVec[0].getSphereRad() );

                        //if(debug) fprintf(a, "The z position is %f.\n", User->posZ);
                        //make sure posZ is within Boundary
                        if (User->posZ > 0) User->posZ = 0;
                        if (User->posZ < (0-BORDER)) User->posZ = (0-BORDER);
                }
		else            // If there are no messages
		{
                        if(sphereVec.size() != 0)
                        sphereVec[0].setSphere(sphereVec[0].getSphereX(), sphereVec[0].getSphereY(), User->posZ, sphereVec[0].getSphereRad() );

                       // if(debug) fprintf(a, "The z position is %f.\n", User->posZ);
                        //make sure posZ is within Boundary
                        if (User->posZ > 0) User->posZ = 0;
                        if (User->posZ < (0-BORDER)) User->posZ = (0-BORDER);

			// Draw the scene.  Watch for ESC key and quit messages from DrawScreen()
			if (active)                             // Program active?
			{
				if (keys[VK_ESCAPE])            // Was ESC pressed?
				{
                                done = true;            // ESC signalled a quit
                                //Free all the memories
                                KillFont();
                                Kill2DFont();
                                sphereVec.clear();
                                threeDTextVec.clear();
				}
				else                            // Not time to quit, Update screen
				{
//fream rate measurement
QueryPerformanceCounter(&S1);
				       DrawScreen();          // Draw the scene
                                       SwapBuffers(hDC);
QueryPerformanceCounter(&S2);                   
timeinms= ( (double)S2.QuadPart-(double)S1.QuadPart-(double)overhead.QuadPart )/(double)prectimebase.QuadPart*1000;
//if(debug)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/25;             //25 Hz, 40 ms per frame
//if(debug)fprintf(g, "%e\n", frameRate-timeinms);
//if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
if(timeinms<frameRate)
{
Sleep(frameRate-timeinms);
}



				}//else
			}//if

			if (keys[VK_F1])                        // Is F1 being pressed?
			{
				keys[VK_F1] = false;            // If so make key false
				KillGLWindow();                 // Kill our current window
				fullscreen =! fullscreen;       // Toggle fullscreen / windowed mode
				// Recreate our OpenGL window
				if (!CreateGLWindow("D3Box 3D Environment",User->WinHeight,User->WinWidth,16,fullscreen, User->WinXpos, User->WinYpos))
				{
					return;               // Quit if window was not created
				}
			} //F1 key
//Change Camera View
                        if (keys[VK_PRIOR])
			{
				z -= 0.2f;
			}
			if (keys[VK_NEXT])
			{
				z += 0.2f;
			}
                        if (keys[VK_LEFT])
                        {
                                x -= 0.2f;
                        }
                        if (keys[VK_RIGHT])
                        {
                                x += 0.2f;
                        }
                        if (keys[VK_UP])
                        {
                                y += 0.2f;
                        }
                        if (keys[VK_DOWN])
                        {
                                y -= 0.2f;
                        }


//Turn the light on and off
                        if (keys['L'] && !lp)	        // L Key Being Pressed Not Held?
                        {
                                lp=TRUE;	        // lp Becomes TRUE
                                light=!light;	        // Toggle Light TRUE/FALSE
                                if (!light)				// If Not Light
                                {
                                        glDisable(GL_LIGHTING);		// Disable Lighting
                                }//if
                                else					// Otherwise
                                {
                                        glEnable(GL_LIGHTING);		// Enable Lighting
                                }//else
                        }//if
                        if (!keys['L'])					// Has L Key Been Released?
                        {
                                lp=FALSE;				// If So, lp Becomes FALSE
                        }

                        if (keys['H'] && !hp)	        // L Key Being Pressed Not Held?
                        {
                                hp=TRUE;	        // lp Becomes TRUE
                                light1=!light1;	        // Toggle Light TRUE/FALSE
                                if (!light1)				// If Not Light
                                {
                                        glDisable(GL_COLOR_MATERIAL);		// Disable Lighting
                                }//if
                                else					// Otherwise
                                {
                                        glEnable(GL_COLOR_MATERIAL);		// Enable Lighting
                                }//else
                        }//if
                        if (!keys['H'])					// Has L Key Been Released?
                        {
                                hp=FALSE;				// If So, lp Becomes FALSE
                        }

                        //makes T looks brighter
                        if (threeDTextVec[0].getStatus() == true)
                        {
                                // glDisable(GL_LIGHTING);
                                camLig.setLightColor(0.5f,0.5f, 0.5f);
                                camLig.setAmbLightBri(170);
                                GLfloat abmBri = (GLfloat)camLig.getAmbLightBri()/255;
                                GLfloat ligBri = (GLfloat)camLig.getLightBri()/255;

        if (abmBri >= 1)
                abmBri = 1.0f;
        if (ligBri >=1 )
                ligBri = 1.0f;

        GLfloat abmLight[] ={abmBri*camLig.getLightColorX(), abmBri*camLig.getLightColorY(), abmBri*camLig.getLightColorZ(),  1.0f};
        GLfloat spec[] = {ligBri*camLig.getLightColorX(), ligBri*camLig.getLightColorY(), ligBri*camLig.getLightColorZ(), 1.0f};
        GLfloat lightPosition[] = {camLig.getLightX(), camLig.getLightY(),  camLig.getLightZ(), 0.0f};

        glLightfv(GL_LIGHT0, GL_AMBIENT, abmLight);		// Setup The Ambient Light
        glLightfv(GL_LIGHT1, GL_AMBIENT, abmLight);		// Setup The Ambient Light
        glLightfv(GL_LIGHT1, GL_DIFFUSE, spec);		        // Setup The Specular Light
        glLightfv(GL_LIGHT1, GL_SPECULAR, spec);		// Setup The Specular Light
        glLightfv(GL_LIGHT1, GL_POSITION,lightPosition);	// Position The Lig


        glMaterialfv(GL_FRONT, GL_DIFFUSE, spec);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
        glMaterialfv(GL_FRONT, GL_AMBIENT, abmLight);
        glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

        glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
        glEnable(GL_COLOR_MATERIAL );
        

	glEnable(GL_LIGHT1);                                    // Enable light one

                        }
                        else
                        {
                             //   glEnable(GL_LIGHTING);		// Enable Lighting
                             camLig.setLightColor(0.5f,0.5f, 0.5f);
                             camLig.setAmbLightBri(255);
                             GLfloat abmBri = (GLfloat)camLig.getAmbLightBri()/255;
                                if (abmBri >= 1)
                                abmBri = 1.0f;
                             GLfloat abmLight[] ={abmBri*camLig.getLightColorX(), abmBri*camLig.getLightColorY(), abmBri*camLig.getLightColorZ(),  1.0f};
                             glLightfv(GL_LIGHT0, GL_AMBIENT, abmLight);		// Setup The Ambient Light
                             glDisable(GL_LIGHT1);
                             glEnable(GL_LIGHT0);
                        }




                        //suspend mode is the starting mode
                        if(firstRun)
                        {
                                User->suspend();
                                firstRun = false;
                        }

		}//else
	}//while
        KillGLWindow();         // Kill the window

}
        catch (exception e)
        {
                //if (debug) fprintf(a, "ERROR: %s.\n", e.what());
                globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Driver() function.\n";
                //if(debug) fprintf(a, "Unknown error.\n");
                 try
                 {
                        throw Exception("Error in Driver() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch
        return;

}//startGLthread

//----------------------------------------------------------------------------


/*
 * Function:    open3D
 * Parameter:   Null Pointer
 * Purpose:     This function creates the 3D window
 * Return:      N/A
 */
void TUser::open3D()
{
       /*shidong starts*/
       if (!glWindowRun)
        {
                glWindowRun = true;
                /*call another thread to run Glwindow*/
                char *args[1];
                args[0] = NULL;

                int thread_id;
                #if     defined(__WIN32__)
                        if ( (thread_id = _beginthread(startGLthread,4096,  (void *)args    )) == (unsigned long)-1)
                #else
                        if ( (thread_id = _beginthread(startGLthread,4096, (void *)args  ) ) == -1)
                #endif
                {
                        if(debug)fprintf(a,"Unable to create thread, errno = %d\n",errno);
                        return;
                }
                if(debug)fprintf(a,"Created thread , ID = %ld\n",thread_id);
        }//if

       /*shidong ends*/
}//open3D

//----------------------------------------------------------------------------


/*
 * Function:    suspend()
 * Parameter:   Null Pointer
 * Purpose:     This function put the 3D Suspend text on the screen to replace everythign else
 * Return:      N/A
 */
void TUser::suspend()
{
        if(debug) fprintf(a, "In Usr->suspend.\n");

        threeDTextVec[0].setStatus(true);
        for (int i=0; i<=cuboidsVec.size(); i++)
        {
                cuboidsVec[i].setStatus(false);
        }

        sphereVec[0].setStatus(false);          //cursor
}//suspend


//----------------------------------------------------------------------------


/*
 * Function:    resume()
 * Parameter:   n/a
 * Purpose:     This function remove the 3D suspend text and restore the active cursor and graphic
 * Return:      N/A
 */
void TUser::resume()
{                     
        if(debug) fprintf(a, "In Usr->resume.\n");
        threeDTextVec[0].setStatus(false);
        if (boundV == true)
        {
                cuboidsVec[0].setStatus(true);
        }
        if (boundV == false)
        {
                cuboidsVec[0].setStatus(false);         //border
        }
}//suspend

//----------------------------------------------------------------------------




/*
 * Function:    setCursorColor()
 * Parameter:
 *      front   -       Color of the cursor if the cursor is at the most front of the screen
 *      back    -       Color of the cursor if the cursor is at the most back of the screen
 * Return:      void
 */
void TUser::setCursorColor(AnsiString front, AnsiString back)
{
        cursorColorF = front;
        cursorColorB = back;
}//setCursorColor



/*
 * Function:    serCursor()
 * Paramters:   posX    --      x coordinate postion
                posY    --      y ""
                posZ    --      z ""
                radius  --      cursor radius
                clR     --      red color component
                clG     --      green color component
                clB     --      blue color component
                bright  --      brightness
                cTexture--      cursor texture file path
 * Purpose:     Create a cursor and set its parameters.  Add the cursor to the spehre vector
 * Return:      n/a
 */
void TUser::setCursor(float posX, float posY, float posZ, float radius, float clR, float clG, float clB, float bright, AnsiString cTexture)
{

        sphere temp;                    //sphere primitive

        //radius and positions are the percentage if the window's dimension.
        radius = (radius/100)*(BORDER/2*2);
        posX = (posX/100)*(BORDER/2*2);             //x coordinate goes from left (0%) to right (100%)
        posX -= BORDER/2;
        posY = (posY/100)*(BORDER/2*2);             //y goes from top (0%) to bottom (100%)
        posY = BORDER/2-posY;
        posZ = (posZ/100)*(BORDER/2*2);             //z goes from back (0%) to front (100%)
        posZ -= BORDER/2;
        if(debug) fprintf(a, "Cursor's radius is: %f, posX is %f, posY is %f, posZ is %f, texture is %s.\n", radius, posX, posY, posZ, cTexture);

        /*IMPORTANT:::::z - 25 for best view*/
        posZ -= 50;


        startZ = posZ;
    /*      */
        temp.setSphere(posX, posY, posZ, radius);
        temp.setColor(clR, clG, clB);
        temp.setPrimitveID(1);
        temp.setRotPointPosition(0.0f, 0.0f, 0.0f);
        temp.setStatus(false);
        temp.setBrightness(bright);
        temp.setTransparency(255);
        temp.setHasTexture(true);
        temp.setTexture(cTexture.c_str());

        if (sphereVec.size() != 0)      //if already one cursor in the vector
        {
                sphereVec[0].setSphere(posX, posY, posZ, radius);
                User->posZ = posZ;
                sphereVec[0].setColor(clR, clG, clB);
                sphereVec[0].setPrimitveID(1);
                sphereVec[0].setRotPointPosition(0.0f, 0.0f, 0.0f);
                sphereVec[0].setStatus(false);
                sphereVec[0].setBrightness(bright);
                sphereVec[0].setTransparency(255);
                //IMPORTANT: Once texture loaded first time, it cannot be changed.
                //If changes occur, it will not reflect in the program because 1st texture is already
                //loaded into the system.
                sphereVec[0].setHasTexture(true);
                sphereVec[0].setTexture(cTexture.c_str());
        }
        else
        {
                sphereVec.push_back(temp);
                sphereVec.back().setElementID(sphereVec.size()-1);
        }
        if (debug) fprintf(a, "Cursor: %d\n", sphereVec.back().getElementID());

}//serCursor
   
//----------------------------------------------------------------------------
void TUser::setWindow(int h, int w, int x, int y)
{
User->WinHeight =h;
User->WinWidth = w;
User->WinXpos = x;
User->WinYpos = y;
}


//----------------------------------------------------------------------------

/*
 * Function:    setTarget()
 * Paramters:   target  --      2D float array that stores the target information
 *              col     --      columen of the array
 *              row     --      rows of the array
 *              tTexture--      targets texture
 *              bTexture--      border texture
 * Purpose:     Create a targets and set its parameters.  Add the targets to the cubiods vector
 * Return:      n/a
 */
void TUser::setTarget(int target[][NTARGS], int col, int row, AnsiString tTexture, AnsiString bTexture, int boundVisible)
{
        if (debug) fprintf(a, "In Set target.\n" );

        if(cuboidsVec.size()==0)        //if nothing are exist in the cubiods vector
        {
                cuboids bound;          //border
                bound.setPrimitveID(3);
                bound.setColor(1.0f, 1.0f, 1.0f);
                bound.setCuboid(0.0f, 0.0f, 0.0f, BORDER,BORDER, BORDER*2.5);    //BORDER IS 100
                bound.setRotPointPosition(0.0f, 0.0f, 0.0f);
                bound.setTransparency(255);
                bound.setBrightness(255);
                if ( boundVisible == 1)
                {
                        bound.setStatus(true);
                        boundV = true;
                }
                else
                {
                        bound.setStatus(false);
                        boundV = false;
                }
                bound.setHasTexture(true);
                bound.setTexture(bTexture.c_str());
                cuboidsVec.push_back(bound);
                cuboidsVec.back().setElementID(cuboidsVec.size()-1);

                if (debug) fprintf(a, "Bound: %d\n", cuboidsVec.back().getElementID());

                for(int add=1; add<=col; add++)
                {
                cuboids toAdd;
                toAdd.setPrimitveID(2);
                toAdd.setColor(1.0f, 0.0f, 0.0f);       //default red color
                toAdd.setRotAngX(0.0f);
                toAdd.setRotPointPosition(0.0f, 0.0f, 0.0f);
                toAdd.setTransparency(255);
                toAdd.setBrightness(255);
                toAdd.setHasTexture(true);
                toAdd.setTexture(tTexture.c_str());
                cuboidsVec.push_back(toAdd);
                cuboidsVec.back().setElementID(cuboidsVec.size()-1);
                if (debug) fprintf(a, "target: %d\n", cuboidsVec.back().getElementID());

                }
        }

        if ( boundVisible == 1)
        {
        boundV = true;
        }
        else
        {
        boundV = false;
        }


        //target[rows][columns]
        for( int j=0; j<col; j++)
        {                      
                float xCord, yCord, zCord, w, h, d;
                xCord = (float)target[0][j]/100*BORDER;
                yCord = (float)target[1][j]/100*BORDER;
                zCord = (float)target[2][j]/100*BORDER;
                w = (float)target[3][j]/100*BORDER;
                h = (float)target[4][j]/100*BORDER;
                d = (float)target[5][j]/100*BORDER;
                xCord -= BORDER/2;                  //x coordinate goes from left (0%) to right (100%)
                yCord = BORDER/2-yCord;              //y goes from top (0%) to bottom (100%)
                zCord -= BORDER/2;                  //z goes from back (0%) to front (100%)

                /*IMPORTANT: for best view, Z-50*/
                zCord -=50;  //BUG!!!!!!!!
                cuboidsVec[j+1].setCuboid(xCord, yCord, zCord, w, h, d);
                if (debug) fprintf(a, "target: %d's paramters are %f, %f, %f, %f, %f, %f.\n", j+1, xCord, yCord, zCord, w, h, d);
                if (cuboidsVec.size()-1 < col)
                {
                        if(j == cuboidsVec.size()-1)
                        {
                                cuboids toAdd;
                                toAdd.setPrimitveID(2);
                                toAdd.setColor(1.0f, 0.0f, 0.0f);       //default red color
                                toAdd.setRotAngX(0.0f);
                                toAdd.setRotPointPosition(0.0f, 0.0f, 0.0f);
                                toAdd.setTransparency(255);
                                toAdd.setBrightness(255);
                                toAdd.setHasTexture(true);
                                toAdd.setTexture(tTexture.c_str());
                                        /*IMPORTANT:::::z - 25 for best view*/
                                toAdd.setCuboid(xCord, yCord, zCord, w, h, d);
                                cuboidsVec.push_back(toAdd);
                                cuboidsVec.back().setElementID(cuboidsVec.size()-1);
                                if (debug) fprintf(a, "target: %d\n", cuboidsVec.back().getElementID());
                        }     
                }
        }//columns
        if (debug) fprintf(a, "cuboids vector's size is %d.\n",cuboidsVec.size());

}//serTarget


