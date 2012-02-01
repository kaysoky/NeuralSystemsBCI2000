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
#ifndef HALF_SPACE_H
#define HALF_SPACE_H

#include "geomObj.h"

class halfSpace : public geomObj
{
private:
        CVector3 mNormal;
public:
        halfSpace( const CVector3& spacePoint, const CVector3& normal )
          : mNormal( normal ) { setOrigin( spacePoint ); }
        void setNormal( const CVector3& n )
          { mNormal = n; }
        const CVector3& getNormal() const
          { return mNormal; }

        virtual testResult isConvex() const
          { return true_; }
        virtual testResult contains( const CVector3& p ) const
          { return ( p - getOrigin() ) * mNormal >= 0 ? true_ : false_; }

        virtual testResult intersectsVolume( const geomObj& ) const;
};

#endif // HALF_SPACE_H
