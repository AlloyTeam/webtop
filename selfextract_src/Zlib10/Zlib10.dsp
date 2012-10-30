# Microsoft Developer Studio Project File - Name="Zlib10" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Zlib10 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Zlib10.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Zlib10.mak" CFG="Zlib10 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Zlib10 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Zlib10 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Zlib10 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Zlib10 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Zlib10 - Win32 Release"
# Name "Zlib10 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Adler32.c
# End Source File
# Begin Source File

SOURCE=.\DEFLATE.C
# End Source File
# Begin Source File

SOURCE=.\INFBLOCK.C
# End Source File
# Begin Source File

SOURCE=.\INFCODES.C
# End Source File
# Begin Source File

SOURCE=.\INFFAST.C
# End Source File
# Begin Source File

SOURCE=.\INFLATE.C
# End Source File
# Begin Source File

SOURCE=.\INFTREES.C
# End Source File
# Begin Source File

SOURCE=.\INFUTIL.C
# End Source File
# Begin Source File

SOURCE=.\TREES.C
# End Source File
# Begin Source File

SOURCE=.\Zlib10.cpp
# End Source File
# Begin Source File

SOURCE=.\zlibengn.cpp
# End Source File
# Begin Source File

SOURCE=.\zlibtest.cpp
# End Source File
# Begin Source File

SOURCE=.\ZUTIL.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DEFLATE.H
# End Source File
# Begin Source File

SOURCE=.\INFBLOCK.H
# End Source File
# Begin Source File

SOURCE=.\INFCODES.H
# End Source File
# Begin Source File

SOURCE=.\INFFAST.H
# End Source File
# Begin Source File

SOURCE=.\INFTREES.H
# End Source File
# Begin Source File

SOURCE=.\INFUTIL.H
# End Source File
# Begin Source File

SOURCE=.\ZCONF.H
# End Source File
# Begin Source File

SOURCE=.\ZLIB.H
# End Source File
# Begin Source File

SOURCE=.\Zlib10.h
# End Source File
# Begin Source File

SOURCE=.\zlibengn.h
# End Source File
# Begin Source File

SOURCE=.\ZUTIL.H
# End Source File
# End Group
# End Target
# End Project
