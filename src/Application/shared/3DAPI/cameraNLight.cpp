/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
/*
This "cameraNLight" class encapsulate the camera and the light in th 3D environment.
*/

//---------------------------------------------------------------------------
#include "header.h"
//---------------------------------------------------------------------------
class cameraNLight
{
private:
        GLfloat camViewX ;
        GLfloat camViewY ;
        GLfloat camViewZ ;
        GLfloat camAimX ;
        GLfloat camAimY ;
        GLfloat camAimZ ;

        int     focLen ;         //focal length

        GLfloat lightX ;
        GLfloat lightY ;
        GLfloat lightZ ;
        GLfloat lightColorX ;
        GLfloat lightColorY ;
        GLfloat lightColorZ ;

        int     ambLightBri;    //ambient light birghtness
        int     lightBri;       //light brightness


public:
/**************************CONSTRUCTOR*****************************************/
        cameraNLight()                          {setAll();}
        void setAll();

/****************************Accessor******************************************/

        GLfloat getCamViewX()                   {return camViewX;}
        GLfloat getCamViewY()                   {return camViewY;}
        GLfloat getCamViewZ()                   {return camViewZ;}
        GLfloat getCamAimX()                    {return camAimX;}
        GLfloat getCamAimY()                    {return camAimY;}
        GLfloat getCamAimZ()                    {return camAimZ;}
        int     getFocLen()                     {return focLen;}
        GLfloat getLightX()                     {return lightX;}
        GLfloat getLightY()                     {return lightY;}
        GLfloat getLightZ()                     {return lightZ;}
        GLfloat getLightColorX()                {return lightColorX;}
        GLfloat getLightColorY()                {return lightColorY;}
        GLfloat getLightColorZ()                {return lightColorZ;}
        int     getLightBri()                    {return lightBri;}
        int     getAmbLightBri()                {return ambLightBri;}

/****************************Modifiers*****************************************/

        void setCamViewPoint(GLfloat X, GLfloat Y, GLfloat Z)
        {
                camViewX = X;
                camViewY = Y;
                camViewZ = Z;
        }
	void setCamAim(GLfloat X, GLfloat Y, GLfloat Z)
        {
                camAimX = X;
                camAimY = Y;
                camAimZ = Z;
        }
        void setLightBri(GLfloat brightness)            {lightBri = brightness;}
        void setFocalLength(int fl)                     {focLen = fl;}
        void setLight(GLfloat x, GLfloat y, GLfloat z)
        {
                lightX = x;
                lightY = y;
                lightZ = z;
        }
	void setLightColor(GLfloat X, GLfloat Y, GLfloat Z)
        {
                lightColorX = X;
                lightColorY = Y;
                lightColorZ = Z;
        }
        void setAmbLightBri(int amb)                {ambLightBri = amb;   }
        


};//cameraNLight


void cameraNLight::setAll()
{
        camViewX = 0.0f;
        camViewY = 0.0f;
        camViewZ = 0.0f;
        camAimX = 0.0f;
        camAimY = 0.0f ;
        camAimZ = 0.0f ;

        focLen = 0;         //focal length

        lightX  = 0.0f;
        lightY = 0.0f ;
        lightZ = 0.0f ;
        lightColorX = 0.0f ;
        lightColorY = 0.0f ;
        lightColorZ = 0.0f ;

        ambLightBri =255 ;    //ambient light birghtness255
        lightBri=255;       //light brightness

}//setAll


