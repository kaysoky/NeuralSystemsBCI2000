////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: A file intended to hold global utility functions common to
//              various operator source files.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UOperatorUtils.h"
#include "UPreferences.h"
#include "Param.h"
#include "Operator.h"
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
OperatorUtils::SaveControl( const TControl* inControl, const string& inAppKey )
{
  TRegistry* pReg = NULL;
  try
  {
    bool savePosition = true;
    const TForm* pForm = dynamic_cast<const TForm*>( inControl );
    if( pForm )
      savePosition = ( pForm->WindowState == wsNormal );
    if( savePosition )
    {
      pReg = new TRegistry( KEY_WRITE );
      pReg->RootKey = HKEY_CURRENT_USER;
      string key = string( KEY_BCI2000 ) + inAppKey + KEY_CONFIG "\\" + typeid( *inControl ).name();
      pReg->OpenKey( key.c_str(), true );
      pReg->WriteInteger( "Left", inControl->Left );
      pReg->WriteInteger( "Top", inControl->Top );
      pReg->WriteInteger( "Height", inControl->Height );
      pReg->WriteInteger( "Width", inControl->Width );
    }
  }
  catch( ERegistryException& )
  {
  }
  delete pReg;
}

void
OperatorUtils::RestoreControl( TControl* inControl, const string& inAppKey )
{
  TRegistry* pReg = NULL;
  try
  {
    pReg = new TRegistry( KEY_READ );
    pReg->RootKey = HKEY_CURRENT_USER;
    string key = string( KEY_BCI2000 ) + inAppKey + KEY_CONFIG "\\" + typeid( *inControl ).name();
    pReg->OpenKey( key.c_str(), false );
    TRect storedRect;
    storedRect.Left = pReg->ReadInteger( "Left" );
    storedRect.Top = pReg->ReadInteger( "Top" );
    storedRect.Right = storedRect.Left + pReg->ReadInteger( "Width" );
    storedRect.Bottom = storedRect.Top + pReg->ReadInteger( "Height" );
    const int minDistance = 10; // Make sure that at least that much of the window is
                                // inside the screen.
    TRect commonRect;
    if( Types::IntersectRect( commonRect, storedRect, Screen->WorkAreaRect )
        && commonRect.Height() >= minDistance
        && commonRect.Width() >= minDistance )
    {
      inControl->Left = storedRect.Left;
      inControl->Top = storedRect.Top;
      inControl->Height = storedRect.Height();
      inControl->Width = storedRect.Width();
    }
  }
  catch( ERegistryException& )
  {
  }
  delete pReg;
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
OperatorUtils::LoadMatrix( const char* inFileName, Param& outParam )
{
  if( inFileName == NULL )
    return ERR_MATNOTFOUND;

  vector<vector<string> > matrix;

  ifstream input( inFileName );
  string line;
  while( getline( input, line ) )
  {
    istringstream is( line );
    vector<string> row;
    string value;
    while( getline( is, value, '\t' ) )
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
      outParam.Value( row, col ) = matrix[ row ][ col ];

  return ERR_NOERR;
}

int
OperatorUtils::SaveMatrix( const char* inFileName, const Param& inParam )
{
  if( inFileName == NULL )
	return ERR_COULDNOTWRITE;

  ofstream output( inFileName );
  for( int row = 0; row < inParam.NumRows(); ++row )
  {
    int col = 0;
    while( col < inParam.NumColumns() - 1 )
      output << inParam.Value( row, col++ ) << '\t';
    output << inParam.Value( row, col ) << endl;
  }
  return output ? ERR_NOERR : ERR_COULDNOTWRITE;
}

