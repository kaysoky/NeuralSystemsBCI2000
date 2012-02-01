//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The Operator module's connection info dialog.
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
#include "ConnectionInfo.h"
#include "ui_ConnectionInfo.h"
#include "../OperatorLib/BCI_OperatorLib.h"

ConnectionInfo* gpConnectionInfo = NULL;

ConnectionInfo::ConnectionInfo( QWidget* parent )
: QDialog( parent ),
  m_ui( new Ui::ConnectionInfo )
{
  m_ui->setupUi( this );
  this->setWindowFlags(
      Qt::Dialog
      | Qt::WindowTitleHint
      | Qt::MSWindowsFixedSizeDialogHint
  );
}

ConnectionInfo::~ConnectionInfo()
{
  delete m_ui;
}

void
ConnectionInfo::UpdateDisplay()
{
  const char* p = NULL;
  p = BCI_GetConnectionInfo( 0 );
  m_ui->label_Source->setText( p );
  BCI_ReleaseObject( p );

  p = BCI_GetConnectionInfo( 1 );
  m_ui->label_SigProc->setText( p );
  BCI_ReleaseObject( p );

  p = BCI_GetConnectionInfo( 2 );
  m_ui->label_App->setText( p );
  BCI_ReleaseObject( p );
}

void
ConnectionInfo::changeEvent( QEvent* e )
{
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
  }
}
