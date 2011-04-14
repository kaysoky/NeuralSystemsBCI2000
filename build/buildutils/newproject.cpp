#include <string>
#include <fstream>
#include <iostream>
using namespace std;

#ifdef _WIN32
string FileSeparator = "\\";
# include "windows.h"
# ifdef _MSC_VER
#  include <direct.h>
#  include "dirent_win.h"
# else // _MSC_VER
#  include <dir.h>
#  include <dirent.h>
# endif // _MSC_VER
#else // _WIN32
string FileSeparator = "/";
# include <sys/stat.h>
# include <dirent.h>
#endif // _WIN32


bool DirectoryExists( string x )
{
  DIR * p = ::opendir( x.c_str() );
  bool exists = ( p != NULL );
  if( p ) ::closedir( p );
  return exists;
}
bool FileExists( string x )
{
	ifstream ifile( x.c_str() );
	return ifile;
}
int MakeDirectory( string x )
{
	int err;
#ifdef _WIN32
	err = ::mkdir( x.c_str() );
#else
	const int rwxr_xr_x = 0755;
	err = ::mkdir( x.c_str(), rwxr_xr_x );
#endif
	if(err) cerr << "failed to create directory " << x << endl;
	return err;
}
string StripString( string x )
{
	string y;
	int len = x.size();
	while( len > 0 && isspace( x[len-1] ) ) len--;
	for( int i = 0; i < len; i++ )
	{
		char c = x[i];
		if( y.size() > 0 || !::isspace(c) ) y += c;
	}
	return y;
}
string StandardizePath( string x )
{
	string y;
	bool ignore = false;
	for( int i = 0; i < x.size(); i++)
	{
		char c = x[i];
		if( c == '\\' || c == '/' )
		{
			if( !ignore ) y += FileSeparator;	
			ignore = ( y.size() > 1 );
		}
		else
		{
			y += c;
			ignore = false;
		}
	}
	return y;
}
int BackTickRep( string dst, string src, string name )
{
	dst = StandardizePath( dst );
	src = StandardizePath( src );
	if( FileExists( dst ) ) { cout << "    file " << dst << " already exists - leaving it unchanged\n"; return 0; }
	ifstream in( src.c_str() );
	if( !in ) { cerr << "internal error: failed to open " << src << " for reading\n"; return 1; }
	ofstream out( dst.c_str() );
	if( !out ) { cerr << "internal error: failed to open " << dst << " for writing\n"; return 1; }
	cout << "    created " << dst << endl;
	while( in.good() && in.peek() != EOF )
	{
		char c = in.get();
		if( c == '`' ) out << name;
		else out << c;
	}
	return 0;
}
string ProcessCMakeLine( string x )
{
	string y;
	bool ignoreSpace = true;
	string punct = "()<>=";
	for( int i = 0; i < x.size(); i++)
	{
		char c = x[i];
		if( c == '#' )
		{
			break;
		}
		else if( isspace( c ) )
		{
			if( !ignoreSpace ) y += " ";
			ignoreSpace = true;
		}
		else if( punct.find( c ) != string::npos )
		{
			y = StripString(y) + c;
			ignoreSpace = true;
		}
		else
		{
			y += c;
			ignoreSpace = false;
		}
	}
	return StripString( y );
}

int main(int argc, const char *argv[])
{
	string default_parent = "../src/custom";
	string templates = "./buildutils/templates";

	string modtype;
	if( argc >= 2 ) modtype = argv[1];
	string name;
	if( argc >= 3 ) name = argv[2];
	string parent;
	if( argc >= 4 ) parent = argv[3];
	
	for( int i = 0; ; i++)
	{
		modtype = StripString( modtype );
		if( modtype == "1" ) modtype = "SignalSource";
		if( modtype == "2" ) modtype = "SignalProcessing";
		if( modtype == "3" ) modtype = "Application";	
		if( modtype == "SignalSource" || modtype == "SignalProcessing" || modtype == "Application" ) break;
		
		if( modtype.size() && i == 0 ) { cerr << "unrecognized module type \"" << modtype << "\"\n"; exit(1); }
		if( modtype.size() ) cout << "ERROR: please enter one of the strings exactly, or one of the numbers\n\n";

		cout << "1 SignalSource\n2 SignalProcessing\n3 Application\nEnter module type [default is 2]: ";
		getline( cin, modtype );
		if( modtype.size() == 0 ) modtype = "2";
		cout << endl;
	}
	while( name.size() == 0 )
	{
		cout << "Enter module name: ";
		getline( cin, name );
		name = StripString( name );
		if( name.size() ) cout << endl;
	}
	while( parent.size() == 0 )
	{
		cout << "Enter parent directory [default (recommended) is " << default_parent << " ]: ";
		getline( cin, parent );
		if( parent.size() == 0 ) parent = default_parent;
		parent = StripString( parent );
		if( parent.size() ) cout << endl;
	}
	string proj = StandardizePath( parent + FileSeparator + name );

	if( !DirectoryExists( parent ) && MakeDirectory( parent.c_str() ) != 0 ) exit(1);
	cout << endl;
	if( DirectoryExists( proj ) ) cout << "directory " << proj << " already exists\n";
	else
	{
		if( MakeDirectory( proj.c_str() ) != 0 ) exit(1);
		cout << "A new " << modtype << " project has been created at " << proj << endl;
	}
		
	if( BackTickRep( proj+FileSeparator+name+".cpp", templates+FileSeparator+"main.cpp", name ) != 0 ) exit(1);
	
	if( modtype == "SignalProcessing" )
		if( BackTickRep( proj+FileSeparator+"PipeDefinition.cpp", templates+FileSeparator+"PipeDefinition.cpp", name ) != 0 ) exit(1);
	
	if( BackTickRep( proj+FileSeparator+"CMakeLists.txt", templates+FileSeparator+"CMakeLists-"+modtype+".txt", name ) != 0 ) exit(1);
	
	cout << endl;
	string cmakeline = "ADD_SUBDIRECTORY( " + name + " )";
	string file = StandardizePath( parent + FileSeparator + "CMakeLists.txt" );
	bool linefound = false, cmOK = false;
	ifstream cmIn( file.c_str() );
	if( cmIn )
	{
		string line;
		while( cmIn.good() )
		{
			getline( cmIn, line );
			if( ProcessCMakeLine( line ) == ProcessCMakeLine( cmakeline ) ) { linefound = true; break; }
		}
		cmIn.close();
	}		
	if( linefound )
	{
		cout << file << " already contains the line " << cmakeline << endl;
		cmOK = true;
	}
	else
	{
		ofstream cmOut( file.c_str(), ofstream::app );
		if( cmOut )
		{
			cmOut << cmakeline << endl;
			cmOut.close();
			cout << "The following line has also been appended to " << file << endl << "    " << cmakeline << endl;
			cmOK = true;
		}
		else cout << "WARNING: failed to open " << file << " in order to append the line " << cmakeline << endl << endl;
	}
	if( cmOK )
	{
		if( StandardizePath( parent+FileSeparator ) != StandardizePath( default_parent+FileSeparator ) )
			cout << "*** HOWEVER you may need to append ADD_SUBDIRECTORY lines in other locations to\n"
				 << "ensure that cmake gets this far. To avoid this problem, you could have used the\n"
				 << "default location " << default_parent << endl;
		cout << endl;
		cout << "Run CMake again to ensure that this project is included in the build." << endl;
	}
	cout << "To expand the project, edit " << StandardizePath( proj+FileSeparator+"CMakeLists.txt" ) << endl;
	if( modtype == "SignalProcessing" )
		cout << "                        and " << StandardizePath( proj+FileSeparator+"PipeDefinition.cpp" ) << endl;
	cout << endl;
	
	return 0;


}
