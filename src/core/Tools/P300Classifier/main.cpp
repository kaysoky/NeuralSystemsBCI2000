#include <QtGui/QApplication>
#include "CmdLine.h"
#include "configdialog.h"
#include "ExceptionCatcher.h"

#ifdef _WIN32
# include <Windows.h>

int main( int, char*[] );

int WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  return main( __argc, __argv );
}
#endif // _WIN32

struct MainLoop
{
  QApplication& app;
  int argc;
  char** argv;
  int result;

  void operator()();
};


int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(configdialog);

  QApplication app(argc, argv);
  MainLoop loop = { app, argc, argv, -1 };
  ExceptionCatcher()
    .SetMessage( "aborting" )
    .Execute( loop );
  return loop.result;
}

void
MainLoop::operator()()
{
  ConfigDialog dialog;

  CCmdLine    cmdLine;
  QString     arg_TrainingDataFiles;
  QString     arg_TestingDataFiles;
  QString     arg_inicfg;
  QStringList arg_TrainingDataFilesList;
  QStringList arg_TestingDataFilesList;
  bool        barg_TrainingDataFiles;
  bool        barg_TestingDataFiles;
  bool        barg_inicfg;

  cmdLine.SplitLine(argc, argv);

  barg_TrainingDataFiles     =cmdLine.HasSwitch("-TrainingDataFiles");
  barg_TestingDataFiles      =cmdLine.HasSwitch("-TestingDataFiles");
  barg_inicfg                =cmdLine.HasSwitch("-inicfg");

  //int co = cmdLine.GetArgumentCount("-TrainingDataFiles");
  if (barg_TrainingDataFiles)
  {
    for (int i=0; i<cmdLine.GetArgumentCount("-TrainingDataFiles"); i++)
    {
        arg_TrainingDataFiles = arg_TrainingDataFiles.fromStdString(cmdLine.GetArgument("-TrainingDataFiles",i));
        arg_TrainingDataFilesList.insert(i, arg_TrainingDataFiles);
    }
  }
  else
  {
      arg_TrainingDataFiles = "";
  }


  if (barg_TestingDataFiles)
  {
   for (int i=0; i<cmdLine.GetArgumentCount("-TestingDataFiles"); i++)
   {
       arg_TestingDataFiles = arg_TestingDataFiles.fromStdString(cmdLine.GetArgument("-TestingDataFiles",i));
       arg_TestingDataFilesList.insert(i, arg_TestingDataFiles);
   }
  }
  else
  {
    arg_TestingDataFiles = "";
  }

  if (barg_inicfg)
  {
    arg_inicfg = arg_inicfg.fromStdString(cmdLine.GetArgument("-inicfg",0));
  }
  else
  {
    arg_inicfg = "";
  }

  dialog.SetFiles(arg_TrainingDataFilesList, arg_TestingDataFilesList, arg_inicfg, barg_TrainingDataFiles);

  result = dialog.exec();
}
