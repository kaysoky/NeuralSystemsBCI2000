//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The Operator module's log window.
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
///////////////////////////////////////////////////////////////////////
#include "SysLog.h"
#include "OperatorUtils.h"
#include <QLayout>
#include <QScrollBar>
#include <QDateTime>

SysLog::SysLog( QWidget* inParent )
: QDialog( inParent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint ),
  mpLog( new QTextBrowser ),
  mAttachToParent( true ),
  mEmpty( true ),
  mDontClose( false ),
  mURLMatcher( "://" )
{
  QHBoxLayout* pLayout = new QHBoxLayout;
  pLayout->setContentsMargins( 0, 0, 0, 0 );
  pLayout->addWidget( mpLog );
  this->setLayout( pLayout );
  this->setWindowTitle( "System Log" );
  mAttachToParent = !OperatorUtils::RestoreWidget( this );

  mpLog->setOpenExternalLinks( true );
  mDefaultFormat = mpLog->currentCharFormat();
}


SysLog::~SysLog()
{
  OperatorUtils::SaveWidget( this );
}


void
SysLog::AllowClose()
{
  mDontClose = false;
}

// The user must close the syslog manually if there are errors/warnings.
bool
SysLog::Close()
{
  if( mDontClose && this->isVisible() )
    return false;

  this->close();
  return true;
}

void
SysLog::Show( bool inForce )
{
  QWidget* pParent = this->parentWidget();
  if( mAttachToParent && pParent && !this->isVisible() )
  {
	  QSize s = pParent->size();
	  s.setHeight( 2 * s.height() );
	  this->resize( s );
	  QPoint p = pParent->frameGeometry().bottomLeft();
	  this->move( p );
    mAttachToParent = true;
  }
	if( inForce || pParent && pParent->isVisible() )
    this->show();
  if( inForce && pParent )
	  pParent->show();
}

void
SysLog::AddEntry( const QString& inText, int inMode )
{
  QMutexLocker lock( &mCritsec );
  QTextCharFormat format = mDefaultFormat;
  QString kind = "", sep = " - ";
  if( inText.size() && inText[0] == '[' )
    sep = " ";
  switch( inMode )
  {
    case logEntryWarning:
      format.setFontPointSize( 10 );
      format.setForeground( QBrush( "darkorange" ) );
      format.setFontWeight( QFont::Bold );
      kind = "[Warning]";
      sep = " ";
      mDontClose = true;
      break;

    case logEntryError:
      format.setFontPointSize( 10 );
      format.setForeground( QBrush( "darkred" ) );
      format.setFontWeight( QFont::Bold );
      mDontClose = true;
      break;

    case logEntryNormal:
    default:
      break;
  }
  QString line = ( mEmpty ? "" : "\n" ) + QDateTime::currentDateTime().toString( Qt::ISODate ) + sep + kind + inText;
  mEmpty = false;

  bool wasAtEnd = true;
  QScrollBar* pScrollBar = mpLog->verticalScrollBar();
  if( pScrollBar && pScrollBar->isVisible() && pScrollBar->isEnabled() )
    wasAtEnd = ( pScrollBar->value() + pScrollBar->singleStep() > pScrollBar->maximum() );

  QTextCursor c = mpLog->textCursor();
  c.movePosition( QTextCursor::End );
  c.insertText( line, format );
  int begin = 0, end = begin;
  while( MatchURL( inText, begin, end ) )
  {
    c.movePosition( QTextCursor::End, QTextCursor::MoveAnchor );
    c.movePosition( QTextCursor::Left, QTextCursor::MoveAnchor, inText.length() - begin );
    c.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor, end - begin );
    QTextCharFormat f;
    f.setFontUnderline( true );
    f.setForeground( Qt::blue );
    f.setAnchor( true );
    f.setAnchorHref( QUrl::fromUserInput( c.selectedText() ).toString() );
    c.mergeCharFormat( f );
    begin = end;
  }
  switch( inMode )
  {
    case logEntryError:
      Show( true );
      break;
    case logEntryWarning:
      Show( false );
      break;
    default:
      break;
  }
  if( wasAtEnd )
  {
    mpLog->moveCursor( QTextCursor::End );
    mpLog->ensureCursorVisible();
  }
}

bool
SysLog::MatchURL( const QString& s, int& begin, int& end )
{
  begin = mURLMatcher.indexIn( s, begin );
  if( --begin < 0 )
    return false;
  end = begin + mURLMatcher.pattern().length();
  while( begin > 0 && s[begin].isLetter() )
    --begin;
  while( end < s.length() && !s[end].isSpace() )
    ++end;
  return true;
}

void
SysLog::moveEvent( QMoveEvent* )
{
  mAttachToParent = false;
}

