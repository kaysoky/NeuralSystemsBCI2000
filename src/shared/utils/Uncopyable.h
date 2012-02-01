//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Inherit from this class to forbid copying and
//   assignment between instances of your derived class.
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
///////////////////////////////////////////////////////////////////////
#ifndef UNCOPYABLE_H
#define UNCOPYABLE_H

class Uncopyable
{
 protected:
  Uncopyable() {}
  ~Uncopyable() {}

 private:
  Uncopyable( const Uncopyable& );
  Uncopyable& operator=( const Uncopyable& );

#if __BORLANDC__  // BCB fails on multiple inheritance from classes without data.
  char mData;
#endif // __BORLANDC__
};

#endif // UNCOPYABLE_H
