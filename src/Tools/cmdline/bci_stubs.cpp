////////////////////////////////////////////////////////////////////////////////
// File:   bci_stubs.cpp
// Date:   Jul 22, 2003
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A file to hold dummy implementations for functions that are
//         unneeded when a filter is wrapped into a command line tool.
////////////////////////////////////////////////////////////////////////////////

#include "shared/UGenericVisualization.h"
bool GenericVisualization::Send( const std::string& ) { return true; }
bool GenericVisualization::Send( const GenericSignal* ) { return true; }
bool GenericVisualization::Send2Operator( const GenericIntSignal*, int ) { return true; }
bool GenericVisualization::SendCfg2Operator( int, int, const char* ) { return true; }
bool GenericVisualization::SendCfg2Operator( int, int, int ) { return true; }
