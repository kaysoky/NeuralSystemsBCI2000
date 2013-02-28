// A tool to demangle cpp mangled/decorated names.
#include <iostream>
#include <string>

#if _WIN32
# include <Windows.h>
# include <Dbghelp.h>
#endif

#if _GNUC_
# include <cxxabi.h>
#endif

using namespace std;

int main( int argc, char** argv )
{
  char buf[2048];
  string s;
  while( getline( cin, s ) )
  {
    string d;
#if _WIN32
    int flags = UNDNAME_NO_ACCESS_SPECIFIERS | UNDNAME_NO_MEMBER_TYPE;
    if( ::UnDecorateSymbolName( s.c_str(), buf, sizeof( buf ) - 1, flags ) )
      d = buf;
#endif // _WIN32
#if _GNUC_
    if( d.empty() || d == s )
    {
      int status = 0;
      char* p = abi::__cxa_demangle( s.c_str(), 0, 0, &status );
      if( status == 0 )
        s = p;
      if( p )
        ::free( p );
    }
#endif // _GNUC_
     cout << ( d.empty() ? s : d ) << "\n";
  }
  return 0;
}
