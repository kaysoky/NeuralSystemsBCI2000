////////////////////////////////////////////////////////////////////////////////
//
// File:   Localization.cpp
//
// Date:   Nov 21, 2003
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: An Environment Extension class that provides facilities for
//         localizing (translating) application modules.
//         This class is instantiated by itself at initialization time.
//         There is at most one instance of this class.
//
//         To add translations to an existing application module, take care of the
//         following:
//         - There are two parameters that govern the behavior of the
//           Localization class:
//           "Localization" defines the language to translate the strings
//           into; if its value matches one of the "LocalizedStrings" row
//           labels, translations will be taken from that row; otherwise,
//           strings will not be translated.
//           "LocalizedStrings" defines string translations. Strings that
//           don't appear as a column label will not be translated.
//           Also, strings with an empty translation entry in "LocalizedStrings"
//           will not be translated.
//         - To provide your translations in a filter constructor, add a line
//             LANGUAGES "Italian", "French",
//           to enumerate the languages you provide translations for,
//           followed by
//             BEGIN_LOCALIZED_STRINGS
//           and a list of translated strings where each entry has the form
//             "Original string", "Italian translation", "French translation",
//           and finally a line
//             END_LOCALIZED_STRINGS
//           to end the enumeration.
//           Even if you don't provide translations this way, operator users may
//           still add their own translations for any language by editing
//           the LocalizedStrings parameter.
//         - Mark localizable string constants by the symbol
//           LocalizableString, i.e.
//             TellUser( LocalizableString( "Well done!" ) );
//           instead of
//             TellUser( "Well done!" );
//         - For GUI elements (e.g. VCL forms) visible to the subject, put a call
//             ApplyLocalizations( pointerToTheGUIElement );
//           inside the Initialize() method of the form's associated
//           GenericFilter.
//         - You should not use LocalizableString on string constants used
//           before the first call to GenericFilter::Initialize() or for
//           initializing static objects of any kind because localization
//           information used will not be valid, or the string may not be
//           updated appropriately then.
//         - Language names are case-insensitive. You may use any legal C
//           identifier for a language name but as a convention we suggest
//           its most common English name, as in
//             Italian, Dutch, French, German
//           with international country abbreviations as optional regional
//           qualifiers as in
//             EnglishUS, EnglishGB, GermanD, GermanCH
//           if that should ever be necessary.
//         - For now, encoding of non-ASCII characters follows the UTF8
//           convention. To ensure platform independent readability of source
//           code files, please use the macros that define HTML character names
//           to their UTF8 encoded string as in
//             "Dansk Sm" oslash "rrebr" oslash "d".
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#define UI_VCL // There is no config.h or comparable central instance for such
               // options yet.

#include "Localization.h"
#include "UBCIError.h"
#include "UGenericFilter.h" // For the *_PARAMETER_DEFINITIONS macros.

#ifdef UI_VCL
# include <vcl.h>
# include <typinfo.hpp>
#endif // UI_VCL

#pragma package(smart_init)

// Name of the parameter that holds the localized strings.
#define STRINGS_PARAM    "LocalizedStrings"
#define LANG_PARAM       "Localization"
#define DEFAULT_LANGUAGE "Default" // A language ID that means "don't localize".

using namespace std;

Localization Localization::sInstance;
Localization::GUIObjectStringsContainer Localization::sGUIObjectStrings;
Localization::LocalizedStringsContainer Localization::sLocalizedStrings;

void
Localization::Publish()
{
  BEGIN_PARAMETER_DEFINITIONS
    "UsrTask string " LANG_PARAM "= " DEFAULT_LANGUAGE " " DEFAULT_LANGUAGE " % % //"
      " Language for user messages",
    "UsrTask matrix " STRINGS_PARAM "= 0 0 % % % //"
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
  string userLanguage = ( const char* )OptionalParameter( LANG_PARAM );
  if( userLanguage != "" && userLanguage != DEFAULT_LANGUAGE )
  {
    const PARAM::labelIndexer& labels = Parameter( STRINGS_PARAM )->LabelsDimension1();
    size_t numLanguages = Parameter( STRINGS_PARAM )->GetNumValuesDimension1();
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
  sLocalizedStrings.clear();
  string userLanguage = ( const char* )OptionalParameter( LANG_PARAM );
  if( userLanguage != ""
      && userLanguage != DEFAULT_LANGUAGE
      && Parameter( STRINGS_PARAM )->LabelsDimension1().Exists( userLanguage ) )
  {
    size_t numStrings = Parameter( STRINGS_PARAM )->GetNumValuesDimension2();
    const PARAM::labelIndexer& labels = Parameter( STRINGS_PARAM )->LabelsDimension2();
    int languageIndex = Parameter( STRINGS_PARAM )->LabelsDimension1()[ userLanguage ];
    for( size_t i = 0; i < numStrings; ++i )
      sLocalizedStrings[ labels[ i ] ] = ( const char* )Parameter( STRINGS_PARAM, languageIndex, i );
  }
}

void
Localization::AddLocalizations( const char** inLanguages, int inNumLanguages,
                                const char** inStrings,   int inNumStrings )
{
  sInstance._AddLocalizations( inLanguages, inNumLanguages, inStrings, inNumStrings );
}

void
Localization::_AddLocalizations( const char** inLanguages, int inNumLanguages,
                                 const char** inStrings,   int inNumStrings ) const
{
  int numLocalizationEntries = inNumStrings / ( inNumLanguages + 1 );
  PARAM& p = *Parameter( STRINGS_PARAM ).operator->();
  if( p.GetNumValues() == 0 && inNumStrings > 0 )
  {
    p.SetDimensions( 0, 1 );
    p.LabelsDimension2()[ 0 ] = inStrings[ 0 ];
  }
  if( p.GetNumValuesDimension1() == 0 && inNumLanguages > 0 )
  {
    p.SetDimensions( 1, 1 );
    p.LabelsDimension1()[ 0 ] = inLanguages[ 0 ];
    p.SetValue( "" );
  }
  for( int i = 0; i < numLocalizationEntries; ++i )
  {
    size_t dim1 = p.GetNumValuesDimension1();
    const char* originalText = inStrings[ i * ( inNumLanguages + 1 ) ];
    if( !p.LabelsDimension2().Exists( originalText ) )
    {
      size_t dim2 = p.GetNumValuesDimension2();
      p.SetDimensions( dim1, dim2 + 1 );
      p.LabelsDimension2()[ dim2 ] = originalText;
      for( size_t k = 0; k < dim1; ++k )
        p.SetValue( "", k, dim2 );
    }
  }
  for( int j = 0; j < inNumLanguages; ++j )
  {
    size_t dim2 = p.GetNumValuesDimension2();
    const char* languageName = inLanguages[ j ];
    if( !p.LabelsDimension1().Exists( languageName ) )
    {
      size_t dim1 = p.GetNumValuesDimension1();
      p.SetDimensions( dim1 + 1, dim2 );
      p.LabelsDimension1()[ dim1 ] = languageName;
      for( size_t k = 0; k < dim2; ++k )
        p.SetValue( "", dim1, k );
    }
  }
  for( int i = 0; i < numLocalizationEntries; ++i )
    for( int j = 0; j < inNumLanguages; ++j )
      Parameter( STRINGS_PARAM, inLanguages[ j ], inStrings[ i * ( inNumLanguages + 1 ) ] )
               = inStrings[ i * ( inNumLanguages + 1 ) + j + 1 ];
}

const char*
Localization::LocalizableString( const char* inString )
{
  const char* result = inString;
  if( inString && *inString != '\0' )
  {
    const string& localization = sLocalizedStrings[ inString ];
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
    if( sGUIObjectStrings[ inObject ].size() < numLocalizableProperties )
    {
      sGUIObjectStrings[ inObject ].resize( numLocalizableProperties );
      for( int i = 0; i < numLocalizableProperties; ++i )
        if( Typinfo::IsPublishedProp( vclObject, localizableProperties[ i ] ) )
          sGUIObjectStrings[ inObject ][ i ] = Typinfo::GetStrProp( vclObject,
                                                localizableProperties[ i ] ).c_str();
    }
    for( int i = 0; i < numLocalizableProperties; ++i )
      if( Typinfo::IsPublishedProp( vclObject, localizableProperties[ i ] ) )
        Typinfo::SetStrProp( vclObject, localizableProperties[ i ],
                   LocalizableString( sGUIObjectStrings[ inObject ][ i ].c_str() ) );

    // If the object is a TWinControl, it has sub-controls.
    TWinControl* vclWinControl = dynamic_cast<TWinControl*>( vclObject );
    if( vclWinControl )
      for( int i = 0; i < vclWinControl->ControlCount; ++i )
        ApplyLocalizations( vclWinControl->Controls[ i ] );
  }
#endif // UI_VCL
}
