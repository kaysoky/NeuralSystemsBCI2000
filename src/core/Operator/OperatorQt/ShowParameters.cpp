//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A dialog displaying parameters with check boxes.
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
#include "ShowParameters.h"
#include "ui_ShowParameters.h"

#include "ParamList.h"
#include "OperatorUtils.h"
#include <QtGui>

ShowParameters::ShowParameters( QWidget* parent, const ParamList& paramlist, int filtertype )
: QDialog(parent),
  m_ui(new Ui::ShowParameters),
  mrParamList( paramlist ),
  mFilterType( filtertype )
{
  m_ui->setupUi(this);

#ifdef Q_WS_MAC
  QDialogButtonBox* pButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
  this->layout()->addWidget( pButtonBox );
  connect( pButtonBox, SIGNAL(accepted()), this, SLOT(accept()) );
#endif // Q_WS_MAC

  connect( this, SIGNAL(finished(int)), this, SLOT(OnClose()) );
  switch( mFilterType )
  {
    case OperatorUtils::loadFilter:
      this->setWindowTitle( "Load Filter" );
      break;
    case OperatorUtils::saveFilter:
      this->setWindowTitle( "Save Filter" );
      break;
  }
  OnShow();
}

ShowParameters::~ShowParameters()
{
  delete m_ui;
}

void
ShowParameters::changeEvent( QEvent* e )
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

void
ShowParameters::OnShow()
{
  m_ui->listWidget->clear();
  for( int i = 0; i < mrParamList.Size(); ++i )
  {
    QListWidgetItem* item = new QListWidgetItem;
    const char* name = mrParamList[ i ].Name().c_str();
    item->setText( name );
    item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
    item->setCheckState( OperatorUtils::GetFilterStatus( name, mFilterType ) ? Qt::Checked : Qt::Unchecked );
    m_ui->listWidget->addItem( item );
  }
}

void
ShowParameters::OnClose()
{
  for( int i = 0; i < m_ui->listWidget->count(); ++i )
  {
    QListWidgetItem* item = m_ui->listWidget->item( i );
    OperatorUtils::SetFilterStatus( item->text().toAscii(), mFilterType, item->checkState() == Qt::Checked );
  }
}
