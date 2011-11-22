// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#include "..\Utils\SQLite\CppSQLite3.h"
#include <string>
#include <atlstr.h>
#include <vector>
#include <time.h>
#pragma once
using namespace std;


class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	typedef struct tagTbCpyNode
	{
		std::string strSour; // Դ����
		std::string strDes;// Ŀ������
	}TBCPYNODE, *PTBCPYNODE;
// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /**/bHandled)
	{

		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);


		OnOK(0,IDOK, m_hWnd, bHandled);
		return TRUE;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	bool CreateDatFile(LPCSTR lpStr)
	{
		ATLASSERT(NULL != lpStr);
		HANDLE hFile = CreateFileA(lpStr, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			::MessageBoxA(NULL, "�������ݿ��ļ�ʧ�ܣ�", "�������ݿ�", MB_OK);
			return false;
		}
		CloseHandle(hFile);

		return true;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 

		TCHAR szPath[MAX_PATH] = { 0 };
		::GetModuleFileName(NULL, szPath, _countof(szPath));
		TCHAR *p = _tcsrchr(szPath, '\\');
		if (p) *p = 0;
		std::string strName  = CW2A(szPath);

		std::string strSys = strName + "\\Config\\SysDB.dat";
		// ����ϵͳ���ݿ�
		if (!CreateDatFile(strSys.c_str()))
			return 0;

		CppSQLite3DB db;
		db.open(strSys.c_str());
		// ����ϵͳ���ݿ��
		CreateSystemTables(db);

		// ��ʼ��ϵͳ���ݿ��
		InitSystemTables(db);

		std::string strGuest = strName + "\\Config\\Guest.dat";
		// ����ϵͳ���ݿ�
		if (!CreateDatFile(strGuest.c_str()))
			return 0;

		CppSQLite3DB dbGuest;
		dbGuest.open(strGuest.c_str());
		// �����û����ݿ�
		CreateUserTables(dbGuest);

		// ��ʼ���û����ݿ�
		InitUserDataBySql(dbGuest);

		// ��ʼ���û����ݿ�
		InitUserDataBySysDB(dbGuest, strSys.c_str(), strGuest.c_str());

		CloseDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}

	// ��ԭ�еİ汾������������Ҫ��һЩ����
	void CreateAccountTables(CppSQLite3DB& db)
	{
		/*db.execDML("CREATE  TABLE IF NOT EXISTS `tbAccount` (\
				   `id` INTeger  NOT NULL PRIMARY KEY AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NOT NULL ,\
				   `tbBank_id` INT(11)  NULL ,\
				   `tbAccountType_id` INT(11)  NOT NULL ,\
				   `AccountNumber` VARCHAR(256) NULL ,\
				   `Comment` VARCHAR(256) NULL,\
				   `keyInfo` varchar(255));");

		db.execDML("CREATE TABLE IF NOT EXISTS 'tbAccountGetBillMonth' (\
			'id' INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
			'tbaccount_id' INTEGER  NOT NULL,\
			'tbmonth' VARCHAR(6)  NOT NULL,\
			'tbKeyInfo' VARCHAR(100)  NOT NULL\
			)");
		db.execDML("CREATE  TABLE IF NOT EXISTS `tbAccountType` (\
				   `id` INTeger  NOT NULL PRIMARY KEY AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NULL);");
		db.execDML("CREATE  TABLE IF NOT EXISTS `tbBank` (\
				   `id` INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,\
				   `name` VARCHAR(256)  NOT NULL,\
				   `classId` INTEGER NOT NULL,\
				   `BankID` VARCHAR(4)  NULL,\
				   `Phone` VARCHAR(256)  NULL,\
				   `Website` vARCHAR(256)  NULL);");
		db.execDML("CREATE  TABLE IF NOT EXISTS `tbCategory1` (\
				   `id` INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NULL ,\
				   `Type` TINYINT NULL);");
		db.execDML("CREATE  TABLE IF NOT EXISTS `tbCategory2` (\
				   `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NULL ,\
				   `tbCategory1_id` INT(11)  NOT NULL);");
		db.execDML("CREATE  TABLE IF NOT EXISTS `tbCurrency` (\
				   `id` INTEGER NOT NULL PRIMARY KEY,\
				   `Name` VARCHAR(256) NOT NULL  );");
		db.execDML("CREATE  TABLE IF NOT EXISTS `tbDBInfo` (\
				   `schema_version` INTEGER  NOT NULL );");
		db.execDML("CREATE TABLE `tbPayee` (\
				   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT,\
				   `Name` VARCHAR(256) NOT NULL,\
				   `email` VARCHAR(256) ,\
				   `tel` VARCHAR(256)\
				   );");
		db.execDML("CREATE  TABLE IF NOT EXISTS `tbSubAccount` (\
				   `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\
				   `tbAccount_id` INT(11)  NOT NULL ,\
				   `tbCurrency_id` INT(11)  NOT NULL ,\
				   `name` VARCHAR(256) NOT NULL,\
				   `OpenBalance` FLOAT NOT NULL ,\
				   `Balance` FLOAT NOT NULL ,\
				   `Days` INT(11)  NULL ,\
				   `EndDate` DATE NULL ,\
				   `Comment` VARCHAR(256) NULL ,\
				   `tbAccountType_id` INT(11) NULL);");
		db.execDML("CREATE  TABLE IF NOT EXISTS `tbTransaction` (\
				   `id` INTEGER NOT NULL  PRIMARY KEY AUTOINCREMENT ,\
				   `TransDate` DEFAULT (DATE(CURRENT_TIMESTAMP,'localtime')) ,\
				   `tbPayee_id` INT(11)  NULL ,\
				   `tbCategory2_id` INT(11)  NULL ,\
				   `Amount` FLOAT NOT NULL DEFAULT 0 ,\
				   `direction` INT(11) NULL DEFAULT 0 ,\
				   `tbSubAccount_id` INT(11)  NOT NULL ,\
				   `tbSubAccount_id1` INT(11)  NULL ,\
				   `ExchangeRate` FLOAT NULL ,\
				   `Comment` VARCHAR(256) NULL ,\
				   `sign` VARCHAR(256) NULL,\
				   `transactionClasses` tinyint (1)DEFAULT 0 NOT NULL,\
				   `isEdit` tinyint (1)DEFAULT 0 NOT NULL);");*/
		db.execDML("CREATE TABLE [tbAccount] ( \
				   [id] bigint PRIMARY KEY NOT NULL , \
				   [Name] VARCHAR(256) NOT NULL , \
				   [tbBank_id] bigint  NULL , \
				   [tbAccountType_id] INT(11)  NOT NULL , \
				   [AccountNumber] VARCHAR(256) NULL , \
				   [Comment] VARCHAR(256) NULL , \
				   [keyInfo] varchar(255) NULL , \
				   [status] int(2) default 0, \
				   [UT] bigint NOT NULL default 0 \
				   );");


		db.execDML("CREATE TABLE [tbAccountGetBillMonth] ( \
				   [id] bigint PRIMARY KEY NOT NULL, \
				   [tbaccount_id] bigint  NOT NULL, \
				   [tbmonth] VARCHAR(6)  NOT NULL, \
				   [tbKeyInfo] VARCHAR(100)  NOT NULL, \
				   [status] int(2) NOT NULL default 0, \
				   [UT] bigint NOT NULL default 0 \
				   );");

		db.execDML("CREATE TABLE [tbAccountType] ( \
				   [id] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT , \
				   [Name] VARCHAR(256) NULL \
				   );");

		db.execDML("CREATE TABLE [tbBank] ( \
				   [id] bigint  NOT NULL PRIMARY KEY, \
				   [name] VARCHAR(256)  NOT NULL, \
				   [classId] INTEGER NOT NULL, \
				   [BankID] VARCHAR(4)  NULL, \
				   [Phone] VARCHAR(256)  NULL, \
				   [Website] vARCHAR(256)  NULL, \
				   [order] int(4) NOT NULL default 0, \
				   [status] int(2) NOT NULL default 0, \
				   [UT] bigint NOT NULL default 0 \
				   );");

		db.execDML("CREATE TABLE [tbCategory1] ( \
				   [id] bigint  NOT NULL PRIMARY KEY , \
				   [Name] VARCHAR(256) NULL , \
				   [Type] TINYINT NULL, \
				   [order] int(4) NOT NULL default 0, \
				   [status] int(2) NOT NULL default 0, \
				   [UT] bigint NOT NULL default 0 \
				   );");

		db.execDML("CREATE TABLE [tbCategory2] ( \
				   [id] bigint NOT NULL PRIMARY KEY , \
				   [Name] VARCHAR(256) NULL , \
				   [tbCategory1_id] bigint  NOT NULL, \
				   [order] int(4) NOT NULL default 0, \
				   [status] int(2) NOT NULL default 0, \
				   [UT] bigint NOT NULL default 0 \
				   );");

		db.execDML("CREATE TABLE [tbCurrency] ( \
				   [id] INTEGER NOT NULL PRIMARY KEY, \
				   [Name] VARCHAR(256), \
				   [order] integer(4) NOT NULL default 0 \
				   );");

		db.execDML("CREATE TABLE [tbPayee] ( \
				   [id] bigint NOT NULL PRIMARY KEY, \
				   [Name] VARCHAR(256) NOT NULL, \
				   [email] VARCHAR(256), \
				   [tel] VARCHAR(256), \
				   [order] int(4) NOT NULL default 0, \
				   [status] int(2) NOT NULL default 0, \
				   [UT] bigint NOT NULL default 0 \
				   );");

		db.execDML("CREATE TABLE [tbSubAccount] ( \
				   [id] bigint NOT NULL PRIMARY KEY, \
				   [tbAccount_id] bigint NOT NULL, \
				   [tbCurrency_id] INT(11) NOT NULL, \
				   [name] VARCHAR(256) NOT NULL, \
				   [OpenBalance] FLOAT NOT NULL, \
				   [Balance] FLOAT NOT NULL , \
				   [Days] INT(11) NULL, \
				   [EndDate] DATE NULL, \
				   [Comment] VARCHAR(256) NULL, \
				   [tbAccountType_id] INT(11) NULL, \
				   [order] int(4) NOT NULL default 0, \
				   [status] int(2) NOT NULL default 0, \
				   [UT] bigint NOT NULL default 0 \
				   );");

		db.execDML("CREATE TABLE [tbTransaction] ( \
				   [id] bigint NOT NULL PRIMARY KEY, \
				   [TransDate] DEFAULT (DATE(CURRENT_TIMESTAMP,'localtime')) , \
				   [tbPayee_id] bigint  NULL , \
				   [tbCategory2_id] bigint  NULL , \
				   [Amount] FLOAT NOT NULL DEFAULT 0 , \
				   [direction] bigint NULL DEFAULT 0 , \
				   [tbSubAccount_id] bigint NOT NULL, \
				   [tbSubAccount_id1] bingint  NULL , \
				   [ExchangeRate] FLOAT NULL , \
				   [Comment] VARCHAR(256) NULL , \
				   [sign] VARCHAR(256) NULL, \
				   [transactionClasses] tinyint(1) default 0 not null, \
				   [isEdit] tinyint(1) default 0 not null, \
				   [order] int(4) NOT NULL default 0, \
				   [status] int(2) NOT NULL default 0, \
				   [UT] bigint NOT NULL default 0 \
				   );");
		db.execDML("CREATE INDEX `fk_tbAccount_tbBank` ON `tbAccount` (`tbBank_id` ASC);");
		db.execDML("CREATE INDEX `fk_tbAccount_tbAccountType1` ON `tbAccount` (`tbAccountType_id` ASC);");
		db.execDML("CREATE INDEX `fk_tbCategory2_tbCategory11` ON `tbCategory2` (`tbCategory1_id` ASC);");
		db.execDML("CREATE INDEX `fk_tbSubAccount_tbAccount1` ON `tbSubAccount` (`tbAccount_id` ASC);");
		db.execDML("CREATE INDEX `fk_tbSubAccount_tbCurrency1` ON `tbSubAccount` (`tbCurrency_id` ASC);");
		db.execDML("CREATE INDEX `fk_tbSubAccount_tbAccountType1` ON `tbSubAccount` (`tbAccountType_id` ASC);");
		db.execDML("CREATE INDEX `fk_tbTransaction_tbPayee1` ON `tbTransaction` (`tbPayee_id` ASC);");
		db.execDML("CREATE INDEX `fk_tbTransaction_tbCategory21` ON `tbTransaction` (`tbCategory2_id` ASC);");
		db.execDML("CREATE INDEX `fk_tbTransaction_tbSubAccount1` ON `tbTransaction` (`tbSubAccount_id` ASC);");
		db.execDML("CREATE INDEX `fk_tbTransaction_tbSubAccount2` ON `tbTransaction` (`tbSubAccount_id1` ASC);");
		db.execDML("CREATE TABLE IF NOT EXISTS 'tbProductChoice' (\
					   'id' bigint  NOT NULL,\
					   'interestchoice' varchar(256)  NULL,\
					   'durationchoice' varchar(256)  NULL,\
					   'currencychoice' varchar(256)  NULL,\
					   'bankchoice' varchar(256)  NULL,\
					   'userId' bigint  NOT NULL,\
					   'UT' bigint  NOT NULL\
					   );");
		db.execDML("insert into tbProductChoice (id, interestchoice,durationchoice,currencychoice,bankchoice,userId,ut) values(0,'1|2|3|4','1|2|3|4|5|6','1|2','1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19',0,0);");

		return;
	}

	void CreateUserTables(CppSQLite3DB& db)
	{
		// 
		db.execDML("CREATE TABLE tbFav(favinfo CHAR(4) UNIQUE, status INT, favorder INT, deleted BOOL);");

		db.execDML("CREATE TABLE tbAuthen(pwd BLOB);");
		db.execDML("CREATE TABLE tbEvent(id INT, event_date INT, description CHAR(256), repeat INT, alarm INT, status INT, datestring CHAR(12));");
		db.execDML("CREATE TABLE tbCoupon(id INT, expire INT, status INT, name CHAR(256), sn CHAR(256), typeid INT);");
		db.execDML("CREATE TABLE tbToday(current INT);");
		db.execDML("CREATE TABLE tbDBInfo(schema_version INT);");

		// �������ʹ�����صı�
		CreateAccountTables(db);

	}

	void InitUserDataBySql(CppSQLite3DB& db)
	{
		//CppSQLite3Buffer bufSQL;
		//bufSQL.format("INSERT INTO tbDBInfo VALUES(%d);", 6); // ���ڵ����ݿ�汾�Ѿ�Ϊ6
		//db.execDML(bufSQL);		

		////��������������Ϊ��һ����¼
		//SYSTEMTIME systime;
		//GetSystemTime(&systime);

		//struct tm tmVal;
		//memset(&tmVal, 0, sizeof(struct tm));
		//tmVal.tm_year = systime.wYear - 1900;
		//tmVal.tm_mon = systime.wMonth - 1;
		//tmVal.tm_mday = systime.wDay;

		//__time32_t tToday = _mktime32(&tmVal);
		//bufSQL.format("INSERT INTO tbToday VALUES(%d);", tToday);
		//db.execDML(bufSQL);

		//db.execDML("INSERT INTO tbfav (favinfo, status, favorder, deleted) VALUES ('a001', 0, 1, 0);");
		
		// �ڴ�ϵͳ���п�������ʱ�Ὣ��Ӧ����ԭ�е���������������²��������Ӧ���ڿ���֮��
		db.execDML("INSERT INTO tbAccountType (id, Name) VALUES (100, \"���ڴ��\");");
		db.execDML("INSERT INTO tbAccountType (id, Name) VALUES (101, \"���ڴ��\");");
		db.execDML("INSERT INTO tbAccountType (id, Name) VALUES (102, \"��Ʋ�Ʒ\");");

		db.execDML("INSERT INTO tbAccount (id, Name, tbAccountType_id) VALUES (1, \"�ҵ��ֽ�\", 1);");
		//db.execDML("INSERT INTO tbAccount (id, Name, tbAccountType_id) VALUES (2, \"���п�\", 3);");
		//db.execDML("INSERT INTO tbAccount (id, Name, tbAccountType_id) VALUES (3, \"���ÿ�\", 2);");

		//db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance, tbAccountType_id) VALUES (1, \"���ڴ��\", 2, 1, 0, 0, 100);");
		////db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance) VALUES (2, \"���ڴ��\", 2, 1, 0, 0);");
		//db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance) VALUES (2, \"�����\", 3, 1, 0, 0);");
		//db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance) VALUES (3, \"��Ԫ\", 3, 2, 0, 0);");
		db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance) VALUES (4, \"�����\", 1, 1, 0, 0);");

		db.execDML("INSERT INTO tbDBInfo (schema_version) VALUES(7);"); //3.1����ǰ�汾Ϊ7
	}

	// �����ݿ�ı�������һ�����ݿ���
	bool CpyFromSameTable(LPCSTR lpDbPath, LPCSTR lpTbName, LPCSTR lpDesDbPath, LPCSTR lpDesTbName, LPCSTR lpCondition = NULL)
	{
		ATLASSERT (NULL != lpDbPath && NULL != lpTbName);
		ATLASSERT (NULL != lpDesDbPath && NULL != lpDesTbName);
		if (NULL == lpDbPath || NULL == lpTbName || NULL == lpDesDbPath || NULL == lpDesTbName)
			return false;

		CppSQLite3DB db;
		db.open(lpDbPath);

		CppSQLite3DB dbDes;
		dbDes.open(lpDesDbPath);

		std::string strSQL;// = "delete from ";
		//strSQL += lpDesTbName;
		// ɾ�������ݿ�ԭ�е�����
		//dbDes.execDML(strSQL.c_str());


		// �õ�Ҫ���������
		strSQL = "select * from ";
		strSQL += lpTbName;
		if (NULL != lpCondition)
		{
			strSQL += lpCondition;
		}
		CppSQLite3Table  tempTb = db.getTable(strSQL.c_str());

		if (tempTb.numFields() <= 0)
		{
			tempTb.finalize ();
			return false;
		}

		std::string strSqlFront = "Insert into ";
		strSqlFront += lpDesTbName;
		//strSqlFront += '(';

		// �������������SQL��Ĳ���ʱ���������
		/*std::string strColNames;
		for (int i = 0; i < dbDes.numFields(); i ++)
		{
		std::string strColName = dbDes.fieldName(i);
		strSqlFront += strColName;
		if (i != dbDes.numFields() - 1)
		{
		strSqlFront += ',';
		}
		}*/

		strSqlFront += " Values ";

		std::string strSQLInsert;
		for (int i = 0; i < tempTb.numRows(); i ++)
		{
			tempTb.setRow (i);

			std::string strColVals;
			strColVals += '(';
			for (int j = 0; j < tempTb.numFields(); j ++)
			{
				strColVals += '\'';
				strColVals += tempTb.fieldValue(j);
				strColVals += '\'';
				if (j != tempTb.numFields() - 1)
				{
					strColVals += ',';
				}
			}

			strColVals += ')';

			std::string strExSQL = strSqlFront + strColVals;
			dbDes.execDML(strExSQL.c_str());


		}

		tempTb.finalize ();

		return true;

	}


	bool CpyTbBankTable(PTBCPYNODE pPath, PTBCPYNODE pTabName, std::vector<PTBCPYNODE>* pColName,  LPSTR lpCondition = NULL)
	{
		ATLASSERT (NULL != pPath && NULL != pTabName && NULL != pColName);
		if (NULL == pPath || NULL == pTabName || NULL == pColName)
			return false;

		// �����ļ��Ƿ����
//		if (!IsFileExist((LPSTR)pPath->strSour.c_str()) || !IsFileExist((LPSTR)pPath->strDes.c_str())) 
//			return false;

		CppSQLite3DB dbSour;
		dbSour.open(pPath->strSour.c_str());

		CppSQLite3DB dbDes;
		dbDes.open(pPath->strDes.c_str());

		std::string strSQL ;//= "delete from ";
		//strSQL += lpDesTbName;
		// ɾ�������ݿ�ԭ�е�����
		//dbDes.execDML(strSQL.c_str());

		int nSize = 0;
		std::string strColsSour, strColsDes;
		std::vector<PTBCPYNODE>::const_iterator iter;
		for (iter = pColName->begin(); iter != pColName->end(); iter ++)
		{
			PTBCPYNODE pTemp = *(iter);
			if (pTemp == NULL)
				continue;

			strColsSour += pTemp->strSour;
			strColsDes += pTemp->strDes;

			nSize ++;

			if (nSize != pColName->size())
			{
				strColsSour += ", ";
				strColsDes += ", ";
			}
		}


		// �õ�Ҫ���������
		strSQL = "select ";
		strSQL += strColsSour;
		strSQL += " from ";
		strSQL += pTabName->strSour;
		if (NULL != lpCondition)
		{
			strSQL += lpCondition;
		}

		// �õ�Ҫ�����ļ�¼
		CppSQLite3Table  tempTb = dbSour.getTable(strSQL.c_str());

		// �õ������SQL���
		std::string strSqlFront = "Insert into ";
		strSqlFront += pTabName->strDes;
		strSqlFront += '(';
		strSqlFront += "id,";
		strSqlFront += strColsDes;
		strSqlFront += ") Values ";

		std::string strSQLInsert;
		for (int i = 0; i < tempTb.numRows(); i ++)
		{
			tempTb.setRow (i);

			std::string strColVals;
			strColVals += '(';

			CString strIndex;
			strIndex.Format(L"%d,", i + 1);
			strColVals += CW2A(strIndex);
			for (int j = 0; j < tempTb.numFields(); j ++)
			{
				strColVals += '\'';
				strColVals += tempTb.fieldValue(j);
				strColVals += '\'';
				if (j != tempTb.numFields() - 1)
				{
					strColVals += ',';
				}
			}

			strColVals += ')';

			std::string strExSQL = strSqlFront + strColVals;

			// ִ�в���
			dbDes.execDML(strExSQL.c_str());


		}

		return true;
	}

	void InitUserDataBySysDB(CppSQLite3DB& db, LPCSTR lpSour, LPCSTR lpDesc)
	{
		ATLASSERT(NULL != lpSour && NULL != lpDesc);
		if (NULL == lpSour || NULL == lpDesc)
			return;

		// ��ͬ�����ݿ�����ͬ�ı�ṹ֮������ݿ���������ʱ����ɾ��Ŀ����е�ԭ������
		CpyFromSameTable(lpSour, "sysAccountType", lpDesc, "tbAccountType");
		CpyFromSameTable(lpSour, "sysCurrency", lpDesc, "tbCurrency");
		CpyFromSameTable(lpSour, "sysCategory1", lpDesc, "tbCategory1");
		CpyFromSameTable(lpSour, "sysCategory2", lpDesc, "tbCategory2");

		// ��ͬ���ݿ�֮�䲻ͬ�ı�ṹ֮������ݿ���������ʱ����ɾ��Ŀ�����ԭ�е�����
		TBCPYNODE path;
		path.strSour = lpSour;
		path.strDes = lpDesc;
		TBCPYNODE tabNode;
		tabNode.strSour = "sysBank";
		tabNode.strDes = "tbBank";
		std::vector<PTBCPYNODE> colVect;
		TBCPYNODE colNode1, colNode2, colNode3;
		colNode1.strSour = "id";
		colNode1.strDes = "BankID";
		colNode2.strSour = "Name";
		colNode2.strDes = "Name";
		colNode3.strSour = "classId";
		colNode3.strDes = "classId";
		colVect.push_back(&colNode1);
		colVect.push_back(&colNode2);
		colVect.push_back(&colNode3);
		CpyTbBankTable(&path, &tabNode, &colVect, " where classId in(0, 1, 2)");//, " where sysAppType_id = '2'"
	}


	void CreateSystemTables(CppSQLite3DB& db)
	{
		db.execDML("CREATE  TABLE IF NOT EXISTS `sysAppType` (\
				   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NOT NULL);");

		db.execDML("CREATE  TABLE IF NOT EXISTS `sysBank` (\
				   `id` VARCHAR(4) NOT NULL PRIMARY KEY,\
				   `Name` VARCHAR(256) UNIQUE NOT NULL,\
				   `classId` INTEGER NOT NULL DEFAULT(99),\
				   `ShortName` VARCHAR(256) UNIQUE NOT NULL,\
				   `Position` INTEGER NOT NULL,\
				   `sysAppType_id` INTEGER NOT NULL);");

		db.execDML("CREATE  TABLE IF NOT EXISTS `sysSubLink` (\
				   `sysBank_id` VARCHAR(4) NOT NULL  ,\
				   `URL` VARCHAR(256) UNIQUE NOT NULL,\
				   `URLText` VARCHAR(256) NOT NULL,\
				   `LinkOrder` INTEGER NOT NULL,\
				   PRIMARY KEY(sysBank_id, URL));");

		db.execDML("CREATE  TABLE IF NOT EXISTS `sysCategory1` (\
				   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NOT NULL,\
				   `Type` TINYINTNOT NULL, \
				   `order` int(4) NOT NULL default 0, \
				   `status` int(2) NOT NULL default 0, \
				   `UT` bigint NOT NULL default 0);");

		db.execDML("CREATE  TABLE IF NOT EXISTS `sysCategory2` (\
				   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NOT NULL,\
				   `sysCategory1_id` TINYINTNOT NULL ,\
				   `order` int(4) NOT NULL default 0, \
				   `status` int(2) NOT NULL default 0, \
				   `UT` bigint NOT NULL default 0);");

		db.execDML("CREATE  TABLE IF NOT EXISTS `sysAccountType` (\
				   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NOT NULL);");

		db.execDML("CREATE  TABLE IF NOT EXISTS `sysCurrency` (\
				   `id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NOT NULL, \
				   `order` integer(4) NOT NULL default 0);");

		db.execDML("CREATE  TABLE IF NOT EXISTS `sysDBInfo` (\
				   `schema_version` INTEGER NOT NULL);");

		db.execDML("CREATE  TABLE IF NOT EXISTS `sysURLWhiteList` (\
				   `sysBank_id` VARCHAR(4) NOT NULL PRIMARY KEY ,\
				   `URLPattern` VARCHAR(256) NOT NULL);");
	}
		
	void InitSystemTables(CppSQLite3DB& db)
	{
		db.execDML("INSERT INTO sysAppType (id, Name) VALUES (1, \"all\");");
		db.execDML("INSERT INTO sysAppType (id, Name) VALUES (2, \"banks\");");
		db.execDML("INSERT INTO sysAppType (id, Name) VALUES (3, \"securities\");");
		db.execDML("INSERT INTO sysAppType (id, Name) VALUES (4, \"insurances\");");
		db.execDML("INSERT INTO sysAppType (id, Name) VALUES (5, \"funds\");");
		db.execDML("INSERT INTO sysAppType (id, Name) VALUES (6, \"payments\");");
		db.execDML("INSERT INTO sysAppType (id, Name) VALUES (7, \"shopping\");");

		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a001\", \"�й�����\", 0, \"boc\", 0, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a002\", \"�й�ũҵ����\", 0, \"abchina\", 1, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a003\", \"�й���������\", 0, \"icbc\", 2, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a004\", \"�й���������\", 0, \"ccb\", 3, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a005\", \"��ͨ����\", 0, \"bankcomm\", 4, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a006\", \"��������\", 0, \"cmbchina\", 5, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a007\", \"��������\", 0, \"ecitic\", 6, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a008\", \"�������\", 0, \"cebbank\", 7, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a009\", \"�ַ�����\", 0, \"spdb\", 8, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a010\", \"���ڷ�չ����\", 0, \"sdb\", 9, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a011\", \"�㷢����\", 0, \"gdb\", 10, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a012\", \"��������\", 0, \"hxb\", 11, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a013\", \"��������\", 0, \"cmbc\", 12, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a014\", \"��ҵ����\", 0, \"cib\", 13, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a015\", \"��������\", 0, \"beijing\", 14, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a016\", \"����ũ������\", 0, \"bjrcb\", 15, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a017\", \"�й�������������\", 0, \"psbc\", 16, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a018\", \"ƽ������\", 0, \"pinganbank\", 17, 2);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"a019\", \"��������\", 0, \"cbhb\", 18, 2);");

		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"b001\", \"����֤ȯ\", 1, \"cs_ecitic\", 0, 3);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"c001\", \"�й����ٱ���\", 1, \"chinalife\", 0, 4);");

		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d001\", \"���Ļ���\", 1, \"chinaamc\", 0, 5);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d002\", \"���̻���\", 1, \"cmfchina\", 1, 5);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d003\", \"��̩����\", 1, \"gtfund\", 2, 5);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d004\", \"�Ϸ�����\", 1, \"southernfund\", 3, 5);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d005\", \"��ʵ����\", 1, \"jsfund\", 4, 5);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d006\", \"��������\", 1, \"fullgoal\", 5, 5);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d007\", \"�׷������\", 1, \"efunds\", 6, 5);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d008\", \"��ɻ���\", 1, \"dcfund\", 7, 5);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"d009\", \"��ʱ����\", 1, \"bosera\", 8, 5);");

		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e001\", \"֧����\", 2, \"alipay\", 0, 6);");
		//db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e002\", \"������֧��\", 2, \"payease\", 1, 6);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e003\", \"����\", 2, \"openunion\", 2, 6);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e004\", \"�ױ�\", 2, \"yeepay\", 3, 6);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e005\", \"�Ƹ�ͨ\", 2, \"tenpay\", 4, 6);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e006\", \"��Ǯ\", 2, \"99bill\", 5, 6);");
		//db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e007\", \"ʢ��ͨ\", 2, \"shengfutong\", 6, 6);");
		db.execDML("INSERT INTO sysBank (id, Name, classId, ShortName, Position, sysAppType_id) VALUES (\"e008\", \"��������\", 2, \"chinapay\", 7, 6);");
		db.execDML("INSERT INTO sysBank (id, Name, ShortName, Position, sysAppType_id) VALUES (\"f001\", \"�����̳�\", \"letao\", 0, 7);");
		db.execDML("INSERT INTO sysBank (id, Name, ShortName, Position, sysAppType_id) VALUES (\"f002\", \"�Ա���\", \"taobao\", 1, 7);");

		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a001\", \"http://www.boc.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a001\", \"https://ebs.boc.cn/BocnetClient/LoginFrame.do?_locale=zh_CN\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a002\", \"http://www.abchina.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a002\", \"http://www.abchina.com/cn/EBanking/Ebanklogin/PCustomerLogin/default.htm\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a003\", \"http://www.icbc.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a003\", \"https://mybank.icbc.com.cn/icbc/perbank/index.jsp\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a003\", \"https://vip.icbc.com.cn/\", \"�����\", 2);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a004\", \"http://www.ccb.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a004\", \"https://ibsbjstar.ccb.com.cn/app/V5/CN/STY1/login.jsp\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a005\", \"http://www.bankcomm.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a005\", \"https://pbank.95559.com.cn/personbank/index.jsp\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a006\", \"http://www.cmbchina.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a006\", \"https://pbsz.ebank.cmbchina.com/CmbBank_GenShell/UI/GenShellPC/Login/Login.aspx\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a007\", \"http://bank.ecitic.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a007\", \"https://e.bank.ecitic.com/perbank5/signIn.do\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a008\", \"http://www.cebbank.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a008\", \"https://www.cebbank.com/per/prePerlogin1.do?_locale=zh_CN\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a008\", \"http://ebank.cebbank.com/preLogin.html\", \"רҵ��\", 2);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a009\", \"http://www.spdb.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a009\", \"http://ebank.spdb.com.cn/login/perlogin.html\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a010\", \"http://www.sdb.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a010\", \"https://ebank.sdb.com.cn/perbank/logon_pro.jsp\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a011\", \"http://www.gdb.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a011\", \"https://ebanks.gdb.com.cn/sperbank/perbankLogin.jsp\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a012\", \"http://www.hxb.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a012\", \"https://ebank.hxb.com.cn/HxPer/basPreLogin.do\", \"����������ͨ��\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a012\", \"https://ebank.hxb.com.cn/HxPer/regLogin01.do\", \"��������ǩԼ��\", 2);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a012\", \"https://dbank.hxb.com.cn/easybanking/jsp/indexCert.jsp\", \"��������֤���\", 3);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a013\", \"http://www.cmbc.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a013\", \"https://ebank.cmbc.com.cn/index_NonPrivate.html\", \"���˴��ڰ�\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a013\", \"https://business.cmbc.com.cn/index_Private.html\", \"�����\", 2);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a013\", \"https://ebank.cmbc.com.cn/indexCCLogin.html\", \"���ÿ�\", 3);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a014\", \"http://www.cib.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a014\", \"https://www.cib.com.cn/index.jsp\", \"������ͨ�û�\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a014\", \"https://www.cib.com.cn/indexCert.jsp\", \"����֤���û�\", 2);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a015\", \"http://www.bankofbeijing.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a015\", \"https://ebank.bankofbeijing.com.cn/bccbpb/accountLogon.jsp\", \"������ͨ�û�\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a015\", \"https://ebank.bankofbeijing.com.cn/bccbpb/fortuneLogon.jsp\", \"���˲Ƹ��û�\", 2);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a016\", \"http://www.bjrcb.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a016\", \"https://ibs.bjrcb.com/per/prelogin.do\", \"����������½\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a016\", \"https://ebank.bjrcb.com/ent/preloginCheque.do\", \"֧������ϵͳ\", 3);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a017\", \"http://www.psbc.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a017\", \"https://pbank.psbc.com/\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a018\", \"http://bank.pingan.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a018\", \"https://www.pingan.com.cn/pinganone/pa/ebanklogin.screen\", \"ƽ��һ��ͨ\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a019\", \"http://www.cbhb.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"a019\", \"https://ebank.cbhb.com.cn/per/prelogin.do\", \"��������\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"b001\", \"http://www.cs.ecitic.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"c001\", \"http://www.chinalife.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d001\", \"http://www.chinaamc.com/\", \"��ҳ\", 0);");	
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d002\", \"http://www.cmfchina.com/index.html\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d002\", \"https://direct.cmfchina.com/index.jsp\", \"�������Ͻ���\", 1);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d003\", \"http://www.gtfund.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d004\", \"http://www.nffund.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d005\", \"http://www.jsfund.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d006\", \"http://www.fullgoal.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d007\", \"http://www.efunds.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d008\", \"http://www.dcfund.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"d009\", \"http://www.bosera.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e001\", \"http://www.alipay.com/\", \"��ҳ\", 0);");	
		//db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e002\", \"http://www.beijing.com.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e003\", \"http://www.openunion.cn/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e003\", \"http://card.openunion.cn/\", \"���Ŀ�\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e004\", \"http://www.yeepay.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e005\", \"http://www.tenpay.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e006\", \"http://www.99bill.com/\", \"��ҳ\", 0);");
	//	db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e007\", \"http://www.yeepay.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"e008\", \"http://www.chinapay.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"f001\", \"http://www.letao.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"f002\", \"http://www.taobao.com/\", \"��ҳ\", 0);");
		db.execDML("INSERT INTO sysSubLink (sysBank_id, URL, URLText, LinkOrder) VALUES (\"f002\", \"http://www.tmall.com/\", \"�Ա��̳�\", 1);");


		// ��ϵͳ����ӵ��û�ģ������
		db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (1, \"�ֽ�\");");
		db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (2, \"���ÿ�\");");
		db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (3, \"���/����\");");
		//db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (4, \"���ڴ浥\");");
		db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (4, \"֧��\");");
		//db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (6, \"����\");");
		db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (5, \"Ͷ��\");");
		db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (6, \"�����Ǯ\");");
		db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (7, \"�����Ǯ\");");
		db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (8, \"�̶��ʲ�\");");
		db.execDML("INSERT INTO sysAccountType (id, Name) VALUES (9, \"����\");");

		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (1, \"�����\");");
		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (2, \"��Ԫ\");");
		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (3, \"ŷԪ\");");
		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (4, \"��Ԫ\");");
		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (5, \"Ӣ��\");");
		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (6, \"�۱�\");");
		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (7, \"���ô�Ԫ\");");
		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (8, \"��Ԫ\");");
		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (9, \"��ʿ����\");");
		db.execDML("INSERT INTO sysCurrency (id, Name) VALUES (10, \"�¼���Ԫ\");");

		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (1, \"ʳƷ����\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (2, \"������ͨ\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (3, \"˽�ҳ�����\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (4, \"�Ӽ���ҵ\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (5, \"��������\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (6, \"ͨѶ����\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (7, \"������\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (8, \"ҽ�Ʊ���\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (9, \"��������Ʒ\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10, \"ѧϰ��ѵ\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (11, \"����˰��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (12, \"ְҵ����\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (13, \"�Ʋ�����\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (14, \"��������\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10018, \"ת��֧��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10019, \"ת������\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10020, \"���ÿ�����\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10021, \"���ÿ�֧��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10023, \"δ����֧��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10024, \"δ��������\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10025, \"������\", 0);");// �ž�Ҫ��9-19


		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (1, \"ʳƷ\", 1);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (2, \"ˮ����ʳ\", 1);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (3, \"�̾�����\", 1);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (4, \"����Ͳ�\", 1);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (5, \"����\", 1);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (6, \"��������\", 2);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (7, \"���⳵\", 2);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (8, \"��;��ͨ\", 2);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (9, \"����\", 2);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10, \"�ͷ�\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (11, \"ά�ޱ���\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (12, \"ͣ����\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (13, \"·�ŷ�\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (14, \"˰�ѱ���\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (15, \"����\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (16, \"�ճ���Ʒ\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (17, \"ˮ��ú��\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (18, \"����\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (19, \"ȡů��\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (20, \"��ҵ�����\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (21, \"ά�ޱ���\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (22, \"���ߵ���\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (23, \"��������\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (24, \"ĸӤ��Ʒ\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (25, \"����\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (26, \"�˶�����\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (27, \"�ۻ�Ͳ�\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (28, \"Ӱ������\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (29, \"����֧��\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (30, \"���ζȼ�\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (31, \"���\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (32, \"�鱨��־\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (33, \"����֧��\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (34, \"����\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (35, \"�̻���\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (36, \"�ֻ���\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (37, \"������\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (38, \"�ʼĿ��\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (39, \"����\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (40, \"��Ʒ���\", 7);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (41, \"Т������\", 7);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (42, \"���ƾ��\", 7);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (43, \"����\", 7);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (44, \"�����\", 8);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (45, \"סԺ��\", 8);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (46, \"ҩƷ��\", 8);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (47, \"������\", 8);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (48, \"����\", 8);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (49, \"�Ҿ�\", 9);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (50, \"����\", 9);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (51, \"�Ҿ�װ��\", 9);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (52, \"���ù���\", 9);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (53, \"�����Ʒ\", 9);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (54, \"��ͨ����\", 9);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (55, \"����\", 9);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (56, \"��ѵ��\", 10);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (57, \"ѧ��\", 10);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (58, \"���Ϸ�\", 10);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (59, \"ѧϰ��Ʒ\", 10);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (60, \"����\", 10);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (61, \"��Ϣ֧��\", 11);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (62, \"˰��֧��\", 11);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (63, \"Ͷ����ʧ\", 11);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (64, \"����֧��\", 11);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (65, \"�⳥����\", 11);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (66, \"������\", 11);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (67, \"����\", 11);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (68, \"��������\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (69, \"�Ӱ�����\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (70, \"��������\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (71, \"��ְ����\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (72, \"��Ӫ����\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (73, \"����\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (74, \"�������\", 13);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (75, \"��Ϣ����\", 13);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (76, \"Ͷ�ʻر�\", 13);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (77, \"����\", 13);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (78, \"�������\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (79, \"�н�����\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (80, \"�̳�\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (81, \"�������\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (82, \"������Ǯ\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (83, \"����\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10059, \"CATA420\", 10018);");//bug 2063
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10060, \"CATA420\", 10019);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10061, \"���ÿ�����\", 10020);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10062, \"���ÿ�֧��\", 10021);");
	//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10063, \"����\", 17);");
	//	db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10064, \"����\", 22);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10065, \"CATA420\", 10023);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10066, \"CATA420\", 10024);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10067, \"CATA420\", 10025)"); // �ž�Ҫ��9-19

		/*db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (1, \"���շ�\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (2, \"������\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (3, \"����\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (4, \"�Ҿ߼ҵ�\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (5, \"����\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (6, \"��ͨ\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (7, \"��˰\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (8, \"����\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (9, \"����֧��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (10, \"����\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (11, \"����Ʒ\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (12, \"ʳ��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (13, \"ͨѶ\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (14, \"��������\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (15, \"�˵�\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (16, \"����\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (17, \"��������\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (18, \"ת��֧��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (19, \"ת������\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (20, \"���ÿ�����\", 1);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (21, \"���ÿ�֧��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (22, \"����֧��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (23, \"δ����֧��\", 0);");
		db.execDML("INSERT INTO sysCategory1 (id, Name, Type) VALUES (24, \"δ��������\", 1);");

		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (1, \"����\", 1);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (2, \"����\", 1);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (3, \"����\", 1);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (4, \"������\", 2);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (5, \"����\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (6, \"����\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (7, \"����\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (8, \"�ֱ�\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (9, \"Ь��\", 3);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (10, \"����\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (11, \"�Ҿ�\", 4);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (12, \"����\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (13, \"���\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (14, \"ҽҩ\", 5);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (15, \"���⳵\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (16, \"�ɻ�Ʊ\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (17, \"������ͨ\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (18, \"��Ʊ\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (19, \"����\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (20, \"�Լݳ�����\", 6);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (21, \"��˰\", 7);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (22, \"�̲�\", 8);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (23, \"ѧ��\", 8);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (24, \"��ķ\", 9);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (25, \"Сʱ��\", 9);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (26, \"���\", 10);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (27, \"����\", 10);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (28, \"�Ҿ���Ʒ\", 11);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (29, \"�����Ʒ\", 11);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (30, \"����\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (31, \"��ʳ\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (32, \"����\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (33, \"ʳ��\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (34, \"�߲�\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (35, \"��ʳ\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (36, \"ˮ��\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (37, \"�����ʳ\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (38, \"�̾�\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (39, \"����\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (40, \"���⵰��\", 12);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (41, \"�̻���\", 13);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (42, \"�ֻ���\", 13);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (43, \"����\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (44, \"����\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (45, \"�鱨��־����\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (46, \"Ӱ��\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (47, \"��Ϸ\", 14);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (48, \"����\", 15);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (49, \"ȡů��\", 15);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (50, \"ˮ������\", 15);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (51, \"��ҵ��\", 15);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (52, \"����\", 16);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (53, \"����\", 16);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (54, \"����\", 17);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (55, \"���ⷿ��\", 17);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (56, \"��Ʊ����\", 17);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (57, \"����\", 17);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (58, \"��Ϣ\", 17);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (59, \"ת��֧��\", 18);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (60, \"ת������\", 19);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (61, \"���ÿ�����\", 20);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (62, \"���ÿ�֧��\", 21);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (63, \"����\", 17);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (64, \"����\", 22);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (65, \"δ����֧��\", 23);");
		db.execDML("INSERT INTO sysCategory2 (id, Name, sysCategory1_id) VALUES (66, \"δ��������\", 24);");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 67, \"CATA420\", 1)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 68,\"CATA420\",2)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 69,\"CATA420\",3)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 70,\"CATA420\",4)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 71,\"CATA420\",5)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 72,\"CATA420\",6)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 73,\"CATA420\",7)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 74,\"CATA420\",8)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 75,\"CATA420\",9)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 76,\"CATA420\",10)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 77,\"CATA420\",11)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 78,\"CATA420\",12)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 79,\"CATA420\",13)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 80,\"CATA420\",14)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 81,\"CATA420\",15)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 82,\"CATA420\",16)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 83,\"CATA420\",17)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 84,\"CATA420\",22)");*/

		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20067, \"CATA420\", 1)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20068,\"CATA420\",2)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20069,\"CATA420\",3)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20070,\"CATA420\",4)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20071,\"CATA420\",5)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20072,\"CATA420\",6)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20073,\"CATA420\",7)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20074,\"CATA420\",8)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20075,\"CATA420\",9)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20076,\"CATA420\",10)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20077,\"CATA420\",11)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20078,\"CATA420\",12)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20079,\"CATA420\",13)");
		db.execDML("insert into sysCategory2 (id,name,sysCategory1_id) values( 20080,\"CATA420\",14)");

	}

};
