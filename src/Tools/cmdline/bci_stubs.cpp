////////////////////////////////////////////////////////////////////////////////
// File:   bci_stubs.cpp
// Date:   Jul 22, 2003
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A file to hold dummy implementations for functions that are
//         unneeded when a filter is wrapped into a command line tool.
////////////////////////////////////////////////////////////////////////////////

#include "shared/UGenericVisualization.h"
GenericVisualization::GenericVisualization(){}
GenericVisualization::GenericVisualization( unsigned char, unsigned char ){}
GenericVisualization::~GenericVisualization(){}
void GenericVisualization::SetSourceID( unsigned char ){}
bool GenericVisualization::Send2Operator( const GenericSignal* ){ return true; }
bool GenericVisualization::Send2Operator( const GenericIntSignal* ){ return true; }
bool GenericVisualization::Send2Operator( const GenericIntSignal*, int ){ return true; }
bool GenericVisualization::SendMemo2Operator( const char* ){ return true; }
bool GenericVisualization::SendCfg2Operator( unsigned char, unsigned char, const char* ){ return true; }
bool GenericVisualization::SendCfg2Operator( unsigned char, unsigned char, int ){ return true; }

