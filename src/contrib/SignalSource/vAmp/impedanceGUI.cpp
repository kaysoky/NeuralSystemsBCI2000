//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "impedanceGUI.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TimpGUI *impGUI;
//---------------------------------------------------------------------------
__fastcall TimpGUI::TimpGUI(TComponent* Owner)
	: TForm(Owner)
{
}

void TimpGUI::setSize(int r, int c)
{
	impGrid->ColCount = c+1;
	impGrid->RowCount = r+1;
	for (int row = 1; row <= r; row++){
		impGrid->Objects[0][row] = (TObject*) clBlack;
		impGrid->Cells[0][row] = row;
	}
	for (int col = 1; col <= c; col++){
		impGrid->Objects[col][0] = (TObject*) clBlack;
		impGrid->Cells[col][0] = col;
	}
}

void TimpGUI::setGrid(vector< vector <float> > g)
{
	char tmp[100];
	for (int r = 0; r < g.size(); r++){
		for (int c = 0; c < g[r].size(); c++){
			if (g[r][c] < 1000){
				sprintf(tmp, "%1.0f Ohm", g[r][c]);
				impGrid->Objects[c+1][r+1] = (TObject*)clGreen;
				impGrid->Cells[c+1][r+1] = tmp;
				//impGrid->Objects[c][r]
			}
			else if (g[r][c] >= 1000 && g[r][c] < 5000){
				sprintf(tmp, "%1.2f kOhm", g[r][c]/1000);
				impGrid->Objects[c+1][r+1] = (TObject*)clGreen;
				impGrid->Cells[c+1][r+1] = tmp;
			}
			else if (g[r][c] >= 5000 && g[r][c] < 30e5){
				sprintf(tmp, "%1.1f kOhm", g[r][c]/1000);
				impGrid->Objects[c+1][r+1] = (TObject*)TColor(0x0000a5FF);
				impGrid->Cells[c+1][r+1] = tmp;
			}
			else if (g[r][c] >= 30e5 && g[r][c] < 1e6){
				sprintf(tmp, "%1.1f kOhm", g[r][c]/1e3);
				impGrid->Objects[c+1][r+1] = (TObject*)clRed;
				impGrid->Cells[c+1][r+1] = tmp;
			}
			else{
				sprintf(tmp, ">1 MOhm");
				impGrid->Objects[c+1][r+1] = (TObject*)clPurple;
				impGrid->Cells[c+1][r+1] = tmp;
			}
		}
	}
	impGrid->Repaint();
}
//---------------------------------------------------------------------------


void __fastcall TimpGUI::impGridDrawCell(TObject *Sender, int ACol, int ARow,
	  TRect &Rect, TGridDrawState State)
{
	/*
	TColor Cell_Color = (TColor) MyStringGrid->Objects[ACol][ARow];
MyStringGrid->Canvas->Font->Color = Cell_Color;
MyStringGrid->Canvas->TextRect(Rect, Rect.Left+2, Rect.Top+3,
MyStringGrid->Cells[ACol][ARow])
	*/
	impGrid->Canvas->Font->Color = (TColor) impGrid->Objects[ACol][ARow];
	impGrid->Canvas->TextRect(Rect, Rect.Left+2, Rect.Top+2, impGrid->Cells[ACol][ARow] );
}
//---------------------------------------------------------------------------

