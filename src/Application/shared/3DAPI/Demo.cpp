#include "Environment3D.cpp"

//demo program without textures, 2D overlay, 2D text

/*
This driver function acts like a main, it listens to all the input and react
correspondly
*/

WINAPI Driver(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow,int posX,int posY)
{

try
{
        MSG msg;                // Windows message structure
	bool done = false;      // bool variable to exit loop

	// Ask the user which screen mode they prefer
	if (MessageBox(NULL,"Would you like to run in fullscreen mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION) == IDNO)
	{
		fullscreen = false;       // Windowed mode
	}

	// Create our OpenGL window
	if (!CreateGLWindow("D3Box 3D Environment",640,480,16,fullscreen, posX, posY))
	{
		return 0;               // Quit if window was not created
	}

        //debug variable will enable the program to print debug files while it is running
        //Set it to false is a good idea, otherwise the size of debug file will increase
        //in a dramatically rate
        debug = true;

//Time measurement for debug prupose
LARGE_INTEGER   S1, S2, prectimebase, overhead, aa, bb;
QueryPerformanceCounter(&S1);
QueryPerformanceCounter(&S2);
overhead.QuadPart = prectime2.QuadPart - prectime1.QuadPart;
QueryPerformanceFrequency(&prectimebase);
double timeinms;



	while(!done)                    // Loop that runs while done = false
	{
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
		}
		else            // If there are no messages
		{
			// Draw the scene.  Watch for ESC key and quit messages from DrawScreen()
			if (active)                             // Program active?
			{
				if (keys[VK_ESCAPE])            // Was ESC pressed?
				{
                                done = true;            // ESC signalled a quit
                                //Free all the memories
                                KillFont();						// Destroy The Font
                                Kill2DFont();
                                sphereVec.clear();						// Destroy The Font
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
if(debug||true)fprintf(g, "%e\n", timeinms);
double frameRate = 1000/25;             //25 Hz, 40 ms per frame
if(debug)fprintf(g, "%e\n", frameRate-timeinms);
if(debug)fprintf(g, "\tAcutal frame Rate is: %e.\n", 1000/timeinms);
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
				if (!CreateGLWindow("NeHe's OpenGL Framework",640,480,16,fullscreen, posX,posY))
				{
					return 0;               // Quit if window was not created
				}
			} //F1 key
//Change Camera View
                        if (keys[VK_PRIOR])
			{
				z -= 0.09f;
			}
			if (keys[VK_NEXT])
			{
				z += 0.09f;
			}
                        if (keys[VK_LEFT])
                        {
                                x -= 0.09f;
                        }
                        if (keys[VK_RIGHT])
                        {
                                x += 0.09f;
                        }
                        if (keys[VK_UP])
                        {
                                y += 0.09f;
                        }
                        if (keys[VK_DOWN])
                        {
                                y -= 0.09f;
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

                        if (keys['M'])
                        {
                                moveObj ( 3,  1, 0, 5, 5, 100);
                                rotateByMidpoint(3, 1, 2, 90, 50);
                                rotateObjByPoint(3,1,0,0,0, 50, 40, 60);
                                moveObj ( 3,  0, 6, 5, 5, 100);
                                moveObj ( 3,  0, 5, 10, 3, 100);
                                rotateByMidpoint(3, 0, 2, 90, 50);
                                rotateObjByPoint(3,0,0,0,0, 50, 40, 60);
                        }
                        if (keys['N'])
                        {
                                moveObj ( 1,  0, 1.9f, 2.0f, -2.0f, 50 );
                        }
                        if (keys['B'])
                        {
                                moveObj ( 1,  1, -7.0f, -2.0f, -2.0f, 100 );
                        }
                        if (keys['V'])
                        {
                                //moveObj ( 2,  2, 3.0f, 3.0f, 3.0f, 50 );
                                moveObj ( 2,  2, 1.0f, 6.0f, -2.0f, 50 );
                                moveObj ( 2,  2, -7.0f, -5.0f, -5.0f, 40 );
                                moveObj ( 2,  2, 3.0f, 3.0f, 3.0f, 40 );
                        }
                        if (keys['C'])
                        {
                               // moveObj ( 2,  1, 8.0f, -9.0f, 1.0f, 100 );
                                moveObj ( 2,  1, 7.0f, 1.0f, -2.0f, 100 );
                                moveObj ( 2,  1, 7.0f, -1.0f, -2.0f, 100 );
                                moveObj ( 2,  1, 7.0f, -1.0f, -2.0f, 100 );
                                moveObj ( 2,  1, -7.0f, 1.0f, 1.0f, 100 );
                        }
                        if (keys['A'])
                        {
                        rotateObjByPoint( 1,  1, 0 ,  0, 0 ,  100, 360,90);
                        rotateObjByPoint( 1,  1, 0 ,  0, 0 ,  100, 360,90);
                        rotateObjByPoint( 2,  2, 0 ,  0, 0 ,  100, 360,90);
                        rotateObjByPoint( 2,  1, 0 ,  0, 0 ,  100, 60, 20);
                        rotateObjByPoint( 2,  1, 0 ,  0, 0 ,  100, 60, 20);
                        rotateByMidpoint(1,1, 1,45, 50);
                        rotateByMidpoint(1,0, 1,45, 50);
                        rotateByMidpoint(2,1, 1,45, 50);
                        rotateByMidpoint(1,1, 2,45, 50);
                        rotateByMidpoint(1,1, 3,45, 50);
                        }
                        if (keys['S'])
                        {
                        }
                        if (keys['D'])
                        {
                                moveObj(2,1,5,3,5,100);
                                moveObj(2,2,3,0.49,5,100);
                               //rotateByMidpoint( 2,2, 1, 34, 50);

                                if (collide(2,1,2,2))
                                {
                                        sphereVec[0].setColor(0,1,1);
                                        cuboidsVec[1].setColor(0,1,1);
                                       // rotateObjByPoint(2,2,2,2,-2,100, 360, 0);
                                        //cuboidsVec[1].setColor(1,0,0);
                                        //cuboidsVec[2].setColor(1,0,0);
                                }  
                        }
		}//else




	}//while

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        if (true)
                {
                        fclose(f);
                        fclose(g);
                }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=
	// Shutdown
	KillGLWindow();         // Kill the window
	return (msg.wParam);    // Exit the program
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Driver() function.\n";
                 try
                 {
                        throw Exception("Error in Driver() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch

}//Driver
//---------------------------------------------------------------------------


WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
       try
        {
                /*
                demo 1  -- demo program without textures, 2D overlay, 2D text
                demo 2  -- demo program with textures
                demo 3  -- demo program with textures, 2D overlay, and 2D text
                */
                int demo = 3;

                if (true)
                {
                        f = fopen("debug.txt" , "w");
                        g = fopen("processor.txt", "w");
                }
                sphere temp;
                sphere temp2;
                temp.setSphere(5.0f, 1.0f, 3.0f, 1.0f);
                temp.setColor(0.0f, 1.0f, 0.0f);
                temp.setPrimitveID(1);
                temp.setRotPointPosition(0.0f, 0.0f, 0.0f);
                temp.setStatus(true);
                temp.setBrightness(200);
                temp.setTransparency(255);
             
                temp.setHasTexture(true);
                temp2.setSphere(0,5, 3, 1);
                temp2.setColor(0.0f, 0.0f,1.0f);
                temp2.setPrimitveID(1);
                temp2.setRotPointPosition(0.0f, 0.0f, 0.0f);
                temp2.setStatus(true);
                temp2.setBrightness(30);
                temp2.setTransparency(255);
                temp2.setHasTexture(true);

                cuboids cub1;
                cub1.setPrimitveID(2);
                cub1.setColor(0.0f, 0.0f, 1.0f);
                cub1.setCuboid(7.0f, -1.8f, 5.0f, 3.0f, 3.0f, 3.0f);
                cub1.setRotAngX(45.0f);
                cub1.setRotPointPosition(0.0f, 0.0f, 0.0f);
                cub1.setTransparency(255);
                cub1.setHasTexture(true);
               cub1.setBrightness(255);

                cuboids cub2;
                cub2.setPrimitveID(2);
                cub2.setColor(0.0f, 1.0f, 0.0f);
                cub2.setCuboid(-1.0f, 5.0f, 0.0f, 2.0f, 2.0f, 2.0f);
                cub2.setRotAngY(45.0f);
                cub2.setRotPointPosition(0.0f, 0.0f, 0.0f);
                cub2.setTransparency(255);
                cub2.setBrightness(255);
                cub2.setHasTexture(true);

                cuboids cub3;
                cub3.setPrimitveID(2);
                cub3.setColor(1.0f, 0.0f, 0.0f);
                cub3.setCuboid(0.0f, 0.0f, -1.0f, 3.0f, 3.0f, 3.0f);
                cub3.setRotAngX(0.0f);
                cub3.setRotPointPosition(0.0f, 0.0f, 0.0f);                                                                            cub3.setTransparency(255);
                cub3.setBrightness(255);
                cub3.setHasTexture(true);

                cuboids bound;
                bound.setPrimitveID(3);
                bound.setColor(1.0f, 1.0f, 1.0f);
                bound.setCuboid(0.0f, 0.0f, 0.0f, 75.0f, 75.0f, 75.0f);
                bound.setRotPointPosition(0.0f, 0.0f, 0.0f);
                bound.setTransparency(255);
                bound.setBrightness(255);
                bound.setStatus(true);
                bound.setHasTexture(true);



                if (demo != 1)
                {
                temp.setTexture("texture/Glass.bmp");
                bound.setTexture("texture/Glass.bmp");
                cub3.setTexture("texture/Glass.bmp");
                cub2.setTexture("texture/Cube.bmp");
                cub1.setTexture("texture/Cube.bmp");
                temp2.setTexture("texture/Glass.bmp");
                }//enable texture

                cuboidsVec.push_back(bound);
                cuboidsVec.back().setElementID(cuboidsVec.size()-1);
                if (debug) fprintf(f, "%d", cuboidsVec.back().getElementID());


                sphereVec.push_back(temp);
                sphereVec.back().setElementID(sphereVec.size()-1);
                if (debug) fprintf(f, "%d", sphereVec.back().getElementID());


                sphereVec.push_back(temp2);
                sphereVec.back().setElementID(sphereVec.size()-1);
                if (debug) fprintf(f, "%d", sphereVec.back().getElementID());

                cuboidsVec.push_back(cub1);
                cuboidsVec.back().setElementID(cuboidsVec.size()-1);
                if (debug) fprintf(f, "%d", cuboidsVec.back().getElementID());

                cuboidsVec.push_back(cub2);
                cuboidsVec.back().setElementID(cuboidsVec.size()-1);
                if (debug) fprintf(f, "%d", cuboidsVec.back().getElementID());

                 cuboidsVec.push_back(cub3);
                cuboidsVec.back().setElementID(cuboidsVec.size()-1);
                if (debug) fprintf(f, "%d\n", cuboidsVec.back().getElementID());


           /*     */

                camLig.setCamAim(0.0f, 0.0f, 0.0f);
                camLig.setCamViewPoint(0.0f, 0.0f, 13.0f);
                camLig.setLight(0.0f, 0.0f, 5.0f);
                camLig.setLightColor(0.5f, 0.5f, 0.05);
                camLig.setLightBri(255);
                camLig.setAmbLightBri(255);
                camLig.setLight(0,0,5);

                threeDText tempF;
                threeDText tempF1;
                tempF.setFont("Times New Roman");
                tempF.setCap("This is 3D text");
                tempF.setFontSize(12);
                tempF.setBrightness(255);
                tempF.setTransparency(240);
                tempF.setColor(0.3f, 1.0f, 1.0f);
                tempF.setOrigin(0.0f, 0.0f, 0.0f);
                tempF.setDirection(2.0f, 0.0f, 0.0f);
                tempF.setStatus(true);

                tempF1.setFont("Arial");
                tempF1.setCap("I need to change the font size.");
                tempF1.setFontSize(30);
                tempF1.setBrightness(255);
                tempF1.setTransparency(100);
                tempF1.setColor(0.0f, 0.0f, 1.0f);
                tempF1.setOrigin(0.0f, 1.0f, 0.0f);
                tempF1.setDirection(2.0f, 1.0f, 0.0f);
                tempF1.setStatus(true);



                threeDTextVec.push_back(tempF);
                threeDTextVec.push_back(tempF1);


                if (demo ==3)//enable 2Doverlay
                {
                overlay.setOverlayPicture("texture/CrossHair.bmp");
                overlay.setOverlayTrans("texture/CrossHairMask.bmp");
                overlay.setPicTexture(true);
                overlay.setTraTexture(true);
                }
                overlay.setCurColor(1.0f, 1.0f, 0.0f);
                overlay.setCurPosition(100.0f, 455.0f);
                overlay.setCursorSizeRad(8.0f);


                twoDText  t1;
                t1.setTextPosition(120.0f,450.0f);
                t1.setTextFont("Times New Roman");
                t1.setCaption("ShiDong 3D Environment, Times New Roman, size 29");
                t1.setTextColor(0.0f, 1.0f, 1.0f);
                t1.setTextSize(29);

                twoDText t2;
                t2.setTextPosition(120.0f, 430.0f);
                t2.setTextFont("Arial");
                t2.setCaption("This is 2D Text, Arial, size 25");
                t2.setTextColor(1.0f, 1.0f, 0.0f);
                t2.setTextSize(25);

                twoDText t3;
                t3.setTextPosition(120.0f, 400.0f);
                t3.setTextFont("Arial");
                t3.setCaption("Hit the target, arial, size 18");
                t3.setTextColor(0.5f, 0.8f, 0.1f);
                t3.setTextSize(18);

                twoDText notPrint;
                notPrint.setTextPosition(120.0f, 300.0f);
                notPrint.setTextFont("Arial");
                notPrint.setCaption("notPrint, arial, size 18");
                notPrint.setTextColor(0.5f, 0.8f, 0.1f);
                notPrint.setTextSize(18);

                if(demo ==3)//enable 2D text
                {
                twoDTextVec.push_back(notPrint);
                twoDTextVec.push_back(t1);
                twoDTextVec.push_back(t2);
                twoDTextVec.push_back(t3);
                }

                int posX = 200;
                int posY = 200;


                model3Dclass toAdd;
                model3Dclass secAdd;
                toAdd.elementID = 0;
                toAdd.primitiveID = 0;
                toAdd.isOn = true;
                toAdd.X = 1;
                toAdd.Y = 1;
                toAdd.Z = 1;
                toAdd.xaxisAngle = 0;
                toAdd.yaxisAngle = 0;
                toAdd.zaxisAngle = 0;
                toAdd.filename3ds = "face.3ds";

                secAdd = toAdd;
                secAdd.elementID = 1;
                secAdd.X = -5;
                secAdd.Y = -5;
                secAdd.Z = -5;
                model3dList.push_back(toAdd);
                model3dList.push_back(secAdd);
       if(debug)fprintf(f, "MAIN: CURRENT OBJECT POSITION IS: X- %f, Y- %f, Z- %f.\n", model3dList[0].X, model3dList[0].Y, model3dList[0].Z);

                return Driver( hInstance,  hPrevInstance,  lpCmdLine,  nCmdShow, posX, posY);



        }//try
        catch (exception e)
        {
        if (debug) fprintf(f, "ERROR: %s.\n", e.what());
        globalError = e.what();

        }//catch
        catch (...)
        {
                globalError = "Unknown error occur in Driver() function.\n";
                 try
                 {
                        throw Exception("Error in Driver() function");

                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }//catch
}//winMain




