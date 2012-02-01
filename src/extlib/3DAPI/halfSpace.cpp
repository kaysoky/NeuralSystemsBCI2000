////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class representing the volume of a half space.
//   The half space is the set of all points lying "above" a given plane,
//   including the plane itself.
//   The plane is defined by a space point and a normal vector.
//   The normal vector points _into_ the half space.
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


