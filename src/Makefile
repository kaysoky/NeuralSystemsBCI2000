##################################################################################
#
# File:   Makefile
#
# Author: juergen.mellinger@uni-tuebingen.de
#
# Date:   Sept 20, 2002
#
# Description:
#         Makefile for the BCI2000 project.
#         Must be run using Borland's make.exe from the Windows NT/2000 
#         command shell.
#
#         Use "make clean && make all" to do a clean rebuild after updating
#         files from SourceSafe.
#
#         Use "make prepare" to keep the Borland IDE from crashing because
#         of write protected files.
#
#         This makefile depends on the existence of a file called ".makefiles" 
#         in the same directory which is either empty or contains
#         an automatically created list of makefiles in subdirectories.
#
#         You will probably want to edit the file ".makefiles" to exclude
#         broken and/or unneeded subprojects. If doing so, uncomment
#         the line defining DONTCLEANME to protect your changes.
#
# Changes:
#
###################################################################################

# How to execute commands -- Borland make does not automatically honor the SHELL macro.
SHELL = cmd /c @prompt $g && 
# Name of file that contains list of subproject makefiles:
MKNAM = .makefiles
# How to zero the subproject makefile list:
EMPTYMK = echo ^# This file is used by MAKE. Do not delete it.>$(MKNAM)
# Extensions of files that need to be always writeable.
FORCEWRITE = *.bpr *.res *.mak
# Extensions of files that will be deleted by 'make clean'.
GARBAGE = *.obj *.tds *.csm *.~* *.\#*

# Workaround macros to access shell operators.
AND = &&
OR = ||

!include ${MKNAM}

!ifndef MAKEFILES
##################################################################################
#
# Bootstrapping: Create list of subproject makefiles
#
##################################################################################
all prepare clean:
  attrib -r ${MKNAM}
  ${EMPTYMK}
  echo ^#DONTCLEANME=true ^# Uncomment this line if you edit this file.>>${MKNAM}
  echo MAKEFILES =\                                                    >>${MKNAM}
  -for /R %i in (*.bpr) do echo %~di%~pi%~ni.mak \                     >>${MKNAM}
  echo.                                                                >>${MKNAM}
  ${MAKE} -${MAKEFLAGS} $@

!else # !ifndef MAKEFILES
##################################################################################
#
# End-user targets
#
##################################################################################

# Make all subprojects
all: ${MAKEFILES:.mak=.makeall}

# Prepare source files for opening projects without crashing(!) the Borland IDE.
prepare: ${MAKEFILES:.mak=.makeprepare}
  -${SHELL} for /R %i in (${FORCEWRITE} ${GARBAGE}) do attrib -r %i
!ifndef DONTCLEANME
  -${SHELL} attrib -r ${MKNAM}
!endif

# Cleanup
clean: prepare ${MAKEFILES:.mak=.makeclean}
  -${SHELL} for /R %i in (${GARBAGE}) do del %i
!ifndef DONTCLEANME
  ${EMPTYMK}
!endif

##################################################################################
#
# Suffix rules for creating and executing makefiles
#
##################################################################################
.bpr.mak:
  bpr2mak $<
  echo.                                    >>$@
  echo prepare:                            >>$@
  echo " " -attrib -r \$\(PROJECT)         >>$@
  echo clean:                              >>$@
  echo " " -del \$\(PROJECT) \$\(OBJFILES) >>$@
  echo.                                    >>$@

.mak.makeall:
  ${SHELL} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS}
  
.mak.makeprepare:
  ${SHELL} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} prepare

.mak.makeclean:
  ${SHELL} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} clean
  del $<
  
!endif # !ifndef MAKEFILES

