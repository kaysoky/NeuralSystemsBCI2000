////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class representing the volume of a half space.
//   The half space is the set of all points lying "above" a given plane,
//   including the plane itself.
//   The plane is defined by a space point and a normal vector.
//   The normal vector points _into_ the half space.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
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
