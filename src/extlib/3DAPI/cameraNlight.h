///////////////////////////////////////////////////////////////////////////////////////
// $Id: cameraNLight.h 1378 2007-06-03 18:04:53Z mellinger $
// Authors: shzeng, schalk@wadsworth.org
// Description: This "cameraNLight" class encapsulate the camera and the light in the
//   3D environment.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMERA_N_LIGHT_H
#define CAMERA_N_LIGHT_H

#include "glheaders.h"
#include "geomObj.h"
#include "cuboids.h"

class projection
{
protected:
        GLfloat mMatrix[16];
public:
        typedef class perspectiveProj perspective;
        typedef class flatProj flat;
        projection()                       {memset(mMatrix,0,sizeof(mMatrix));}
        virtual ~projection()              {}
        void setMatrix( const GLfloat* m ) {memcpy(mMatrix,m,sizeof(mMatrix));}
        const GLfloat* getMatrix() const   {return mMatrix;}
};

class perspectiveProj : public projection
{
public:
        perspectiveProj( GLfloat fieldOfView, GLfloat aspect, GLfloat depth );
};

class flatProj : public projection
{
public:
        flatProj( GLfloat height, GLfloat width, GLfloat depth );
};


class cameraNLight
{
private:
        GLfloat camViewX ;
        GLfloat camViewY ;
        GLfloat camViewZ ;
        GLfloat camAimX ;
        GLfloat camAimY ;
        GLfloat camAimZ ;
        GLfloat camUpX;
        GLfloat camUpY;
        GLfloat camUpZ;

        GLfloat lightX ;
        GLfloat lightY ;
        GLfloat lightZ ;
        GLfloat lightColorR ;
        GLfloat lightColorG ;
        GLfloat lightColorB ;

        GLfloat ambLightBri;    //ambient light brightness
        GLfloat lightBri;       //light brightness
        GLfloat fieldOfView;    //viewing angle in up direction, 0 for flat projection
        geomObj sceneDimensions; //an ellipsoid that contains all scene objects
        int     quality;        // rendering quality: 0 is raw

public:
/**************************CONSTRUCTOR*****************************************/
        cameraNLight()                          {setAll();}
        void setAll();

/****************************Accessor******************************************/

        GLfloat getCamViewX() const             {return camViewX;}
        GLfloat getCamViewY() const             {return camViewY;}
        GLfloat getCamViewZ() const             {return camViewZ;}
        GLfloat getCamAimX() const              {return camAimX;}
        GLfloat getCamAimY() const              {return camAimY;}
        GLfloat getCamAimZ() const              {return camAimZ;}
        GLfloat getCamUpX() const               {return camUpX;}
        GLfloat getCamUpY() const               {return camUpY;}
        GLfloat getCamUpZ() const               {return camUpZ;}
        GLfloat getLightX() const               {return lightX;}
        GLfloat getLightY() const               {return lightY;}
        GLfloat getLightZ() const               {return lightZ;}
        GLfloat getLightColorR() const          {return lightColorR;}
        GLfloat getLightColorG() const          {return lightColorG;}
        GLfloat getLightColorB() const          {return lightColorB;}
        GLfloat getLightBri() const             {return lightBri;}
        GLfloat getAmbLightBri() const          {return ambLightBri;}
        GLfloat getFieldOfView() const          {return fieldOfView;}
 const geomObj& getSceneDim() const             {return sceneDimensions;}
            int getQuality() const              {return quality;} // rendering quality

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
        void setCamUp( GLfloat x, GLfloat y, GLfloat z )
        {
                camUpX = x;
                camUpY = y;
                camUpZ = z;
        }
        void setLightBri(GLfloat brightness)            {lightBri = brightness;}
        void setLight(GLfloat x, GLfloat y, GLfloat z)
        {
                lightX = x;
                lightY = y;
                lightZ = z;
        }
        void setLightColor(GLfloat r, GLfloat g, GLfloat b)
        {
                lightColorR = r;
                lightColorG = g;
                lightColorB = b;
        }
        void setAmbLightBri(GLfloat amb)                {ambLightBri = amb;}
        void setFieldOfView(GLfloat fov)                {fieldOfView = fov;}
        void setSceneDim( const geomObj& sceneDim )     {sceneDimensions = sceneDim;}
        void setBoundingBox( const cuboid& );
        void setQuality( int qual )                     {quality = qual;}

        void apply() const;

};//cameraNLight

inline
void cameraNLight::setAll()
{
        camViewX = 0.0f;
        camViewY = 0.0f;
        camViewZ = 0.0f;
        camAimX= 0.0f;
        camAimY = 0.0f;
        camAimZ = 0.0f;
        camUpX = 0;
        camUpY = 1;
        camUpZ = 0;

        lightX  = 0.0f;
        lightY = 0.0f ;
        lightZ = 0.0f ;
        lightColorR = 0.0f ;
        lightColorG = 0.0f ;
        lightColorB = 0.0f ;

        ambLightBri = 1;    //ambient light birghtness
        lightBri = 1;       //light brightness
        fieldOfView = 60;
        quality = 1;
}//setAll

#endif // CAMERA_N_LIGHT_H
