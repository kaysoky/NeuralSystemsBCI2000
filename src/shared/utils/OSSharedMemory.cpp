//////////////////////////////////////////////////////////////////////
// $Id: OSThreadLocal.h 4105 2012-06-13 15:24:54Z mellinger $
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
#include "OSSharedMemory.h"

#if _WIN32
# include <Windows.h>
#else
# include <sys/mman.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#endif // _WIN32

#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace std;

static string RandString()
{
  static struct Init
  { Init() { ::srand( static_cast<unsigned int>( ::time( 0 ) ) ); }
  } init;
  ostringstream oss;
  oss << hex << ::rand();
  return oss.str();
}


OSSharedMemory::OSSharedMemory( const std::string& inName, size_t inSize )
: mName( inName ),
  mServer( inSize != 0 ),
  mSize( inSize ),
  mpMemory( 0 )
{
  Initialize();
}

OSSharedMemory::OSSharedMemory( size_t inSize )
: mServer( true ),
  mSize( inSize ),
  mpMemory( 0 )
{
  Initialize();
}

OSSharedMemory::~OSSharedMemory()
{
  UnmapMemory();
  Close();
  if( mServer )
    Destroy();
}

void
OSSharedMemory::Initialize()
{
  mHandle.fd = -1;
  NormalizeName();
  if( mServer )
    Create();
  else
    Open();
  MapMemory();
}
  
void
OSSharedMemory::NormalizeName()
{
  if( mName.empty() )
    mName = RandString();
  if( mName[0] != '/' )
    mName = '/' + mName;
  size_t pos = 1;
  while( pos = mName.find_first_of( "/\\<>|", pos ) != string::npos )
    mName[pos++] = '_';
  if( mName.length() > 255 )
    mName = mName.substr( 128 );
}

void
OSSharedMemory::Create()
{
#if _WIN32
  mHandle.h = 0;
  while( mHandle.h == 0 )
  {
    mHandle.h = ::CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, mSize, mName.c_str() + 1 );
    if( ::GetLastError() == ERROR_ALREADY_EXISTS )
    {
      mHandle.h = 0;
      mName += RandString();
      NormalizeName();
    }
  }
#else // _WIN32
  errno = 0;
  mHandle.fd = -1;
  while( mHandle.fd < 0 && errno == 0 )
  {
    mHandle.fd = ::shm_open( mName.c_str(), O_RDRW | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR );
    if( mHandle.fd < 0 && errno == EEXIST )
    {
      errno = 0;
      mName += RandString();
      NormalizeName();
    }
  }
  if( mHandle.fd >= 0 && ::ftruncate( mHandle.fd, mSize ) )
  {
    ::close( mHandle.fd );
    mHandle.fd = -1;
  }
#endif // _WIN32
}

void
OSSharedMemory::Destroy()
{
#if _WIN32
  /* will be destroyed after all handles are closed */
#else // _WIN32
  ::shm_unlink( mName.c_str() );
#endif // _WIN32
}

void
OSSharedMemory::Open()
{
#if _WIN32
  mHandle.h = ::OpenFileMappingA( FILE_MAP_ALL_ACCESS, FALSE, mName.c_str() + 1 );
#else // _WIN32
  mHandle.fd = ::shm_open( mName.c_str(), O_RDRW, S_IRUSR | S_IWUSR );
  if( mHandle.fd >= 0 )
  {
    struct stat s = { 0 };
    if( !::fstat( mHandle.fd, &s ) )
      mSize = s.st_size;
  }
#endif // _WIN32
}

void
OSSharedMemory::Close()
{
#if _WIN32
  ::CloseHandle( mHandle.h );
#else // _WIN32
  ::close( mHandle.fd );
#endif // _WIN32
}

void
OSSharedMemory::MapMemory()
{
#if _WIN32
  if( mHandle.h )
    mpMemory = ::MapViewOfFile( mHandle.h, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
#else // _WIN32
  if( mHandle.fd >= 0 )
    mpMemory = ::mmap( 0, 0, PROT_READ | PROT_WRITE, MAP_SHARED, mHandle.fd, 0 );
#endif // _WIN32
}

void
OSSharedMemory::UnmapMemory()
{
  if( !mpMemory )
    return;
#if _WIN32
  ::UnmapViewOfFile( mpMemory );
#else // _WIN32
  ::munmap( mpMemory, mSize );
#endif // _WIN32
  mpMemory = 0;
}
