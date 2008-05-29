##################################################################################
# $Id$
# Author: juergen.mellinger@uni-tuebingen.de
# Description:
#         Makefile for the BCI2000 project.
#         Must be run using Borland's make.exe from the Windows NT/2000 
#         command shell.
#
#         Use "make clean && make all" to do a clean rebuild after updating
#         files from CVS.
#
#         Use "make prepare" to avoid Borland IDE problems with write protected
#         project files.
#
#         This makefile depends on the existence of a file called "makefiles" 
#         in the same directory which is either empty or contains
#         an automatically created list of makefiles in subdirectories.
#
#         You will probably want to edit the file "makefiles" to exclude
#         broken and/or unneeded subprojects. If doing so, uncomment
#         the line defining DONTCLEANME to protect your changes.
#
# (C) 2000-2008, BCI2000 Project
# http://www.bci2000.org
###################################################################################

# How to execute commands.
SHELL = cmd /c @prompt $s && 
# Ignore a command's return code (minus sign appears unreliable).
IGNORE = || echo.
# Name of file that contains list of subproject makefiles.
MKNAM = makefiles
# How to zero the subproject makefile list.
EMPTYMK = echo ^# Execute 'make prepare' to populate this file.>$(MKNAM)
# Extensions of files that need to be always writeable.
FORCEWRITE = *.bpr *.res *.mak *.pdf *.exe
# Extensions of files that will be deleted by 'make clean'.
GARBAGE = *.obj *.tds *.csm *.~* *.\#* *.docinfo
# How to report when we are done with a target.
DONE = @${SHELL} echo. && echo Done with target $@. && echo.
# Where to put binaries below the installation root.
BINDIR = prog
# Where to put tools below the installation root if no separate tools directory is specified.
TOOLSBINDIR = tools
# Where to put documents.
DOCDIR = doc

# Path to TortoiseSVN
TORTOISEDIR = %ProgramFiles%\TortoiseSVN\bin
# Path to distribution creation tool.
SEVENZIPDIR = %ProgramFiles%\7-Zip
# Name of the binary distribution archive.
BINDISTNAME = BCI2000Setup
# Names of top-level directories that should be contained in the binary distribution.
BINDISTDIRS = batch data doc parms prog tools
# Additional files to include into the binary distribution
BINDISTFILES = *.html
# Path/filename elements that should be excluded from the binary distribution
NODISTFILES = *.tds *\CVS .empty *\.svn

# Workaround macros to access shell operators.
AND = &&
OR = ||

!include ${MKNAM}

!ifndef TOOLSINSTALLDIR
TOOLSINSTALLDIR = ${INSTALLDIR}\${TOOLSBINDIR}
!endif

!ifndef MAKEFILES
##################################################################################
#
# Bootstrapping: Create list of subproject makefiles
#
##################################################################################
all build cmdline prepare clean install update bindist pdf test:
  attrib -r ${MKNAM}
  ${EMPTYMK}
  echo ^#DONTCLEANME=true ^# Uncomment this line if you edit this file.   >>${MKNAM}
  echo ^#INSTALLDIR=      ^# The root directory for "make install".       >>${MKNAM}
  echo ^#TOOLSINSTALLDIR= ^# Optionally, a separate tools destination\
                             directory for "make install".                >>${MKNAM}
  echo MAKEFILES =\                                                       >>${MKNAM}
  -for /R %i in (*.bpr) do \
            echo %i | grep core ${AND} echo ^"%~di%~pi%~ni.mak^" \        >>${MKNAM}
  -for /R %i in (*.bpr) do \
            echo %i | grep contrib ${AND} echo ^#^"%~di%~pi%~ni.mak^" \   >>${MKNAM}
  echo.                                                                   >>${MKNAM}
  echo LATEXDOCS =\                                                       >>${MKNAM}
  -for /R %i in (*.tex) do grep \\documentclass "%i" ${AND} echo ^"%i^" \ >>${MKNAM}
  echo.                                                                   >>${MKNAM}
  ${MAKE} -${MAKEFLAGS} $@

!else # !ifndef MAKEFILES
##################################################################################
#
# User targets
#
##################################################################################

# Make all subprojects.
all: prepare ${MAKEFILES:.mak=.all} cmdline mex
  ${DONE}
  
# Update version and time stamp, then make all subprojects.
build:
  if exist ${TORTOISEDIR}\SubWCRev.exe \
    ${TORTOISEDIR}\SubWCRev.exe . shared\config\Version.h.in shared\config\Version.h
  ${MAKE} -${MAKEFLAGS} all
  ${MAKE} -${MAKEFLAGS} test
  ${DONE}
  
# Make command line tools.
cmdline:
  ${SHELL} cd core\Tools\cmdline ${AND} make install
  ${DONE}
  
# Make mex files.
mex:
  ${SHELL} cd core\Tools\mex ${AND} make install
  ${DONE}
  
# Prepare source files for opening projects without confusing the Borland IDE.
prepare: ${MAKEFILES:.mak=.prepare}
  ${SHELL} (for /R %i in (${FORCEWRITE} ${GARBAGE}) do attrib -r "%i") ${IGNORE}
!ifndef DONTCLEANME
  ${SHELL} attrib -r "${MKNAM}" ${IGNORE}
!endif
  ${DONE}

# Cleanup.
clean: prepare ${MAKEFILES:.mak=.clean}
  ${SHELL} (for /R %i in (${GARBAGE}) do del "%i") ${IGNORE}
!ifndef DONTCLEANME
  ${EMPTYMK}
!endif
  ${SHELL} cd ..\${BINDIR} ${AND} (for /R %i in (${GARBAGE}) do del "%i") ${IGNORE}
  ${SHELL} cd ..\${TOOLSBINDIR} ${AND} (for /R %i in (${GARBAGE}) do del "%i") ${IGNORE}
  ${SHELL} cd core\Tools\cmdline ${AND} make clean ${IGNORE}
  ${SHELL} cd core\Tools\mex ${AND} make clean ${IGNORE}
  ${DONE}
  
# Install executables.
install: ${MAKEFILES:.mak=.install}
  ${DONE}

# Replace installed executables with current versions.
update: ${MAKEFILES:.mak=.update}
  ${DONE}
  
# Create an installation archive.
bindist: build
  del ..\${BINDISTNAME}.exe ${IGNORE}
  ${SHELL} cd .. \
    ${AND} "${SEVENZIPDIR}\7z" a ${BINDISTNAME}.7z ${BINDISTDIRS} -ms=e \
    ${AND} "${SEVENZIPDIR}\7z" a ${BINDISTNAME}.7z ${BINDISTFILES} -ms=e \
    ${AND} "${SEVENZIPDIR}\7z" d ${BINDISTNAME}.7z ${GARBAGE} ${NODISTFILES} -r \
    ${AND} copy /b  src\BCI2000.sfx + ${BINDISTNAME}.7z ${BINDISTNAME}.exe \
    ${AND} del ${BINDISTNAME}.7z
  ${DONE}

# Test executables.
test:
  ${SHELL} cd buildutils\tests ${AND} run_tests
  ${DONE}

# Create pdfs from tex files.
pdf: ${LATEXDOCS:.tex=.pdf} ${LATEXDOCS:.tex=.install}
  ${DONE}
    
##################################################################################
#
# Suffix rules for creating and executing makefiles
#
##################################################################################
.bpr.mak:
  (bpr2mak $<) & echo.
  # Add our own additional targets to the makefile created by bpr2mak.
  @echo.                                                          >>$@
  @echo install_supplements:                                      >>$@
  @echo !ifdef DESTDIR                                            >>$@
  @echo ^^ if exist install.cmd install.cmd ^^"$^(DESTDIR)^\^"    >>$@
  @echo !else                                                     >>$@
  @echo ^^ if exist install.cmd install.cmd ^^"$^(PROJECT)^\..^\^">>$@
  @echo !endif                                                    >>$@
  @echo prepare:                                                  >>$@
  @echo ^^ -attrib -r $^^(PROJECT)                                >>$@
  @echo ^^ if not exist obj mkdir obj                             >>$@
  @echo install: $^^(PROJECT) install_supplements                 >>$@
  @echo ^^ if not exist ^^"$^(DESTDIR)^" mkdir ^^"$^(DESTDIR)^"   >>$@
  @echo ^^ xcopy /d /y $^^(PROJECT) ^^"$^(DESTDIR)^"              >>$@
  @echo update: $^^(PROJECT) install_supplements                  >>$@
  @echo ^^ if exist ^^"$^(DESTDIR)^" \
            xcopy /d /u /y $^^(PROJECT) ^^"$^(DESTDIR)^"          >>$@
  @echo clean:                                                    >>$@
  @echo ^^ -del $^^(PROJECT) $^^(OBJFILES)                        >>$@
  @echo.                                                          >>$@
  # We cannot tolerate absolute paths in makefiles.
  @${SHELL} \
  ( \
   grep "^[A-Z]*PATH[A-Z]*.*:" $@ \
    ${AND} echo Found absolute path^^(s^^) in makefile -- sorry, aborting. \
    ${AND} echo Fix paths in $< using the IDE. \
    ${AND} exit 1 \
  ) \
  ${OR} exit 0
  ${DONE}

.mak.all:
  ${SHELL} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} \
                      ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} install_supplements
  del shared\obj\bci2000.csm shared\obj\bci2000.^#* > nul 2>&1 ${IGNORE}
  ${DONE}
  
.mak.docinfo:
  ${SHELL} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} docinfo 
  ${DONE}

.mak.prepare:
  ${SHELL} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} prepare
  ${DONE}

.mak.clean:
  ${SHELL} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} clean
  del $<
  ${DONE}
  
.mak.install:
!ifdef INSTALLDIR
  ${SHELL} echo ${<D} | grep \\Tools\\ \
    ${AND} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} \
                              -DDESTDIR="${TOOLSINSTALLDIR}" install \
    ${OR}  cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} \
                            -DDESTDIR="${INSTALLDIR}\${BINDIR}" install
  ${DONE}
!else  # !ifdef INSTALLDIR
  @echo.
  @echo Please specify a root directory to install into, as in
  @echo make -DINSTALLDIR="c:\program files\bci2000" install
  @exit 1
!endif # !ifdef INSTALLDIR

.mak.update:
!ifdef INSTALLDIR
  ${SHELL} echo ${<D} | grep \\Tools\\ \
    ${AND} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} \
                              -DDESTDIR="${TOOLSINSTALLDIR}" update \
    ${OR}  cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS} \
                            -DDESTDIR="${INSTALLDIR}\${BINDIR}" update
  ${DONE}
!else  # !ifdef INSTALLDIR
  @echo.
  @echo Please specify a root directory to install into, as in
  @echo make -DINSTALLDIR="c:\program files\bci2000" update
  @exit 1
!endif # !ifdef INSTALLDIR

.tex.pdf:
  # We run pdflatex twice to get references right.
  ${SHELL} cd ${<D} ${AND} pdflatex ${<F} ${AND} pdflatex ${<F}
  ${DONE}
  
.tex.install:
  ${SHELL} if not exist ..\${DOCDIR} mkdir ..\${DOCDIR}
  ${SHELL} move $*.pdf ..\${DOCDIR}
  ${DONE}

!endif # !ifndef MAKEFILES

