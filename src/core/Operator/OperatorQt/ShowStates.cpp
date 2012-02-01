//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A dialog displaying system states.
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
#include "ShowStates.h"
#include "ui_ShowStates.h"
#include "BCI_OperatorLib.h"

#include <QDialogButtonBox>

ShowStates::ShowStates( QWidget* parent )
: QDialog( parent ),
  m_ui( new Ui::ShowStates )
{
  m_ui->setupUi( this );

  QDialogButtonBox* pButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
  this->layout()->addWidget( pButtonBox );
  connect( pButtonBox, SIGNAL(accepted()), this, SLOT(accept()) );

  this->setWindowTitle( "System States" );
  Populate();
}

ShowStates::~ShowStates()
{
  delete m_ui;
}

void ShowStates::changeEvent( QEvent* e )
{
  QDialog::changeEvent(e);
  switch (e->type())
  {
    case QEvent::LanguageChange:
      m_ui->retranslateUi(this);
      break;
    default:
      break;
  }
}

void ShowStates::Populate()
{
  m_ui->listWidget->clear();
  int i = 0;
  const char* p = NULL;
  do
  {
    p = BCI_GetState( i++ );
    if( p )
    {
      QListWidgetItem* item = new QListWidgetItem;
      item->setText( p );
      m_ui->listWidget->addItem( item );
      BCI_ReleaseObject( p );
    }
  } while( p != NULL );
}
