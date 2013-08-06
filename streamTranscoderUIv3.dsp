# Microsoft Developer Studio Project File - Name="streamTranscoderUIv3" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=streamTranscoderUIv3 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "streamTranscoderUIv3.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "streamTranscoderUIv3.mak" CFG="streamTranscoderUIv3 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "streamTranscoderUIv3 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "streamTranscoderUIv3 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "streamTranscoderUIv3 - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../external/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_VORBIS" /D "HAVE_LAME" /D "HAVE_FAAC" /D "HAVE_AACP" /D "HAVE_FLAC" /D "_AFXDLL" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 vorbis.lib ogg.lib libfaac.lib delayimp.lib Winmm.lib ws2_32.lib libOggFLAC.lib libFLAC.lib pthreadVSE.lib libmad.lib /nologo /subsystem:windows /machine:I386 /libpath:"../external/lib"

!ELSEIF  "$(CFG)" == "streamTranscoderUIv3 - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../external/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "HAVE_VORBIS" /D "HAVE_LAME" /D "HAVE_FAAC" /D "HAVE_AACP" /D "HAVE_FLAC" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vorbis.lib ogg.lib libfaac.lib delayimp.lib Winmm.lib ws2_32.lib libOggFLAC.lib libFLAC.lib pthreadVSE.lib libmad.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../external/lib"

!ENDIF 

# Begin Target

# Name "streamTranscoderUIv3 - Win32 Release"
# Name "streamTranscoderUIv3 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cbuffer.c
# End Source File
# Begin Source File

SOURCE=.\decodethread.cpp
# End Source File
# Begin Source File

SOURCE=.\mp3decoder.cpp
# End Source File
# Begin Source File

SOURCE=.\reconnectthread.cpp
# End Source File
# Begin Source File

SOURCE=.\socket_client.c
# End Source File
# Begin Source File

SOURCE=.\sourcethread.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\streamTranscoderUIv3.cpp
# End Source File
# Begin Source File

SOURCE=.\streamTranscoderUIv3.rc
# End Source File
# Begin Source File

SOURCE=.\streamTranscoderUIv3Dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\vorbisdecoder.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cbuffer.h
# End Source File
# Begin Source File

SOURCE=.\decodethread.h
# End Source File
# Begin Source File

SOURCE=.\enc_if.h
# End Source File
# Begin Source File

SOURCE=.\mp3decoder.h
# End Source File
# Begin Source File

SOURCE=.\reconnectthread.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\socket_client.h
# End Source File
# Begin Source File

SOURCE=.\sourcethread.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\streamTranscoderUIv3.h
# End Source File
# Begin Source File

SOURCE=.\streamTranscoderUIv3Dlg.h
# End Source File
# Begin Source File

SOURCE=.\vorbisdecoder.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\oddsock_logo.bmp
# End Source File
# Begin Source File

SOURCE=.\res\streamTranscoderUIv3.ico
# End Source File
# Begin Source File

SOURCE=.\res\streamTranscoderUIv3.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
