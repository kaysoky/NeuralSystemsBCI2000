////////////////////////////////////////////////////////////////////////////////
//
// File: UOperatorUtils.cpp
//
// Date: June 27, 2002
//
// Description: A file intended to hold global utility functions common to
//              different operator sources.
//
// Changes: June 27, 2002, juergen.mellinger@uni-tuebingen.de:
//          - Created file.
//          - Moved UpdateState() from ../shared/UState.h to here.
//          June 10, 2004, juergen.mellinger@uni-tuebingen.de:
//          - Removed UpdateState() -- use TfMain::UpdateState() instead.
//          Dec 30, 2004, juergen.mellinger@uni-tuebingen.de:
//          - Moved User Level accessors and Matrix I/O functions from TfMain
//            to here.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UOperatorUtils.h"
#include "UPreferences.h"
#include "UParameter.h"
#include "defines.h"

#include <VCL.h>
#include <Registry.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <typeinfo>

using namespace std;

void
OperatorUtils::SaveControl( const TControl* c )
{
  TRegistry* reg = NULL;
  try
  {
    reg = new TRegistry( KEY_WRITE );
    reg->RootKey = HKEY_CURRENT_USER;
    reg->OpenKey( ( string( KEY_BCI2000 KEY_OPERATOR KEY_CONFIG "\\" ) + typeid( *c ).name() ).c_str(), true );
    reg->WriteInteger( "Left", c->Left );
    reg->WriteInteger( "Top", c->Top );
    reg->WriteInteger( "Height", c->Height );
    reg->WriteInteger( "Width", c->Width );
  }
  catch( ERegistryException& )
  {
  }
  delete reg;
}

void
OperatorUtils::RestoreControl( TControl* c )
{
  TRegistry* reg = NULL;
  try
  {
    reg = new TRegistry( KEY_READ );
    reg->RootKey = HKEY_CURRENT_USER;
    reg->OpenKey( ( string( KEY_BCI2000 KEY_OPERATOR KEY_CONFIG "\\" ) + typeid( *c ).name() ).c_str(), false );
    TRect storedRect;
    storedRect.Left = reg->ReadInteger( "Left" );
    storedRect.Top = reg->ReadInteger( "Top" );
    storedRect.Right = storedRect.Left + reg->ReadInteger( "Width" );
    storedRect.Bottom = storedRect.Top + reg->ReadInteger( "Height" );
    const int minDistance = 10; // Make sure that at least that much of the window is
                                // inside the screen.
    TRect commonRect;
    if( Types::IntersectRect( commonRect, storedRect, Screen->WorkAreaRect )
        && commonRect.Height() >= minDistance
        && commonRect.Width() >= minDistance )
    {
      c->Left = storedRect.Left;
      c->Top = storedRect.Top;
      c->Height = storedRect.Height();
      c->Width = storedRect.Width();
    }
  }
  catch( ERegistryException& )
  {
  }
  delete reg;
}

int
OperatorUtils::UserLevel()
{
  if( fPreferences && fPreferences->preferences )
    return fPreferences->preferences->UserLevel;
  return USERLEVEL_BEGINNER;
}

// retrieves the user level of one particular parameter
int
OperatorUtils::GetUserLevel( const char* inName )
{
TRegistry       *my_registry;
AnsiString      keyname;
int             ret;
TStringList     *value_names;

 my_registry=new TRegistry();
 value_names=new TStringList();

 keyname=AnsiString(KEY_BCI2000 KEY_OPERATOR KEY_PARAMETERS "\\" ) + inName;
 ret=USERLEVEL_ADVANCED;
 if (my_registry->OpenKey(keyname, false))
    {
    my_registry->GetValueNames(value_names);
    // let's check whether the value "UserLevel" actually exists
    // (so that we don't throw many exceptions)
    if (value_names->IndexOf("UserLevel") > -1)
       {
       try  // if it gets here, it should actually exist
        {
        ret=my_registry->ReadInteger("UserLevel");
        }
       catch(...) {;}
       }
    }

 delete my_registry;
 delete value_names;
 return(ret);
}

// sets the user level of one particular parameter
void
OperatorUtils::SetUserLevel( const char* inName , int userlevel)
{
TRegistry       *my_registry;
AnsiString      keyname;

 my_registry=new TRegistry();

 try
  {
  keyname=AnsiString(KEY_BCI2000 KEY_OPERATOR KEY_PARAMETERS "\\" ) + inName;
  my_registry->CreateKey(keyname);
  }
 catch (...)
  {;}

 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     my_registry->WriteInteger("UserLevel", userlevel);
     }
    catch(...)
     {;}
    }

  delete my_registry;
}

int
OperatorUtils::LoadMatrix( const char* inFileName, PARAM& outParam )
{
  vector<vector<string> > matrix;

  ifstream input( inFileName );
  string line;
  while( getline( input, line ) )
  {
    istringstream is( line );
    vector<string> row;
    string value;
    while( is >> value )
      row.push_back( value );
    if( !row.empty() )
      matrix.push_back( row );
  }
  if( matrix.empty() )
    return ERR_MATNOTFOUND;

  size_t numRows = matrix.size(),
         numCols = matrix[ 0 ].size();
  for( size_t row = 1; row < numRows; ++row )
    if( matrix[ row ].size() != numCols )
      return ERR_MATLOADCOLSDIFF;

  outParam.SetDimensions( numRows, numCols );
  for( size_t row = 0; row < numRows; ++row )
    for( size_t col = 0; col < numCols; ++col )
      outParam.SetValue( matrix[ row ][ col ], row, col );

  return ERR_NOERR;
}

int
OperatorUtils::SaveMatrix( const char* inFileName, const PARAM& inParam ) 
{
  ofstream output( inFileName );
  for( size_t row = 0; row < inParam.GetNumValuesDimension1(); ++row )
  {
    for( size_t col = 0; col < inParam.GetNumValuesDimension2(); ++col )
      output << ' ' << setw( 8 ) << inParam.GetValue( row, col );
    output << endl;
  }
  return output ? ERR_NOERR : ERR_COULDNOTWRITE;
}

