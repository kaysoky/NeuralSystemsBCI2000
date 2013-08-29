//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A shared memory wrapper class.
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
#ifndef OS_SHARED_MEMORY_H
#define OS_SHARED_MEMORY_H

#include "Uncopyable.h"
#include <string>

class OSSharedMemory : public Uncopyable
{
 public:
  OSSharedMemory( const std::string& name, size_t = 0 );
  OSSharedMemory( size_t );
  ~OSSharedMemory();

  const std::string& Name() const
    { return mName; }
  void* Memory() const
    { return mpMemory; }

 private:
  void Initialize();
  void NormalizeName();
  void Create();
  void Destroy();
  void Open();
  void Close();
  void MapMemory();
  void UnmapMemory();

  std::string mName;
  bool mServer;
  size_t mSize;
  void* mpMemory;
  union { int fd; void* h; } mHandle;
};

#endif // OS_SHARED_MEMORY_H
