#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "UEditMatrix.h"
#include "UBCIError.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CSPIN"
#pragma resource "*.dfm"
TfEditMatrix *fEditMatrix;
//---------------------------------------------------------------------------
__fastcall TfEditMatrix::TfEditMatrix(TComponent* Owner)
: TForm(Owner),
  lock( new TCriticalSection )
{
#ifdef LABEL_INDEXING
  bToggleEditing->OnClick = ToggleLabelEditing;
#else
  bToggleEditing->Visible = false;
#endif
  EditEntries();
}
//---------------------------------------------------------------------------
__fastcall TfEditMatrix::~TfEditMatrix()
{
  delete lock;
}

void TfEditMatrix::SetDisplayedParam( PARAM* inParam )
{
  Lock();
  matrix_param = inParam;
  matrix_param_name = matrix_param->GetName();
  cRowsMax->Value = matrix_param->GetNumValuesDimension1();
  cColumnsMax->Value = matrix_param->GetNumValuesDimension2();
  UpdateDisplay();
  Unlock();
}

AnsiString TfEditMatrix::GetDisplayedParamName() const
{
  return matrix_param_name;
}

void __fastcall TfEditMatrix::bChangeMatrixSizeClick(TObject *Sender)
{
int res, row, col;

 res=Application->MessageBox("Do you really want to change the size of the matrix ?", "Warning", MB_YESNO);
 if (res == IDYES)
    {
    // set the values in the spreadsheet
    // if there is no input, set it to "0"
#ifdef LABEL_INDEXING
       StringGrid->RowCount=cRowsMax->Value+1;
       StringGrid->ColCount=cColumnsMax->Value+1;
       for( row=1; row<StringGrid->RowCount; ++row )
       {
         if( StringGrid->Cells[ 0 ][ row ] == "" )
           StringGrid->Cells[ 0 ][ row ] = PARAM::labelIndexer::TrivialLabel( row - 1 ).c_str();
         for( col = 1; col < StringGrid->ColCount; ++col )
           if( StringGrid->Cells[ col ][ row ] == "" )
             StringGrid->Cells[ col ][ row ]= "0";
       }
       for( col = 1; col < StringGrid->ColCount; ++col )
         if( StringGrid->Cells[ col ][ 0 ] == "" )
            StringGrid->Cells[ col ][ 0 ] = PARAM::labelIndexer::TrivialLabel( col - 1 ).c_str();
       EditEntries();
#else
    for (row=0; row<cRowsMax->Value+1; row++)
     for (col=0; col<cColumnsMax->Value+1; col++)
      if (StringGrid->Cells[col][row] == "")
         StringGrid->Cells[col][row]="0";
    UpdateDisplay();
#endif
    }
 else
    {
    cRowsMax->Value=StringGrid->RowCount-1;
    cColumnsMax->Value=StringGrid->ColCount-1;
    }
}
//---------------------------------------------------------------------------


void TfEditMatrix::UpdateDisplay()
{
 Lock();
 // set the window title and comment
 Caption="Edit Matrix "+AnsiString(matrix_param->GetName());
 tComment->Caption=AnsiString(matrix_param->GetComment());

 // given the pointer, write the parameter's data into the spread sheet
 StringGrid->RowCount=cRowsMax->Value+1;
 StringGrid->ColCount=cColumnsMax->Value+1;
 StringGrid->ScrollBars=ssBoth;

 // set the columns' and rows' titles
#ifdef LABEL_INDEXING
 {
   StringGrid->Cells[ 0 ][ 0 ] = "";
   size_t col = 0;
   while( col < matrix_param->GetNumValuesDimension2() )
   {
     StringGrid->Cells[ col + 1 ][ 0 ] = matrix_param->LabelsDimension2()[ col ].c_str();
     ++col;
   }
   while( ( int )col < fEditMatrix->StringGrid->ColCount - 1 )
   {
     StringGrid->Cells[ col + 1 ][ 0 ] = PARAM::labelIndexer::TrivialLabel( col ).c_str();
     ++col;
   }
   size_t row = 0;
   while( row < matrix_param->GetNumValuesDimension1() )
   {
      StringGrid->Cells[ 0 ][ row + 1 ] = matrix_param->LabelsDimension1()[ row ].c_str();
      ++row;
   }
   while( ( int )row < fEditMatrix->StringGrid->RowCount - 1 )
   {
     StringGrid->Cells[ 0 ][ row + 1 ] = PARAM::labelIndexer::TrivialLabel( row ).c_str();
     ++row;
   }
 }
#else
 for (int col=1; col<fEditMatrix->StringGrid->ColCount; col++)
  StringGrid->Cells[col][0] = IntToStr(col);
 for (int row=1; row<fEditMatrix->StringGrid->RowCount; row++)
  StringGrid->Cells[0][row] = IntToStr(row);
#endif

 // set the values in the spreadsheet
 for (size_t row=0; row<matrix_param->GetNumValuesDimension1(); row++)
  for (size_t col=0; col<matrix_param->GetNumValuesDimension2(); col++)
   StringGrid->Cells[col+1][row+1]=matrix_param->GetValue(row, col);

 EditEntries();

 Unlock();
}



// **************************************************************************
// Function:   FormClose
// Purpose:    When the user closes the form, the data in the matrix parameter are updated
// Parameters: Sender - pointer to the sending object
// Returns:    N/A
// **************************************************************************
void __fastcall TfEditMatrix::FormClose(TObject *Sender, TCloseAction &Action)
{
 // Application->MessageBox("Do you want to use the changes you made ?"

 Lock();

 // update the matrix parameter's size
 matrix_param->SetDimensions(StringGrid->RowCount-1, StringGrid->ColCount-1);
 
#ifdef LABEL_INDEXING
 // set the column and row labels
 for( size_t col = 0; col < matrix_param->GetNumValuesDimension2(); ++col )
   matrix_param->LabelsDimension2()[ col ] = StringGrid->Cells[ col + 1 ][ 0 ].c_str();
 for( size_t row = 0; row < matrix_param->GetNumValuesDimension1(); ++row )
   matrix_param->LabelsDimension1()[ row ] = StringGrid->Cells[ 0 ][ row + 1 ].c_str();
#endif

 // set the values in the parameter according to the values in the spreadsheet
 for (size_t row=0; row<matrix_param->GetNumValuesDimension1(); row++)
  for (size_t col=0; col<matrix_param->GetNumValuesDimension2(); col++)
   matrix_param->SetValue(StringGrid->Cells[col+1][row+1].c_str(), row, col);

 Unlock();
}
//---------------------------------------------------------------------------

void __fastcall TfEditMatrix::ToggleLabelEditing( TObject* Sender )
{
    bool currentlyEditingLabels = StringGrid->FixedCols == 0 || StringGrid->FixedRows == 0;
    if( currentlyEditingLabels )
      EditEntries();
    else
      EditLabels();
}

void TfEditMatrix::EditLabels()
{
  StringGrid->FixedCols = 0;
  StringGrid->FixedRows = 0;
  bToggleEditing->Caption = "lock labels";
}

void TfEditMatrix::EditEntries()
{
  StringGrid->FixedCols = 1;
  StringGrid->FixedRows = 1;
  bToggleEditing->Caption = "unlock labels";
  AdaptColumnWidths();
}

void TfEditMatrix::AdaptColumnWidths()
{
  StringGrid->Canvas->Font = StringGrid->Font;
  int additionalSpace = StringGrid->Canvas->TextWidth( "  " );

  for( int col = 0; col < StringGrid->ColCount; ++col )
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



