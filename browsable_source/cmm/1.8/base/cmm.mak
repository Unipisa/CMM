# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103
# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=cmm - Win32 Release
!MESSAGE No configuration specified.  Defaulting to cmm - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "cmm - Win32 Release" && "$(CFG)" != "cmm - Win32 Debug" &&\
 "$(CFG)" != "lib - Win32 Release" && "$(CFG)" != "lib - Win32 Debug" &&\
 "$(CFG)" != "test2 - Win32 Release" && "$(CFG)" != "test2 - Win32 Debug" &&\
 "$(CFG)" != "test3 - Win32 Release" && "$(CFG)" != "test3 - Win32 Debug" &&\
 "$(CFG)" != "test4 - Win32 Release" && "$(CFG)" != "test4 - Win32 Debug" &&\
 "$(CFG)" != "test5 - Win32 Release" && "$(CFG)" != "test5 - Win32 Debug" &&\
 "$(CFG)" != "test6 - Win32 Release" && "$(CFG)" != "test6 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "cmm.mak" CFG="cmm - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cmm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cmm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "test2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test2 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "test3 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test3 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "test4 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test4 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "test5 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test5 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "test6 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test6 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "test3 - Win32 Debug"

!IF  "$(CFG)" == "cmm - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "test6 - Win32 Release" "test5 - Win32 Release" "test4 - Win32 Release"\
 "test3 - Win32 Release" "test2 - Win32 Release" "lib - Win32 Release" 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/cmm.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/cmm.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/cmm.lib" 
LIB32_OBJS=

!ELSEIF  "$(CFG)" == "cmm - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cmm___Wi"
# PROP BASE Intermediate_Dir "cmm___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "test6 - Win32 Debug" "test5 - Win32 Debug" "test4 - Win32 Debug"\
 "test3 - Win32 Debug" "test2 - Win32 Debug" "lib - Win32 Debug" 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/cmm.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/cmm.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/cmm.lib" 
LIB32_OBJS=

!ELSEIF  "$(CFG)" == "lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "lib\Release"
# PROP BASE Intermediate_Dir "lib\Release"
# PROP BASE Target_Dir "lib"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir "lib"
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\cmm.lib"

CLEAN : 
	-@erase ".\Release\cmm.lib"
	-@erase ".\Release\memory.obj"
	-@erase ".\Release\tempheap.obj"
	-@erase ".\Release\cmm.obj"
	-@erase ".\Release\msw.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/lib.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/lib.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\cmm.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/cmm.lib" 
LIB32_OBJS= \
	"$(INTDIR)/memory.obj" \
	"$(INTDIR)/tempheap.obj" \
	"$(INTDIR)/cmm.obj" \
	"$(INTDIR)/msw.obj"

"$(OUTDIR)\cmm.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "lib\Debug"
# PROP BASE Intermediate_Dir "lib\Debug"
# PROP BASE Target_Dir "lib"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir "lib"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\cmm.lib"

CLEAN : 
	-@erase ".\Debug\cmm.lib"
	-@erase ".\Debug\memory.obj"
	-@erase ".\Debug\cmm.obj"
	-@erase ".\Debug\msw.obj"
	-@erase ".\Debug\tempheap.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/lib.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/lib.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\cmm.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/cmm.lib" 
LIB32_OBJS= \
	"$(INTDIR)/memory.obj" \
	"$(INTDIR)/cmm.obj" \
	"$(INTDIR)/msw.obj" \
	"$(INTDIR)/tempheap.obj"

"$(OUTDIR)\cmm.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "test2\Release"
# PROP BASE Intermediate_Dir "test2\Release"
# PROP BASE Target_Dir "test2"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir "test2"
OUTDIR=.\Release
INTDIR=.\Release

ALL : "lib - Win32 Release" "$(OUTDIR)\test2.exe"

CLEAN : 
	-@erase ".\Release\test2.exe"
	-@erase ".\Release\test2.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test2.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test2.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/test2.pdb" /machine:I386 /out:"$(OUTDIR)/test2.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test2.obj" \
	".\Release\cmm.lib"

"$(OUTDIR)\test2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "test2\Debug"
# PROP BASE Intermediate_Dir "test2\Debug"
# PROP BASE Target_Dir "test2"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir "test2"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "lib - Win32 Debug" "$(OUTDIR)\test2.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\test2.exe"
	-@erase ".\Debug\test2.obj"
	-@erase ".\Debug\test2.ilk"
	-@erase ".\Debug\test2.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test2.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test2.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/test2.pdb" /debug /machine:I386 /out:"$(OUTDIR)/test2.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test2.obj" \
	".\Debug\cmm.lib"

"$(OUTDIR)\test2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "test3\Release"
# PROP BASE Intermediate_Dir "test3\Release"
# PROP BASE Target_Dir "test3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir "test3"
OUTDIR=.\Release
INTDIR=.\Release

ALL : "lib - Win32 Release" "$(OUTDIR)\test3.exe"

CLEAN : 
	-@erase ".\Release\test3.exe"
	-@erase ".\Release\test3.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test3.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test3.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/test3.pdb" /machine:I386 /out:"$(OUTDIR)/test3.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test3.obj" \
	".\Release\cmm.lib"

"$(OUTDIR)\test3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "test3\Debug"
# PROP BASE Intermediate_Dir "test3\Debug"
# PROP BASE Target_Dir "test3"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir "test3"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "lib - Win32 Debug" "$(OUTDIR)\test3.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\test3.exe"
	-@erase ".\Debug\test3.obj"
	-@erase ".\Debug\test3.ilk"
	-@erase ".\Debug\test3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test3.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test3.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/test3.pdb" /debug /machine:I386 /out:"$(OUTDIR)/test3.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test3.obj" \
	".\Debug\cmm.lib"

"$(OUTDIR)\test3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test4 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "test4\Release"
# PROP BASE Intermediate_Dir "test4\Release"
# PROP BASE Target_Dir "test4"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir "test4"
OUTDIR=.\Release
INTDIR=.\Release

ALL : "lib - Win32 Release" "$(OUTDIR)\test4.exe"

CLEAN : 
	-@erase ".\Release\test4.exe"
	-@erase ".\Release\test4.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test4.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test4.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/test4.pdb" /machine:I386 /out:"$(OUTDIR)/test4.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test4.obj" \
	".\Release\cmm.lib"

"$(OUTDIR)\test4.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test4 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "test4\Debug"
# PROP BASE Intermediate_Dir "test4\Debug"
# PROP BASE Target_Dir "test4"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir "test4"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "lib - Win32 Debug" "$(OUTDIR)\test4.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\test4.exe"
	-@erase ".\Debug\test4.obj"
	-@erase ".\Debug\test4.ilk"
	-@erase ".\Debug\test4.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test4.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test4.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/test4.pdb" /debug /machine:I386 /out:"$(OUTDIR)/test4.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test4.obj" \
	".\Debug\cmm.lib"

"$(OUTDIR)\test4.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test5 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "test5\Release"
# PROP BASE Intermediate_Dir "test5\Release"
# PROP BASE Target_Dir "test5"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir "test5"
OUTDIR=.\Release
INTDIR=.\Release

ALL : "lib - Win32 Release" "$(OUTDIR)\test5.exe"

CLEAN : 
	-@erase ".\Release\test5.exe"
	-@erase ".\Release\test5.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test5.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test5.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/test5.pdb" /machine:I386 /out:"$(OUTDIR)/test5.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test5.obj" \
	".\Release\cmm.lib"

"$(OUTDIR)\test5.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test5 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "test5\Debug"
# PROP BASE Intermediate_Dir "test5\Debug"
# PROP BASE Target_Dir "test5"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir "test5"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "lib - Win32 Debug" "$(OUTDIR)\test5.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\test5.exe"
	-@erase ".\Debug\test5.obj"
	-@erase ".\Debug\test5.ilk"
	-@erase ".\Debug\test5.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test5.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test5.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/test5.pdb" /debug /machine:I386 /out:"$(OUTDIR)/test5.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test5.obj" \
	".\Debug\cmm.lib"

"$(OUTDIR)\test5.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test6 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "test6\Release"
# PROP BASE Intermediate_Dir "test6\Release"
# PROP BASE Target_Dir "test6"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir "test6"
OUTDIR=.\Release
INTDIR=.\Release

ALL : "lib - Win32 Release" "$(OUTDIR)\test6.exe"

CLEAN : 
	-@erase ".\Release\test6.exe"
	-@erase ".\Release\test6.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test6.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test6.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/test6.pdb" /machine:I386 /out:"$(OUTDIR)/test6.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test6.obj" \
	".\Release\cmm.lib"

"$(OUTDIR)\test6.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test6 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "test6\Debug"
# PROP BASE Intermediate_Dir "test6\Debug"
# PROP BASE Target_Dir "test6"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir "test6"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "lib - Win32 Debug" "$(OUTDIR)\test6.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\test6.exe"
	-@erase ".\Debug\test6.obj"
	-@erase ".\Debug\test6.ilk"
	-@erase ".\Debug\test6.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/test6.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/test6.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/test6.pdb" /debug /machine:I386 /out:"$(OUTDIR)/test6.exe" 
LINK32_OBJS= \
	"$(INTDIR)/test6.obj" \
	".\Debug\cmm.lib"

"$(OUTDIR)\test6.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "cmm - Win32 Release"
# Name "cmm - Win32 Debug"

!IF  "$(CFG)" == "cmm - Win32 Release"

!ELSEIF  "$(CFG)" == "cmm - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "lib"

!IF  "$(CFG)" == "cmm - Win32 Release"

"lib - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Release" 

!ELSEIF  "$(CFG)" == "cmm - Win32 Debug"

"lib - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "test2"

!IF  "$(CFG)" == "cmm - Win32 Release"

"test2 - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test2 - Win32 Release" 

!ELSEIF  "$(CFG)" == "cmm - Win32 Debug"

"test2 - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test2 - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "test3"

!IF  "$(CFG)" == "cmm - Win32 Release"

"test3 - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test3 - Win32 Release" 

!ELSEIF  "$(CFG)" == "cmm - Win32 Debug"

"test3 - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test3 - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "test4"

!IF  "$(CFG)" == "cmm - Win32 Release"

"test4 - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test4 - Win32 Release" 

!ELSEIF  "$(CFG)" == "cmm - Win32 Debug"

"test4 - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test4 - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "test5"

!IF  "$(CFG)" == "cmm - Win32 Release"

"test5 - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test5 - Win32 Release" 

!ELSEIF  "$(CFG)" == "cmm - Win32 Debug"

"test5 - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test5 - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "test6"

!IF  "$(CFG)" == "cmm - Win32 Release"

"test6 - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test6 - Win32 Release" 

!ELSEIF  "$(CFG)" == "cmm - Win32 Debug"

"test6 - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="test6 - Win32 Debug" 

!ENDIF 

# End Project Dependency
# End Target
################################################################################
# Begin Target

# Name "lib - Win32 Release"
# Name "lib - Win32 Debug"

!IF  "$(CFG)" == "lib - Win32 Release"

!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\tempheap.h

!IF  "$(CFG)" == "lib - Win32 Release"

!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cmm.h

!IF  "$(CFG)" == "lib - Win32 Release"

!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\machine.h

!IF  "$(CFG)" == "lib - Win32 Release"

!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\msw.cpp

!IF  "$(CFG)" == "lib - Win32 Release"

DEP_CPP_MSW_C=\
	".\cmm.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\machine.h"\
	".\msw.h"\
	

"$(INTDIR)\msw.obj" : $(SOURCE) $(DEP_CPP_MSW_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

DEP_CPP_MSW_C=\
	".\cmm.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\machine.h"\
	".\msw.h"\
	

"$(INTDIR)\msw.obj" : $(SOURCE) $(DEP_CPP_MSW_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\msw.h

!IF  "$(CFG)" == "lib - Win32 Release"

!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tempheap.cpp

!IF  "$(CFG)" == "lib - Win32 Release"

DEP_CPP_TEMPH=\
	".\tempheap.h"\
	".\cmm.h"\
	".\machine.h"\
	".\msw.h"\
	

"$(INTDIR)\tempheap.obj" : $(SOURCE) $(DEP_CPP_TEMPH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

DEP_CPP_TEMPH=\
	".\tempheap.h"\
	".\cmm.h"\
	".\machine.h"\
	".\msw.h"\
	

"$(INTDIR)\tempheap.obj" : $(SOURCE) $(DEP_CPP_TEMPH) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cmm.cpp

!IF  "$(CFG)" == "lib - Win32 Release"

DEP_CPP_CMM_C=\
	".\cmm.h"\
	".\machine.h"\
	".\msw.h"\
	
NODEP_CPP_CMM_C=\
	".\heap"\
	".\if"\
	

"$(INTDIR)\cmm.obj" : $(SOURCE) $(DEP_CPP_CMM_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

DEP_CPP_CMM_C=\
	".\cmm.h"\
	".\machine.h"\
	".\msw.h"\
	

"$(INTDIR)\cmm.obj" : $(SOURCE) $(DEP_CPP_CMM_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\memory.cpp

!IF  "$(CFG)" == "lib - Win32 Release"

DEP_CPP_MEMOR=\
	".\machine.h"\
	
NODEP_CPP_MEMOR=\
	".\CmmLeastDescribedAddress"\
	

"$(INTDIR)\memory.obj" : $(SOURCE) $(DEP_CPP_MEMOR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

DEP_CPP_MEMOR=\
	".\machine.h"\
	

"$(INTDIR)\memory.obj" : $(SOURCE) $(DEP_CPP_MEMOR) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "test2 - Win32 Release"
# Name "test2 - Win32 Debug"

!IF  "$(CFG)" == "test2 - Win32 Release"

!ELSEIF  "$(CFG)" == "test2 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "lib"

!IF  "$(CFG)" == "test2 - Win32 Release"

"lib - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Release" 

!ELSEIF  "$(CFG)" == "test2 - Win32 Debug"

"lib - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\test2.cpp

!IF  "$(CFG)" == "test2 - Win32 Release"

DEP_CPP_TEST2=\
	".\cmm.h"\
	".\machine.h"\
	".\msw.h"\
	

"$(INTDIR)\test2.obj" : $(SOURCE) $(DEP_CPP_TEST2) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "test2 - Win32 Debug"

DEP_CPP_TEST2=\
	".\cmm.h"\
	".\machine.h"\
	

"$(INTDIR)\test2.obj" : $(SOURCE) $(DEP_CPP_TEST2) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "test3 - Win32 Release"
# Name "test3 - Win32 Debug"

!IF  "$(CFG)" == "test3 - Win32 Release"

!ELSEIF  "$(CFG)" == "test3 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "lib"

!IF  "$(CFG)" == "test3 - Win32 Release"

"lib - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Release" 

!ELSEIF  "$(CFG)" == "test3 - Win32 Debug"

"lib - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\test3.cpp

!IF  "$(CFG)" == "test3 - Win32 Release"

DEP_CPP_TEST3=\
	".\cmm.h"\
	".\machine.h"\
	

"$(INTDIR)\test3.obj" : $(SOURCE) $(DEP_CPP_TEST3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "test3 - Win32 Debug"

DEP_CPP_TEST3=\
	".\cmm.h"\
	".\machine.h"\
	".\msw.h"\
	

"$(INTDIR)\test3.obj" : $(SOURCE) $(DEP_CPP_TEST3) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "test4 - Win32 Release"
# Name "test4 - Win32 Debug"

!IF  "$(CFG)" == "test4 - Win32 Release"

!ELSEIF  "$(CFG)" == "test4 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "lib"

!IF  "$(CFG)" == "test4 - Win32 Release"

"lib - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Release" 

!ELSEIF  "$(CFG)" == "test4 - Win32 Debug"

"lib - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\test4.cpp

!IF  "$(CFG)" == "test4 - Win32 Release"

DEP_CPP_TEST4=\
	".\cmm.h"\
	".\machine.h"\
	

"$(INTDIR)\test4.obj" : $(SOURCE) $(DEP_CPP_TEST4) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "test4 - Win32 Debug"

DEP_CPP_TEST4=\
	".\cmm.h"\
	".\machine.h"\
	".\msw.h"\
	

"$(INTDIR)\test4.obj" : $(SOURCE) $(DEP_CPP_TEST4) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "test5 - Win32 Release"
# Name "test5 - Win32 Debug"

!IF  "$(CFG)" == "test5 - Win32 Release"

!ELSEIF  "$(CFG)" == "test5 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "lib"

!IF  "$(CFG)" == "test5 - Win32 Release"

"lib - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Release" 

!ELSEIF  "$(CFG)" == "test5 - Win32 Debug"

"lib - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\test5.cpp

!IF  "$(CFG)" == "test5 - Win32 Release"

DEP_CPP_TEST5=\
	".\tempheap.h"\
	".\cmm.h"\
	".\machine.h"\
	".\msw.h"\
	

"$(INTDIR)\test5.obj" : $(SOURCE) $(DEP_CPP_TEST5) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "test5 - Win32 Debug"

DEP_CPP_TEST5=\
	".\tempheap.h"\
	".\cmm.h"\
	".\machine.h"\
	

"$(INTDIR)\test5.obj" : $(SOURCE) $(DEP_CPP_TEST5) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "test6 - Win32 Release"
# Name "test6 - Win32 Debug"

!IF  "$(CFG)" == "test6 - Win32 Release"

!ELSEIF  "$(CFG)" == "test6 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "lib"

!IF  "$(CFG)" == "test6 - Win32 Release"

"lib - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Release" 

!ELSEIF  "$(CFG)" == "test6 - Win32 Debug"

"lib - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\cmm.mak CFG="lib - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\test6.cpp
DEP_CPP_TEST6=\
	".\tempheap.h"\
	".\cmm.h"\
	".\machine.h"\
	

"$(INTDIR)\test6.obj" : $(SOURCE) $(DEP_CPP_TEST6) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
