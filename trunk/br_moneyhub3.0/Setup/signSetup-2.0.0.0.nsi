RequestExecutionLevel admin

!include "version.nsi"
!include "winver.nsh"
!include "x64.nsh"
!include "wordfunc.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
!include "nsDialogs.nsh"

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "�ƽ��"
!define PRODUCT_PUBLISHER "�������ź�ͨ�Ƽ����޹�˾"
!define PRODUCT_WEB_SITE "http://www.finantech.cn"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\MoneyHub.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define PWD_DIR "."
!define BUILDTOOL "build_tool" 

!ifdef INNER
	;;=================================================================================
	;;===== INNER =====================================================================
	;;=================================================================================
	
	; MUI 1.67 compatible ------
	!include "MUI.nsh"
	
	!define MUI_ABORTWARNING
	!define MUI_ICON "setup.ico"
	!define MUI_UNICON "uninstall.ico"
	!define MUI_WELCOMEFINISHPAGE_BITMAP "setup.bmp"
	!define MUI_UNWELCOMEFINISHPAGE_BITMAP "setup.bmp"
	
	!insertmacro MUI_UNPAGE_INSTFILES
	!insertmacro MUI_LANGUAGE "SimpChinese"
	
	; MUI end ------
	
	Name "${PRODUCT_NAME}${PRODUCT_VERSION}"
	OutFile "$%TEMP%\tempinstaller.exe"
	SetCompress off
	ShowUnInstDetails show
	
	SilentInstall silent
	
	BrandingText "���ź�ͨ"
	
	VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "ProductName" "�ƽ��"
	VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "Comments" "�ƽ��ж�س���"
	VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "CompanyName" "���ź�ͨ"
	VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "LegalCopyright" "���ź�ͨ��Ȩ����"
	VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "FileDescription" "�ƽ��ж�س���"
	VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "FileVersion" "${PRODUCT_VERSION}"
	VIProductVersion "${PRODUCT_VERSION}"
	
	Function .onInit
	  WriteUninstaller "$%TEMP%\MoneyHub_Uninst.exe"
	FunctionEnd
	
	Section "MainSection" SEC01
	SectionEnd
	
	Function un.onUninstSuccess
		HideWindow
		MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) �ѳɹ��ش���ļ�����Ƴ���"
		
		ClearErrors
		FileOpen $0 $TEMP\A9BD62759DBE4df1B7F7F619F99F17FB r
		IfErrors done
		FileRead $0 $1
		FileClose $0
		ExecShell "open" $1
	done:
		Delete $TEMP\A9BD62759DBE4df1B7F7F619F99F17FB  
	FunctionEnd
	
	Function un.onInit
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
		;StrCmp $2 "$(^Name)" 0 loop
		System::Call "user32::ShowWindow(i r1,i 9) i."         ; If minimized then maximize
		System::Call "user32::SetForegroundWindow(i r1) i."    ; Bring to front
		Abort
	launch:
		
		MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "��ȷʵҪ��ȫ�Ƴ� $(^Name) ���������е������" IDYES +2
		Abort
		
	instchk:
		FindWindow $0 "MONEYHUB_SIGNAL_WND"
		IntCmp $0 1 isexist notexist 
	isexist: 
		MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "��⵽�ƽ���������У�ж���޷�������$\r$\n��رղƽ�㣬�����������ԡ���$\r$\n���ߵ�����ȡ�����˳�ж�ء�" IDRETRY instchk
		Abort
	notexist:
	FunctionEnd
	
	Section Uninstall 
		UnRegDLL "$INSTDIR\bankactivex.dll"
	
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
		StrCpy $0 "moneyhub_svc.exe"
		KillProc::FindProcesses
		StrCmp $0 "0" completed3
	killagain3:
		Sleep 1500
		StrCpy $0 "moneyhub_svc.exe"
		KillProc::KillProcesses
		StrCmp $1 "0" completed3
		Goto killagain3
	completed3:
	
	
	    StrCpy $0 "moneyhub_pop.exe"
		KillProc::FindProcesses
		StrCmp $0 "0" completed4
	killagain4:
		Sleep 1500
		StrCpy $0 "moneyhub_pop.exe"
		KillProc::KillProcesses
		StrCmp $1 "0" completed4
		Goto killagain4
	completed4:
	
	     ;ж������
		 Sleep 2000
		 ExecWait '"$INSTDIR\MoneyHub.exe" -d'
		;ɾ�������û��µ�����ͼ��
		SetShellVarContext all
		Delete "$DESKTOP\�ƽ��.lnk"
		Delete "$STARTMENU\�ƽ��.lnk"
		Delete "$QUICKLAUNCH\�ƽ��.lnk"
		RMDir /r "$SMPROGRAMS\�ƽ��"
	
		;ɾ����ǰ�û��µ�����ͼ��
		SetShellVarContext current
		Delete "$DESKTOP\�ƽ��.lnk"
		Delete "$STARTMENU\�ƽ��.lnk"
		Delete "$QUICKLAUNCH\�ƽ��.lnk"
		RMDir /r "$SMPROGRAMS\�ƽ��"
	
		RMDir /r "$TEMP\BankTemp"
		RMDir /r "$TEMP\BankUpdate"
		;RMDir /r "$INSTDIR"
		Delete "$INSTDIR\*.*"
		RMDir /r "$INSTDIR\Config"
		RMDir /r "$INSTDIR\Html"
		RMDir /r "$INSTDIR\Skin"
		RMDir /r "$INSTDIR\UILogos"
		
		DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "MoneyHub"
		DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "MoneyhubAgent"
		DeleteRegKey HKCU "Software\Bank"
		DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
		DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
		
		CreateDirectory "$APPDATA\MoneyHub"
		FileOpen $0 "$APPDATA\MoneyHub\Update.log" a
		FileSeek $0 0 END
		${GetTime} "" "L" $1 $2 $3 $4 $5 $6 $7
		FileWrite $0 '[$3-$2-$1 $5:$6:$7] : ж�� �ƽ��${PRODUCT_VERSION}'
		FileWrite $0 '$\r$\n'
		FileClose $0
		
		MessageBox MB_YESNO "�Ƿ����û�����?" IDYES true IDNO false
	false:
		RMDir /r "$APPDATA\MoneyHub"
		RMDir /r "$INSTDIR"
	true:
		SetAutoClose true
	SectionEnd
	;;=================================================================================
!else 
	;;=================================================================================
	;;===== OUTER =====================================================================
	;;=================================================================================
	
	; MUI 1.67 compatible ------
	!include "MUI.nsh"
	
	; MUI Settings
	!define MUI_ABORTWARNING
	!define MUI_ICON "setup.ico"
	!define MUI_UNICON "setup.ico"
	!define MUI_WELCOMEFINISHPAGE_BITMAP "setup.bmp"
	!define MUI_UNWELCOMEFINISHPAGE_BITMAP "setup.bmp"
	
	; Welcome page
	!insertmacro MUI_PAGE_WELCOME
	; License page
	;!define MUI_LICENSEPAGE_RADIOBUTTONS
	;!define MUI_LICENSEPAGE_RADIOBUTTONS_TEXT_ACCEPT "����Э���е�����"
	;!define MUI_LICENSEPAGE_RADIOBUTTONS_TEXT_DECLINE "������Э���е�����"
	!insertmacro MUI_PAGE_LICENSE "license.rtf"
	; Directory page
	;!insertmacro MUI_PAGE_DIRECTORY
	; Instfiles page
	!insertmacro MUI_PAGE_INSTFILES
	; Finish page
	!define MUI_FINISHPAGE_RUN "$INSTDIR\MoneyHub.exe"
	!insertmacro MUI_PAGE_FINISH
	
	; Uninstaller pages
	!insertmacro MUI_UNPAGE_INSTFILES
	
	; Language files
	!insertmacro MUI_LANGUAGE "SimpChinese"
	
	; MUI end ------
	
	!echo "Outer invocation"
	!system "$\"${NSISDIR}\makensis$\" /DINNER signSetup-2.0.0.0.nsi" = 0
	!system "$%TEMP%\tempinstaller.exe"
	!system '"${BUILDTOOL}\Signtool.exe" sign /f "${BUILDTOOL}\test.pfx" /p "1" /t "http://timestamp.verisign.com/scripts/timstamp.dll" /d "�ƽ��ͻ���" /du "www.finantech.cn" /v $%TEMP%\MoneyHub_Uninst.exe"' = 0            
	!system "copy $%TEMP%\MoneyHub_Uninst.exe   .\bak\MoneyHub_Uninst.exe"            
	            
	!system "${BUILDTOOL}\MakeCHK_release.bat"  
	
	Name "${PRODUCT_NAME}${PRODUCT_VERSION}"
	OutFile ".\sign\Setup-${PRODUCT_VERSION}.exe"
	InstallDir "$PROGRAMFILES\MoneyHub"
	InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
	ShowInstDetails nevershow
	ShowUnInstDetails show
	
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
		
	  ;${If} ${RunningX64}
	   ; MessageBox MB_ICONSTOP|MB_OK "�ƽ�㲻��������64λ����ϵͳ��"
	    ;Quit
	  ;${EndIf}
	  
	  ${If} ${RunningX64}
	  ${AndIf} ${AtMostWinVista}
	  ${AndIfNot} ${AtLeastServicePack} 1
	     MessageBox MB_ICONSTOP|MB_OK "�ƽ����ҪVISTA 64bit SP1����°汾�Ĳ���ϵͳ"
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
	    MessageBox MB_YESNO|MB_ICONQUESTION "�Ѿ���װ�ƽ��$R2���Ƿ������װ?" IDYES done IDNO nocont
	    nocont:
	    MessageBox MB_YESNO|MB_ICONQUESTION "���Ҫ�˳���װ?" IDNO chkver
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
	  MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "��⵽�ƽ���������У���װ�޷�������$\r$\n��رղƽ�㣬�����������ԡ���$\r$\n���ߵ�����ȡ�����˳���װ��" IDRETRY instchk
	  Abort
	  notexist:
	FunctionEnd
	
	!define sysGetDiskFreeSpaceEx 'kernel32::GetDiskFreeSpaceExA(t, *l, *l, *l) i'
	Function FreeDiskSpace
		System::Call '${sysGetDiskFreeSpaceEx}(r0,.,,.r1)'
		System::Int64Op $1 / 1024
		Pop $1
	FunctionEnd
	
	Section "MainSection" SEC01
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
		StrCpy $0 "moneyhub_svc.exe"
		KillProc::FindProcesses
		StrCmp $0 "0" completed3
	killagain3:
		Sleep 1500
		StrCpy $0 "moneyhub_svc.exe"
		KillProc::KillProcesses
		StrCmp $1 "0" completed3
		Goto killagain3
	completed3:
	
	
	    StrCpy $0 "moneyhub_pop.exe"
		KillProc::FindProcesses
		StrCmp $0 "0" completed4
	killagain4:
		Sleep 1500
		StrCpy $0 "moneyhub_pop.exe"
		KillProc::KillProcesses
		StrCmp $1 "0" completed4
		Goto killagain4
	completed4:
	
		;��ʼ��װ
		SetDetailsPrint none
		UnRegDLL "$INSTDIR\bankactivex.dll"
		ExecWait '"$INSTDIR\MoneyHub.exe" -d'
		Sleep 2000
		DeleteRegKey HKCU "Software\Bank"
		;ɾ�����¹������пؼ�
		RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\cs_ecitic"
		RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\chinalife"
		RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\letao"
		RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\taobao"
		;RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks\ecitic"
		;RMDir /r "$PROGRAMFILES\MoneyHub\BankInfo\banks"
		;RMDir /r "$APPDATA\MoneyHub\Data"
		Delete "$PROGRAMFILES\MoneyHub\moneyhub_pop.exe"
		
		;remove syslist.txt & info.xml
		File /r /x .svn /x *.lib /x *.exp /x *.pdb /x *.ilk /x *.map /x BankActiveXTest.html /x BankLoaderTester.exe /x DigsigGen.exe /x ModuleVerifierTester.exe /x usbkeyinfo.xml /x VerifyList.xml /x CloudCheck.ini /x syslist.txt /x info.xml /x BlackList.txt /x BillUrl.xml ${PWD_DIR}\bak\*.*

		;����Ȩ�ޣ������κ��˶�д���Ա�֤����Ȩ
		AccessControl::GrantOnFile "$INSTDIR" "(BU)" "FullAccess"
		
		;����appdata\bankconfig
		;CreateDirectory "$APPDATA\bankconfig"
		;CopyFiles $INSTDIR\BankInfo\banks $APPDATA\bankconfig

		;���Ż�ȯҳ����Ҫ�ļ���ͼƬ�ŵ�APPDATA��
		;SetShellVarContext all
		;CreateDirectory "$APPDATA\MoneyHub\Coupons\images"
		;SetOutPath "$APPDATA\MoneyHub\Coupons\images"
		;File /r /x .svn ${PWD_DIR}\bak\Html\CouponPage\images\*.*
		;��license.dll��pecsp.dll�ŵ�system32��
		;SetOutPath "$WINDIR\System32"
		;File /r /x .svn ${PWD_DIR}\bak\BankInfo\banks\cmbc\license.dll
		;File /r /x .svn ${PWD_DIR}\bak\BankInfo\banks\sdb\pecsp.dll
		;SetOutPath "$INSTDIR"
		
		RegDLL "$INSTDIR\bankactivex.dll"
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
		
		CreateDirectory "$SMPROGRAMS\�ƽ��"
		CreateShortCut "$SMPROGRAMS\�ƽ��\�ƽ��.lnk" "$INSTDIR\MoneyHub.exe" "" "" "" "" "" "MoneyHub"
		CreateShortCut "$DESKTOP\�ƽ��.lnk" "$INSTDIR\MoneyHub.exe" "" "" "" "" "" "MoneyHub"
		CreateShortCut "$STARTMENU\�ƽ��.lnk" "$INSTDIR\MoneyHub.exe" "" "" "" "" "" "MoneyHub"
		CreateShortCut "$QUICKLAUNCH\�ƽ��.lnk" "$INSTDIR\MoneyHub.exe" "" "" "" "" "" "MoneyHub"
		CreateShortCut "$SMPROGRAMS\�ƽ��\ж�زƽ��.lnk" "$INSTDIR\MoneyHub_Uninst.exe" "" "" "" "" "" "�ƽ��ж�س���"
		
		;����ʱִ����
		DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "MoneyHub"
		DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "MoneyhubAgent"
		WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "MoneyhubAgent" "$INSTDIR\moneyhub_svc.exe"
	
		ClearErrors
	SectionEnd
	
	Section "��ȫ���" Security_Section_ID
		!insertmacro MUI_HEADER_TEXT "��ȫ���" "���ڽ��а�ȫ��⣬���Ժ�..."
		DetailPrint "���ڽ��а�ȫ��⣬�˹��̿���Ҫ���������ӣ����Ժ�..."
		SetDetailsPrint none
		ExecWait '"$INSTDIR\MoneyHub.exe" -ncheck' $0
		
		${If} ${Errors}
			${OrIf} $0 != 0
			DetailPrint "��ȫ���ʧ�ܣ�"
			MessageBox MB_OK|MB_ICONSTOP "��װʧ�ܣ����˳�����װ"
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
		DetailPrint "��ȫ���ɹ�"
	SectionEnd
	
	Section -Post
		Exec '"$INSTDIR\moneyhub_svc.exe"'
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
		FileWrite $0 '[$3-$2-$1 $5:$6:$7] : ��װ �ƽ��${PRODUCT_VERSION} �ɹ�'
		FileWrite $0 '$\r$\n'
		FileClose $0
	SectionEnd
	
	Function DoUninstall
		!insertmacro MUI_HEADER_TEXT "ж��" "����ж�أ����Ժ�..."
		DetailPrint "����ж�أ����Ժ�..."
		Sleep 1000
		UnRegDLL "$INSTDIR\bankactivex.dll"
		
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
		StrCpy $0 "moneyhub_svc.exe"
		KillProc::FindProcesses
		StrCmp $0 "0" completed3
	killagain3:
		Sleep 1500
		StrCpy $0 "moneyhub_svc.exe"
		KillProc::KillProcesses
		StrCmp $1 "0" completed3
		Goto killagain3
	completed3:
	
	
	    StrCpy $0 "moneyhub_pop.exe"
		KillProc::FindProcesses
		StrCmp $0 "0" completed4
	killagain4:
		Sleep 1500
		StrCpy $0 "moneyhub_pop.exe"
		KillProc::KillProcesses
		StrCmp $1 "0" completed4
		Goto killagain4
	completed4:
	
	    ;ж������
		Sleep 2000
		ExecWait '"$INSTDIR\MoneyHub.exe" -d'
	 
		;ɾ�������û��µ�����ͼ��
		SetShellVarContext all
		Delete "$DESKTOP\�ƽ��.lnk"
		Delete "$STARTMENU\�ƽ��.lnk"
		Delete "$QUICKLAUNCH\�ƽ��.lnk"
		RMDir /r "$SMPROGRAMS\�ƽ��"
	
		;ɾ����ǰ�û��µ�����ͼ��
		SetShellVarContext current
		Delete "$DESKTOP\�ƽ��.lnk"
		Delete "$STARTMENU\�ƽ��.lnk"
		Delete "$QUICKLAUNCH\�ƽ��.lnk"
		RMDir /r "$SMPROGRAMS\�ƽ��"
	
		RMDir /r "$TEMP\BankTemp"
		RMDir /r "$TEMP\BankUpdate"
		RMDir /r "$INSTDIR"
		
		DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "MoneyHub"
		DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "MoneyhubAgent"
		DeleteRegKey HKCU "Software\Bank"
		DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
		DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
		SetDetailsPrint none
		SetAutoClose true
	FunctionEnd
	;;=================================================================================
!endif
