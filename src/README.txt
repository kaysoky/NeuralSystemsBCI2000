Getting started with the BCI2000 source code
07/19/2005                 Juergen Mellinger
============================================

This document covers the following topics:

(1) Compiling BCI2000 from the command line
(2) Compiling BCI2000 using the Borland IDE
(3) Starting up BCI2000
(4) Creating up-to date versions of the documentation in PDF format
(5) Compiling BCI2000 with Borland C++ Builder 2006


(1) Compiling BCI2000 from the command line
    =======================================
  - Open a Windows cmd shell, e.g. by choosing "Run..." from the Start
    Menu and entering "cmd". 
  - Change to the "src" directory where this document resides.
  - Execute "make" from the command prompt.
  - If you updated or edited any of the source files, and experience 
    linker errors or other unexpected behavior, execute
    "make clean && make all".
    
(2) Compiling BCI2000 using the Borland IDE
    =======================================
  - Open the file "BCI2000.bpg" with the IDE by double-clicking it.
  - Make sure the "ProjectManager" view is visible to the left.
  - Right-click the topmost project (located immediately below the line
    reading "BCI2000"), and choose "Make all from here".
  - To get rid of possible inconsistencies from earlier builds, choose
    "Build all from here" instead of "Make all from here".

(3) Starting up BCI2000
    ===================
  - After compilation, a desired configuration of BCI2000 may be started
    by executing an appropriate batch file from the top level "batch"
    directory. Once a particular configuration of BCI2000 is started,
    simply load a parameter file with the same name as the batch file
    from the "parms" directory.
  - If you need a configuration for which no batch file exists, just 
    modify a copy of a batch file that is close to your needs.

(4) Creating up-to date versions of the documentation in PDF format
    ===============================================================
  - If you have a LaTeX distribution installed that provides the pdflatex
    program, you can create PDF format documents.
  - Make sure that the pdflatex executable is inside the search path
    defined by the PATH environment variable.
  - Execute "make pdf" from the BCI2000/src directory.

(5) Compiling BCI2000 with Borland C++ Builder 2006
    ===============================================
  - Open the file "BCI2000.bpg" from within Borland Developer Studio.
    This will import all existing projects into BDS project files.
  - For each project, open the "Options..." dialog, go to
    "C++ Compiler->Precompiled Headers", and uncheck all check boxes.
  - In the project manager (at the top right), right-click the topmost 
    project and choose "Make all from here".
  - To get rid of possible inconsistencies from earlier builds, choose
    "Build all from here" instead of "Make all from here".
  - Command-line builds cannot be done with Borland C++ Builder 2006
    because it lacks a project-to-makefile conversion utility.