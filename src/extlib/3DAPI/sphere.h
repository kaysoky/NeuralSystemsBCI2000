////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: A 3D sphere.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SPHERE_H
#define SPHERE_H

#include "primObj.h"

class sphere: public sceneObj
{
private:
        GLfloat mRadius;      //radius of the sphere

public:
        enum { primitiveID = 1 };
/*******************************CONSTRUCTOR************************************/
        sphere(Scene& inScene, GLfloat x=0, GLfloat y=0, GLfloat z=0, GLfloat rad=1)
          :sceneObj(inScene), mRadius(rad) {setOrigin(x,y,z);}

        void setRadius( GLfloat rad )                   {mRadius = rad;}
        GLfloat getRadius() const                       {return mRadius;}

        virtual testResult isConvex() const { return true_; }
        virtual testResult contains( const CVector3& p ) const;
        virtual testResult intersectsVolume( const geomObj& ) const;
        
protected:
        virtual void onRender() const;
};//sphere

#endif // SPHERE_H
