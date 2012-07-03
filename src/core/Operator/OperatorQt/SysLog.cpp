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
#include <QDateTime>

SysLog::SysLog( QWidget* inParent )
: QDialog( inParent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint ),
  mpLog( new QTextEdit ),
  mEmpty( true ),
  mDontClose( false )
{
  QHBoxLayout* pLayout = new QHBoxLayout;
  pLayout->setContentsMargins( 0, 0, 0, 0 );
  pLayout->addWidget( mpLog );
  this->setLayout( pLayout );
  this->setWindowTitle( "System Log" );
  this->resize( 600, 250 );
  OperatorUtils::RestoreWidget( this );

  mpLog->setReadOnly( true );
  mDefaultFormat = mpLog->currentCharFormat();
}


SysLog::~SysLog()
{
  OperatorUtils::SaveWidget( this );
}

// The user must close the syslog manually if there are errors/warnings.
bool
SysLog::Close( bool inForce )
{
  if( !inForce && mDontClose && this->isVisible() )
    return false;

  this->close();
  return true;
}

void
SysLog::AddEntry( const QString& inText, int inMode )
{
  QMutexLocker lock( &mCritsec );
  QTextCharFormat format = mDefaultFormat;
  QString kind = "";
  switch( inMode )
  {
    case logEntryWarning:
      format.setFontPointSize( 10 );
      format.setForeground( QBrush( Qt::darkGreen ) );
      format.setFontWeight( QFont::Bold );
      kind = "Warning: ";
      mDontClose = true;
      break;

    case logEntryError:
      format.setFontPointSize( 10 );
      format.setForeground( QBrush( Qt::darkRed ) );
      format.setFontWeight( QFont::Bold );
      mDontClose = true;
      break;

    case logEntryNormal:
    default:
      break;
  }
  QString line = ( mEmpty ? "" : "\n" ) + QDateTime::currentDateTime().toString( Qt::ISODate ) + " - " + kind + inText;
  mEmpty = false;
  mpLog->moveCursor( QTextCursor::End );
  mpLog->setCurrentCharFormat( format );
  mpLog->insertPlainText( line );
  switch( inMode )
  {
    case logEntryError:
      this->parentWidget()->show();
      this->show();
      break;
    case logEntryWarning:
      if( this->parentWidget()->isVisible() )
        this->show();
      break;
    default:
      break;
  }
  mpLog->moveCursor( QTextCursor::End );
  mpLog->ensureCursorVisible();
}
