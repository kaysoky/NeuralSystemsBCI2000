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
#include "PCHIncludes.h"
#pragma hdrstop

#include "cuboids.h"
#include "halfSpace.h"


halfSpace
cuboid::getHalfSpace( int n ) const
{
  CVector3 direction = { 0, 0, 0 };
  switch( n / 2 % 3 )
  {
    case 0:
      direction = getXAxis() * ( mWidth / 2 );
      break;
    case 1:
      direction = getYAxis() * ( mHeight / 2 );
      break;
    case 2:
      direction = getZAxis() * ( mDepth / 2 );
      break;
  }
  if( n % 2 )
    direction = -1 * direction;
  return halfSpace( getOrigin() + direction, -direction );
}


CVector3
cuboid::getVertex( int n ) const
{
  CVector3 deltaX = mWidth / 2 * getXAxis(),
           deltaY = mHeight / 2 * getYAxis(),
           deltaZ = mDepth / 2 * getZAxis();
  float sx = n & 1      ? -1 : 1,
        sy = n & 1 << 1 ? -1 : 1,
        sz = n & 1 << 2 ? -1 : 1;
  return getOrigin() + sx * deltaX + sy * deltaY + sz * deltaZ;
}

geomObj::testResult
cuboid::contains( const CVector3& inPoint ) const
{ // A point is contained in the cuboid when it is contained in all half
  // spaces whose intersection defines the cuboid.
  testResult result = true_;
  for( int i = 0; result == true_ && i < 6; ++i )
    result = this->getHalfSpace( i ).contains( inPoint );
  return result;
}

geomObj::testResult
cuboid::intersectsVolume( const geomObj& inObj ) const
{
  // If any of the object's points lies inside the cuboid, we are done:
  if( true_ == this->contains( inObj.getOrigin() ) )
    return true_;
  // The object's "space point" is outside the cuboid, so we need to test for
  // intersection with the cuboid's half spaces.
  // To intersect the cuboid, the object must intersect all of its half spaces.
  // This is a necessary condition but not sufficient in general,
  // except for convex objects.
  if( true_ != inObj.isConvex() )
    return undetermined;

  testResult result = true_;
  for( int i = 0; result == true_ && i < 6; ++i )
    result = this->getHalfSpace( i ).intersectsVolume( inObj );
  return result;
}

halfSpace
invertedCuboid::getHalfSpace( int n ) const
{
  halfSpace h = cuboid::getHalfSpace( n );
  h.setNormal( -h.getNormal() );
  return h;
}

geomObj::testResult
invertedCuboid::contains( const CVector3& p ) const
{
  testResult result = false_;
  for( int i = 0; result == false_ && i < 6; ++i )
    result = this->getHalfSpace( i ).contains( p );
  return result;
}

geomObj::testResult
invertedCuboid::intersectsVolume( const geomObj& inObj ) const
{
  testResult result = false_;
  for( int i = 0; result == false_ && i < 6; ++i )
    result = this->getHalfSpace( i ).intersectsVolume( inObj );
  return result;
}


void
cuboid::onRender() const
{
  doRender( false );
}

void
cuboid::doRender( bool inInvert ) const
{ // Objects are expected to render themselves in a coordinate system that
  // has the object's center as its origin, and is not rotated.
  glEnable( GL_NORMALIZE ); // This saves us normalizing our normal vectors.
  // Draw the cuboid
  for( int perm = 0; perm < 6; ++perm )
    if( mFaceVisible[perm] )
      drawFace( perm, inInvert );
}

void
cuboid::drawFace( int inPerm, bool inInvertFaces ) const
{
  const int numGridSteps = 10; // how many grid points in each dimension
  // Determine all vectors involved in the computation of vertices/normals.
  float inv = inInvertFaces ? -1 : 1;
  CVector3 e[] =
  { // A right-handed reference frame associated with the xy face:
    { inv * getWidth() / 2, 0, 0 }, // center->right edge
    { 0, inv * getHeight() / 2, 0 }, // center->top edge
    { 0, 0, inv * getDepth() / 2 }  // normal
  };
  // From the permutation number, compute a permutation of the reference frame's
  // basis vectors, and a sign:
  int px = ( inPerm / 2 ) % 3,
      py = ( px + 1 ) % 3,
      pz = ( py + 1 ) % 3;
  float s = inPerm % 2 ? 1 : -1; // The effect of s is to invert the normal, and swap p1 with p2
  
  CVector3 n  =  s*e[pz],
           p0 =  - e[px] -   e[py] + n, // bottom left corner
           p1 = -s*e[px] + s*e[py] + n, // top left corner
           p2 =  s*e[px] - s*e[py] + n, // bottom right corner
           u1 = p1 - p0, // texture coordinate unit vectors
           u2 = p2 - p0;

  for( int i = 0; i < numGridSteps; ++i )
  {
    CVector2 t1 = { 0, (i+0.)/numGridSteps },
             t2 = { 0, (i+1.)/numGridSteps };
    CVector3 q1 = p0 + t1.x * u1 + t1.y * u2,
             q2 = p0 + t2.x * u1 + t2.y * u2;

    glNormal3f( n.x, n.y, n.z );
    glBegin( GL_QUAD_STRIP );
    glTexCoord2f( t1.x, t1.y );
    glVertex3f( q1.x, q1.y, q1.z );
    glTexCoord2f( t2.x, t2.y );
    glVertex3f( q2.x, q2.y, q2.z );
    for( int j = 0; j < numGridSteps; ++j )
    {
      CVector2 t3 = { (j+1.)/numGridSteps, t1.y },
               t4 = { t3.x, t2.y };
      CVector3 q3 = p0 + t3.x * u1 + t3.y * u2,
               q4 = p0 + t4.x * u1 + t4.y * u2;
      glTexCoord2f( t3.x, t3.y );
      glVertex3f( q3.x, q3.y, q3.z );
      glTexCoord2f( t4.x, t4.y );
      glVertex3f( q4.x, q4.y, q4.z );
    }
    glEnd();
  }
};


void
invertedCuboid::onRender() const
{
  cuboid::doRender( true );
}

