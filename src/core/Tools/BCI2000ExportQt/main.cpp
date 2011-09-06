#include <QtGui/QApplication>
#include "Settings.h"
#include "MainWindow.h"
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

const char* cProgramName = "BCI2000Export";

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  a.setOrganizationName( "BCI2000" );
  a.setOrganizationDomain( "bci2000.org" );
  a.setApplicationName( cProgramName );
  Settings::SetFile();
  MainWindow w;
  w.show();

  std::string message = "Aborting ";
  message += a.applicationName().toLocal8Bit().constData();
  FunctionCall< int() >
    call( &QApplication::exec );
  bool finished = ExceptionCatcher()
    .SetMessage( message )
    .Run( call );
  return finished ? call.Result() : -1;
}
