////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: geomObj holds common geometric properties of 3D scene objects,
//   and provides an interface for hit testing (volume intersection).
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "geomObj.h"

using namespace std;

// Transforms
enum // Indices into the transformation matrix
{
  xx =  0, xy =  1, xz =  2,
  yx =  4, yy =  5, yz =  6,
  zx =  8, zy =  9, zz = 10,
  cx = 12, cy = 13, cz = 14,
};


geomObj::geomObj()
{ // Initialize the world transform to the identity matrix.
  for( size_t i = 0; i < sizeof( mWorldTransform ) / sizeof( *mWorldTransform ); ++i )
    mWorldTransform[ i ] = ( i % 5 ) ? 0 : 1;
  CVector3 nullVector = { 0, 0, 0 };
  mVelocity = nullVector;
  mAngVelocity = nullVector;
}

void
geomObj::setOrigin( const CVector3& c )
{
  mWorldTransform[ cx ] = c.x;
  mWorldTransform[ cy ] = c.y;
  mWorldTransform[ cz ] = c.z;
}

CVector3
geomObj::getOrigin() const
{
  CVector3 c =
  {
    mWorldTransform[ cx ],
    mWorldTransform[ cy ],
    mWorldTransform[ cz ]
  };
  return c;
}

void
geomObj::setXAxis( const CVector3& c )
{
  mWorldTransform[ xx ] = c.x;
  mWorldTransform[ xy ] = c.y;
  mWorldTransform[ xz ] = c.z;
}

CVector3
geomObj::getXAxis() const
{
  CVector3 c =
  {
    mWorldTransform[ xx ],
    mWorldTransform[ xy ],
    mWorldTransform[ xz ]
  };
  return c;
}

void
geomObj::setYAxis( const CVector3& c )
{
  mWorldTransform[ yx ] = c.x;
  mWorldTransform[ yy ] = c.y;
  mWorldTransform[ yz ] = c.z;
}

CVector3
geomObj::getYAxis() const
{
  CVector3 c =
  {
    mWorldTransform[ yx ],
    mWorldTransform[ yy ],
    mWorldTransform[ yz ]
  };
  return c;
}

void
geomObj::setZAxis( const CVector3& c )
{
  mWorldTransform[ zx ] = c.x;
  mWorldTransform[ zy ] = c.y;
  mWorldTransform[ zz ] = c.z;
}

CVector3
geomObj::getZAxis() const
{
  CVector3 c =
  {
    mWorldTransform[ zx ],
    mWorldTransform[ zy ],
    mWorldTransform[ zz ]
  };
  return c;
}

void
geomObj::translate( const CVector3& v )
{
  setOrigin( getOrigin() + v );
}

void
geomObj::rotate( const CVector3& inVecAngle )
{
  rotate( Length( inVecAngle ), inVecAngle );
}

void
geomObj::rotate( GLfloat inAngle, const CVector3& inAxis )
{ // Rotate around an axis that contains the global origin.
  double length = Length( inAxis );
  if( length > 0 )
  {
    double sinAng = ::sin( inAngle * M_PI / 180 ),
           cosAng = ::cos( inAngle * M_PI / 180 );
    CVector3 a = 1 / length * inAxis,
             xPrime = ( 1 - cosAng ) * a.x * a,
             yPrime = ( 1 - cosAng ) * a.y * a,
             zPrime = ( 1 - cosAng ) * a.z * a;

    xPrime.x += cosAng;
    xPrime.y -= sinAng * a.z;
    xPrime.z += sinAng * a.y;

    yPrime.x += sinAng * a.z;
    yPrime.y += cosAng;
    yPrime.z -= sinAng * a.x;

    zPrime.x -= sinAng * a.y;
    zPrime.y += sinAng * a.x;
    zPrime.z += cosAng;

    CVector3 v = getOrigin();
    setOrigin( v.x * xPrime + v.y * yPrime + v.z * zPrime );
    v = getXAxis();
    setXAxis( v.x * xPrime + v.y * yPrime + v.z * zPrime );
    v = getYAxis();
    setYAxis( v.x * xPrime + v.y * yPrime + v.z * zPrime );
    v = getZAxis();
    setZAxis( v.x * xPrime + v.y * yPrime + v.z * zPrime );
  }
}

void
geomObj::spin( const CVector3& inVecAngle )
{
  spin( Length( inVecAngle ), inVecAngle );
}

void
geomObj::spin( GLfloat inAngle, const CVector3& inAxis )
{ // Rotate around an axis that contains the object's center.
  static CVector3 nullVector = { 0, 0, 0 };
  CVector3 origin = getOrigin();
  setOrigin( nullVector );
  rotate( inAngle, inAxis );
  setOrigin( origin );
}

void
geomObj::scale( GLfloat x, GLfloat y, GLfloat z )
{ // Scale without affecting the object's position.
  setXAxis( x * getXAxis() );
  setYAxis( y * getYAxis() );
  setZAxis( z * getZAxis() );
}

bool
geomObj::VolumeIntersection( const geomObj& p1, const geomObj& p2 )
{
  testResult s = p1.intersectsVolume( p2 );
  if( s == undetermined )
    s = p2.intersectsVolume( p1 );
  return s == true_;
}

