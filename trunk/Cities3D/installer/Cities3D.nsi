; Cities3D 0.11.0 setup.
; Same component layout as Install_CitiesOnline_v0.9.7, without Cities Online.
;
; Build with trunk/Cities3D/installer/build-windows-installer.ps1
; (NSIS 3). Defines: OUTFILE, LICENSE, README. USERLEVEL and FASTTEST
; are for the script's install check only.

Unicode True
ManifestDPIAware True
ManifestSupportedOS all

!define VERSION "0.11.0"
!define PRODUCT "Cities3D"

!ifndef OUTFILE
  !define OUTFILE "build\Install_${PRODUCT}_v${VERSION}.exe"
!endif
!ifndef LICENSE
  !define LICENSE "build\license.txt"
!endif
!ifndef README
  !define README "build\README.txt"
!endif
!ifndef NOTICES
  !define NOTICES "build\THIRD-PARTY-LICENSES.txt"
!endif

!ifdef FASTTEST
  SetCompressor zlib
!else
  ; Same solid LZMA the 0.9.7 installer used. A 64 MB dictionary
  ; covers the whole payload, which is what keeps the download small.
  SetCompressor /SOLID lzma
  SetCompressorDictSize 64
!endif
SetDatablockOptimize on
CRCCheck on
WindowIcon on
AllowRootDirInstall false

Name "${PRODUCT} ${VERSION}"
OutFile "${OUTFILE}"
BrandingText "${PRODUCT} ${VERSION}"
Caption "${PRODUCT} ${VERSION} Setup"

!ifdef USERLEVEL
  RequestExecutionLevel user
  !define REGROOT HKCU
  InstallDir "$LOCALAPPDATA\${PRODUCT}"
!else
  RequestExecutionLevel admin
  !define REGROOT HKLM
  InstallDir "$PROGRAMFILES64\${PRODUCT}"
!endif
InstallDirRegKey ${REGROOT} "Software\${PRODUCT}" "Install_Dir"

VIProductVersion "0.11.0.0"
VIAddVersionKey "ProductName" "${PRODUCT}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "CompanyName" "${PRODUCT}"
VIAddVersionKey "FileDescription" "${PRODUCT} ${VERSION} Setup"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "LegalCopyright" "Copyright (C) 2001-2009 Jason Fugate. GPL-2.0-or-later."

!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"
!include "WinVer.nsh"
!include "Sections.nsh"
!include "FileFunc.nsh"
!include "nsDialogs.nsh"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "cities3d-header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "cities3d-header.bmp"
!define MUI_ICON "cities3d-inst.ico"
!define MUI_UNICON "cities3d-uninst.ico"
!define MUI_COMPONENTSPAGE_CHECKBITMAP "cities3d-check.bmp"
!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_NODESC

!define MUI_WELCOMEPAGE_TITLE "Welcome to ${PRODUCT} ${VERSION}"
!define MUI_WELCOMEPAGE_TEXT "This setup installs ${PRODUCT} ${VERSION} for local and network play.$\r$\n$\r$\n${PRODUCT} is free software under the GNU General Public License, version 2 or later. Source code is linked from the README copied next to the program.$\r$\n$\r$\n64-bit Windows 10 or later is required.$\r$\n$\r$\nClick Next to continue."

!define MUI_FINISHPAGE_TITLE "Installation complete"
!define MUI_FINISHPAGE_TEXT "${PRODUCT} ${VERSION} has been installed.$\r$\n$\r$\nClick Finish to close this setup."
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${PRODUCT}"
!define MUI_FINISHPAGE_RUN_FUNCTION LaunchCities3D
!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create a desktop shortcut"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION CreateDesktopShortcut

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${LICENSE}"

!define MUI_PAGE_HEADER_TEXT "Components"
!define MUI_PAGE_HEADER_SUBTEXT "Select the components to install."
!insertmacro MUI_PAGE_COMPONENTS

!define MUI_PAGE_HEADER_TEXT "Installation folder"
!define MUI_PAGE_HEADER_SUBTEXT "Choose the folder in which to install ${PRODUCT}."
!insertmacro MUI_PAGE_DIRECTORY

Var STARTMENU_FOLDER
Var user_nat
Var lcid
Var lang_sec
!define MUI_PAGE_HEADER_TEXT "Start Menu folder"
!define MUI_PAGE_HEADER_SUBTEXT "Choose the Start Menu folder for the shortcuts."
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT ${REGROOT}
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${PRODUCT}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenu"
!insertmacro MUI_PAGE_STARTMENU start_menu $STARTMENU_FOLDER

!define MUI_PAGE_HEADER_TEXT "Installing"
!define MUI_PAGE_HEADER_SUBTEXT "Installing ${PRODUCT}..."
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_PAGE_FINISH

!define MUI_PAGE_HEADER_TEXT "Confirm"
!define MUI_PAGE_HEADER_SUBTEXT "Remove ${PRODUCT} from this computer."
!insertmacro MUI_UNPAGE_CONFIRM

!define MUI_PAGE_HEADER_TEXT "Uninstalling"
!define MUI_PAGE_HEADER_SUBTEXT "Removing ${PRODUCT}..."
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; Standard tiles, sounds, and the Standard skin are required, as in 0.9.7.
!define Standard_is_default

!macro add_tile dir tile
  Section "${tile}" tile_${dir}
    !ifdef ${dir}_is_default
      SectionIn RO
    !endif
    SetOutPath "$INSTDIR\tex\${dir}"
    File "..\distribute\tex\${dir}\*.tex"
  SectionEnd
!macroend

!macro add_tile_small dir tile
  !insertmacro add_tile ${dir}_Small "${tile} (small)"
!macroend

!macro add_tile_both dir tile
  !insertmacro add_tile ${dir} ${tile}
  !insertmacro add_tile_small ${dir} ${tile}
!macroend

!macro rm_tile dir
  Delete "$INSTDIR\tex\${dir}\*.tex"
  RMDir "$INSTDIR\tex\${dir}"
!macroend

!macro rm_tile_both dir
  !insertmacro rm_tile ${dir}
  !insertmacro rm_tile ${dir}_Small
!macroend

!macro add_sound dir sound
  Section "${sound}" sound_${dir}
    !ifdef ${dir}_is_default
      SectionIn RO
    !endif
    SetOutPath "$INSTDIR\sounds\${dir}"
    File "..\distribute\sounds\${dir}\*.mp3"
  SectionEnd
!macroend

!macro rm_sound dir
  Delete "$INSTDIR\sounds\${dir}\*.mp3"
  RMDir "$INSTDIR\sounds\${dir}"
!macroend

!macro add_skin dir skin
  Section "${skin}" skin_${dir}
    !ifdef ${dir}_is_default
      SectionIn RO
    !endif
    SetOutPath "$INSTDIR\skins\${dir}"
    File "..\distribute\skins\${dir}\*.*"
    ; gtkrc has no extension. FindFirstFile's *.* still matches it;
    ; this second copy covers a host where that wildcard does not.
    !if /FileExists "..\distribute\skins\${dir}\gtkrc"
      File "..\distribute\skins\${dir}\gtkrc"
    !endif
  SectionEnd
!macroend

!macro rm_skin dir
  Delete "$INSTDIR\skins\${dir}\*.*"
  Delete "$INSTDIR\skins\${dir}\gtkrc"
  RMDir "$INSTDIR\skins\${dir}"
!macroend

!macro add_lang dir lang nat
  Section /o "${lang} (${nat})" lang_${dir}
    SetOutPath "$INSTDIR\locale\${dir}"
    File "..\distribute\locale\${dir}\*.mo"
  SectionEnd
!macroend

!macro rm_lang dir
  Delete "$INSTDIR\locale\${dir}\*.mo"
  RMDir "$INSTDIR\locale\${dir}"
!macroend

Section "${PRODUCT} (required)"
  SectionIn RO

  SetOutPath "$INSTDIR"
  File "..\distribute\Cities3D.exe"
  File "..\distribute\DatabaseCore.dll"
  File "..\distribute\GLCore.dll"
  File "..\distribute\glew32.dll"
  File "..\distribute\jpeg62.dll"
  File "..\distribute\libexpat.dll"
  File "..\distribute\liblzma.dll"
  File "..\distribute\libpng16.dll"
  File "..\distribute\libsharpyuv.dll"
  File "..\distribute\libwebp.dll"
  File "..\distribute\libwebpdecoder.dll"
  File "..\distribute\libwebpdemux.dll"
  File "..\distribute\ModelCore.dll"
  File "..\distribute\msvcp140.dll"
  File "..\distribute\pcre2-16.dll"
  File "..\distribute\RulesCore.dll"
  File "..\distribute\tiff.dll"
  File "..\distribute\UICore.dll"
  File "..\distribute\vcruntime140.dll"
  File "..\distribute\vcruntime140_1.dll"
  File "..\distribute\wxbase333u_net_vc_x64_custom.dll"
  File "..\distribute\wxbase333u_vc_x64_custom.dll"
  File "..\distribute\wxbase333u_xml_vc_x64_custom.dll"
  File "..\distribute\wxmsw333u_aui_vc_x64_custom.dll"
  File "..\distribute\wxmsw333u_core_vc_x64_custom.dll"
  File "..\distribute\wxmsw333u_gl_vc_x64_custom.dll"
  File "..\distribute\wxmsw333u_html_vc_x64_custom.dll"
  File "..\distribute\z.dll"
  File "/oname=COPYING" "${LICENSE}"
  File "/oname=README.txt" "${README}"
  File "/oname=THIRD-PARTY-LICENSES.txt" "${NOTICES}"

  SetOutPath "$INSTDIR\plugins"
  File "..\distribute\plugins\CitiesRules.dll"
  File "..\distribute\plugins\CoreRules.dll"
  File "..\distribute\plugins\NetworkRules.dll"
  File "..\distribute\plugins\SeafarersRules.dll"
  File "..\distribute\plugins\StandardRules.dll"
  File "..\distribute\plugins\VolcanoRules.dll"

  SetOutPath "$INSTDIR\data"
  File "..\distribute\data\maps.db"
  File "..\distribute\data\mapkey.db"

  SetOutPath "$INSTDIR\options"
  File "..\distribute\options\C&K_Basic_Game.xml"
  File "..\distribute\options\C&K_European_Deck.xml"
  File "..\distribute\options\C&K_Saboteur_Deck.xml"
  File "..\distribute\options\Standard_Basic_Game.xml"

  WriteRegStr ${REGROOT} "Software\${PRODUCT}" "Install_Dir" "$INSTDIR"
  WriteRegStr ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayName" "${PRODUCT} ${VERSION}"
  WriteRegStr ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayVersion" "${VERSION}"
  WriteRegStr ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "Publisher" "${PRODUCT}"
  WriteRegStr ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayIcon" "$INSTDIR\Cities3D.exe"
  WriteRegStr ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
  WriteRegStr ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "URLInfoAbout" "https://github.com/skyblaster/Cities3D"
  WriteRegStr ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "HelpLink" "https://github.com/skyblaster/Cities3D"
  WriteRegDWORD ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "NoModify" 1
  WriteRegDWORD ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN start_menu
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT}.lnk" "$INSTDIR\Cities3D.exe" "" "$INSTDIR\Cities3D.exe" 0 "" "" "Play ${PRODUCT}"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall ${PRODUCT}.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0 "" "" "Uninstall ${PRODUCT}"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

SectionGroup "Tilesets"
  !insertmacro add_tile_both Standard Standard
  !insertmacro add_tile_both Impressionist Impressionist
SectionGroupEnd

SectionGroup "Sounds"
  !insertmacro add_sound Standard Standard
SectionGroupEnd

SectionGroup "Skins"
  !insertmacro add_skin Standard Standard
  !insertmacro add_skin Brushed_Metal "Brushed Metal"
SectionGroupEnd

SectionGroup /e "Languages"
  !insertmacro add_lang ca_ES Catalan Spain
  !insertmacro add_lang cs_CZ Czech "Czech Republic"
  !insertmacro add_lang da_DK Danish Denmark
  !insertmacro add_lang de_AT German Austria
  !insertmacro add_lang el_GR Greek Greece
  !insertmacro add_lang en_US English USA
  !insertmacro add_lang es_CO Spanish Colombia
  !insertmacro add_lang fi_FI Finnish Finland
  !insertmacro add_lang fr_FR French France
  !insertmacro add_lang hu_HU Hungarian Hungary
  !insertmacro add_lang is_IS Icelandic Iceland
  !insertmacro add_lang it_IT Italian Italy
  !insertmacro add_lang ja_JP Japanese Japan
  !insertmacro add_lang ko_KR Korean Korea
  !insertmacro add_lang nl_BE Dutch Belgium
  !insertmacro add_lang nl_NL Dutch Netherlands
  !insertmacro add_lang pl_PL Polish Poland
  !insertmacro add_lang pt_PT Portuguese Portugal
  !insertmacro add_lang ru_RU Russian Russia
  !insertmacro add_lang sv_SE Swedish Sweden
  !insertmacro add_lang tr_TR Turkish Turkey
SectionGroupEnd

Section "Uninstall"
  Delete "$INSTDIR\Cities3D.exe"
  Delete "$INSTDIR\DatabaseCore.dll"
  Delete "$INSTDIR\GLCore.dll"
  Delete "$INSTDIR\glew32.dll"
  Delete "$INSTDIR\jpeg62.dll"
  Delete "$INSTDIR\libexpat.dll"
  Delete "$INSTDIR\liblzma.dll"
  Delete "$INSTDIR\libpng16.dll"
  Delete "$INSTDIR\libsharpyuv.dll"
  Delete "$INSTDIR\libwebp.dll"
  Delete "$INSTDIR\libwebpdecoder.dll"
  Delete "$INSTDIR\libwebpdemux.dll"
  Delete "$INSTDIR\ModelCore.dll"
  Delete "$INSTDIR\msvcp140.dll"
  Delete "$INSTDIR\pcre2-16.dll"
  Delete "$INSTDIR\RulesCore.dll"
  Delete "$INSTDIR\tiff.dll"
  Delete "$INSTDIR\UICore.dll"
  Delete "$INSTDIR\vcruntime140.dll"
  Delete "$INSTDIR\vcruntime140_1.dll"
  Delete "$INSTDIR\wxbase333u_net_vc_x64_custom.dll"
  Delete "$INSTDIR\wxbase333u_vc_x64_custom.dll"
  Delete "$INSTDIR\wxbase333u_xml_vc_x64_custom.dll"
  Delete "$INSTDIR\wxmsw333u_aui_vc_x64_custom.dll"
  Delete "$INSTDIR\wxmsw333u_core_vc_x64_custom.dll"
  Delete "$INSTDIR\wxmsw333u_gl_vc_x64_custom.dll"
  Delete "$INSTDIR\wxmsw333u_html_vc_x64_custom.dll"
  Delete "$INSTDIR\z.dll"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\THIRD-PARTY-LICENSES.txt"
  Delete "$INSTDIR\cities3d-last-rule.txt"
  Delete "$INSTDIR\uninstall.exe"

  Delete "$INSTDIR\plugins\CitiesRules.dll"
  Delete "$INSTDIR\plugins\CoreRules.dll"
  Delete "$INSTDIR\plugins\NetworkRules.dll"
  Delete "$INSTDIR\plugins\SeafarersRules.dll"
  Delete "$INSTDIR\plugins\StandardRules.dll"
  Delete "$INSTDIR\plugins\VolcanoRules.dll"
  RMDir "$INSTDIR\plugins"

  Delete "$INSTDIR\data\maps.db"
  Delete "$INSTDIR\data\mapkey.db"

  Delete "$INSTDIR\logs\*.log"
  RMDir "$INSTDIR\logs"

  IfFileExists "$INSTDIR\options\*.xml" 0 rmoptdir
    IfSilent rmoptdir ask_options
    ask_options:
      MessageBox MB_YESNO "Remove user-created option sets?" IDYES option_yes IDNO option_no
      option_yes:
        Delete "$INSTDIR\options\*.xml"
        RMDir "$INSTDIR\options"
        Goto optdone
      option_no:
        Goto optdone
  rmoptdir:
    Delete "$INSTDIR\options\C&K_Basic_Game.xml"
    Delete "$INSTDIR\options\C&K_European_Deck.xml"
    Delete "$INSTDIR\options\C&K_Saboteur_Deck.xml"
    Delete "$INSTDIR\options\Standard_Basic_Game.xml"
    RMDir "$INSTDIR\options"
  optdone:

  !insertmacro rm_tile_both Standard
  !insertmacro rm_tile_both Impressionist
  RMDir "$INSTDIR\tex"

  !insertmacro rm_sound Standard
  RMDir "$INSTDIR\sounds"

  !insertmacro rm_skin Standard
  !insertmacro rm_skin Brushed_Metal
  RMDir "$INSTDIR\skins"

  !insertmacro rm_lang ca_ES
  !insertmacro rm_lang cs_CZ
  !insertmacro rm_lang da_DK
  !insertmacro rm_lang de_AT
  !insertmacro rm_lang el_GR
  !insertmacro rm_lang en_US
  !insertmacro rm_lang es_CO
  !insertmacro rm_lang fi_FI
  !insertmacro rm_lang fr_FR
  !insertmacro rm_lang hu_HU
  !insertmacro rm_lang is_IS
  !insertmacro rm_lang it_IT
  !insertmacro rm_lang ja_JP
  !insertmacro rm_lang ko_KR
  !insertmacro rm_lang nl_BE
  !insertmacro rm_lang nl_NL
  !insertmacro rm_lang pl_PL
  !insertmacro rm_lang pt_PT
  !insertmacro rm_lang ru_RU
  !insertmacro rm_lang sv_SE
  !insertmacro rm_lang tr_TR
  RMDir "$INSTDIR\locale"

  ; Silent uninstall keeps saved games. An attended uninstall asks.
  IfSilent keep_saves ask_saves
  ask_saves:
    MessageBox MB_YESNO "Remove player and game databases?" IDYES delete_saves IDNO keep_saves
  delete_saves:
    Delete "$INSTDIR\data\players.db"
    Delete "$INSTDIR\data\games.db"
    Delete "$INSTDIR\data\gamekey.db"
    Delete "$INSTDIR\data\replay.db"
    Delete "$INSTDIR\data\replaykey.db"
  keep_saves:

  RMDir "$INSTDIR\data"

  Push $R0
  !insertmacro MUI_STARTMENU_GETFOLDER start_menu $R0
  Delete "$SMPROGRAMS\$R0\${PRODUCT}.lnk"
  Delete "$SMPROGRAMS\$R0\Uninstall ${PRODUCT}.lnk"
  RMDir "$SMPROGRAMS\$R0"
  Pop $R0

  Delete "$DESKTOP\${PRODUCT}.lnk"

  DeleteRegKey ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"
  DeleteRegKey ${REGROOT} "Software\${PRODUCT}"

  RMDir "$INSTDIR"
SectionEnd

; LCID to language section. Order matches the 0.9.7 installer.
; English is the fallback, then the system and user locales are added.
Function LCID2Lang
  StrCpy $lang_sec ${lang_en_US}
  StrCmp $lcid "0403" 0 +3
    StrCpy $lang_sec ${lang_ca_ES}
    Goto lcid_done
  StrCmp $lcid "0405" 0 +3
    StrCpy $lang_sec ${lang_cs_CZ}
    Goto lcid_done
  StrCmp $lcid "0406" 0 +3
    StrCpy $lang_sec ${lang_da_DK}
    Goto lcid_done
  StrCmp $lcid "0c07" 0 +3
    StrCpy $lang_sec ${lang_de_AT}
    Goto lcid_done
  StrCmp $lcid "0408" 0 +3
    StrCpy $lang_sec ${lang_el_GR}
    Goto lcid_done
  StrCmp $lcid "240a" 0 +3
    StrCpy $lang_sec ${lang_es_CO}
    Goto lcid_done
  StrCmp $lcid "040b" 0 +3
    StrCpy $lang_sec ${lang_fi_FI}
    Goto lcid_done
  StrCmp $lcid "040c" 0 +3
    StrCpy $lang_sec ${lang_fr_FR}
    Goto lcid_done
  StrCmp $lcid "040e" 0 +3
    StrCpy $lang_sec ${lang_hu_HU}
    Goto lcid_done
  StrCmp $lcid "040f" 0 +3
    StrCpy $lang_sec ${lang_is_IS}
    Goto lcid_done
  StrCmp $lcid "0410" 0 +3
    StrCpy $lang_sec ${lang_it_IT}
    Goto lcid_done
  StrCmp $lcid "0411" 0 +3
    StrCpy $lang_sec ${lang_ja_JP}
    Goto lcid_done
  StrCmp $lcid "0412" 0 +3
    StrCpy $lang_sec ${lang_ko_KR}
    Goto lcid_done
  StrCmp $lcid "0813" 0 +3
    StrCpy $lang_sec ${lang_nl_BE}
    Goto lcid_done
  StrCmp $lcid "0413" 0 +3
    StrCpy $lang_sec ${lang_nl_NL}
    Goto lcid_done
  StrCmp $lcid "0415" 0 +3
    StrCpy $lang_sec ${lang_pl_PL}
    Goto lcid_done
  StrCmp $lcid "0816" 0 +3
    StrCpy $lang_sec ${lang_pt_PT}
    Goto lcid_done
  StrCmp $lcid "0419" 0 +3
    StrCpy $lang_sec ${lang_ru_RU}
    Goto lcid_done
  StrCmp $lcid "041d" 0 +3
    StrCpy $lang_sec ${lang_sv_SE}
    Goto lcid_done
  StrCmp $lcid "041f" 0 +3
    StrCpy $lang_sec ${lang_tr_TR}
    Goto lcid_done
  lcid_done:
FunctionEnd

Function EnsureCities3DClosed
  check_running:
    nsExec::ExecToStack 'cmd /c tasklist /NH /FI "IMAGENAME eq Cities3D.exe" | findstr /I /C:"Cities3D.exe"'
    Pop $0
    Pop $1
    ${If} $0 == "0"
      MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "Cities3D is currently running. Close it before continuing." IDRETRY check_running IDCANCEL give_up
      give_up:
        Abort
    ${EndIf}
FunctionEnd

Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_OK|MB_ICONSTOP "${PRODUCT} ${VERSION} requires 64-bit Windows."
    Abort
  ${EndIf}
  ${IfNot} ${AtLeastWin10}
    MessageBox MB_OK|MB_ICONSTOP "${PRODUCT} ${VERSION} requires Windows 10 or later."
    Abort
  ${EndIf}

  !ifndef USERLEVEL
    SetShellVarContext all
    SetRegView 64
  !endif

  Call EnsureCities3DClosed

  !insertmacro SelectSection ${lang_en_US}
  ReadRegStr $lcid HKLM "SYSTEM\CurrentControlSet\Control\Nls\Language" "Default"
  Call LCID2Lang
  !insertmacro SelectSection $lang_sec
  ReadRegStr $user_nat HKCU "Control Panel\International" "Locale"
  StrCpy $lcid $user_nat 4 -4
  Call LCID2Lang
  !insertmacro SelectSection $lang_sec
FunctionEnd

Function un.onInit
  !ifndef USERLEVEL
    SetShellVarContext all
    SetRegView 64
  !endif
  Call un.EnsureCities3DClosed
FunctionEnd

Function un.EnsureCities3DClosed
  check_running:
    nsExec::ExecToStack 'cmd /c tasklist /NH /FI "IMAGENAME eq Cities3D.exe" | findstr /I /C:"Cities3D.exe"'
    Pop $0
    Pop $1
    ${If} $0 == "0"
      MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "Cities3D is currently running. Close it before continuing." IDRETRY check_running IDCANCEL give_up
      give_up:
        Abort
    ${EndIf}
FunctionEnd

Function .onInstSuccess
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  WriteRegDWORD ${REGROOT} "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "EstimatedSize" $0
FunctionEnd

Function LaunchCities3D
  SetOutPath "$INSTDIR"
  Exec '"$INSTDIR\Cities3D.exe"'
FunctionEnd

Function CreateDesktopShortcut
  CreateShortCut "$DESKTOP\${PRODUCT}.lnk" "$INSTDIR\Cities3D.exe" "" "$INSTDIR\Cities3D.exe" 0 "" "" "Play ${PRODUCT}"
FunctionEnd
