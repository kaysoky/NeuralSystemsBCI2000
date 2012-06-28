////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A tool that reads a list of project definitions from stdin
//   and creates C++Builder projects from templates.
//   Run this program from where it resides:
//     BMake < BCI2000Projects.txt
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

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>

#include "FileUtils.h"

using namespace std;
using namespace FileUtils;

const string cMark = "#";

string gName, gOutputPath;
set<string> gIncludes, gFiles, gLibs, gResFiles, gForms, gDefines;
vector<string> gProjects, gProjectFiles;
map<string, string> gVariables;

void ClearProject()
{
  gName.clear();
  gIncludes.clear();
  gFiles.clear();
  gLibs.clear();
  gResFiles.clear();
  gForms.clear();
  gDefines.clear();
  gVariables.clear();
}

void ClearProjectGroup()
{
  gProjects.clear();
  gProjectFiles.clear();
}

void SubstituteVariables( string& ioString )
{
  const int maxLevel = 15;
  int level = 0;
  bool didReplace = true;
  while( didReplace )
  {
    if( ++level > maxLevel )
      throw runtime_error(
        "Maximum recursion level reached when substituting "
        "variables"
      );
    didReplace = false;
    map<string, string>::const_iterator i;
    for( i = gVariables.begin(); i != gVariables.end(); ++i )
    {
      size_t pos;
      while( ( pos = ioString.find( i->first ) ) != string::npos )
      {
        ioString.replace( pos, i->first.length(), i->second );
        didReplace = true;
      }
    }
  }
}

istream& Read( istream& is )
{
  string line;
  while( getline( is, line ) && !line.empty() )
  {
    string path = EnsureSeparator( gVariables["#ROOT#"] ) + line;
    if( line.find( "=" ) != string::npos )
    {
      gDefines.insert( line );
    }
    else if( line.find( cMark ) == 0 )
    {
      size_t pos = line.find( cMark, cMark.length() );
      if( pos != string::npos )
      {
        pos += cMark.length();
        string name = line.substr( 0, pos ),
               value = line.substr( pos );
        SubstituteVariables( value );
        gVariables[name] = value;
      }
    }
    else if( IsDirectory( path ) )
    {
      gIncludes.insert( path );
    }
    else if( !ExtractExtension( path ).empty() )
    {
      if( !IsFile( path ) )
        throw runtime_error( "File does not exist: " + path );
      gIncludes.insert( ExtractDirectoryS( path ) );
      string ext = ExtractExtension( path );
      if( !stricmp( ext.c_str(), ".lib" ) )
        gLibs.insert( path );
      else if( !stricmp( ext.c_str(), ".res" ) )
        gResFiles.insert( path );
      else if( !stricmp( ext.c_str(), ".dfm" ) )
        gForms.insert( path );
      else
        gFiles.insert( path );
    }
    else
    {
      istringstream iss( line );
      string includefile, name;
      iss >> includefile;
      if( iss >> name )
        gName = name;
      includefile = InstallationDirectory() + includefile + ".files";
      ifstream file( includefile.c_str() );
      if( !file.is_open() )
        throw runtime_error( "Cannot open include file: " + includefile );
      while( Read( file ) )
        ;
    }
  }
  return is;
}

void FileFromTemplate( const string& inTemplate, const string& inFile )
{
  ofstream output( inFile.c_str() );
  if( !output.is_open() )
    throw runtime_error( "Cannot write to output file: " + inFile );
  string inName = InstallationDirectory() + inTemplate;
  ifstream input( inName.c_str() );
  if( !input.is_open() )
  {
    inName += ".template";
    input.clear();
    input.open( inName.c_str() );
  }
  if( !input.is_open() )
    throw runtime_error( "Could not open template file " + inName );
  string content;
  getline( input, content, '\0' );
  SubstituteVariables( content );
  output.write( content.data(), content.length() );
}

void
WriteProjectGroup()
{
  string name = gVariables["#PROJECTGROUP#"];
  if( name.empty() )
    throw runtime_error( "#PROJECTGROUP# expected" );
  string outName = gOutputPath + name + ".bpg",
         templateName = gVariables["#PROJECTGROUPTEMPLATE#"];
  if( templateName.empty() )
    throw runtime_error( "Project group template not defined" );
  FileFromTemplate( templateName, outName );
}

void
WriteProject()
{
  if( gName.empty() && !gFiles.empty() )
    throw runtime_error( "No name has been set for the current project" );
  if( gName.empty() )
  {
    WriteProjectGroup();
    ClearProjectGroup();
    return;
  }
  string projectsDir = "projects\\";
  MakeDirectory( gOutputPath + projectsDir );
  MakeDirectory( gOutputPath + projectsDir + gName + ".obj" );
  string outName = projectsDir + gName,
         templateName = gVariables["#PROJECTTEMPLATE#"],
         projectFile = outName + ".bpr";
  if( templateName.empty() )
    throw runtime_error( "Project template not defined" );
  FileFromTemplate( templateName, gOutputPath + projectFile );
  string& mainsource = gVariables["#MAINSOURCE#"];
  if( !IsFile( mainsource ) )
    FileFromTemplate( "Mainsource.bpf", gOutputPath + outName + ".bpf" );

  gProjects.push_back( gName );
  gProjectFiles.push_back( projectFile );
}

void
MakeVariables()
{
  gVariables["#NAME#"] = gName;

  string& includes = gVariables["#PATH#"];
  set<string>::const_iterator i = gIncludes.begin();
  if( i != gIncludes.end() )
    includes = *i++;
  for( i; i != gIncludes.end(); ++i )
    includes.append( ";" ).append( *i );

  string& defines = gVariables["#DEFINES#"];
  i = gDefines.begin();
  if( i != gDefines.end() )
    defines = *i++;
  for( ; i != gDefines.end(); ++i )
    defines.append( ";" ).append( *i );

  string& filelist = gVariables["#FILELIST#"];
  for( i = gFiles.begin(); i != gFiles.end(); ++i )
  {
    string unitname = ExtractBase( *i ),
           formname;
    set<string>::const_iterator j = gForms.begin();
    for( ; formname.empty() && j != gForms.end(); ++j )
    {
      if( !stricmp( unitname.c_str(), ExtractBase( *j ).c_str() ) )
      {
        ifstream form( j->c_str() );
        string ignore;
        form >> ws >> ignore >> ws;
        while( form && ::isalnum( form.peek() ) )
          formname += form.get();
        if( !form )
          throw runtime_error( "Could not read form file: " + *j );
      }
    }

    filelist += "      <FILE FILENAME=\"";
    filelist += *i;
    filelist += "\" FORMNAME=\"";
    filelist += formname;
    filelist += "\" UNITNAME=\"";
    filelist += unitname;
    filelist += "\" CONTAINERID=\"CCompiler\" DESIGNCLASS=\"\" LOCALCOMMAND=\"\"/>\n";
  }
  for( i = gLibs.begin(); i != gLibs.end(); ++i )
  {
    filelist += "        <FILE FILENAME=\"";
    filelist += *i;
    filelist += "\" FORMNAME=\"\" UNITNAME=\"";
    filelist += ExtractFile( *i );
    filelist += "\" CONTAINERID=\"LibTool\" DESIGNCLASS=\"\" LOCALCOMMAND=\"\"/>\n";
  }
  for( i = gResFiles.begin(); i != gResFiles.end(); ++i )
  {
    filelist += "        <FILE FILENAME=\"";
    filelist += *i;
    filelist += "\" FORMNAME=\"\" UNITNAME=\"";
    filelist += ExtractFile( *i );
    filelist += "\" CONTAINERID=\"ResTool\" DESIGNCLASS=\"\" LOCALCOMMAND=\"\"/>\n";
  }
  string& mainsource = gVariables["#MAINSOURCE#"];
  if( mainsource.empty() )
    mainsource = gName + ".bpf";
  if( gFiles.find( mainsource ) == gFiles.end() )
  {
    filelist += "        <FILE FILENAME=\"";
    filelist += mainsource;
    filelist += "\" FORMNAME=\"\" UNITNAME=\"";
    filelist += ExtractBase( mainsource );
    filelist += "\" CONTAINERID=\"";
    string ext = ExtractExtension( mainsource );
    if( !ext.empty() )
      ext = ext.substr( 1 );
    filelist += ext;
    filelist += "\" DESIGNCLASS=\"\" LOCALCOMMAND=\"\"/>\n";
  }

  string& objfiles = gVariables["#OBJFILES#"];
  for( i = gFiles.begin(); i != gFiles.end(); ++i )
    objfiles += gName + ".obj\\" + ExtractBase( *i ) + ".obj\n";

  string& libfiles = gVariables["#LIBFILES#"];
  for( i = gLibs.begin(); i != gLibs.end(); ++i )
    libfiles += *i + "\n";

  string& resfiles = gVariables["#RESFILES#"];
  for( i = gResFiles.begin(); i != gResFiles.end(); ++i )
    resfiles += *i + "\n";

  string& forms = gVariables["#FORMS#"];
  for( i = gForms.begin(); i != gForms.end(); ++i )
    forms += *i + "\n";

  string& executables = gVariables["#EXECUTABLES#"];
  executables.clear();
  for( size_t i = 0; i < gProjects.size(); ++i )
    executables += " \\\n" + gProjects[i] + ".exe";

  string& targets = gVariables["#TARGETS#"];
  targets.clear();
  for( size_t i = 0; i < gProjects.size(); ++i )
  {
    targets += gProjects[i] + ".exe: ";
    targets += gProjectFiles[i] + "\n";
    targets += "  $(ROOT)\\bin\\bpr2mak $**\n";
    targets += "  $(ROOT)\\bin\\make -$(MAKEFLAGS) -f$*.mak\n\n";
  }
}

int
main( int inArgc, char** inArgv )
{
  gOutputPath = "../";
  if( inArgc > 1 )
    gOutputPath = EnsureSeparator( inArgv[1] );
  try
  {
    while( cin )
    {
      Read( cin );
      MakeVariables();
      WriteProject();
      ClearProject();
    }
  }
  catch( const exception& e )
  {
    cerr << e.what() << endl;
    return -1;
  }
  return 0;
}

