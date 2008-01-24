////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: A simple task filter that illustrates how to implement a
//   BCI2000 application module based on Microsoft's MFC library.
//
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TASK_FILTER_H
#define TASK_FILTER_H

#include "StdAfx.h"
#include "GenericFilter.h"
#include "MFCdemo.h"
#include "MFCdemoDlg.h"

class TaskFilter : public GenericFilter
{
  public:
            TaskFilter();
    virtual ~TaskFilter();

    virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void Initialize( const SignalProperties&, const SignalProperties& );
    virtual void Process( const GenericSignal& Input, GenericSignal& Output );
    virtual void StartRun();
    virtual void StopRun();
	virtual bool AllowsVisualization() const { return false; }

  private:
    float       mCursorSpeed;
    CMFCdemoDlg mWindow;
} ;

#endif // TASK_FILTER_H
