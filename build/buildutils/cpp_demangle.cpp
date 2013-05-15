// A tool to demangle cpp mangled/decorated names.
#include <iostream>
#include <string>

#if _WIN32
# include <Windows.h>
#endif

#if __GNUC__
# include <malloc.h>
# include <cxxabi.h>
#endif

using namespace std;

#if _WIN32
#define UNDNAME_NO_ACCESS_SPECIFIERS 0x80
#define UNDNAME_NO_MEMBER_TYPE 0x200
typedef DWORD (WINAPI *pUnDecorateSymbolNameA)( PCSTR, PSTR, DWORD, DWORD );
static pUnDecorateSymbolNameA UnDecorateSymbolNameA = 0;
#endif // _WIN32

int main( int argc, char** argv )
{
#if _WIN32
  HMODULE hLib = ::LoadLibraryA( "dbghelp" );
  if( hLib )
    UnDecorateSymbolNameA = (pUnDecorateSymbolNameA)::GetProcAddress( hLib, "UnDecorateSymbolNameA" );
#endif
  char buf[2048];
  string s;
  while( getline( cin, s ) )
  {
    string d;
#if _WIN32
    int flags = UNDNAME_NO_ACCESS_SPECIFIERS | UNDNAME_NO_MEMBER_TYPE;
    if( UnDecorateSymbolNameA &&
        UnDecorateSymbolNameA( s.c_str(), buf, sizeof( buf ) - 1, flags )
    )
      d = buf;
#endif // _MSC_VER
#if __GNUC__
    if( d.empty() || d == s )
    {
      int status = 0;
      char* p = abi::__cxa_demangle( s.c_str(), 0, 0, &status );
      if( status == 0 )
        s = p;
      if( p )
        ::free( p );
    }
#endif // __GNUC__
     cout << ( d.empty() ? s : d ) << "\n";
  }
  return 0;
}
