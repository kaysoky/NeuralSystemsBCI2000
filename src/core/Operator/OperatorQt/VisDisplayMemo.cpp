////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class for memo (text) type visualization displays.
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

#include "VisDisplayMemo.h"

#include <QTextEdit>
#include <QString>
#include <QHBoxLayout>

#include <string>
#include <limits>

using namespace std;

VisDisplayMemo::VisDisplayMemo( const std::string& inSourceID )
: VisDisplayLayer( inSourceID ),
  mNumLines( 0 ),
  mpTextDisplay( new QTextEdit )
{
  QHBoxLayout* pLayout = new QHBoxLayout;
  pLayout->setContentsMargins( 0, 0, 0, 0 );
  pLayout->addWidget( mpTextDisplay );
  this->setLayout( pLayout );
  mpTextDisplay->setReadOnly( true );
  mpTextDisplay->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  mpTextDisplay->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  mpTextDisplay->setLineWrapMode( QTextEdit::NoWrap );
  Restore();
}

VisDisplayMemo::~VisDisplayMemo()
{
  Save();
}

void
VisDisplayMemo::SetConfig( ConfigSettings& inConfig )
{
  VisDisplayBase::SetConfig( inConfig );
  inConfig.Get( CfgID::NumLines, mNumLines );
  if( mNumLines < 1 )
    mNumLines = numeric_limits<int>::max();
}

void
VisDisplayMemo::Restore()
{
  VisDisplayBase::Restore();
  this->show();
}

void
VisDisplayMemo::Save() const
{
  VisDisplayBase::Save();
  Visconfigs()[ mVisID ].Put( CfgID::NumLines, mNumLines, MessageDefined );
}

void
VisDisplayMemo::HandleMemo( const QString& inText )
{
#ifdef TODO
# error Remove lines when their number exceeds the numlines property.
#endif
  string s = inText.toLocal8Bit().constData();
  size_t pos = 0;
  while( ( pos = s.find_first_of( "\n\r" ) ) != s.npos )
  {
    mpTextDisplay->append( QString::fromLocal8Bit( s.substr( 0, pos ).c_str() ) );
    s.erase( 0, pos + 1 );
  }
  if( !s.empty() )
  {
    mpTextDisplay->append( QString::fromLocal8Bit( s.c_str() ) );
  }
  mpTextDisplay->ensureCursorVisible();
}
