# Microsoft Developer Studio Project File - Name="j2k_to_image" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=j2k_to_image - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "j2k_to_image.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "j2k_to_image.mak" CFG="j2k_to_image - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "j2k_to_image - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "j2k_to_image - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "j2k_to_image - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../libopenjpeg" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DONT_HAVE_GETOPT" /YX /FD /c
# ADD BASE RSC /l 0x80c /d "NDEBUG"
# ADD RSC /l 0x80c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "j2k_to_image - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "j2k_to_image___Win32_Debug"
# PROP BASE Intermediate_Dir "j2k_to_image___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "j2k_to_image___Win32_Debug"
# PROP Intermediate_Dir "j2k_to_image___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../libopenjpeg" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DONT_HAVE_GETOPT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x80c /d "_DEBUG"
# ADD RSC /l 0x80c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "j2k_to_image - Win32 Release"
# Name "j2k_to_image - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\libopenjpeg\bio.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\cio.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\dwt.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\fix.c
# End Source File
# Begin Source File

SOURCE=.\compat\getopt.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\int.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\j2k.c
# End Source File
# Begin Source File

SOURCE=.\j2k_to_image.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\jp2.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\jpt.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\mct.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\mqc.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\pi.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\raw.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\t1.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\t2.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\tcd.c
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\tgt.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\libopenjpeg\bio.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\cio.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\dwt.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\fix.h
# End Source File
# Begin Source File

SOURCE=.\compat\getopt.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\int.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\j2k.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\jp2.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\jpt.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\mct.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\mqc.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\openjpeg.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\pi.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\raw.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\t1.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\t2.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\tcd.h
# End Source File
# Begin Source File

SOURCE=..\libopenjpeg\tgt.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
