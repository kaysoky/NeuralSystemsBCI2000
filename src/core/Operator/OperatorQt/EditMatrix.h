//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: An editor for matrix-valued parameters.
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
#ifndef EDITMATRIX_H
#define EDITMATRIX_H

#include <QDialog>
#include <QMutex>

class Param;
class QMenu;
class QTableWidgetItem;
class QModelIndex;

namespace Ui
{
  class EditMatrix;
}

class EditMatrix : public QDialog
{
  Q_OBJECT
 public:
  static const int cSpinBoxMax = 65535;

  EditMatrix(QWidget *parent = 0);
  ~EditMatrix();

 protected:
  void changeEvent(QEvent *e);

 private:
  Ui::EditMatrix *m_ui;

 public:
  void    SetDisplayedParam( Param* );
  QString GetDisplayedParamName() const;

 private slots:
  void OnChangeMatrixSizeClick();
  void OnClose();

 private: // User declarations
  void UpdateDisplay();
  void UpdateParam();
  void Lock()   { mLock.lock(); }
  void Unlock() { mLock.unlock(); }

  void AdaptColumnWidths();
  void EditSubMatrix( int, int );
  void PlainCellToMatrix( int, int );
  void MatrixToPlainCell( int, int );
  void Help( int, int );

  QString HeaderEditor( const QString& );
  void SetSubmatrixProperties( QTableWidgetItem* );
  void SetNormalEntryProperties( QTableWidgetItem* );
  bool IsSubmatrix( QTableWidgetItem* );

 private slots:
  void ShowContextMenu( const QPoint& );
  void PerformContextAction( QAction* );
  void EditHLabel( int );
  void EditVLabel( int );
  void EditItem( QTableWidgetItem* );

 private:
  void BuildContextMenu();

 private:
  static int  sNumInstances;
  Param*      mpMatrixParam;
  QString     mMatrixParamName;
  QMutex      mLock;
  EditMatrix* mpSubEditor;

  QMenu*      mpContextMenu;
  QAction*    mpEditSubMatrix,
         *    mpConvertToSubMatrix,
         *    mpReplaceWithSingleCell,
         *    mpHelp;

  int mContextRow, mContextCol;
};

extern EditMatrix* gpEditMatrix;

#endif // EDITMATRIX_H
