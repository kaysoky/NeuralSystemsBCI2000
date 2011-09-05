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

static const int cTextMargin = 2;

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
  connect( m_ui->treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(OnExpandCollapse()) );
  connect( m_ui->treeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(OnExpandCollapse()) );
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
  m_ui->treeWidget->clear();
  for( int i = 0; i < mrParamList.Size(); ++i )
  {
    const HierarchicalLabel& path = mrParamList[i].Sections();
    QTreeWidgetItem* pParent = m_ui->treeWidget->invisibleRootItem();
    for( HierarchicalLabel::const_iterator j = path.begin(); j != path.end(); ++j )
    {
      int c = 0;
      while( c < pParent->childCount() )
      {
        if( pParent->child( c )->text( 0 ) == j->c_str() )
          break;
        else
          ++c;
      }
      if( c < pParent->childCount() )
      {
        pParent = pParent->child( c );
      }
      else
      {
        pParent = new QTreeWidgetItem( pParent, QStringList( j->c_str() ) );
        pParent->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsTristate | Qt::ItemIsEnabled );
        pParent->setCheckState( 0, Qt::PartiallyChecked );
        AdjustSize( pParent, cTextMargin );
      }
    }

    const char* pName = mrParamList[i].Name().c_str();
    QTreeWidgetItem* pItem = new QTreeWidgetItem( pParent, QStringList( pName ) );
    pItem->setToolTip( 0, QString::fromLocal8Bit( mrParamList[i].Comment().c_str() ) );
    pItem->setWhatsThis( 0, pItem->toolTip( 0 ) );
    pItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
    pItem->setCheckState( 0, OperatorUtils::GetFilterStatus( pName, mFilterType ) ? Qt::Checked : Qt::Unchecked );
    AdjustSize( pItem, cTextMargin );
  }

  m_ui->treeWidget->resizeColumnToContents( 0 );
}

void
ShowParameters::OnClose()
{
  for( QTreeWidgetItemIterator i( m_ui->treeWidget, QTreeWidgetItemIterator::NoChildren ); *i != NULL; ++i )
    OperatorUtils::SetFilterStatus( (*i)->text( 0 ).toAscii(), mFilterType, (*i)->checkState( 0 ) == Qt::Checked );
}

void
ShowParameters::OnExpandCollapse()
{
  m_ui->treeWidget->resizeColumnToContents( 0 );
}

void
ShowParameters::AdjustSize( QTreeWidgetItem* inpItem, int inMargin )
{
  for( int i = 0; i < inpItem->columnCount(); ++i )
  {
    QRect r = QFontMetrics( inpItem->font( i ) ).boundingRect( inpItem->text( i ) );
    r.adjust( -inMargin, -inMargin, inMargin, inMargin );
    inpItem->setSizeHint( i, r.size() );
  }
}


