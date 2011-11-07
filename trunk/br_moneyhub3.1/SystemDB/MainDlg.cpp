#include<stdafx.h>
//#include"MainDlg.h"

//
//LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	{
//		// center the dialog on the screen
//		CenterWindow();
//
//		// set icons
//		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
//			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
//		SetIcon(hIcon, TRUE);
//		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
//			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
//		SetIcon(hIconSmall, FALSE);
//
//		// register object for message filtering and idle updates
//		CMessageLoop* pLoop = _Module.GetMessageLoop();
//		ATLASSERT(pLoop != NULL);
//		pLoop->AddMessageFilter(this);
//		pLoop->AddIdleHandler(this);
//
//		UIAddChildWindowContainer(m_hWnd);
//
//		return TRUE;
//	}
//
//	LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	{
//		// unregister message filtering and idle updates
//		CMessageLoop* pLoop = _Module.GetMessageLoop();
//		ATLASSERT(pLoop != NULL);
//		pLoop->RemoveMessageFilter(this);
//		pLoop->RemoveIdleHandler(this);
//
//		return 0;
//	}
//
//	LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	{
//		CAboutDlg dlg;
//		dlg.DoModal();
//		return 0;
//	}
//
//	LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	{
//		// TODO: Add validation code 
//		CloseDialog(wID);
//		return 0;
//	}
//
//	LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	{
//		CloseDialog(wID);
//		return 0;
//	}
//
//	void CMainDlg::CloseDialog(int nVal)
//	{
//		DestroyWindow();
//		::PostQuitMessage(nVal);
//	}
//	bool CMainDlg::CreateSystemTables(LPSTR lpStrPath)
//{
//	ATLASSERT (NULL != lpStrPath);
//	if (NULL == lpStrPath)
//		return false;
//
//	HANDLE hFile = CreateFileA(lpStrPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
//	if (INVALID_HANDLE_VALUE == hFile)
//		return false;
//
//	CloseHandle(hFile);
//
//	CppSQLite3DB db;
//	db.open(lpStrPath);
//
//	db.execDML("CREATE  TABLE IF NOT EXISTS `sysAppType` (\
//			   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
//			   `Name` VARCHAR(256) NOT NULL);");
//
//	db.execDML("CREATE  TABLE IF NOT EXISTS `sysBank` (\
//			   `id` VARCHAR(4) NOT NULL PRIMARY KEY,\
//			   `Name` VARCHAR(256) UNIQUE NOT NULL,\
//			   `classId` INTEGER NOT NULL DEFAULT(99),\
//			   `ShortName` VARCHAR(256) UNIQUE NOT NULL,\
//			   `Position` INTEGER NOT NULL,\
//			   `sysAppType_id` INTEGER NOT NULL);");
//
//	db.execDML("CREATE  TABLE IF NOT EXISTS `sysSubLink` (\
//			   `sysBank_id` VARCHAR(4) NOT NULL  ,\
//			   `URL` VARCHAR(256) UNIQUE NOT NULL,\
//			   `URLText` VARCHAR(256) NOT NULL,\
//			   `LinkOrder` INTEGER NOT NULL,\
//			   PRIMARY KEY(sysBank_id, URL));");
//
//	db.execDML("CREATE  TABLE IF NOT EXISTS `sysCategory1` (\
//			   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
//			   `Name` VARCHAR(256) NOT NULL,\
//			   `Type` TINYINTNOT NULL);");
//
//	db.execDML("CREATE  TABLE IF NOT EXISTS `sysCategory2` (\
//			   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
//			   `Name` VARCHAR(256) NOT NULL,\
//			   `sysCategory1_id` TINYINTNOT NULL);");
//
//	db.execDML("CREATE  TABLE IF NOT EXISTS `sysAccountType` (\
//			   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
//			   `Name` VARCHAR(256) NOT NULL);");
//
//	db.execDML("CREATE  TABLE IF NOT EXISTS `sysCurrency` (\
//			   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
//			   `Name` VARCHAR(256) NOT NULL);");
//
//	db.execDML("CREATE  TABLE IF NOT EXISTS `sysDBInfo` (\
//			   `schema_version` INTEGER NOT NULL);");
//
//	db.execDML("CREATE  TABLE IF NOT EXISTS `sysURLWhiteList` (\
//			   `sysBank_id` VARCHAR(4) NOT NULL PRIMARY KEY ,\
//			   `URLPattern` VARCHAR(256) NOT NULL);");
//
//	
//	db.execDML("INSERT INTO sysAppType (id, Name) VALUES (1, \"all\");");
//	db.execDML("INSERT INTO sysAppType (id, Name) VALUES (2, \"banks\");");
//	db.execDML("INSERT INTO sysAppType (id, Name) VALUES (3, \"securities\");");
//	db.execDML("INSERT INTO sysAppType (id, Name) VALUES (4, \"insurances\");");
//	db.execDML("INSERT INTO sysAppType (id, Name) VALUES (5, \"funds\");");
//	db.execDML("INSERT INTO sysAppType (id, Name) VALUES (6, \"payments\");");
//	db.execDML("INSERT INTO sysAppType (id, Name) VALUES (7, \"shopping\");");
//
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a001\", \"�й�����\", 0, \"boc\", 0, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a002\", \"�й�ũҵ����\", 0, \"abchina\", 1, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a003\", \"�й���������\", 0, \"icbc\", 2, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a004\", \"�й���������\", 0, \"ccb\", 3, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a005\", \"��ͨ����\", 0, \"bankcomm\", 4, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a006\", \"��������\", 0, \"cmbchina\", 5, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a007\", \"��������\", 0, \"ecitic\", 6, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a008\", \"�������\", 0, \"cebbank\", 7, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a009\", \"�ַ�����\", 0, \"spdb\", 8, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a010\", \"���ڷ�չ����\", 0, \"sdb\", 9, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a011\", \"�㶫��չ����\", 0, \"gdb\", 10, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a012\", \"��������\", 0, \"hxb\", 11, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a013\", \"��������\", 0, \"cmbc\", 12, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a014\", \"��ҵ����\", 0, \"cib\", 13, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a015\", \"��������\", 0, \"beijing\", 14, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a016\", \"����ũ������\", 0, \"bjrcb\", 15, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a017\", \"�й�������������\", 0, \"psbc\", 16, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a018\", \"ƽ������\", 0, \"pinganbank\", 17, 2);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"b001\", \"����֤ȯ\", 1, \"cs_ecitic\", 0, 3);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"c001\", \"�й����ٱ���\", 1, \"chinalife\", 0, 4);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d001\", \"���Ļ���\", 1, \"chinaamc\", 0, 5);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d002\", \"���̻���\", 1, \"cmfchina\", 1, 5);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e001\", \"֧����\", 2, \"alipay\", 0, 6);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e002\", \"������֧��\", 2, \"payease\", 1, 6);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e003\", \"����\", 2, \"lianxin\", 2, 6);");
//	db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e004\", \"�ױ�֧��\", 2, \"yeepay\", 3, 6);");
//	db.execDML("INSERT INTO sysBank (id, Name, ShortName, Position, sysAppType_id) VALUES (\"f001\", \"�����̳�\", \"letao\", 0, 7);");
//	db.execDML("INSERT INTO sysBank (id, Name, ShortName, Position, sysAppType_id) VALUES (\"f002\", \"�Ա���\", \"taobao\", 1, 7);");
//
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a001\", \"http://www.boc.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a001\", \"https://ebs.boc.cn/BocnetClient/LoginFrame.do?_locale=zh_CN\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a002\", \"http://www.abchina.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a002\", \"http://www.abchina.com/cn/EBanking/Ebanklogin/PCustomerLogin/default.htm\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a003\", \"http://www.icbc.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a003\", \"https://mybank.icbc.com.cn/icbc/perbank/index.jsp\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a003\", \"https://vip.icbc.com.cn/\", \"�����\", 2);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a004\", \"http://www.ccb.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a004\", \"https://ibsbjstar.ccb.com.cn/app/V5/CN/STY1/login.jsp\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a005\", \"http://www.bankcomm.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a005\", \"https://pbank.95559.com.cn/personbank/index.jsp\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a006\", \"http://www.cmbchina.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a006\", \"https://pbsz.ebank.cmbchina.com/CmbBank_GenShell/UI/GenShellPC/Login/Login.aspx\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a007\", \"http://bank.ecitic.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a007\", \"https://e.bank.ecitic.com/perbank5/signIn.do\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a008\", \"http://www.cebbank.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a008\", \"https://www.cebbank.com/per/prePerlogin1.do?_locale=zh_CN\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a008\", \"http://ebank.cebbank.com/preLogin.html\", \"רҵ��\", 2);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a009\", \"http://www.spdb.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a009\", \"http://ebank.spdb.com.cn/login/perlogin.html\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a010\", \"http://www.sdb.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a010\", \"https://ebank.sdb.com.cn/perbank/logon_pro.jsp\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a011\", \"http://www.gdb.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a011\", \"https://ebanks.gdb.com.cn/sperbank/perbankLogin.jsp\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a012\", \"http://www.hxb.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a012\", \"https://ebank.hxb.com.cn/HxPer/basPreLogin.do\", \"����������ͨ��\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a012\", \"https://ebank.hxb.com.cn/HxPer/regLogin01.do\", \"��������ǩԼ��\", 2);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a012\", \"https://dbank.hxb.com.cn/easybanking/jsp/indexCert.jsp\", \"��������֤���\", 3);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a013\", \"http://www.cmbc.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a013\", \"https://ebank.cmbc.com.cn/index_NonPrivate.html\", \"���˴��ڰ�\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a013\", \"https://business.cmbc.com.cn/index_Private.html\", \"�����\", 2);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a013\", \"https://ebank.cmbc.com.cn/indexCCLogin.html\", \"���ÿ�\", 3);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a014\", \"http://www.cib.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a014\", \"https://www.cib.com.cn/index.jsp\", \"������ͨ�û�\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a014\", \"https://www.cib.com.cn/indexCert.jsp\", \"����֤���û�\", 2);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a015\", \"http://www.bankofbeijing.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a015\", \"https://ebank.bankofbeijing.com.cn/bccbpb/accountLogon.jsp\", \"������ͨ�û�\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a015\", \"https://ebank.bankofbeijing.com.cn/bccbpb/fortuneLogon.jsp\", \"���˲Ƹ��û�\", 2);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a016\", \"http://www.bjrcb.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a016\", \"https://ibs.bjrcb.com/per/prelogin.do\", \"����������½\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a016\", \"https://ebank.bjrcb.com/ent/preloginCheque.do\", \"֧������ϵͳ\", 3);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a017\", \"http://www.psbc.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a017\", \"https://pbank.psbc.com/\", \"��������\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a018\", \"http://bank.pingan.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a018\", \"https://www.pingan.com.cn/pinganone/pa/ebanklogin.screen\", \"ƽ��һ��ͨ\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"b001\", \"http://www.cs.ecitic.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"c001\", \"http://www.chinalife.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d001\", \"http://www.chinaamc.com/\", \"��ҳ\", 0);");	
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d002\", \"http://www.cmfchina.com/index.html\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d002\", \"https://direct.cmfchina.com/index.jsp\", \"�������Ͻ���\", 1);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e001\", \"http://www.alipay.com/\", \"��ҳ\", 0);");	
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e002\", \"http://www.beijing.com.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e003\", \"http://www.openunion.cn/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e004\", \"http://www.yeepay.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"f001\", \"http://www.letao.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"f002\", \"http://www.taobao.com/\", \"��ҳ\", 0);");
//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"f002\", \"http://www.tmall.com/\", \"�Ա��̳�\", 1);");
//	
//	
//	// ��ϵͳ����ӵ��û�ģ������
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (1, \"�ֽ�\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (2, \"���ÿ�\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (3, \"���/����\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (4, \"���ڴ浥\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (5, \"֧��\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (6, \"����\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (7, \"Ͷ��\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (8, \"������˵�Ǯ\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (9, \"Ƿ���˵�Ǯ\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (10, \"�̶��ʲ�\");");
//	db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (11, \"����\");");
//
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (1, \"�����\");");
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (2, \"��Ԫ\");");
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (3, \"ŷԪ\");");
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (4, \"��Ԫ\");");
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (5, \"Ӣ��\");");
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (6, \"�۱�\");");
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (7, \"���ô�Ԫ\");");
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (8, \"�Ĵ�����Ԫ\");");
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (9, \"��ʿ����\");");
//	db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (10, \"�¼���Ԫ\");");
//
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (1, \"���շ�\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (2, \"������\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (3, \"����\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (4, \"�Ҿ߼ҵ�\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (5, \"����\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (6, \"��ͨ\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (7, \"��˰\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (8, \"����\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (9, \"����֧��\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10, \"����\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (11, \"����Ʒ\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (12, \"ʳ��\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (13, \"ͨѶ\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (14, \"��������\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (15, \"�˵�\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (16, \"����\", 1);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (17, \"��������\", 1);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (18, \"ת��֧��\", 0);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (19, \"ת������\", 1);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (20, \"���ÿ�����\", 1);");
//	db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (21, \"���ÿ�֧��\", 0);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (1, \"����\", 1);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (2, \"����\", 1);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (3, \"����\", 1);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (4, \"������\", 2);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (5, \"����\", 3);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (6, \"����\", 3);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (7, \"����\", 3);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (8, \"�ֱ�\", 3);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (9, \"Ь��\", 3);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10, \"����\", 4);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (11, \"�Ҿ�\", 4);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (12, \"����\", 5);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (13, \"���\", 5);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (14, \"ҽҩ\", 5);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (15, \"���⳵\", 6);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (16, \"�ɻ�Ʊ\", 6);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (17, \"������ͨ\", 6);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (18, \"��Ʊ\", 6);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (19, \"����\", 6);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (20, \"�Լݳ�����\", 6);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (21, \"��˰\", 7);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (22, \"�̲�\", 8);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (23, \"ѧ��\", 8);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (24, \"��ķ\", 9);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (25, \"Сʱ��\", 9);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (26, \"���\", 10);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (27, \"����\", 10);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (28, \"�Ҿ���Ʒ\", 11);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (29, \"�����Ʒ\", 11);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (30, \"����\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (31, \"��ʳ\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (32, \"����\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (33, \"ʳ��\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (34, \"�߲�\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (35, \"��ʳ\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (36, \"ˮ��\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (37, \"�����ʳ\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (38, \"�̾�\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (39, \"����\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (40, \"���⵰��\", 12);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (41, \"�̻���\", 13);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (42, \"�ֻ���\", 13);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (43, \"����\", 14);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (44, \"����\", 14);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (45, \"�鱨��־����\", 14);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (46, \"Ӱ��\", 14);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (47, \"��Ϸ\", 14);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (48, \"����\", 15);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (49, \"ȡů��\", 15);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (50, \"ˮ������\", 15);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (51, \"��ҵ��\", 15);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (52, \"����\", 16);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (53, \"����\", 16);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (54, \"����\", 17);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (55, \"���ⷿ��\", 17);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (56, \"��Ʊ����\", 17);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (57, \"����\", 17);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (58, \"��Ϣ\", 17);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (59, \"ת��֧��\", 18);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (60, \"ת������\", 19);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (61, \"���ÿ�����\", 20);");
//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (62, \"���ÿ�֧��\", 21);");
//
//	return false;
//
//}