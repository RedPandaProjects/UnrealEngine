# Microsoft Developer Studio Project File - Name="Core" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Core - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Core.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Core.mak" CFG="Core - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Core - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Core - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/Unreal/Core", EAAAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Core - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Lib"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MD /W4 /WX /vd0 /GX /O2 /I "..\Inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D CORE_API=__declspec(dllexport) /Yu"CorePrivate.h" /FD /Zm256 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 gdi32.lib user32.lib kernel32.lib winmm.lib shell32.lib ole32.lib /nologo /base:"0x10100000" /subsystem:windows /dll /incremental:yes /machine:I386 /out:"..\..\System\Core.dll"

!ELSEIF  "$(CFG)" == "Core - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Lib"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MDd /W4 /WX /vd0 /Zi /Od /I "..\Inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D CORE_API=__declspec(dllexport) /Yu"CorePrivate.h" /FD /Zm256 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 gdi32.lib user32.lib kernel32.lib winmm.lib shell32.lib ole32.lib /nologo /base:"0x10100000" /subsystem:windows /dll /debug /machine:I386 /out:"..\..\System\Core.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Core - Win32 Release"
# Name "Core - Win32 Debug"
# Begin Group "Src"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\Core.cpp
# ADD CPP /Yc"CorePrivate.h"
# End Source File
# Begin Source File

SOURCE=.\CorePrivate.h
# End Source File
# Begin Source File

SOURCE=.\UnCache.cpp
# End Source File
# Begin Source File

SOURCE=.\UnClass.cpp
# End Source File
# Begin Source File

SOURCE=.\UnCorSc.cpp
# End Source File
# Begin Source File

SOURCE=.\UnFile.cpp
# End Source File
# Begin Source File

SOURCE=.\UnLinker.h
# End Source File
# Begin Source File

SOURCE=.\UnMem.cpp
# End Source File
# Begin Source File

SOURCE=.\UnName.cpp
# End Source File
# Begin Source File

SOURCE=.\UnObj.cpp
# End Source File
# Begin Source File

SOURCE=.\UnOutDev.cpp
# End Source File
# Begin Source File

SOURCE=.\UnPlat.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\UnProp.cpp
# End Source File
# End Group
# Begin Group "Classes"

# PROP Default_Filter "*.uc"
# Begin Source File

SOURCE=..\Classes\Object.uc
# End Source File
# Begin Source File

SOURCE=..\Classes\Subsystem.uc
# End Source File
# End Group
# Begin Group "Inc"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\Inc\Core.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnArc.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnBuild.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnCache.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnCId.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnClass.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnCorObj.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnFile.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnMem.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnName.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnNames.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnObjBas.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnObjVer.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnPlatfm.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnScript.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnStack.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnTemplate.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnType.h
# End Source File
# Begin Source File

SOURCE=..\Inc\UnVcWin32.h
# End Source File
# End Group
# End Target
# End Project
