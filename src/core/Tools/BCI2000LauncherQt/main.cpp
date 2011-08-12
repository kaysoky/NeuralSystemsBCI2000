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

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  a.setOrganizationName( "BCI2000" );
  a.setOrganizationDomain( "bci2000.org" );
  a.setApplicationName( "BCI2000Launcher" );
  Settings::SetFile();
  MainWindow w;
  w.show();

  std::string message = "aborting ";
  message += a.applicationName().toLocal8Bit().constData();
  struct
  {
    QApplication& app;
    int result;
    void operator()()
    { result = app.exec(); }
  } functor = { a, -1 };
  ExceptionCatcher().SetMessage( message )
                    .Execute( functor );
  return functor.result;
}
