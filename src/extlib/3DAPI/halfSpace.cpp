////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class representing the volume of a half space.
//   The half space is the set of all points lying "above" a given plane,
//   including the plane itself.
//   The plane is defined by a space point and a normal vector.
//   The normal vector points _into_ the half space.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "halfSpace.h"
#include "sphere.h"
#include "cuboids.h"

#include <cmath>

using namespace std;

geomObj::testResult
halfSpace::intersectsVolume( const geomObj& inObj ) const
{
  testResult result = undetermined;
  const sphere* pSphere = dynamic_cast<const sphere*>( &inObj );
  if( pSphere )
  { // A sphere intersects a half space if its center's distance to the plane
    // is below its radius, or if it is beyond the plane.
    CVector3 sphereCenter = pSphere->getOrigin();
    float radius = pSphere->getRadius(),
          length = ::sqrt( mNormal * mNormal );
    result = ( sphereCenter - this->getOrigin() ) * mNormal >= -radius * length
             ? true_
             : false_;
  }
  const cuboid* pCuboid = dynamic_cast<const cuboid*>( &inObj );
  if( pCuboid )
  { // A cuboid intersects a half space if at least one of its vertices is in the half space.
    result = false_;
    for( int i = 0; result == false_ && i < 8; ++i )
      result = this->contains( pCuboid->getVertex( i ) );
  }
  return result;
}


