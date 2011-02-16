#include "get_cmd_parameters.h"

///////////////////////////////////////////////////////////////////////////////
/// Get initial parameters and training-test files from a command line. 
/// @param [in] argc        Number of input arguments.	
/// @param [in] argv        Name of the input arguments.
/// @param [out] IniParam   A structure with all the initial parameters.
/// @param [out] szFile     Name of the file (.ini) containing all the initial parameters.
/// @param [out] fPathArr   String with the path of all training-test files.
/// \author Cristhian Potes
/// \date July 15, 2009
///////////////////////////////////////////////////////////////////////////////
void get_cmd_parameters(int argc, char* argv[],  InitialParameter &IniParam, string &szFile, 
                        vector<string> &fPathArr_Training, vector<string> &fPathArr_Testing) 
{
  CCmdLine    cmdLine;
  int i;
  bool        barg_training_files;
  bool        barg_testing_files;
  bool        barg_inicfg;

  if (cmdLine.SplitLine(argc, argv) < 1) //Get the input arguments
  {
      // no switches were given on the command line, abort
      show_help();
      exit(0);
    }

  if (cmdLine.HasSwitch("-h"))
    {
       show_help();
       exit(0);
    }

  barg_training_files  = cmdLine.HasSwitch("-training_files");
  barg_testing_files  = cmdLine.HasSwitch("-testing_files");
  barg_inicfg = cmdLine.HasSwitch("-inicfg");

  if (!barg_inicfg) printf("\n error: -inicfg parameter missing");
    if (!barg_training_files) printf("\n error: -training_files parameter missing");

  if (!(barg_inicfg || barg_training_files)) exit(0);
  
  for (i=0; i<cmdLine.GetArgumentCount("-training_files"); i++)
    fPathArr_Training.push_back(cmdLine.GetArgument("-training_files",i));

  if (barg_testing_files)
  {
    for (i=0; i<cmdLine.GetArgumentCount("-testing_files"); i++)
      fPathArr_Testing.push_back(cmdLine.GetArgument("-testing_files",i));

  }

  szFile = cmdLine.GetArgument("-inicfg",0);

}
///////////////////////////////////////////////////////////////////////////////
/// Shows help. 
///////////////////////////////////////////////////////////////////////////////
void show_help()
{
  printf ("\n ------------------------------ usage begin -----------------------------------");
  printf ("\n");
  printf ("\n P300_GUI");
  printf ("\n");
  printf ("\n mandatory parameters:");
  printf ("\n ---------------------");
  printf ("\n -inicfg inifile for features");
  printf ("\n -input inputfile");
  printf ("\n ------------------------------- usage end -----------------------------------");

}
