//---------------------------------------------------------------------------

#ifndef impedanceGUIH
#define impedanceGUIH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <ExtCtrls.hpp>
#include <vector>
#include <string>
#include <stdio.h>

using namespace std;
//---------------------------------------------------------------------------
class TimpGUI : public TForm
{
__published:	// IDE-managed Components
	TStringGrid *impGrid;
	void __fastcall impGridDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State);
private:	// User declarations
public:		// User declarations
	__fastcall TimpGUI(TComponent* Owner);
	void setGrid(vector< vector <float> > g);
	void setSize(int r, int c);
};
//---------------------------------------------------------------------------
//extern PACKAGE TimpGUI *impGUI;
//---------------------------------------------------------------------------
#endif
