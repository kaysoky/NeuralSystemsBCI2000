////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An Environment Extension class that provides facilities for
//         localizing (translating) application modules.
//         This class is instantiated by itself at initialization time.
//         There is at most one instance of this class.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#define UI_VCL // There is no config.h or comparable central instance for such
               // options yet.

#include "Localization.h"
#include "LabelIndex.h"
#include "BCIError.h"

#ifdef UI_VCL
# include <vcl.h>
# include <typinfo.hpp>
#endif // UI_VCL

// Name of the parameter that holds the localized strings.
#define STRINGS_PARAM    "LocalizedStrings"
#define LANG_PARAM       "Language"
#define SECTION          "Application:Localization"
#define DEFAULT_LANGUAGE "Default" // A language ID that means "don't localize".

using namespace std;

Localization Localization::sInstance;

Localization::GUIObjectStringsContainer&
Localization::GUIObjectStrings()
{
  static Localization::GUIObjectStringsContainer GUIObjectStrings_;
  return GUIObjectStrings_;
}

Localization::LocalizedStringsContainer&
Localization::LocalizedStrings()
{
  static Localization::LocalizedStringsContainer localizedStrings;
  return localizedStrings;
}

void
Localization::Publish()
{
  BEGIN_PARAMETER_DEFINITIONS
    SECTION " string " LANG_PARAM "= " DEFAULT_LANGUAGE " " DEFAULT_LANGUAGE " % % //"
      " Language for user messages",
    SECTION " matrix " STRINGS_PARAM "= 0 0 % % % //"
      " Localized user messages",
  END_PARAMETER_DEFINITIONS
}

void
Localization::Preflight() const
{
  // The localizer depends only on the parameters defined inside its
  // constructor. There are no range limitations, and a missing language
  // entry does not constitute an error.
  // However, we emit a warning if the user requires a language that is
  // not present in the LocalizedStrings parameter.
  string userLanguage = OptionalParameter( LANG_PARAM );
  if( userLanguage != "" && userLanguage != DEFAULT_LANGUAGE )
  {
    const LabelIndex& labels = Parameter( STRINGS_PARAM )->RowLabels();
    size_t numLanguages = Parameter( STRINGS_PARAM )->NumRows();
    bool foundLanguage = false;
    for( size_t i = 0; i < numLanguages && !foundLanguage; ++i )
      foundLanguage = ( userLanguage == labels[ i ] );
    if( !foundLanguage )
      bciout << "Language requested in the \"" LANG_PARAM "\" parameter is not "
             << "present in the \"" STRINGS_PARAM "\" parameter."
             << endl;
  }
}

void
Localization::Initialize()
{
  LocalizedStrings().clear();
  string userLanguage = OptionalParameter( LANG_PARAM );
  if( userLanguage != ""
      && userLanguage != DEFAULT_LANGUAGE
      && Parameter( STRINGS_PARAM )->RowLabels().Exists( userLanguage ) )
  {
    size_t numStrings = Parameter( STRINGS_PARAM )->NumColumns();
    const LabelIndex& labels = Parameter( STRINGS_PARAM )->ColumnLabels();
    int languageIndex = Parameter( STRINGS_PARAM )->RowLabels()[ userLanguage ];
    for( size_t i = 0; i < numStrings; ++i )
      LocalizedStrings()[ labels[ i ] ] = Parameter( STRINGS_PARAM )( languageIndex, i );
  }
}

void
Localization::AddLocalizations( const char** inLanguages, int inNumLanguages,
                                const char** inStrings,   int inNumStrings )
{
  sInstance.AddLocalizations_( inLanguages, inNumLanguages, inStrings, inNumStrings );
}

void
Localization::AddLocalizations_( const char** inLanguages, int inNumLanguages,
                                 const char** inStrings,   int inNumStrings ) const
{
  int numLocalizationEntries = inNumStrings / ( inNumLanguages + 1 );
  Param& p = *Parameter( STRINGS_PARAM ).operator->();
  if( p.NumValues() == 0 && inNumStrings > 0 )
  {
    p.SetDimensions( 0, 1 );
    p.ColumnLabels()[ 0 ] = inStrings[ 0 ];
  }
  if( p.NumRows() == 0 && inNumLanguages > 0 )
  {
    p.SetDimensions( 1, 1 );
    p.RowLabels()[ 0 ] = inLanguages[ 0 ];
    p.Value() = "";
  }
  for( int i = 0; i < numLocalizationEntries; ++i )
  {
    size_t dim1 = p.NumRows();
    const char* originalText = inStrings[ i * ( inNumLanguages + 1 ) ];
    if( !p.ColumnLabels().Exists( originalText ) )
    {
      size_t dim2 = p.NumColumns();
      p.SetDimensions( dim1, dim2 + 1 );
      p.ColumnLabels()[ dim2 ] = originalText;
      for( size_t k = 0; k < dim1; ++k )
        p.Value( k, dim2 ) = "";
    }
  }
  for( int j = 0; j < inNumLanguages; ++j )
  {
    size_t dim2 = p.NumColumns();
    const char* languageName = inLanguages[ j ];
    if( !p.RowLabels().Exists( languageName ) )
    {
      size_t dim1 = p.NumRows();
      p.SetDimensions( dim1 + 1, dim2 );
      p.RowLabels()[ dim1 ] = languageName;
      for( size_t k = 0; k < dim2; ++k )
        p.Value( dim1, k ) = "";
    }
  }
  for( int i = 0; i < numLocalizationEntries; ++i )
    for( int j = 0; j < inNumLanguages; ++j )
      Parameter( STRINGS_PARAM )( inLanguages[ j ], inStrings[ i * ( inNumLanguages + 1 ) ] )
               = inStrings[ i * ( inNumLanguages + 1 ) + j + 1 ];
}

const char*
Localization::LocalizableString( const char* inString )
{
  const char* result = inString;
  if( inString && *inString != '\0' )
  {
    const string& localization = LocalizedStrings()[ inString ];
    if( !localization.empty() )
      result = localization.c_str();
  }
  return result;
}

void
Localization::ApplyLocalizations( void* inObject )
{
#ifdef UI_VCL
  TObject* vclObject = reinterpret_cast<TObject*>( inObject );
  if( dynamic_cast<TObject*>( vclObject ) )
  {
    const char* localizableProperties[] =
    {
      "Caption",
      "Text",
    };
    const int numLocalizableProperties
              = sizeof( localizableProperties ) / sizeof( *localizableProperties );

    // We use the VCL Typinfo routines to check whether a given object has one
    // of the properties we consider localizable.
    if( GUIObjectStrings()[ inObject ].size() < numLocalizableProperties )
    {
      GUIObjectStrings()[ inObject ].resize( numLocalizableProperties );
      for( int i = 0; i < numLocalizableProperties; ++i )
        if( Typinfo::IsPublishedProp( vclObject, localizableProperties[ i ] ) )
          GUIObjectStrings()[ inObject ][ i ] = Typinfo::GetStrProp( vclObject,
                                                localizableProperties[ i ] ).c_str();
    }
    for( int i = 0; i < numLocalizableProperties; ++i )
      if( Typinfo::IsPublishedProp( vclObject, localizableProperties[ i ] ) )
        Typinfo::SetStrProp( vclObject, localizableProperties[ i ],
                   LocalizableString( GUIObjectStrings()[ inObject ][ i ].c_str() ) );

    // If the object is a TWinControl, it has sub-controls.
    TWinControl* vclWinControl = dynamic_cast<TWinControl*>( vclObject );
    if( vclWinControl )
      for( int i = 0; i < vclWinControl->ControlCount; ++i )
        ApplyLocalizations( vclWinControl->Controls[ i ] );
  }
#endif // UI_VCL
}





