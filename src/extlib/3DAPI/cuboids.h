////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: Two classes that represent cuboids.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef CUBOIDS_H
#define CUBOIDS_H

#include "primObj.h"
#include "component.h"
#include "halfSpace.h"

// A cuboid
class cuboid : public sceneObj
{
private:
        GLfloat mWidth,
                mHeight,
                mDepth;
        bool    mFaceVisible[6];

public:
        enum { primitiveID = 2 };
/*******************************CONSTRUCTOR************************************/
        cuboid( Scene& inScene, GLfloat x=0, GLfloat y=0, GLfloat z=0, GLfloat w=1, GLfloat h=1, GLfloat d=1)
        : sceneObj( inScene ), mWidth(w), mHeight(h), mDepth(d)
        {
          setOrigin(x,y,z);
          for(int i = 0; i < 6; ++i)
            mFaceVisible[i] = true;
        }
/**************************Modifiers & Accessors*******************************/
        void    setDimensions( GLfloat width, GLfloat height, GLfloat depth )
                { mWidth = width; mHeight = height; mDepth = depth; }
        GLfloat getWidth() const                {return mWidth;}
        GLfloat getHeight() const               {return mHeight;}
        GLfloat getDepth() const                {return mDepth;}
        void    setFaceVisible( int i, bool b ) {mFaceVisible[i%6]=b;}
        bool    getFaceVisible( int i ) const   {return mFaceVisible[i%6];}
        
        virtual CVector3  getVertex( int ) const;
        virtual halfSpace getHalfSpace( int ) const;

        virtual testResult isConvex() const     { return true_; }
        virtual testResult contains( const CVector3& ) const;
        virtual testResult intersectsVolume( const geomObj& ) const;

protected:
        virtual void onRender() const;
        void doRender( bool invert ) const;

private:
        void drawFace( int, bool ) const;

};//cuboid

// The inversion of a cuboid (all space is part of it except the cuboid's volume)
class invertedCuboid : public cuboid
{
public:
        enum { primitiveID = 3 };
        invertedCuboid(Scene& inScene, GLfloat x=0, GLfloat y=0, GLfloat z=0, GLfloat w=1, GLfloat h=1, GLfloat d=1)
        : cuboid( inScene, x, y, z, w, h, d ) {}
        virtual halfSpace getHalfSpace( int ) const;
        virtual testResult isConvex() const { return false_; }
        virtual testResult contains( const CVector3& ) const;
        virtual testResult intersectsVolume( const geomObj& ) const;

protected:
        virtual void onRender() const;
};

#endif // CUBOIDS_H
