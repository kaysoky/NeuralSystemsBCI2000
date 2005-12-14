#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "UEditMatrix.h"
#include "UBCIError.h"
#include "UOperatorUtils.h"

#include <string>
#include <assert>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CSPIN"
#pragma resource "*.dfm"

using namespace std;

TfEditMatrix *fEditMatrix;

int TfEditMatrix::sNumInstances = 0;

static const char* cSubEditTag = "\1subedit";
static const char* cSubEditCaption = "Matrix...";
static const char* cHint = "Right-click cells for a context menu.";

// A list of context menu entries with Action, Enabler, Checker, and Caption.
struct TfEditMatrix::MenuItemEntry TfEditMatrix::sMenuItems[] =
{
  { LabelEditing, NULL, LabelEditing_Checked, "Edit labels" },
  { AdaptColumnWidth, NULL, NULL, "Adapt column width..." },
  { NULL, NULL, NULL, "-" },
  { EditSubMatrix, EditSubMatrix_Enabled, NULL, "Edit sub-matrix..." },
  { PlainCellToMatrix, PlainCellToMatrix_Enabled, NULL, "Convert to sub-matrix..." },
  { MatrixToPlainCell, MatrixToPlainCell_Enabled, NULL, "Replace by single cell..." },
};

__fastcall
TfEditMatrix::TfEditMatrix(TComponent* Owner)
: TForm(Owner),
  mpLock( new TCriticalSection ),
  mpSubEditor( NULL ),
  mContextRow( -1 ),
  mContextCol( -1 )
{
  ++sNumInstances;
  if( sNumInstances == 1 )
    OperatorUtils::RestoreControl( this );
  BuildContextMenu();
  StringGrid->Hint = cHint;
  EditEntries();
}

__fastcall
TfEditMatrix::~TfEditMatrix()
{
  if( sNumInstances == 1 )
    OperatorUtils::SaveControl( this );
  delete mpLock;
  --sNumInstances;
}

void TfEditMatrix::SetDisplayedParam( PARAM* inParam )
{
  Lock();
  mpMatrixParam = inParam;
  mMatrixParamName = mpMatrixParam->GetName();
  cNumRows->Value = mpMatrixParam->GetNumRows();
  cNumCols->Value = mpMatrixParam->GetNumColumns();
  for( int col = 0; col < StringGrid->ColCount; ++col )
    for( int row = 0; row < StringGrid->RowCount; ++row )
      StringGrid->Cells[ col ][ row ] = "";
  UpdateDisplay();
  Unlock();
}

AnsiString TfEditMatrix::GetDisplayedParamName() const
{
  return mMatrixParamName;
}

void
TfEditMatrix::UpdateDisplay()
{
  Lock();
  if( mpSubEditor != NULL )
    mpSubEditor->Close();

  // set the window title and comment
  Caption = "Edit Matrix " + AnsiString( mpMatrixParam->GetName() );
  tComment->Caption = AnsiString( mpMatrixParam->GetComment() );

  // given the pointer, write the parameter's data into the spread sheet
  StringGrid->RowCount = cNumRows->Value + 1;
  StringGrid->ColCount = cNumCols->Value + 1;
  StringGrid->ScrollBars = ssBoth;

  // set the columns' and rows' titles
  {
    StringGrid->Cells[ 0 ][ 0 ] = "";
    size_t col = 0;
    while( col < mpMatrixParam->GetNumColumns() )
    {
      StringGrid->Cells[ col + 1 ][ 0 ] = mpMatrixParam->ColumnLabels()[ col ].c_str();
      ++col;
    }
    while( ( int )col < fEditMatrix->StringGrid->ColCount - 1 )
    {
      StringGrid->Cells[ col + 1 ][ 0 ] = PARAM::labelIndexer::TrivialLabel( col ).c_str();
      ++col;
    }
    size_t row = 0;
    while( row < mpMatrixParam->GetNumValuesDimension1() )
    {
      StringGrid->Cells[ 0 ][ row + 1 ] = mpMatrixParam->RowLabels()[ row ].c_str();
      ++row;
    }
    while( ( int )row < fEditMatrix->StringGrid->RowCount - 1 )
    {
      StringGrid->Cells[ 0 ][ row + 1 ] = PARAM::labelIndexer::TrivialLabel( row ).c_str();
      ++row;
    }
  }

  // set the values in the spreadsheet
  for( size_t row = 0; row < mpMatrixParam->GetNumRows(); ++row )
    for( size_t col = 0; col < mpMatrixParam->GetNumColumns(); ++col )
      if( mpMatrixParam->Value( row, col ).Kind() == PARAM::paramValue::Matrix )
        StringGrid->Cells[ col + 1 ][ row + 1 ] = cSubEditTag;
      else
        StringGrid->Cells[ col + 1 ][ row + 1 ] = mpMatrixParam->GetValue( row, col );

  SelectTopLeftCell();
  EditEntries();

  Unlock();
}

void
TfEditMatrix::UpdatePARAM()
{
  Lock();

  // set the column and row labels
  for( size_t col = 0; col < mpMatrixParam->GetNumValuesDimension2(); ++col )
    mpMatrixParam->LabelsDimension2()[ col ] = StringGrid->Cells[ col + 1 ][ 0 ].c_str();
  for( size_t row = 0; row < mpMatrixParam->GetNumValuesDimension1(); ++row )
    mpMatrixParam->LabelsDimension1()[ row ] = StringGrid->Cells[ 0 ][ row + 1 ].c_str();

  // set the values in the parameter according to the values in the spreadsheet
  for( size_t row = 0; row < mpMatrixParam->GetNumRows(); ++row )
    for( size_t col = 0; col < mpMatrixParam->GetNumColumns(); ++col )
      if( StringGrid->Cells[ col + 1 ][ row + 1 ] != cSubEditTag )
        mpMatrixParam->SetValue( StringGrid->Cells[ col + 1 ][ row + 1 ].c_str(), row, col );

  Unlock();
}

void
TfEditMatrix::EditLabels()
{
  StringGrid->FixedCols = 0;
  StringGrid->FixedRows = 0;
}

void
TfEditMatrix::EditEntries()
{
  StringGrid->FixedCols = 1;
  StringGrid->FixedRows = 1;
  AdaptColumnWidth();
  SelectTopLeftCell();
}

void
TfEditMatrix::SelectTopLeftCell()
{
  TGridRect nullSelection;
  nullSelection.Left = nullSelection.Right
    = nullSelection.Top = nullSelection.Bottom = 0;
  StringGrid->Selection = nullSelection;
}

void
TfEditMatrix::LabelEditing( int inRow, int inColumn )
{
  if( LabelEditing_Checked( inRow, inColumn ) )
  {
    UpdatePARAM();
    EditEntries();
  }
  else
    EditLabels();
}

bool
TfEditMatrix::LabelEditing_Checked(  int /*inRow*/, int /*inColumn*/ )
{
  return StringGrid->FixedCols == 0 || StringGrid->FixedRows == 0;
}

void
TfEditMatrix::AdaptColumnWidth( int /*inRow*/, int inColumn )
{
  StringGrid->Canvas->Font = StringGrid->Font;
  int additionalSpace = StringGrid->Canvas->TextWidth( "  " );

  int beginCol = inColumn,
      endCol = inColumn + 1;
  if( inColumn < 0 )
  {
    beginCol = 0;
    endCol = StringGrid->ColCount;
  }
  for( int col = beginCol; col < endCol; ++col )
  {
    int colMaxWidth = 0;
    for( int row = 0; row < StringGrid->RowCount; ++row )
    {
      int curLabelWidth = StringGrid->Canvas->TextWidth( StringGrid->Cells[ col ][ row ] );
      if( curLabelWidth > colMaxWidth )
        colMaxWidth = curLabelWidth;
    }
    colMaxWidth += additionalSpace;
    if( colMaxWidth > StringGrid->DefaultColWidth )
      StringGrid->ColWidths[ col ] = colMaxWidth;
    else
      StringGrid->ColWidths[ col ] = StringGrid->DefaultColWidth;
  }
}

void
TfEditMatrix::EditSubMatrix( int inRow, int inCol )
{
  mpSubEditor = new TfEditMatrix( NULL );
  mpSubEditor->Top = mpSubEditor->Top + 20 * ( sNumInstances - 1 );
  mpSubEditor->Left = mpSubEditor->Left + 20 * ( sNumInstances - 1 );
  mpSubEditor->SetDisplayedParam( mpMatrixParam->Value( inRow - 1, inCol - 1 ) );
  mpSubEditor->Caption = Caption + "("
                         + mpMatrixParam->RowLabels()[ inRow - 1 ].c_str() + ","
                         + mpMatrixParam->ColumnLabels()[ inCol - 1 ].c_str() + ")";
  mpSubEditor->tComment->Caption = mpMatrixParam->GetComment();
  mpSubEditor->ShowModal();
  delete mpSubEditor;
  mpSubEditor = NULL;
}

bool
TfEditMatrix::EditSubMatrix_Enabled( int inRow, int inCol ) const
{
  return StringGrid->Cells[ inCol ][ inRow ] == cSubEditTag;
}

void
TfEditMatrix::PlainCellToMatrix( int inRow, int inCol )
{
  TRect cellRect = StringGrid->CellRect( mContextCol, mContextRow );
  StringGrid->EditorMode = false;
  StringGrid->Canvas->DrawFocusRect( cellRect );
  bool convert = (
    IDYES == Application->MessageBox(
      "You are about to change this cell into a 1x1-sub-matrix.\n"
      "Proceed?",
      "Confirmation",
      MB_YESNO | MB_ICONQUESTION )
  );
  StringGrid->Canvas->DrawFocusRect( cellRect );
  if( convert )
  {
    TGridRect selection = StringGrid->Selection;
    if( selection.Top == inRow && selection.Left == inCol )
      SelectTopLeftCell();
    PARAM p( "{ matrix 1 1 0 }" );
    p.Value( 0, 0 ) = StringGrid->Cells[ inCol ][ inRow ].c_str();
    mpMatrixParam->Value( inRow - 1, inCol - 1 ) = p;
    StringGrid->Cells[ inCol ][ inRow ] = cSubEditTag;
    AdaptColumnWidth( inRow, inCol );
  }
}

bool
TfEditMatrix::PlainCellToMatrix_Enabled( int inRow, int inCol ) const
{
  return inRow > 0 && inRow < StringGrid->RowCount
         && inCol > 0 && inCol < StringGrid->ColCount
         && StringGrid->Cells[ inCol ][ inRow ] != cSubEditTag;
}

void
TfEditMatrix::MatrixToPlainCell( int inRow, int inCol )
{
  TRect cellRect = StringGrid->CellRect( mContextCol, mContextRow );
  StringGrid->Canvas->DrawFocusRect( cellRect );
  int subMatrixRows = mpMatrixParam->Value( inRow - 1, inCol - 1 )->GetNumRows(),
      subMatrixCols = mpMatrixParam->Value( inRow - 1, inCol - 1 )->GetNumColumns();

  string message = "You are about to replace this sub-matrix by a single value.\n";
  if( subMatrixRows > 1 && subMatrixCols > 1 )
    message += "This will discard all sub-matrix entries except one.\n";
  message += "Proceed?";

  bool convert = (
    IDYES == Application->MessageBox( message.c_str(), "Confirmation", MB_YESNO | MB_ICONQUESTION )
  );
  StringGrid->Canvas->DrawFocusRect( cellRect );
  if( convert )
  {
    // Descend into sub-matrices until a single value is found.
    const PARAM::paramValue* val = &mpMatrixParam->Value( inRow - 1, inCol - 1 )->Value( 0, 0 );
    while( val->Kind() == PARAM::paramValue::Matrix )
      val = &val->ToParam()->Value( 0, 0 );
    mpMatrixParam->Value( inRow - 1, inCol - 1 ) = *val;
    StringGrid->Cells[ inCol ][ inRow ] = mpMatrixParam->GetValue( inRow - 1, inCol - 1 );
    AdaptColumnWidth();
  }
}

bool
TfEditMatrix::MatrixToPlainCell_Enabled( int inRow, int inCol ) const
{
  return inRow > 0 && inRow < StringGrid->RowCount
         && inCol > 0 && inCol < StringGrid->ColCount
         && StringGrid->Cells[ inCol ][ inRow ] == cSubEditTag;
}

void
TfEditMatrix::BuildContextMenu()
{
  TPopupMenu* menu = new TPopupMenu( this );
  for( int i = 0; i < sizeof( sMenuItems ) / sizeof( *sMenuItems ); ++i )
  {
    TMenuItem* newItem = new TMenuItem( menu );
    menu->Items->Add( newItem );
    newItem->Caption = sMenuItems[ i ].mCaption;
    newItem->Tag = i;
    newItem->OnClick = PopupMenuItemClick;
  }
  OnContextPopup = PopupMenuPopup;
  PopupMenu = menu;
}

void
__fastcall
TfEditMatrix::PopupMenuPopup( TObject* /*inSender*/, const TPoint& inWhere, bool& outHandled )
{
  TPoint localPoint = StringGrid->ParentToClient( inWhere, this );
  StringGrid->MouseToCell( localPoint.x, localPoint.y, mContextCol, mContextRow );
  if( localPoint.x < 0 || localPoint.y < 0 || mContextRow < 0 || mContextCol < 0 )
    outHandled = true;
  else
  {
    TPopupMenu* menu = PopupMenu;
    assert( menu != NULL );
    for( int i = 0; i < menu->Items->Count && i < sizeof( sMenuItems ) / sizeof( *sMenuItems ); ++i )
    {
      if( sMenuItems[ i ].mpGetChecked )
        menu->Items->Items[ i ]->Checked
          = ( this->*sMenuItems[ i ].mpGetChecked )( mContextRow, mContextCol );
      if( sMenuItems[ i ].mpGetEnabled )
        menu->Items->Items[ i ]->Enabled
          = ( this->*sMenuItems[ i ].mpGetEnabled )( mContextRow, mContextCol );
    }
  }
}

void
__fastcall
TfEditMatrix::PopupMenuItemClick( TObject* inSender )
{
  TMenuItem* item = dynamic_cast<TMenuItem*>( inSender );
  assert( item != NULL );
  MenuItemEntry::MenuAction action = sMenuItems[ item->Tag ].mpAction;
  assert( action != NULL );
  ( this->*action )( mContextRow, mContextCol );
}

//---------------------------------------------------------------------------
// IDE-Managed event handlers.
//---------------------------------------------------------------------------

void __fastcall
TfEditMatrix::bChangeMatrixSizeClick( TObject* inSender )
{
  int res, row, col;

  if( IDYES == Application->MessageBox(
        "Do you really want to change the size of the matrix?",
        "Confirmation",
        MB_YESNO | MB_ICONQUESTION ) )
  {
    Lock();
    UpdatePARAM();
    mpMatrixParam->SetDimensions( cNumRows->Value, cNumCols->Value );
    UpdateDisplay();
    Unlock();
    EditEntries();
  }
  else
  {
    cNumRows->Value = StringGrid->RowCount - 1;
    cNumCols->Value = StringGrid->ColCount - 1;
  }
}

//---------------------------------------------------------------------------

void __fastcall
TfEditMatrix::FormClose( TObject*, TCloseAction& )
{
  UpdatePARAM();
}

//---------------------------------------------------------------------------

void __fastcall
TfEditMatrix::StringGridSelectCell( TObject* inSender,
                                       int inCol, int inRow, bool &outCanSelect )
{
  TStringGrid* grid = dynamic_cast<TStringGrid*>( inSender );
  if( grid->Cells[ inCol ][ inRow ] == cSubEditTag )
  {
    EditSubMatrix( inRow, inCol );
    outCanSelect = false;
  }
}

//---------------------------------------------------------------------------

void __fastcall
TfEditMatrix::StringGridDrawCell( TObject* inSender,
                            int inCol, int inRow, TRect& inRect, TGridDrawState )
{
  TStringGrid* grid = dynamic_cast<TStringGrid*>( inSender );
  if( grid->Cells[ inCol ][ inRow ] == cSubEditTag )
  {
    grid->Canvas->Pen->Width = 3;
    grid->Canvas->Pen->Color = clBtnShadow;
    grid->Canvas->Brush->Color = clBtnFace;
    grid->Canvas->Rectangle( inRect );
    grid->Canvas->Font->Color = clBtnText;
    grid->Canvas->TextRect( inRect, inRect.left + 2, inRect.top + 2, cSubEditCaption );
  }
}

//---------------------------------------------------------------------------

