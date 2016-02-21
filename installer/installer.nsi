;NSIS Modern User Interface
;Start Menu Folder Selection Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
;--------------------------------
;General
;WSS/Unity specific

  !define UnityExeName "Mown"
  !define ProgramRegistryName "Mown"
  !define ProgramReadableName "Mown"
  !define ProgramVersion "0.0.1"
  
  
  ;Name and file
  Name "${ProgramRegistryName}"
  OutFile "Mown_win32.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${ProgramRegistryName}"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Beuiot\${ProgramRegistryName}" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

BrandingText "${ProgramReadableName} ${ProgramVersion}"
  !define MUI_ABORTWARNING

  ;----- uninstall first stuff
  Function .onInit
 
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramRegistryName}" \
  "UninstallString"
  StrCmp $R0 "" done
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "${ProgramReadableName} is already installed. $\n$\nClick `OK` to remove the \
  previous version or `Cancel` to cancel this upgrade." \
  IDOK uninst
  Abort
 
;Run the uninstaller
uninst:
  ClearErrors
  ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller done
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
  no_remove_uninstaller:
 
done:
 
FunctionEnd

;--- end of uninstall stuff

  
  
  
;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "agreement.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Beuiot.info\${ProgramRegistryName}" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
  Function finishpageaction
CreateShortcut "$DESKTOP\${ProgramReadableName}.lnk" "$INSTDIR\${UnityExeName}.exe"
FunctionEnd

!define MUI_FINISHPAGE_SHOWREADME ""
;!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create Desktop Shortcut"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION finishpageaction

!define MUI_FINISHPAGE_RUN_NOTCHECKED
!define MUI_FINISHPAGE_RUN "$INSTDIR\${UnityExeName}.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${ProgramReadableName}"
!insertmacro MUI_PAGE_FINISH

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "${ProgramReadableName}" SecDummy
SectionIn RO

  SetOutPath "$INSTDIR"
  
  ;ADD YOUR OWN FILES HERE...
  File "..\mown-win32\${UnityExeName}.exe"
  ;File /r "${UnityExeName}_Data"
  File /r ..\mown-win32\*.*
  
  ;Store installation folder
  WriteRegStr HKCU "Software\Beuiot.info\${ProgramRegistryName}" "" $INSTDIR
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramRegistryName}" \
                 "DisplayName" "${ProgramRegistryName}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramRegistryName}" \
                 "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
  
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${ProgramRegistryName}.lnk" "$INSTDIR\${UnityExeName}.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecDummy ${LANG_ENGLISH} "Install ${ProgramReadableName}."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\${UnityExeName}.exe"
  Delete "$DESKTOP\${ProgramReadableName}.lnk"
  
  ;RMDir /r "$INSTDIR\audio"
  ;RMDir /r "$INSTDIR\bearer"
  ;RMDir /r "$INSTDIR\iconengines"
  ;RMDir /r "$INSTDIR\imageformats"
  ;RMDir /r "$INSTDIR\mediaservice"
  ;RMDir /r "$INSTDIR\platforms"
  ;RMDir /r "$INSTDIR\playlistformats"
  ;RMDir /r "$INSTDIR\position"
  ;RMDir /r "$INSTDIR\printsupport"
  ;RMDir /r "$INSTDIR\sensorgestures"
  ;RMDir /r "$INSTDIR\sensors"
  ;RMDir /r "$INSTDIR\sqldrivers"
  
  RMDir /r "$INSTDIR"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
	

  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${ProgramRegistryName}.lnk"
  
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  
  DeleteRegKey /ifempty HKCU "Software\Beuiot.info\${ProgramRegistryName}"
  DeleteRegKey /ifempty HKCU "Software\Beuiot.info"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramRegistryName}"

SectionEnd