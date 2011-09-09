RequestExecutionLevel admin

!include "version.nsi"
!include "winver.nsh"
!include "x64.nsh"
!include "wordfunc.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
!include "nsDialogs.nsh"
;!include "Image.nsh"

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "�ƽ��"
!define PRODUCT_PUBLISHER "�������ź�ͨ�Ƽ����޹�˾"
!define PRODUCT_WEB_SITE "http://www.finantech.cn"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\MoneyHub.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"


; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "setup.ico"
!define MUI_UNICON "uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "setup.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "setup.bmp"

; Welcome page
!define MUI_WELCOMEPAGE_TITLE "��ӭʹ�á��ƽ�㡱������"
!define MUI_WELCOMEPAGE_TEXT "\r\n��������򵼽�ָ������ɡ��ƽ�㡱���������̡�\r\n\r\n�ڿ�ʼ����֮ǰ�������ȹر���������Ӧ�ó����⽫�����������򡱸���ָ����ϵͳ�ļ���������Ҫ����������ļ������\r\n\r\n����[����]��ʼ�������̡�"
!insertmacro MUI_PAGE_WELCOME
; License page
;!insertmacro MUI_PAGE_LICENSE "license.txt"
; Directory page
;!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!define MUI_PAGE_HEADER_TEXT "��������"
!define MUI_PAGE_HEADER_SUBTEXT "���ƽ�㡱������������Ⱥ�..."
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_TITLE "������ɡ��ƽ�㡱������"
!define MUI_FINISHPAGE_TEXT "���ƽ�㡱��������$_CLICK"
!define MUI_FINISHPAGE_RUN "$INSTDIR\MoneyHub.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "SimpChinese"

; MUI end ------

Name "${PRODUCT_NAME}${PRODUCT_VERSION}"
OutFile "Output\Setup-${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\MoneyHub"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails nevershow
ShowUnInstDetails show
InstallButtonText "����"
Caption "�ƽ�� ����"

BrandingText "���ź�ͨ"

VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "ProductName" "�ƽ��"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "Comments" "�ƽ�㰲װ����"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "CompanyName" "���ź�ͨ"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "LegalCopyright" "���ź�ͨ��Ȩ����"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "FileDescription" "�ƽ�㰲װ����"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "FileVersion" "${PRODUCT_VERSION}"
VIProductVersion "${PRODUCT_VERSION}"

Function .onInit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Only One Instance                                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  BringToFront
  System::Call "kernel32::CreateMutexA(i 0, i 0, t '$(^Name)') i .r0 ?e"
  Pop $0
  StrCmp $0 0 launch
   StrLen $0 "$(^Name)"
   IntOp $0 $0 + 1
  loop:
    FindWindow $1 '#32770' '' 0 $1
    IntCmp $1 0 +5
    System::Call "user32::GetWindowText(i r1, t .r2, i r0) i."
    StrCmp $2 "$(^Name)" 0 loop
    System::Call "user32::ShowWindow(i r1,i 9) i."         ; If minimized then maximize
    System::Call "user32::SetForegroundWindow(i r1) i."    ; Bring to front
    Abort
  launch:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; �����������                                                       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  System::Call "Kernel32::GetSystemDefaultLangID(v ..) i .s"
  Pop $0
  IntOp $0 $0 & 0xFFFF
  
 ${If} $0 != 2052 
   MessageBox MB_ICONSTOP|MB_OK "Moneyhub can only be installed in the Simplified Chinese version of Windows."
   Abort
 ${EndIf}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ������ϵͳ                                                       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
  ${If} ${RunningX64}
    MessageBox MB_ICONSTOP|MB_OK "�ƽ�㲻��������64λ����ϵͳ��"
    Quit
  ${EndIf}

  ${IfNot} ${AtLeastWinXP}
    MessageBox MB_ICONSTOP|MB_OK "�ƽ����ҪWindows XP SP2����°汾�Ĳ���ϵͳ"
    Quit
  ${EndIf}
  ${If} ${IsWinXP}
  ${AndIfNot} ${AtLeastServicePack} 2
    MessageBox MB_ICONSTOP|MB_OK "�ƽ����ҪWindows XP SP2����°汾�Ĳ���ϵͳ"
    Quit  
  ${EndIf} 
  ${If} ${IsWin2003}
    MessageBox MB_ICONSTOP|MB_OK "�ƽ�㲻��������Windows 2003��"
    Quit
  ${EndIf}
  ${If} ${IsWin2008}
    MessageBox MB_ICONSTOP|MB_OK "�ƽ�㲻��������Windows 2008��"
    Quit
  ${EndIf}
  ${If} ${IsWin2008R2}
    MessageBox MB_ICONSTOP|MB_OK "�ƽ�㲻��������Windows 2008 R2��"
    Quit
  ${EndIf}
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ��鱾�ذ汾                                                       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
  Push $R0
  ClearErrors
  ReadRegStr $R0 HKLM "${PRODUCT_DIR_REGKEY}" ""
  IfErrors done

  chkver:
  ReadRegStr $R2 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion"
  ${VersionCompare} "$R2" "${PRODUCT_VERSION}" $1
  ${If} $1 == "1"
    MessageBox MB_YESNO|MB_ICONQUESTION "�Ѿ���װ�ƽ��$R2���Ƿ��������?" IDYES done IDNO nocont
    nocont:
    MessageBox MB_YESNO|MB_ICONQUESTION "���Ҫ�˳�����?" IDNO chkver
    Quit
  ${EndIf}
    
  done:
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; �����Ƿ�����������                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  instchk:
  FindWindow $0 "MONEYHUB_SIGNAL_WND"
  IntCmp $0 1 isexist notexist 
  isexist: 
  MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "��⵽�ƽ���������У������޷�������$\r$\n��رղƽ�㣬�����������ԡ���$\r$\n���ߵ�����ȡ�����˳���װ��" IDRETRY instchk
  Abort
  notexist:    
FunctionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ���Ӳ�̿ռ�
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
!define sysGetDiskFreeSpaceEx 'kernel32::GetDiskFreeSpaceExA(t, *l, *l, *l) i'
Function FreeDiskSpace
	System::Call '${sysGetDiskFreeSpaceEx}(r0,.,,.r1)'
	System::Int64Op $1 / 1024
	Pop $1
FunctionEnd

Section "MainSection"
	SetDetailsView hide
	;���Ӳ�̿ռ�
	StrCpy $0 'C:\'
	Call FreeDiskSpace
	StrCpy $2 100000
	System::Int64Op $1 > $2
	Pop $3
	
	IntCmp $3 1 okay
	MessageBox MB_OK "���̿ռ䲻�㣬�ƽ����Ҫ����100MBӲ�̿ռ䣬���ͷſռ�����ԡ�"
	Quit
okay:
	SetOutPath "$INSTDIR"

	;��鵱ǰϵͳ���Ƿ��вƽ����������  
	StrCpy $0 "MoneyHub.exe"
	KillProc::FindProcesses
	StrCmp $0 "0" completed1
killagain1:
	Sleep 1500
	StrCpy $0 "MoneyHub.exe"
	KillProc::KillProcesses
	StrCmp $1 "0" completed1
	Goto killagain1
completed1:
	StrCpy $0 "MoneyHub_Updater.exe"
	KillProc::FindProcesses
	StrCmp $0 "0" completed2
killagain2:
	Sleep 1500
	StrCpy $0 "MoneyHub_Updater.exe"
	KillProc::KillProcesses
	StrCmp $1 "0" completed2
	Goto killagain2
completed2:
	StrCpy $0 "moneyhub_pop.exe"
	KillProc::FindProcesses
	StrCmp $0 "0" completed3
killagain3:
	Sleep 1500
	StrCpy $0 "moneyhub_pop.exe"
	KillProc::KillProcesses
	StrCmp $1 "0" completed3
	Goto killagain3
completed3:
	;��ʼ����
	SetDetailsPrint none
	ExecWait '"$INSTDIR\MoneyHub.exe" -d'
	DeleteRegKey HKCU "Software\Bank"

	;ɾ�����¹������пؼ�
	;RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\alipay"
	;RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\cmbchina"
	;RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\icbc"
	;RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\bjrcb"
	;RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\ecitic"

	File /r /x .svn /x *.lib /x *.exp /x *.pdb /x *.ilk /x *.map /x BankActiveXTest.html /x BankLoaderTester.exe /x DigsigGen.exe /x ModuleVerifierTester.exe /x usbkeyinfo.xml /x VerifyList.xml /x info.xml /x CloudCheck.ini /x syslist.txt /x BlackList.txt /x BillUrl.xml ..\_Bin\Debug\*.*
	;��license.dll��pecsp.dll�ŵ�system32��
	;SetOutPath "$WINDIR\System32"
	;File /r /x .svn ..\_Bin\Debug\BankInfo\banks\cmbc\license.dll
	;File /r /x .svn ..\_Bin\Debug\BankInfo\banks\sdb\pecsp.dll
	;SetOutPath "$INSTDIR"

	ExecWait '"$INSTDIR\MoneyHub.exe" -i' $0
	${If} $0 != 0
		DetailPrint "��װ����ʧ�ܣ�"
		MessageBox MB_OK|MB_ICONSTOP "��װ����ʧ�ܣ����˳�����װ��"
		Call DoUninstall
		
		CreateDirectory "$APPDATA\MoneyHub"
		FileOpen $0 "$APPDATA\MoneyHub\Update.log" a
		FileSeek $0 0 END
		${GetTime} "" "L" $1 $2 $3 $4 $5 $6 $7
		FileWrite $0 '[$3-$2-$1 $5:$6:$7] : ��װ �ƽ��${PRODUCT_VERSION} ʧ��'
		FileWrite $0 '$\r$\n'
		FileClose $0
		
		Quit
	${EndIf}

	ClearErrors
SectionEnd

Section -Post
	Exec '"$INSTDIR\moneyhub_svc.exe"'
	WriteUninstaller "$INSTDIR\MoneyHub_Uninst.exe"
	WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\MoneyHub.exe"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\MoneyHub_Uninst.exe"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\MoneyHub.exe"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
	
	Delete "$DESKTOP\�����ƽ��.lnk"
	DeleteRegKey HKCU "Software\Bank\Update"
	
	CreateDirectory "$APPDATA\MoneyHub"
	FileOpen $0 "$APPDATA\MoneyHub\Update.log" a
	FileSeek $0 0 END
	${GetTime} "" "L" $1 $2 $3 $4 $5 $6 $7
	FileWrite $0 '[$3-$2-$1 $5:$6:$7] : ���� �ƽ��${PRODUCT_VERSION}'
	FileWrite $0 '$\r$\n'
	FileClose $0
SectionEnd
