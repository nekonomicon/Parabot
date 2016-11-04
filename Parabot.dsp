# Microsoft Developer Studio Project File - Name="Parabot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Parabot - Win32 BC
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "Parabot.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "Parabot.mak" CFG="Parabot - Win32 BC"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Parabot - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Parabot - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Parabot - Win32 BC" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Parabot - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /Ob2 /I "bot" /I "utils" /I "F:/Half Life/SDK 2.2/SDK/Source Code/common" /I "F:/Half Life/SDK 2.2/SDK/Source Code/dlls" /I "F:/Half Life/SDK 2.2./SDK/Source Code/engine" /I "F:/Half Life/SDK 2.2/SDK/Source Code/pm_shared" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes /map /debug /machine:I386 /def:"Parabot.def"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "Parabot - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "pia" /I "bot" /I "utils" /I "F:/Half Life/SDK 2.2/SDK/Source Code/common" /I "F:/Half Life/SDK 2.2/SDK/Source Code/dlls" /I "F:/Half Life/SDK 2.2./SDK/Source Code/engine" /I "F:/Half Life/SDK 2.2/SDK/Source Code/pm_shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fr /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /def:"Parabot.def" /pdbtype:con
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "Parabot - Win32 BC"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Parabot___Win32_BC"
# PROP BASE Intermediate_Dir "Parabot___Win32_BC"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Parabot___Win32_BC"
# PROP Intermediate_Dir "Parabot___Win32_BC"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "bot" /I "utils" /I "F:/Half Life/SDK 2.2/SDK/Source Code/common" /I "F:/Half Life/SDK 2.2/SDK/Source Code/dlls" /I "F:/Half Life/SDK 2.2./SDK/Source Code/engine" /I "F:/Half Life/SDK 2.2/SDK/Source Code/pm_shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fr /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "bot" /I "utils" /I "F:/Half Life/SDK 2.2/SDK/Source Code/common" /I "F:/Half Life/SDK 2.2/SDK/Source Code/dlls" /I "F:/Half Life/SDK 2.2./SDK/Source Code/engine" /I "F:/Half Life/SDK 2.2/SDK/Source Code/pm_shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fr /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# SUBTRACT BASE BSC32 /nologo
# SUBTRACT BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /def:"Parabot.def" /pdbtype:con
# SUBTRACT BASE LINK32 /pdb:none /incremental:no
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /def:"Parabot.def" /pdbtype:con
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "Parabot - Win32 Release"
# Name "Parabot - Win32 Debug"
# Name "Parabot - Win32 BC"
# Begin Group "DLL-Source"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\bot\bot.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\bot_client.cpp
# End Source File
# Begin Source File

SOURCE=.\dlls\commands.cpp
# End Source File
# Begin Source File

SOURCE=.\dlls\dll.cpp
# End Source File
# Begin Source File

SOURCE=.\dlls\engine.cpp
# End Source File
# Begin Source File

SOURCE=.\dlls\h_export.cpp
# End Source File
# Begin Source File

SOURCE=.\dlls\linkfunc.cpp
# End Source File
# Begin Source File

SOURCE=.\dlls\startframe.cpp
# End Source File
# Begin Source File

SOURCE=.\dlls\util.cpp
# End Source File
# End Group
# Begin Group "DLL-Header"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\bot\bot.h
# End Source File
# Begin Source File

SOURCE=.\bot\bot_client.h
# End Source File
# Begin Source File

SOURCE=.\bot\bot_func.h
# End Source File
# Begin Source File

SOURCE=.\bot\bot_weapons.h
# End Source File
# Begin Source File

SOURCE=.\dlls\engine.h
# End Source File
# Begin Source File

SOURCE=.\dlls\util.h
# End Source File
# End Group
# Begin Group "Parabot-Source"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\bot\marker.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\parabot.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_action.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_cell.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_chat.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_combat.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_combatgoals.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_configuration.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_focus.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_global.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_goalfinder.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_goals.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_journey.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_kills.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_mapcells.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_mapgraph.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_mapimport.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_navpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_needs.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_observer.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_path.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_perception.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_roaming.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_sectors.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_vistable.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pb_weaponhandling.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pbt_dynarray.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\pbt_priorityqueue.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\sounds.cpp
# End Source File
# Begin Source File

SOURCE=.\bot\utilityfuncs.cpp
# End Source File
# End Group
# Begin Group "Parabot-Header"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\bot\hl_classes.h
# End Source File
# Begin Source File

SOURCE=.\bot\marker.h
# End Source File
# Begin Source File

SOURCE=.\bot\parabot.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_action.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_cell.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_chat.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_combat.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_configuration.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_focus.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_global.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_goalfinder.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_goals.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_journey.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_kills.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_mapcells.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_mapgraph.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_navpoint.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_needs.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_observer.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_path.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_perception.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_roaming.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_sectors.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_vistable.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_weapon.h
# End Source File
# Begin Source File

SOURCE=.\bot\pb_weaponhandling.h
# End Source File
# Begin Source File

SOURCE=.\bot\PBT_DynArray.h
# End Source File
# Begin Source File

SOURCE=.\bot\pbt_priorityqueue.h
# End Source File
# Begin Source File

SOURCE=.\bot\sounds.h
# End Source File
# Begin Source File

SOURCE=.\bot\utilityfuncs.h
# End Source File
# End Group
# Begin Group "Utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\utils\checkvec.h
# End Source File
# Begin Source File

SOURCE=.\utils\dynpq.h
# End Source File
# End Group
# Begin Group "HL-Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE="F:\Half Life\Sdk 2.2\Sdk\Source Code\dlls\animation.cpp"
# End Source File
# End Group
# End Target
# End Project
