////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that displays a dialog window showing
//   the BCI2000 logo, and versioning information.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "AboutBox.h"
#include "Version.h"
#include "GUI.h"
#include "images/BCI2000logo_mini.h"
#include <sstream>

using namespace std;
using namespace GUI;

AboutBox::AboutBox()
{
  SetVersionInfo( BCI2000_VERSION );
}

AboutBox&
AboutBox::SetVersionInfo( const std::string& s )
{
  istringstream iss( s );
  iss >> mVersionInfo;
  return *this;
}

AboutBox&
AboutBox::Display() const
{
  VersionInfo info = mVersionInfo;
  string versionNumber = info[ "Version" ];
  info.erase( "Version" );
  string versionDetails;
  for( VersionInfo::reverse_iterator i = info.rbegin(); i != info.rend(); ++i )
    versionDetails += ", " + i->first + ": " + i->second;
  versionDetails.erase( 0, 2 );

#ifdef __BORLANDC__
  TForm* pForm = new TForm( static_cast<TComponent*>( NULL ) );
  pForm->BorderStyle = bsDialog;
  pForm->Position = poScreenCenter;
  pForm->Caption = ( string( "About " ) + mApplicationName + "..." ).c_str();

  TPanel* pPanel = new TPanel( pForm );
  pPanel->Parent = pForm;
  pPanel->BorderStyle = bsNone;
  pPanel->BorderWidth = 0;
  pPanel->BevelOuter = bvLowered;
  pPanel->BevelInner = bvNone;
  pPanel->Color = clWhite;
  pPanel->Left = 10;
  pPanel->Top = 10;
  pPanel->Width = pForm->ClientWidth - 2 * pPanel->Left;

  Graphics::TBitmap* pBitmap = new Graphics::TBitmap;
  int logoWidth = GraphicResource::Width( Resources::BCI2000logo_mini ),
      logoHeight = GraphicResource::Width( Resources::BCI2000logo_mini );
  pBitmap->Width = logoWidth + 1;
  pBitmap->Height = logoHeight + 1;
  pBitmap->Canvas->Brush->Color = pPanel->Color;
  pBitmap->Canvas->FillRect( TRect( 0, 0, pBitmap->Width, pBitmap->Height ) );
  DrawContext dc = { pBitmap->Canvas->Handle, { 0, 0, logoWidth, logoHeight } };
  GraphicResource::Render<RenderingMode::Transparent>( Resources::BCI2000logo_mini, dc );

  TPicture* pPicture = new TPicture;
  pPicture->Graphic = pBitmap;
  TImage* pImage = new TImage( pForm );
  pImage->Parent = pPanel;
  pImage->Left = pPanel->Left;
  pImage->Top = pPanel->Top;
  pImage->Width = pBitmap->Width;
  pImage->Height = pBitmap->Height;
  pImage->Picture = pPicture;

  TLabel* pNameLabel = new TLabel( pForm );
  pNameLabel->Parent = pPanel;
  pNameLabel->Top = pImage->Top;
  pNameLabel->Left = 3 * pImage->Left + pImage->Width;
  pNameLabel->Font->Size *= 2;
  pNameLabel->Font->Style = pNameLabel->Font->Style << fsBold;
  pNameLabel->Caption = mApplicationName.c_str();

  TLabel* pCopyrightLabel = new TLabel( pForm );
  pCopyrightLabel->Parent = pPanel;
  pCopyrightLabel->Alignment = taLeftJustify;
  pCopyrightLabel->Caption = ( versionNumber + "\n\n" + BCI2000_COPYRIGHT ).c_str();
  pCopyrightLabel->Top = pNameLabel->Top + pNameLabel->Height;
  pCopyrightLabel->Left = pNameLabel->Left;

  int textWidth = max( pCopyrightLabel->Width, pNameLabel->Width );
  pPanel->ClientWidth = 4 * pImage->Left + pImage->Width + textWidth;
  pPanel->ClientHeight = pImage->Height + 2 * pImage->Top;
  pForm->ClientWidth = 2 * pPanel->Left + pPanel->Width;

  TLabel* pVersionLabel = new TLabel( pForm );
  pVersionLabel->Parent = pForm;
  pVersionLabel->Alignment = taCenter;
  pVersionLabel->Top = 2 * pPanel->Top + pPanel->Height;
  pVersionLabel->WordWrap = true;
  pVersionLabel->Left = pPanel->Left;
  pVersionLabel->Width = pPanel->Width;
  pVersionLabel->Caption = versionDetails.c_str();
  pVersionLabel->Left = ( pForm->ClientWidth - pVersionLabel->Width ) / 2;

  TButton* pButton = new TButton( pForm );
  pButton->Parent = pForm;
  pButton->ModalResult = mrOk;
  pButton->Caption = "Close";
  pButton->Left = ( pForm->ClientWidth - pButton->Width ) / 2;
  pButton->Top = pVersionLabel->Top + pVersionLabel->Height + pButton->Height / 2;

  pForm->ClientHeight = pButton->Top + ( 3 * pButton->Height ) / 2;

  pForm->ShowModal();
  delete pForm;
#endif // __BORLANDC__
  return *const_cast<AboutBox*>( this );
}


