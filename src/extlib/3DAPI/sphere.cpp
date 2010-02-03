////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: A 3D sphere.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "sphere.h"

geomObj::testResult
sphere::contains( const CVector3& inPoint ) const
{
  CVector3 center = getOrigin();
  return ( inPoint - center ) * ( inPoint - center ) <= mRadius
         ? true_
         : false_;
}

geomObj::testResult
sphere::intersectsVolume( const geomObj& inObj ) const
{
  testResult result = undetermined;
  const sphere* pSphere = dynamic_cast<const sphere*>( &inObj );
  if( pSphere )
  {
    CVector3 center1 = getOrigin(),
             center2 = pSphere->getOrigin();
    float radSum = mRadius + pSphere->mRadius;
    result = ( center1 - center2 ) * ( center1 - center2 ) <= radSum * radSum
             ? true_
             : false_;
  }
  return result;
}

void
sphere::onRender() const
{
  GLUquadricObj* q = gluNewQuadric();  // Create A New Quadric
  gluQuadricNormals( q, GL_SMOOTH );   // Generate Smooth Normals For The Quad
  gluQuadricTexture( q, GL_TRUE );     // Enable Texture Coords For The Quad
  gluSphere( q, mRadius, 100, 100 ); // Draw  Sphere
  gluDeleteQuadric( q );               // free the memory
}

