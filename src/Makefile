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
#         Use "make prepare" to avoid Borland IDE problems with write protected
#         project files.
#
#         This makefile depends on the existence of a file called ".makefiles" 
#         in the same directory which is either empty or contains
#         an automatically created list of makefiles in subdirectories.
#
#         You will probably want to edit the file ".makefiles" to exclude
#         broken and/or unneeded subprojects. If doing so, uncomment
#         the line defining DONTCLEANME to protect your changes.
#
###################################################################################

# How to execute commands.
SHELL = cmd /c @prompt $s && 
# Name of file that contains list of subproject makefiles.
MKNAM = .makefiles
# How to zero the subproject makefile list.
EMPTYMK = echo ^# This file is used by MAKE. Do not delete it.>$(MKNAM)
# Extensions of files that need to be always writeable.
FORCEWRITE = *.bpr *.res *.mak *.pdf *.exe
# Extensions of files that will be deleted by 'make clean'.
GARBAGE = *.obj *.tds *.csm *.~* *.\#* *.docinfo
# How to report when we are done with a target.
DONE = @${SHELL} echo. && echo Done with target "$@". && echo.
# Where to put binaries below the installation root.
BINDIR = prog
# Where to put tools below the installation root if no separate tools directory is specified.
TOOLSBINDIR = tools

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
all docinfo prepare clean install update pdf:
  attrib -r ${MKNAM}
  ${EMPTYMK}
  echo ^#DONTCLEANME=true ^# Uncomment this line if you edit this file. >>${MKNAM}
  echo ^#INSTALLDIR=      ^# The root directory for "make install".     >>${MKNAM}
  echo ^#TOOLSINSTALLDIR= ^# Optionally, a separate tools destination\
                             directory for "make install".              >>${MKNAM}
  echo MAKEFILES =\                                                     >>${MKNAM}
  -for /R %i in (*.bpr) do echo ^"%~di%~pi%~ni.mak^" \                  >>${MKNAM}
  echo.                                                                 >>${MKNAM}
  echo LATEXDOCS =\                                                     >>${MKNAM}
  -for /R %i in (*.tex) do grep \\documentclass %i ${AND} echo ^"%i^" \ >>${MKNAM}
  echo.                                                                 >>${MKNAM}
  ${MAKE} -${MAKEFLAGS} $@

!else # !ifndef MAKEFILES
##################################################################################
#
# User targets
#
##################################################################################

# Make all subprojects.
all: prepare ${MAKEFILES:.mak=.all}
  ${DONE}
  
# Do some automatic documentation.
docinfo: ${MAKEFILES:.mak=.docinfo}
  ${DONE}

# Prepare source files for opening projects without confusing the Borland IDE.
prepare: ${MAKEFILES:.mak=.prepare}
  -${SHELL} for /R %i in (${FORCEWRITE} ${GARBAGE}) do attrib -r %i
!ifndef DONTCLEANME
  -${SHELL} attrib -r ${MKNAM}
!endif
  ${DONE}

# Cleanup.
clean: prepare ${MAKEFILES:.mak=.clean}
  -${SHELL} for /R %i in (${GARBAGE}) do del %i
!ifndef DONTCLEANME
  ${EMPTYMK}
!endif
  ${DONE}
  
# Install executables.
install: ${MAKEFILES:.mak=.install}
  ${DONE}

# Replace installed executables with current versions.
update: ${MAKEFILES:.mak=.update}
  ${DONE}

# Create pdfs from tex files.
pdf: ${LATEXDOCS:.tex=.pdf}
  ${DONE}
  
##################################################################################
#
# Suffix rules for creating and executing makefiles
#
##################################################################################
.bpr.mak:
  (bpr2mak $<) & echo.
  # Add our own additional targets to the makefile created by bpr2mak.
  @echo.                                                             >>$@
  @echo docinfo: $^^(PROJECT) obj\Documentar.obj                     >>$@
  @echo ^^ -$^^(BCB)\BIN\$^^(LINKER) $^^(LFLAGS) \                   >>$@
  @echo ^^ -L$^^(LIBPATH) obj\Documentar.obj $^^(ALLOBJ),\           >>$@
  @echo ^^ _$^^(PROJECT),, $^^(ALLLIB), $^^(DEFFILE), $^^(ALLRES)    >>$@
  @echo ^^ -_$^^(PROJECT) ^^>$^^(PROJECT:.exe=.info)                 >>$@
  @echo ^^ -del _$^^(PROJECT)                                        >>$@
  @echo ^^ -del _*.tds                                               >>$@
  @echo prepare:                                                     >>$@
  @echo ^^ -attrib -r $^^(PROJECT)                                   >>$@
  @echo ^^ if not exist obj mkdir obj                                >>$@
  @echo install: $^^(PROJECT)                                        >>$@
  @echo ^^ if not exist ^^"$^(DESTDIR)^" mkdir ^^"$^(DESTDIR)^"      >>$@
  @echo ^^ xcopy /y $^^(PROJECT) ^^"$^(DESTDIR)^"                    >>$@
  @echo update: $^^(PROJECT)                                         >>$@
  @echo ^^ if exist ^^"$^(DESTDIR)^" \
            xcopy /d /u /y $^^(PROJECT) ^^"$^(DESTDIR)^"             >>$@
  @echo clean:                                                       >>$@
  @echo ^^ -del $^^(PROJECT) $^^(OBJFILES)                           >>$@
  @echo.                                                             >>$@
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
  ${SHELL} cd ${<D} ${AND} ${MAKE} -f${<F} -${MAKEFLAGS}
  -del shared\obj\bci2000.csm shared\obj\bci2000.^#* > nul 2>&1
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
  # We run pdflatex twice to get the references right.
  ${SHELL} cd ${<D} ${AND} pdflatex ${<F} ${AND} pdflatex ${<F}
  ${DONE}
  
!endif # !ifndef MAKEFILES

