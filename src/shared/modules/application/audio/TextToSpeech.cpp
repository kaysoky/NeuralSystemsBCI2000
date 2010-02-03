////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A simple wrapper class for text-to-speech audio output.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TextToSpeech.h"
#include "BCIError.h"
#include <string>
#include <SAPI.h>

using namespace std;

int TextToSpeech::sNumInstances = 0;

TextToSpeech::TextToSpeech()
: mpVoice( NULL )
{
  if( sNumInstances++ < 1 )
    ::CoInitialize( NULL );
  ::CoCreateInstance( CLSID_SpVoice, NULL, CLSCTX_ALL,
                        IID_ISpVoice, reinterpret_cast<void**>( &mpVoice ) );
  if( mpVoice == NULL )
    bcierr << "Could not initialize text-to-speech subsystem" << endl;
}

TextToSpeech::~TextToSpeech()
{
  if( --sNumInstances < 1 )
    ::CoUninitialize();
  if( mpVoice != NULL )
    mpVoice->Release();
}

TextToSpeech&
TextToSpeech::Speak()
{
  if( mpVoice != NULL )
  {
    const ctype<wchar_t>& ct = use_facet< ctype<wchar_t> >( locale() );
    wstring ws( mText.size(), '\0' );
    for( size_t i = 0; i < mText.size(); ++i )
        ws[i] = ct.widen( mText[i] );
    mpVoice->SetVolume( mVolume * 100 );
    if( S_OK != mpVoice->Speak(
                  ws.c_str(),
                  SPF_ASYNC | SPF_PURGEBEFORESPEAK,
                  NULL ) )
      bcierr << "Could not speak text \"" << mText << "\"" << endl;
  }
  return *this;
}

TextToSpeech&
TextToSpeech::Stop()
{
  if( mpVoice != NULL )
    mpVoice->Speak( NULL, SPF_ASYNC | SPF_PURGEBEFORESPEAK, NULL );
  return *this;
}

