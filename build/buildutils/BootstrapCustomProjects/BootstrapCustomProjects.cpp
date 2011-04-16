#include <cstdlib> // for system()
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
using namespace std;

#ifdef _WIN32
string gDosLineEnding = "\n"; // looks paradoxical I know.  But *because* we're on windows, this will be auto-translated to \r\n during writing
string gFileSeparator = "\\";
# include "windows.h"
# ifdef _MSC_VER
#  include <direct.h>
#  include "dirent_win.h"
# else // _MSC_VER
#  include <dir.h>
#  include <dirent.h>
# endif // _MSC_VER
#else // _WIN32
string gDosLineEnding = "\r\n"; // we're dealing with template files that have a DOS line-ending format. Let's keep them that way.
string gFileSeparator = "/";
# include <sys/stat.h>
# include <dirent.h>
#endif // _WIN32


typedef string (*string2string) (string x);

bool
DirectoryExists( string x )
{
  DIR * p = ::opendir( x.c_str() );
  bool exists = ( p != NULL );
  if( p ) ::closedir( p );
  return exists;
}

bool
FileExists( string x )
{
	ifstream ifile( x.c_str() );
	return ifile;
}

int
MakeDirectory( string x )
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

string
StandardizePath( string x )
{ // change all occurrences of either '\\' or '/' into the variant appropriate for the current platform, and eliminate multiple slashes (exception: allow Windows-style double slashes at the beginning of a path)
	string y;
	bool ignore = false;
	for( unsigned int i = 0; i < x.size(); i++)
	{
		char c = x[i];
		if( c == '\\' || c == '/' )
		{
			if( i == x.size() - 1 ) break;
			if( !ignore ) y += gFileSeparator;	
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

string
Fullfile( string parent, string child )
{
	if( parent.size() == 0 ) return child;
	return StandardizePath( parent + gFileSeparator + child );
}

string
GetCWD()
{ // get the name of the current directory as a string (empty on failure)
	const int bufSize = 1024;
	char buf[bufSize];
	const char *cwd = ::getcwd( buf, bufSize );
	string result;
	if( cwd ) result = cwd;
	return StandardizePath( result );
}

void
FileParts( string fullpath, string& parent, string& stem, string& extension )
{ // break a string into three parts: directory (before the last slash), stem (after the last slash, before the last dot) and extension (after the last slash, from the last dot onwards)
	fullpath = StandardizePath( fullpath );
	parent = ""; stem = ""; extension = "";
	unsigned int parentLength = fullpath.size();
	for( parentLength = fullpath.size(); parentLength > 0; parentLength-- )
		if( fullpath[parentLength-1] == gFileSeparator[0] ) break;
	unsigned int dotPos=fullpath.size();
	for(  unsigned int i = parentLength; i < fullpath.size(); i++ )
		if( fullpath[i] == '.' ) dotPos = i;
		
	parent = fullpath.substr( 0, parentLength );
	if( parentLength < fullpath.size() ) stem = fullpath.substr( parentLength, dotPos - parentLength );
	if( dotPos < fullpath.size() ) extension = fullpath.substr( dotPos );
}

string
RealPath( string x )
{ // return the absolute path corresponding to path x (if no such directory exists, just return a standardized copy of x)
	if( x.size() == 0 ) return x;
	if( !DirectoryExists( x ) )
	{
		string parent, stem, xtn;
		FileParts( x, parent, stem, xtn );
		return Fullfile( RealPath( parent ), stem + xtn );
	}
	x = StandardizePath( x );
	string oldd = GetCWD();
	int err = ::chdir( x.c_str() );
	if( err == 0 ) x = GetCWD();
	chdir( oldd.c_str() );
	return x;
}

bool
SamePath( string a, string b )
{
	return RealPath( a ) == RealPath( b );
}

string
StripString( string x )
{ // return a version of x from which whitespace has been stripped from the beginning and end
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

string
ProcessCMakeLine( string x )
{ // eliminate comments starting with #, remove whitespace from before and after parentheses and comparators, collapse muliple whitespace into one space, remove leading and trailing whitespace
	string y;
	bool ignoreSpace = true;
	string punct = "()<>=";
	for( unsigned int i = 0; i < x.size(); i++)
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

string
ProcessCPPLine( string x )
{
	string y;
	bool ignoreSpace = true;
	string punct = "()[]{}<>=!,*/-+;&|";
	for( unsigned int i = 0; i < x.size(); i++)
	{
		char c = x[i];
		if( c == '/' && i+1 < x.size() && x[i+1] == '/' )
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

int
BackTickRep( string dst, string src, string name )
{ // Copy file src to dst, but replace all occurrences of the backtick character '`' with the given name
	dst = RealPath( dst );
	src = RealPath( src );
	if( FileExists( dst ) ) { cout << "    file already exists - leaving it unchanged: " << dst << endl; return 0; }
	ifstream in( src.c_str() );
	if( !in ) { cerr << "internal error: failed to open file for reading: " << src << endl; return 1; }
	ofstream out( dst.c_str() );
	if( !out ) { cerr << "internal error: failed to open file for writing: " << dst << endl; return 1; }
	cout << "    created " << dst << endl;
	while( in.good() && in.peek() != EOF )
	{
		char c = in.get();
		if( c == '`' ) out << name;
		else out << c;
	}
	return 0;
}

bool
ContainsLine( string fileName, string targetLine,  string2string proc=NULL, bool partialMatch=false )
{ // test whether or not the named text file contains the target line (before matching, optionally apply string transformation proc to both strings, to cope with syntax invariances) 
	bool linefound = false;
	ifstream s( fileName.c_str() );
	if( proc ) targetLine = proc( targetLine );
	if( s )
	{
		string line;
		while( s.good() )
		{
			getline( s, line );
			if( proc ) line = proc( line );
			if( partialMatch && targetLine.size() < line.size() ) line = line.substr( 0, targetLine.size() );
			if( line == targetLine ) { linefound = true; break; }
		}
		s.close();
	}
	return linefound;
}

int
AppendToFile( string fileName, string line )
{
	ofstream sOut( fileName.c_str(), ofstream::app );
	if( !sOut ) { cerr << "internal error: failed to open " << fileName << " for appending" << endl; return 1; }
	sOut << line << gDosLineEnding;
	sOut.close();
	return 0;
}

int
RemoveLine( string fileName, string targetLine, string2string proc=NULL )
{
	stringstream content;
	if( proc ) targetLine = proc( targetLine );
	ifstream sIn( fileName.c_str() );
	if( !sIn ) { cerr << "internal error: failed to open file " << fileName << " for reading\n"; return 1; }
	while( sIn.good() )
	{
		string line;
		getline( sIn, line );
		if( proc ) line = proc( line );
		if( line != targetLine ) content << line << gDosLineEnding;
	}
	sIn.close();
	ofstream sOut( fileName.c_str() );
	if( !sOut ) { cerr << "internal error: failed to open file " << fileName << " for writing\n"; return 1; }
	sOut << content.str();
	sOut.close();
	return 0;
}

string gDefaultParent = StandardizePath( "../src/custom" );
string gTemplatesDir  = StandardizePath( "./buildutils/BootstrapCustomProjects/templates" );
string gPipeDefError  = "#error    The module will do nothing unless you declare some filters here. Add/uncomment them as appropriate, then remove this error line.";

void
ParseName( string& name, string& parent )
{
	string p, stem, xtn;
	if( DirectoryExists( name ) )
	{
		parent = name;
		name = "";
	}
	else
	{
		FileParts( StripString( name ), p, stem, xtn );
		if( stem.size() ) name = stem;
		if( p.size() ) parent = p;
	}
}
void
ParseName( string& name )
{
	string p;
	ParseName( name, p );
}

int NewFilter(int argc, const char *argv[])
{
	string modtype, name, proj;
	
	if( argc >= 2 ) modtype = argv[1];
	if( argc >= 3 ) { name = argv[2]; ParseName( name, proj ); }
	if( argc >= 4 ) proj = argv[3];

	for( int i = 0; ; i++)
	{
		modtype = StripString( modtype );
		if( modtype == "1" ) modtype = "GenericADC";
		if( modtype == "2" ) modtype = "GenericFilter";
		if( modtype == "3" ) modtype = "ApplicationBase";	
		if( modtype == "GenericADC" || modtype == "GenericFilter" || modtype == "ApplicationBase" ) break;
		
		if( modtype.size() && i == 0 ) { cerr << "unrecognized filter type \"" << modtype << "\" - should be 1, 2 or 3\n"; return 1; }
		if( modtype.size() ) cout << "ERROR: please enter one of the strings exactly, or one of the numbers\n\n";

		cout << "1 GenericADC (for SignalSource modules)\n";
		cout << "2 GenericFilter (for all modules, but usually SignalProcessing)\n";
		cout << "3 ApplicationBase (for application modules)\n";
		cout << "Enter filter type [default is 2]: ";
		getline( cin, modtype );
		if( modtype.size() == 0 ) modtype = "2";
		cout << endl;
	} 
	
	while( name.size() == 0 )
	{
		string p;
		cout << "Enter filter name: ";
		getline( cin, name );
		ParseName( name, p );
		if( name.size() > 0 && proj.size() == 0 ) proj = p;
		if( name.size() ) cout << endl;
	}
	while( proj.size() == 0 )
	{
		cout << "Enter parent module's project directory: ";
		getline( cin, proj );
		proj = StripString( proj );
		if( proj.size() > 0 && !DirectoryExists( proj ))
		{
			cout << "directory " << proj << " does not exist\n";
			proj = "";
		}
		if( proj.size() ) cout << endl;
	}
	proj = RealPath( proj );
	if( !DirectoryExists( proj ) ) { cerr << "directory does not exist: " << proj << endl; return 1; }
	ParseName( name );
	cout << "\nAdding filter " << name << " to module " << proj << endl;
	
	for( unsigned int i = 0; i < name.size(); i++ )
	{
		char c = name[i];
		if( !::isalpha( c ) && !::isdigit( c ) && c != '_' ) { cerr << "\"" << name << "\" is an illegal filter name (name may only contain alphanumeric characters and underscore)\n"; return 1; }
		if( i == 0 && ::isdigit( c ) ) { cerr << "\"" << name << "\" is an illegal filter name (name may not start with a numeral)\n"; return 1; }
	}
	
	string srcname = name + ".cpp";
	string hdrname = name + ".h";
	string templateStem;
	if( modtype == "GenericADC" ) templateStem = "TemplateADC";
	if( modtype == "GenericFilter" ) templateStem = "TemplateFilter";
	if( modtype == "ApplicationBase" ) templateStem = "TemplateApplication";
	
	if( BackTickRep( Fullfile( proj, srcname ), Fullfile( gTemplatesDir, templateStem+".cpp" ), name ) != 0 ) return 1;
	if( BackTickRep( Fullfile( proj, hdrname ), Fullfile( gTemplatesDir, templateStem+".h" ),   name ) != 0 ) return 1;

	string cmFile = RealPath( Fullfile( proj, "CMakeLists.txt" ) );
	cout << endl;
	if( DirectoryExists( cmFile ) ) cout << "*** WARNING: could not operate on " << cmFile << " because it appears to be a directory, not a file\n";
	if( FileExists( cmFile ) )
	{
		stringstream content;
		stringstream msg;
		ifstream sIn( cmFile.c_str() );
		bool lookingForSRC = false, addedSRC = false, lookingForHDR = false, addedHDR = false;
		while( sIn.good() )
		{
			string line;
			getline( sIn, line );
			while( line.size() && line[line.size()-1] == '\r' ) line = line.substr( 0, line.size()-1 );
			string pline = ProcessCMakeLine( line );
			if( lookingForSRC && pline == srcname ) { lookingForSRC = false; cout << "    SRC_PROJECT already contains " << srcname << " in " << cmFile << endl; }
			if( lookingForHDR && pline == hdrname ) { lookingForHDR = false; cout << "    HDR_PROJECT already contains " << hdrname << "   in " << cmFile << endl; }
			if( pline == "SET(SRC_PROJECT" ) lookingForSRC = true;
			if( lookingForSRC && pline == ")" ) { content << "  " << srcname << gDosLineEnding; lookingForSRC = false; addedSRC = true; msg << "    added " << srcname << " to SRC_PROJECT in " << cmFile << endl; }
			if( pline == "SET(HDR_PROJECT" ) lookingForHDR = true;
			if( lookingForHDR && pline == ")" ) { content << "  " << hdrname << gDosLineEnding; lookingForHDR = false; addedHDR = true; msg << "    added " << hdrname << "   to HDR_PROJECT in " << cmFile << endl; }
			content << line << gDosLineEnding;
		}
		sIn.close();
		if( addedSRC || addedHDR )
		{
			ofstream sOut( cmFile.c_str() );
			if( !sOut ) { cerr << "internal error: could not open file for output: " << cmFile << endl; return 1; }
			sOut << content.str();
			cout << msg.str(); 
			content.str("");
			msg.str("");
			sOut.close();
		}
	}
	else cout << "*** WARNING: could not find file " << cmFile << endl ;
	
	string pdFile = RealPath( Fullfile( proj, "PipeDefinition.cpp" ) );
	if( FileExists( pdFile ) )
	{
		string includeLine = "#include \"" + hdrname + "\"";
		string filterLine = "Filter( " + name + ", 2.X );";
		string errorLine = "#error   Change the location token 2.X in the line above, to ensure that " + name + " is sorted to the desired position in the filter sequence. Then, delete this error line";
		cout << endl;
		if( ContainsLine( pdFile, includeLine, ProcessCPPLine ) ) cout << "    " << includeLine << " is already present in " << pdFile << endl;
		else
		{
			if( AppendToFile( pdFile, includeLine ) != 0 ) return 1;
			cout << "    appended " << includeLine << " to " << pdFile << endl;
		}
		if( ContainsLine( pdFile, "Filter("+name+",", ProcessCPPLine, true ) ) cout << "    " << filterLine << " is already present in " << pdFile << endl;
		else
		{
			if( AppendToFile( pdFile, filterLine+gDosLineEnding+errorLine+gDosLineEnding ) != 0 ) return 1;
			cout << "    appended " << filterLine << " to " << pdFile << endl;
		}
		if( RemoveLine( pdFile, gPipeDefError, StripString ) != 0 ) return 1;
	}
	// else cout << "    file does not exist: " << pdFile << endl;
	
	return 0;
}

int NewModule(int argc, const char *argv[])
{

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
		
		if( modtype.size() && i == 0 ) { cerr << "unrecognized module type \"" << modtype << "\" - should be 1, 2, or 3\n"; return 1; }
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
		cout << "Enter parent directory [default (recommended) is " << gDefaultParent << " ]: ";
		getline( cin, parent );
		if( parent.size() == 0 ) parent = gDefaultParent;
		parent = StripString( parent );
		if( parent.size() ) cout << endl;
	}
	string proj = Fullfile( parent, name );

	if( !DirectoryExists( parent ) && MakeDirectory( parent.c_str() ) != 0 ) return 1;
	cout << endl;
	if( DirectoryExists( proj ) ) cout << "directory already exists: " << RealPath( proj ) << endl;
	else
	{
		if( MakeDirectory( proj.c_str() ) != 0 ) return 1;
		cout << "A new " << modtype << " project has been created at " << RealPath( proj ) << endl;
	}
		
	if( BackTickRep( Fullfile( proj, name+".cpp" ), Fullfile( gTemplatesDir, "main.cpp" ), name ) != 0 ) return 1;
	
	if( modtype == "SignalProcessing" )
	{
		string pdFile = Fullfile( proj, "PipeDefinition.cpp" );
		bool adderr = !FileExists( pdFile );
		if( BackTickRep( pdFile, Fullfile( gTemplatesDir, "PipeDefinition.cpp" ), name ) != 0 ) return 1;
		if( adderr && AppendToFile( pdFile, gPipeDefError ) != 0 ) return 1;
	}	
	if( BackTickRep( Fullfile( proj, "CMakeLists.txt" ), Fullfile( gTemplatesDir, "CMakeLists-"+modtype+".txt" ), name ) != 0 ) return 1;
	
	if( modtype == "SignalSource" && 0 ) // TODO: this segfaults. come up with a better alternative to this stupid const char* [] fiddling
	{
		int argc = 4;
		char* argv[3];
		string adcname = name;
		if( name.size() > 6 && name.substr( name.size()-6 ) == "Source" ) adcname = name.substr( 0, name.size()-6 );
		adcname += "ADC";
		string one = "1";
		argv[0] = new char[one.size()+2]; sprintf( argv[0], "%s", one.c_str() );
		argv[1] = new char[adcname.size()+2]; sprintf( argv[1], "%s", adcname.c_str() );
		argv[2] = new char[proj.size()+2]; sprintf( argv[2], "%s", proj.c_str() );
		NewFilter( argc, (const char **)argv );
		delete [] argv[0];
		delete [] argv[1];
		delete [] argv[2];
	}	

	cout << endl;
	string pcmLine = "ADD_SUBDIRECTORY( " + name + " )";
	string pcmFile = RealPath( Fullfile( parent, "CMakeLists.txt" ) );
	bool cmOK = false;
	if( ContainsLine( pcmFile, pcmLine, ProcessCMakeLine ) )
	{
		cout << pcmLine << " is already present in " << pcmFile << endl;
		cmOK = true;
	}
	else
	{
		ofstream cmOut( pcmFile.c_str(), ofstream::app );
		if( cmOut )
		{
			cmOut << pcmLine << gDosLineEnding;
			cmOut.close();
			cout << "The following line has also been appended to " << pcmFile << endl << "    " << pcmLine << endl;
			cmOK = true;
		}
		else cout << "WARNING: failed to open " << pcmFile << " in order to append the line " << pcmLine << endl << endl;
	}
	if( cmOK )
	{
		if( !SamePath( parent, gDefaultParent ) )
			cout << "*** HOWEVER you may need to append ADD_SUBDIRECTORY lines in other locations to\n"
				 << "ensure that cmake gets this far. To avoid this problem, you could have used the\n"
				 << "default location " << gDefaultParent << endl;
		cout << endl;
		cout << "Run CMake again to ensure that this module is included in the build." << endl;
	}
	
	cout << "To expand the module, edit " << RealPath( Fullfile( proj, "CMakeLists.txt" ) ) << endl;
	if( modtype == "SignalProcessing" )
		cout << "                       and " << RealPath( Fullfile( proj, "PipeDefinition.cpp" ) ) << endl;
	cout << endl;
	
	return 0;
}

#ifndef MAIN_FUNCTION
#define MAIN_FUNCTION NewModule
#endif
#ifndef WAIT_AT_END
#define WAIT_AT_END true
#endif

int main( int argc, const char* argv[] )
{
	int result;
	result = MAIN_FUNCTION( argc, argv );
#ifdef _WIN32 // here be lameness
	if( WAIT_AT_END ) { cout << endl; system( "pause" ); }
	return result;
#endif
}
