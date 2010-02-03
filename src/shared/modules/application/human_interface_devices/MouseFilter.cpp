/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: pbrunner@wadsworth.org
// Description: The mouse filter captures the mouse position on the screen
//   in device coordinates. The coordinates are always recorded.
//   State Variables:
//     Each cursor position is stored in device coordinates (i.e. coordinates
//     that are in units of screen pixels):
//       CursorPosX
//       CursorPosY
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "MouseFilter.h"

RegisterFilter( MouseFilter, 3.0 );

// **************************************************************************
// Function:   MouseFilter
// Purpose:    This is the constructor for the MouseFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
MouseFilter::MouseFilter()
{

  BEGIN_STATE_DEFINITIONS
    "MousePosX 16 0 0 0",
    "MousePosY 16 0 0 0",
  END_STATE_DEFINITIONS

  m_last_value_x      = 0;
  m_last_value_y      = 0;
  m_mouseworking      = true;
  m_nBlockMissed      = 0;

}

// **************************************************************************
// Function:   ~MouseFilter
// Purpose:    This is the destructor for the MouseFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
MouseFilter::~MouseFilter()
{
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties.
// Returns:    N/A
// **************************************************************************
void MouseFilter::Preflight( const SignalProperties& Input,
                                   SignalProperties& Output ) const
{
  Output = Input;
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the MouseFilter
// Parameters: Input and output signal properties.
// Returns:    N/A
// **************************************************************************
void MouseFilter::Initialize( const SignalProperties&, const SignalProperties& )
{
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the calibration routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void MouseFilter::Process( const GenericSignal& Input, GenericSignal& Output)
{
 try{

    m_last_value_x = Mouse->CursorPos.x;
    m_last_value_y = Mouse->CursorPos.y;

    if (m_mouseworking == false) {
      bciout << "The screensaver stopped! " << m_nBlockMissed << " Blocks were missed" << std::endl;
      m_nBlockMissed = 0;
    }

    m_mouseworking = true;

 } catch(...) {

    if (m_mouseworking) {
      bciout << "The screensaver started, no valid mouse position can be recorded!" << std::endl;
    }

    m_mouseworking = false;
    m_nBlockMissed++;
 }

  State("MousePosX")=m_last_value_x;
  State("MousePosY")=m_last_value_y;

  Output = Input;

}



