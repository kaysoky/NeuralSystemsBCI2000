MFC demo project readme
=======================

The MFC demo project illustrates how to build a BCI2000 application module
under Microsoft Visual C++ using the MFC GUI framework.

When creating your own Application Module on top of this demo, you will
need to modify code of two classes.

TaskFilter:
 A BCI2000 filter that contains the actual functionality of the 
 application module -- registering parameters and states, and trial
 sequencing.

MFCdemoDlg:
 An MFC dialog class that provides a window and a device context for
 graphics output.

Building the demo requires Visual C++ 7.1 (released in 2003) or newer.
