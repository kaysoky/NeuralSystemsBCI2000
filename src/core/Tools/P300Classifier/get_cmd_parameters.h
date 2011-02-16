#ifndef _get_cmd_parameters_h
#define _get_cmd_parameters_h

#include <iostream>
#include <string>
#include <vector>
#include "ReadIniParameters.h"
#include "CmdLine.h"

void get_cmd_parameters(int argc, char* argv[],  InitialParameter &IniParam, string &szFile, 
						vector<string> &fPathArr, vector<string> &fPathArr_Testing);
void show_help();

#endif
