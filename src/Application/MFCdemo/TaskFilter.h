////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File:    TaskFilter.h
//
// Author:  juergen.mellinger@uni-tuebingen.de
//
// Description: A simple task filter that illustrates how to implement an
//          application module with Microsoft's MFC library.
//
// $Log$
// Revision 1.1  2006/03/30 13:49:16  mellinger
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef TaskFilterH
#define TaskFilterH

#include "StdAfx.h"
#include "UGenericFilter.h"
#include "MFCdemo.h"
#include "MFCdemoDlg.h"

class TaskFilter : public GenericFilter
{
  public:
            TaskFilter();
    virtual ~TaskFilter();

    virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void Initialize();
    virtual void Process( const GenericSignal* Input, GenericSignal* Output );
    virtual void StartRun();
    virtual void StopRun();

  private:
    CMFCdemoDlg mWindow;
} ;

#endif // TaskFilterH
