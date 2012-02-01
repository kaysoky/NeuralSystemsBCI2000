////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that displays a dialog window showing
//   the BCI2000 logo, and versioning information.
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

#include "AboutBox.h"
#include "Version.h"
#include "GUI.h"
#include "images/BCI2000logo_mini.h"
#include "strings/BCI2000_credits.h"
#include <sstream>

#include <QtGui>
#include <ctype.h>

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

const AboutBox&
AboutBox::Display() const
{
  VersionInfoBase info = mVersionInfo;
  string versionNumber = info[ "Version" ];
  info.erase( "Version" );
  string versionDetails;
  for( VersionInfoBase::reverse_iterator i = info.rbegin(); i != info.rend(); ++i )
    versionDetails += ", " + i->first + ": " + i->second;
  versionDetails.erase( 0, 2 );

  QDialog dialog;
  dialog.setWindowTitle( ( string( "About " ) + mApplicationName + "..." ).c_str() );
  dialog.setWindowFlags( Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint
                         | Qt::CustomizeWindowHint | Qt::WindowTitleHint );

  int logoWidth = GraphicResource::Width( Resources::BCI2000logo_mini ),
      logoHeight = GraphicResource::Height( Resources::BCI2000logo_mini );
  QPixmap logo( logoWidth + 16, logoHeight + 16 );
  logo.fill( Qt::white );
  DrawContext dc = { { &logo, NULL }, { 8, 8, logoWidth + 8, logoHeight + 8 } };
  GraphicResource::Render<RenderingMode::Opaque>( Resources::BCI2000logo_mini, dc );

  QLabel* pVersionLabel = new QLabel( versionDetails.c_str() );
  pVersionLabel->setWordWrap( true );
  pVersionLabel->setAlignment( Qt::AlignHCenter );

  QGraphicsScene* pGraphicsScene = new QGraphicsScene;
  QGraphicsView* pGraphicsView = new QGraphicsView( pGraphicsScene );
  QGraphicsPixmapItem* pLogoItem = pGraphicsScene->addPixmap( logo );
  qreal textLeft = pLogoItem->boundingRect().right();
  QFont font = pVersionLabel->font();
  font.setWeight( QFont::Bold );
  font.setPixelSize( 18 );
  font.setStyleStrategy( QFont::PreferAntialias );
  QGraphicsTextItem* pNameItem = pGraphicsScene->addText( mApplicationName.c_str(), font );
  pNameItem->setPos( textLeft, pNameItem->y() );
  QGraphicsTextItem* pVersionItem = pGraphicsScene->addText( versionNumber.c_str(), pVersionLabel->font() );
  pVersionItem->setPos( textLeft, pNameItem->boundingRect().bottom() - 8 );

  istringstream iss( Resources::BCI2000_credits );
  string line;
  int bottom = pVersionItem->boundingRect().bottom() + pVersionItem->boundingRect().height() 
               + Resources::BCI2000_credits_emptyLineHeight;
  for( int i = 0; i < Resources::BCI2000_credits_leadingLines; ++i )
  {
    getline( iss, line );
    QGraphicsTextItem* pText = pGraphicsScene->addText( line.c_str(), pVersionLabel->font() );
    pText->setPos( textLeft, bottom );
    bottom += pText->boundingRect().height() - Resources::BCI2000_credits_emptyLineHeight;
  }
  
  while( getline( iss, line ) )
  {
    QFont lineFont = pVersionLabel->font();
    if( line.length() > 1 && !::isspace( line[0] ) && ::isupper( line[1] ) )
      lineFont.setBold( true );
    else if( !line.empty() && !::isspace( line[0] ) )
      lineFont.setItalic( true );
    QGraphicsTextItem* pText = pGraphicsScene->addText( line.c_str(), lineFont );
    pText->setPos( textLeft, bottom );
    if( line.empty() )
      bottom += Resources::BCI2000_credits_emptyLineHeight;
    else
      bottom += pText->boundingRect().height() - Resources::BCI2000_credits_emptyLineHeight;
  }

  QDialogButtonBox* pButtonBox = new QDialogButtonBox( QDialogButtonBox::Close );
  QVBoxLayout* pVLayout = new QVBoxLayout;

  pVLayout->addWidget( pGraphicsView );
  pVLayout->addWidget( pVersionLabel );
  pVLayout->addWidget( pButtonBox );
  dialog.setLayout( pVLayout );
  dialog.connect( pButtonBox, SIGNAL(rejected()), &dialog, SLOT(reject()) );
  dialog.exec();

  return *this;
}


