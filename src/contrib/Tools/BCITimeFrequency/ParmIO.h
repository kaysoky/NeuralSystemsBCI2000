/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//--- ParmIO.h ---

#ifndef PIO
#define PIO

#include <stdio.h>
#include "StateForm1.h"
#include "InputForm1.h"
#include "ProcessForm1.h"
#include "OutputForm1.h"

class ParIO
{
private:
        FILE *sfile;
        FILE *gfile;
        TUseStateForm *usf;
        TInputForm *iform;
        TProcessForm *pform;
        TOutputForm *oform;
        void SaveInputForm( void );
        void SaveStateForm( void );
        void SaveProcessForm( void );
        void SaveOutputForm( void );
       
public:
        ParIO( );
        void GetF( FILE *, TUseStateForm * ,TInputForm *, TProcessForm *, TOutputForm *);
        void SaveF( FILE *, TUseStateForm * ,TInputForm *, TProcessForm *, TOutputForm *);

}	;

#endif
