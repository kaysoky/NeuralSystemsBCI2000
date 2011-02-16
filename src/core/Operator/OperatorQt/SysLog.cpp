//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The Operator module's log window.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
: QDialog( inParent ),
  mpLog( new QTextEdit ),
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
  mCritsec.lock();
  QTextCharFormat oldFormat = mpLog->currentCharFormat(),
                  format = oldFormat;
  switch( inMode )
  {
    case logEntryWarning:
      format.setFontPointSize( 10 );
      format.setForeground( QBrush( Qt::darkGreen ) );
      format.setFontWeight( QFont::Bold );
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
  QString line = QDateTime::currentDateTime().toString( Qt::ISODate ) + " - " + inText + "\n";
  mpLog->setCurrentCharFormat( format );
  mpLog->insertPlainText( line );
  mpLog->setCurrentCharFormat( oldFormat );
  switch( inMode )
  {
    case logEntryWarning:
    case logEntryError:
      this->show();
      break;
    default:
      break;
  }
  mpLog->ensureCursorVisible();
  mCritsec.unlock();
}
