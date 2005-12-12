# ---------------------------------------------------------------------------
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

# ---------------------------------------------------------------------------
# IDE SECTION
# ---------------------------------------------------------------------------
# The following section of the project makefile is managed by the BCB IDE.
# It is recommended to use the IDE to change any of the values in this
# section.
# ---------------------------------------------------------------------------

VERSION = BCB.06.00
# ---------------------------------------------------------------------------
PROJECT = ..\..\..\prog\BufferRead.exe
OBJFILES = obj\BufferRead.obj obj\UBCItime.obj obj\UGenericSignal.obj \
    obj\UGenericVisualization.obj obj\UParameter.obj obj\UState.obj \
    obj\UStatus.obj obj\USysCommand.obj obj\BCIDirectry.obj \
    obj\UGenericFilter.obj obj\UEnvironment.obj obj\UBCIError.obj \
    obj\MeasurementUnits.obj obj\MessageHandler.obj obj\TCPStream.obj \
    obj\DataIOFilter.obj obj\TransmissionFilter.obj obj\UCoreMain.obj \
    obj\BCIDatFileWriter.obj obj\BufferReadADC.obj obj\loadsblib.obj \
    obj\EncodedString.obj obj\ParamRef.obj
RESFILES = 
MAINSOURCE = BufferRead.cpp
RESDEPEN = $(RESFILES) ..\..\shared\UCoreMain.dfm
LIBFILES = 
IDLFILES = 
IDLGENFILES = 
LIBRARIES = vclx.lib bcbsmp.lib tee.lib vcl.lib rtl.lib
PACKAGES = rtl.bpi vcl.bpi vclx.bpi vcljpg.bpi bcbsmp.bpi qrpt.bpi dbrtl.bpi \
    vcldb.bpi bdertl.bpi ibsmp.bpi vcldbx.bpi nmfast.bpi dclocx.bpi Tee4C.bpi \
    TeeDB4C.bpi TeeUI4C.bpi TeePro4C.bpi TeeGL4C.bpi TeeQR4C.bpi
SPARELIBS = rtl.lib vcl.lib tee.lib bcbsmp.lib vclx.lib
DEFFILE = 
OTHERFILES = 
# ---------------------------------------------------------------------------
DEBUGLIBPATH = $(BCB)\lib\debug
RELEASELIBPATH = $(BCB)\lib\release
USERDEFINES = _DEBUG;MODTYPE=1
SYSDEFINES = NO_STRICT
INCLUDEPATH = ..\;..;..\..\Modules;DAS_lib;..\..\SHARED;$(BCB)\include;$(BCB)\include\vcl
LIBPATH = ..\;..;..\..\Modules;DAS_lib;..\..\SHARED;$(BCB)\lib\obj;$(BCB)\lib
WARNINGS= -w-par -w-8027 -w-8026
PATHCPP = .;..\..\SHARED;..\..\SHARED;..\..\SHARED;..\..\SHARED;..\..\SHARED;..\..\SHARED;..\..\SHARED;..\..\SHARED;..\..\shared;..\..\shared;..\..\shared;..\..\shared;..\..\shared;..\..\shared;..;..;..\..\shared;..;..\..\shared;..\..\shared
PATHASM = .;
PATHPAS = .;
PATHRC = .;
PATHOBJ = .;$(LIBPATH)
# ---------------------------------------------------------------------------
CFLAG1 = -Od -H=..\..\shared\obj\bci2000.csm -Hc -Vx -Ve -X- -r- -a8 -5 -b- -k -y \
    -v -vi- -c -tW -tWM
IDLCFLAGS = 
PFLAGS = -N2obj -N0obj -$Y+ -$W -$O- -$A8 -v -JPHNE -M
RFLAGS = 
AFLAGS = /mx /w2 /zi
LFLAGS = -Iobj -D"" -aa -Tpe -x -Gn -v
# ---------------------------------------------------------------------------
ALLOBJ = c0w32.obj sysinit.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) $(LIBRARIES) import32.lib cp32mt.lib
# ---------------------------------------------------------------------------
!ifdef IDEOPTIONS

[Version Info]
IncludeVerInfo=0
AutoIncBuild=0
MajorVer=1
MinorVer=0
Release=0
Build=0
Debug=0
PreRelease=0
Special=0
Private=0
DLL=0

[Version Info Keys]
CompanyName=
FileDescription=
FileVersion=1.0.0.0
InternalName=
LegalCopyright=
LegalTrademarks=
OriginalFilename=
ProductName=
ProductVersion=1.0.0.0
Comments=

[Debugging]
DebugSourceDirs=$(BCB)\source\vcl

!endif





# ---------------------------------------------------------------------------
# MAKE SECTION
# ---------------------------------------------------------------------------
# This section of the project file is not used by the BCB IDE.  It is for
# the benefit of building from the command-line using the MAKE utility.
# ---------------------------------------------------------------------------

.autodepend
# ---------------------------------------------------------------------------
!if "$(USERDEFINES)" != ""
AUSERDEFINES = -d$(USERDEFINES:;= -d)
!else
AUSERDEFINES =
!endif

!if !$d(BCC32)
BCC32 = bcc32
!endif

!if !$d(CPP32)
CPP32 = cpp32
!endif

!if !$d(DCC32)
DCC32 = dcc32
!endif

!if !$d(TASM32)
TASM32 = tasm32
!endif

!if !$d(LINKER)
LINKER = ilink32
!endif

!if !$d(BRCC32)
BRCC32 = brcc32
!endif


# ---------------------------------------------------------------------------
!if $d(PATHCPP)
.PATH.CPP = $(PATHCPP)
.PATH.C   = $(PATHCPP)
!endif

!if $d(PATHPAS)
.PATH.PAS = $(PATHPAS)
!endif

!if $d(PATHASM)
.PATH.ASM = $(PATHASM)
!endif

!if $d(PATHRC)
.PATH.RC  = $(PATHRC)
!endif

!if $d(PATHOBJ)
.PATH.OBJ  = $(PATHOBJ)
!endif
# ---------------------------------------------------------------------------
$(PROJECT): $(OTHERFILES) $(IDLGENFILES) $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) -L$(LIBPATH) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES)
!
# ---------------------------------------------------------------------------
.pas.hpp:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.pas.obj:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.cpp.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.cpp.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.asm.obj:
    $(BCB)\BIN\$(TASM32) $(AFLAGS) -i$(INCLUDEPATH:;= -i) $(AUSERDEFINES) -d$(SYSDEFINES:;= -d) $<, $@

.rc.res:
    $(BCB)\BIN\$(BRCC32) $(RFLAGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -fo$@ $<



# ---------------------------------------------------------------------------




 
docinfo: $(PROJECT) obj\Documentar.obj 
 -$(BCB)\BIN\$(LINKER) $(LFLAGS) \ 
 -L$(LIBPATH) obj\Documentar.obj $(ALLOBJ),\ 
 _$(PROJECT),, $(ALLLIB), $(DEFFILE), $(ALLRES) 
 -_$(PROJECT) >$(PROJECT:.exe=.info) 
 -del _$(PROJECT) 
 -del _*.tds 
prepare: 
 -attrib -r $(PROJECT) 
 if not exist obj mkdir obj 
install: $(PROJECT) 
 if not exist "$(DESTDIR)" mkdir "$(DESTDIR)" 
 xcopy /y $(PROJECT) "$(DESTDIR)" 
update: $(PROJECT) 
 if exist "$(DESTDIR)"   xcopy /d /u /y $(PROJECT) "$(DESTDIR)" 
clean: 
 -del $(PROJECT) $(OBJFILES) 
 
