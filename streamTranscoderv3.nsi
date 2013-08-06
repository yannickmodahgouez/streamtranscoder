Function .onInit
  SetOutPath $TEMP
  File /oname=spltmp.bmp "oddsock_logo.bmp"
  splash::show 2500 $TEMP\spltmp
  Delete $TEMP\spltmp.bmp
FunctionEnd
;
; This script will generate an installer that installs a Winamp plug-in.
; It also puts a license page on, for shits and giggles.
;
; This installer will automatically alert the user that installation was
; successful, and ask them whether or not they would like to make the 
; plug-in the default and run Winamp.
;
; The name of the installer
Name "Stream Transcoder V3"

; The file to write
OutFile "streamTranscoderv3_3.1.11.exe"


LicenseText "StreamTranscoderV3 is released under the GNU Public License"
LicenseData COPYING

UninstallText "This will uninstall StreamTranscoderV3. Hit next to continue."

; The default installation directory
InstallDir $PROGRAMFILES\StreamTranscoderV3

; The text to prompt the user to enter a directory
DirText "Please select your install path below :"
; DirShow hide

; automatically close the installer when done.
; AutoCloseWindow true
; hide the "show details" box
ShowInstDetails show

ComponentText "This will install Stream Transcoder V3"

; The stuff to install
Section "Stream Transcoder V3"

  SectionIn 1
  SetOutPath $INSTDIR

  File "Release\streamTranscoderUIv3.exe"
  File "Release\streamTranscoderv3Service.exe"
  File "..\external\lib\pthreadVSE.dll"

  WriteUninstaller "streamTranscoderv3-uninst.exe"
  ; prompt user, and if they select no, skip the following 3 instructions.
SectionEnd
Section "Create Desktop Shortcut"
  SectionIn 1
  CreateShortCut "$DESKTOP\StreamTranscoderV3.lnk" "$INSTDIR\streamTranscoderUIv3.exe" ""
SectionEnd


Section "OggFLAC DLLs"
SectionIn 1 2
SetOutPath $INSTDIR
File "..\external\lib\libOggFLAC.dll"
File "..\external\lib\libFLAC.dll"
SectionEnd
Section "Vorbis 1.1 DLLs"
SectionIn 1 3
SetOutPath $INSTDIR
File "..\external\lib\vorbis_1.1.0_dlls\ogg.dll"
File "..\external\lib\vorbis_1.1.0_dlls\vorbis.dll"
SectionEnd
Section /o "Vorbis 1.1 aoTuVb4.51 DLLs - Specially tuned dlls"
SectionIn 1 3
SetOutPath $INSTDIR
File "..\external\lib\vorbis_aotuVb451_dlls\ogg.dll"
File "..\external\lib\vorbis_aotuVb451_dlls\vorbis.dll"
SectionEnd
Section /o "LAME encoder dll (for MP3 encoding)"
SectionIn 1 5
MessageBox MB_YESNO|MB_ICONQUESTION \
             "In order to broadcast in MP3, you must download the lame encoder dll (lame_enc.dll).  Due to legal issues, this cannot be distributed by me.  A browser will now open to a location that has been known to have copies. Place the dll 'lame_enc.dll' in the main Winamp program directory." \
             IDNO NoLAME
ExecShell open http://www.rarewares.org/mp3.html
NoLAME:
SectionEnd
Section /o "AAC encoder dll (for AAC encoding)"
SectionIn 1 6
MessageBox MB_YESNO|MB_ICONQUESTION \
             "In order to broadcast in AAC, you must download the FAAC encoder dll (libfaac.dll).  Due to legal issues, this cannot be distributed by me.  A browser will now open to a location that has been known to have copies. Place the dll 'libfacc.dll' in the main Winamp program directory." \
             IDNO NoAAC
ExecShell open http://www.rarewares.org/aac.html
NoAAC:
SectionEnd
Section /o "Install As a Service"
SectionIn 1 7
ExecWait '"$INSTDIR\streamTranscoderv3Service.exe" "install" "$INSTDIR"'
SectionEnd



; special uninstall section.
Section "uninstall"
ExecWait '"$INSTDIR\streamTranscoderv3Service.exe" "remove"'
Delete "$INSTDIR\streamTranscoderUIv3.exe"
Delete "$INSTDIR\streamTranscoderv3Service.exe"
Delete "$INSTDIR\libOggFLAC.dll"
Delete "$INSTDIR\libFLAC.dll"
Delete "$INSTDIR\ogg.dll"
Delete "$INSTDIR\vorbis.dll"
Delete "$INSTDIR\pthreadVSE.dll"

MessageBox MB_OK "StreamTranscoderV3 uninstalled." IDOK 0 ; skipped if file doesn't exist
SectionEnd
; eof
