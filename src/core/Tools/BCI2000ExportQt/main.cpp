#include <QtGui/QApplication>
#include "MainWindow.h"

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
    MainWindow w;
    w.show();
    return a.exec();
}
