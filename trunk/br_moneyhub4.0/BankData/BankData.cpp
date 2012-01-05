// BankData.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "BankData.h"
#include <shlobj.h>
#include <tchar.h>
#include <atlconv.h>
#include <time.h>
#include <algorithm>
#include <assert.h>
#include <atlstr.h>
#include <atltime.h>
#include "../Utils/RecordProgram/RecordProgram.h"
#include "../include/ConvertBase.h"
#include "../Utils/SecurityCache/comm.h"

CRITICAL_SECTION CBankData::m_cs;

CBankData::CBankData()
{
	m_eGetIDMode = egSingle;//Ĭ��ģʽ�£�����id��ԭ��Ϊ��һ��
	m_initID = 0;
	InitializeCriticalSection(&m_cs);
	InitSysDbTempFile();
	if(m_UserIDMap.size() != 0)
         m_UserIDMap.clear();
}

CBankData::~CBankData()
{
	DeleteCriticalSection(&m_cs);
	/*TranslateSourceFile2DestinationFile ((LPSTR)m_strUserTpDbPath.c_str(), (LPSTR)m_strUserSourDbPath.c_str());
	DeleteFileA (m_strUserTpDbPath.c_str());
	DeleteFileA(m_strSysTpDbPath.c_str());*/
}
void CBankData::SetCurrency(string info)
{
	if(info.size() <= 30)
		return;

	ObjectLock lock(this);

	TiXmlDocument xmlDoc;

	xmlDoc.Parse(info.c_str()); 

	if (xmlDoc.Error())
	{
		CRecordProgram::GetInstance()->FeedbackError(L"Common", xmlDoc.Error(), L"TiXmlDocument������ʧ��");
		return;
	}

	const TiXmlNode* pRoot = xmlDoc.FirstChild("exchangerate"); // ANSI string 
	if (NULL == pRoot)
		return;

	CppSQLite3Buffer bufSQL;
	
	try{
		int sysCurrency_id1 = 2;
		for (const TiXmlNode *pCurrency = pRoot->FirstChild("currency"); pCurrency != NULL; pCurrency = pRoot->IterateChildren("currency", pCurrency))
		{
			string rate = pCurrency->FirstChild("rate")->FirstChild()->Value(); 
			string updatetime = pCurrency->FirstChild("updatetime")->FirstChild()->Value(); 


			bufSQL.format("Update datExchangeRate SET ExchangeRate=%s Where sysCurrency_id1=%d;", rate.c_str(), sysCurrency_id1);
			GetDataDbObject()->execDML(bufSQL);
			sysCurrency_id1 ++;
			
		}
		xmlDoc.Clear();
	}
	catch(CppSQLite3Exception&)
	{
		return;
	}

	 
}
int CBankData::GetAllUserTodayAlarmsNumber(vector<DATUSEREVENT>& tEvent)
{
	ObjectLock lock(this);

	CppSQLite3Buffer bufSQL;
	try
	{
		//////////////////////////////////////////////////////////////////////////
		// ��ý����ʱ��

		SYSTEMTIME systime;
		GetSystemTime(&systime);

		struct tm tmVal;
		memset(&tmVal, 0, sizeof(struct tm));
		tmVal.tm_year = systime.wYear - 1900;
		tmVal.tm_mon = systime.wMonth - 1;
		tmVal.tm_mday = systime.wDay;

		__time32_t tToday = _mktime32(&tmVal);

		//////////////////////////////////////////////////////////////////////////
		// From tbTotalEvent
		bufSQL.format("SELECT userid, mail, autoinfo FROM datUserInfo;");

		USES_CONVERSION;
		CppSQLite3Query q2 = GetDataDbObject()->execQuery(bufSQL);
		while(!q2.eof())
		{
			DATUSEREVENT userevent;
			userevent.rTime = 2147483647;//long ���ֵ
			userevent.iNotify = 0;
			userevent.iRemind = 0;
			string userid = q2.getStringField("userid");
			if( userid != "")
			{
				string account = q2.getStringField("mail");
				userevent.account = A2W(account.c_str());
				userevent.bRemind = (q2.getIntField("autoinfo") == 1) ? true:false;

				if (GetDataDbObject()->tableExists("tbTotalEvent"))
				{
					bufSQL.format("SELECT * FROM tbTotalEvent WHERE mark=0 AND status=1 AND datUserInfo_userid='%s' ORDER BY event_date;",userid.c_str());

					CppSQLite3Query q = GetDataDbObject()->execQuery(bufSQL);
					while (!q.eof())
					{
						INT64 id = q.getInt64Field("id");
						__time32_t event_date = q.getIntField("event_date");
						int repeat1 = q.getIntField("repeat");
						int alarm = q.getIntField("alarm");

						__time32_t alarm_date;

						if (IsAlarmEvent(tToday, event_date, repeat1, alarm, &alarm_date))
						{
							if(event_date <  userevent.rTime)
								userevent.rTime = event_date;//��¼�¼���ʱ������ͬ��
							userevent.iRemind ++;
						}

						q.nextRow();
					}							
					q.finalize();

					tEvent.push_back(userevent);
				}
			}
			q2.nextRow();
		}
		q2.finalize();

	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"GetAllUserTodayAlarmsNumber error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return 0;
	}
	return 1;

}

void CBankData::SetUserAutoLogin(wstring mail)
{
	char bufSQL[256] = { 0 };
	USES_CONVERSION;
	string smail = W2A(mail.c_str());

	sprintf_s(bufSQL, 256, "SELECT autoload FROM datUserInfo WHERE mail='%s';", smail.c_str());
	CppSQLite3Query q2 = GetDataDbObject()->execQuery(bufSQL);
	if(!q2.eof())
	{
		int autoload = q2.getIntField("autoload");
		if(autoload != 1)
		{
			sprintf_s(bufSQL, 256, "UPDATE datUserInfo SET autoload=0;");
			GetDataDbObject()->execDML(bufSQL);
		}
		sprintf_s(bufSQL, 256, "UPDATE datUserInfo SET popLoad=0;");
		GetDataDbObject()->execDML(bufSQL);

		sprintf_s(bufSQL, 256, "UPDATE datUserInfo SET popLoad=1 WHERE mail='%s';", smail.c_str());
		GetDataDbObject()->execDML(bufSQL);
	}
	q2.finalize();
	return;

}
void CBankData::SetUserAlarmsConfig(vector<DATUSEREVENT>& tEvent)
{
	USES_CONVERSION;
	vector<DATUSEREVENT>::iterator ite = tEvent.begin();
	CppSQLite3Buffer bufSQL;
	for(;ite != tEvent.end(); ite ++)
	{
		int choose = ((*ite).bRemind == true) ? 1:0;
		bufSQL.format("UPDATE datUserInfo SET autoinfo=%d WHERE mail='%s';", choose, W2A((*ite).account.c_str()));
		GetDataDbObject()->execDML(bufSQL);
	}
}

CBankData* CBankData::GetInstance()
{
	static CBankData bank_data;
	return &bank_data;
}

void CBankData::split(std::string& s, std::string& delim,std::vector< std::string >* ret)
{
	size_t last = 0;
	size_t index=s.find_first_of(delim,last);
	while (index!=std::string::npos)
	{
		ret->push_back(s.substr(last,index-last));
		last=index+1;
		index=s.find_first_of(delim,last);
	}
	if (index-last>0)
	{
		ret->push_back(s.substr(last,index-last));
	}
} 

bool CBankData::ReOpenDB(std::string strUsID)
{
	if(strUsID == "")//Ĭ�ϴ��û�Guest���ݿ�
	{
		try
		{
			m_dbUser.close();
			m_dbUser.open(GetDbPath());
			return true;
		}
		catch (CppSQLite3Exception& ex)
		{
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"OpenDb error:%d", ex.errorCode()));
			if(ex.errorMessage() != NULL)
				CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
			return false;
		}
	}
	else
	{
		try
		{
			m_dbUser.close();
			m_dbUser.open(GetDbPath(strUsID));
			return true;
		}
		catch (CppSQLite3Exception& ex)
		{
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"OpenDb error:%d", ex.errorCode()));
			if(ex.errorMessage() != NULL)
				CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
			return false;
		}
	}
	return true;
}
bool CBankData::OpenDB(std::string strUsID)
{
	if(strUsID == "")//Ĭ�ϴ��û�Guest���ݿ�
	{
		try
		{
			m_dbUser.open(GetDbPath());
			return true;
		}
		catch (CppSQLite3Exception& ex)
		{
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"OpenDb error:%d", ex.errorCode()));
			if(ex.errorMessage() != NULL)
				CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
			return false;
		}
	}
	return true;	
}
void CBankData::CloseDB(std::string strUsID)
{
	if(strUsID == "")
	{
		try
		{
			m_dbUser.close();
		}
		catch(CppSQLite3Exception& ex)
		{
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"CloseDb error:%d", ex.errorCode()));
			if(ex.errorMessage() != NULL)
				CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
			return;
		}
	}

}
bool CBankData::CheckUserDB()
{	
	try{
		m_dbUser.checkDB();
		return true;
	}
	catch(...)
	{		
		return OpenDB();
	}
}

// �û������϶������ղ�˳��
bool CBankData::ChangeOrder(const char* pBankID, int nTo, int nFrom)
{
	ATLASSERT (nTo > 0 && nFrom > 0 && pBankID != NULL);
	if (nTo <= 0 || nFrom <= 0 || pBankID == NULL)
		return false;

	if (nTo == nFrom)
		return false;

	int nTempTo = -1;
	int nTempFrom = -1;

	if(!CheckUserDB())
		return false;
	
	// ��ȡ���ݿ������еļ�¼��
	ObjectLock lock(this);
	try
	{
		CppSQLite3Binary blob;
		CppSQLite3Query q = m_dbDataDB.execQuery("SELECT * FROM datFav order by favorder desc;");
		
		for (int i = 1; !q.eof (); i ++, q.nextRow ())
		{
			if (nTo == i)
			{
				nTempTo = q.getIntField ("favorder");
			}
			if (nFrom == i)
			{
				nTempFrom = q.getIntField ("favorder");
			}
		}

		q.finalize();
			
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"ChangeOrder1 error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}
	if (-1 == nTempTo || -1 == nTempFrom)
		return false;
	nTo = nTempTo; // �õ�nTo������Ӧ��favorder��ֵ
	nFrom = nTempFrom;


	CppSQLite3Buffer bufSQL1, bufSQL2;

	if (nTo <  nFrom)
	{
		bufSQL1.format("update datFav set favorder = favorder + 1 where favorder between %d and %d;", nTo, nFrom);
		bufSQL2.format ("update datFav set favorder = %d where favinfo = '%s';", nTo, pBankID); 
	}
	else
	{
		bufSQL1.format("update datFav set favorder = favorder - 1 where favorder between '%d' and '%d';", nFrom, nTo);
		bufSQL2.format ("update datFav set favorder = %d where favinfo = '%s';", nTo, pBankID); 
	}

	try
	{

		m_dbDataDB.execDML(bufSQL1);
		m_dbDataDB.execDML(bufSQL2);
		
		return true;
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"ChangeOrder2 error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}
}

bool CBankData::CreateNewUserDB(LPSTR lpstrUserDBName, LPSTR lpPassword, int nLen)
{
	string strFilePath = m_strUtfDataPath + lpstrUserDBName;

	wstring strPath = m_strDataPath;
	strPath += CA2W(lpstrUserDBName);
	// ��ϵͳ���ݿ��п���
	CopyFile(m_strGuestTemplete.c_str(), strPath.c_str(), false);

	// �������ݿ�����
	CppSQLite3DB tempDB;
	if (!tempDB.InitDbPassword(strFilePath.c_str(), lpPassword, nLen))
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, L"Init new datadb password error");
	}
	tempDB.close();

	return true;
}

void CBankData::CreateUserDbFromGuestDb(LPSTR lpstrUserDBName, LPSTR lpPassword, int nLen)
{
	string strFilePath = m_strUtfDataPath + lpstrUserDBName;
	/*string strGuestPath = m_strUtfDataPath + MONHUB_GUEST_USERID;
	strGuestPath += ".dat";*/

	wstring strPath = m_strDataPath;
	strPath += CA2W(lpstrUserDBName);

	wstring strGPath = m_strDataPath;
	strGPath += CA2W(MONHUB_GUEST_USERID);
	strGPath += L".dat";

	// �ر��û���
	m_dbUser.close();

	// ����Guest.dat
	CopyFile(strGPath.c_str(), strPath.c_str(), false);
	DeleteFile(strGPath.c_str());

	// ��ϵͳ���ݿ��п���
	CopyFile(m_strGuestTemplete.c_str(), strGPath.c_str(), false);

	// �������ݿ�����
	CppSQLite3DB tempDB;
	if (!tempDB.InitDbPassword(strFilePath.c_str(), lpPassword, nLen))
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, L"Init new datadb password error");
	}

	// ����DataDb���е�datUserInfo_userid
	string strUserID = lpstrUserDBName;
	// ȥ��.dat
	strUserID = strUserID.substr(0, strUserID.length() - 4);

	string strSQL = "update tbTotalEvent set datUserInfo_userid = '";
	strSQL += strUserID;
	strSQL += "' where datUserInfo_userid = '";
	strSQL += MONHUB_GUEST_USERID;
	strSQL += "'";
	
	string strQuery = QuerySQL(strSQL, "DataDB");
	tempDB.close();
}

bool CBankData::SetCurrentUserDB(LPSTR lpstrUserDBName, LPSTR lpPassword, int nLen)
{
	USES_CONVERSION;
	ATLASSERT(NULL != lpstrUserDBName);
	if (NULL == lpstrUserDBName)
		return false;

	try
	{
		m_dbUser.close();

		string strFilePath = m_strUtfDataPath + lpstrUserDBName;

		// ���µ�ǰ�û����·��
		m_strUtfUserDbPath = strFilePath;
		m_strUserDbPath = m_strDataPath;
		m_strUserDbPath += CA2W(lpstrUserDBName);

		m_dbUser.open(strFilePath.c_str(), lpPassword, nLen);

	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"OpenDb error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

	try
	{
		// �������ݿ��Ƿ�ɹ���
		m_dbUser.execQuery("select * from tbBank");
	}
	catch(CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, L"SetCurrentUserDB error:tbBank" );
		return false;
	}

	return true;
}

bool CBankData::IsUserDBExist(LPSTR lpstrUserDBName)
{
	wstring strPath = m_strDataPath;
	strPath += CA2W(lpstrUserDBName);
	return IsFileExist((LPWSTR)strPath.c_str()); // �����ǰ�û����ݿⲻ����
}

// �޸ĵ�ǰ�û�����
bool CBankData::ChangeUserDBPwd(const LPSTR lpstrUserDBName, const char* pOldPwd, const int nOldLen, const char* pNewPwd, const int nNewLen)
{
	ATLASSERT(NULL != pOldPwd && NULL != pNewPwd && NULL != lpstrUserDBName);
	if (NULL == pOldPwd || NULL == pNewPwd || NULL == lpstrUserDBName)
		return false;

	string strFilePath = m_strUtfDataPath + lpstrUserDBName;
	
	CppSQLite3DB tempDB;
	// �ѿ����ݿ�
	tempDB.open(strFilePath.c_str(), pOldPwd, nOldLen);

	// �޸�����
	tempDB.ChangePassword(pOldPwd, nOldLen, pNewPwd, nNewLen);
	tempDB.close();


	return true;
}

void CBankData::InitSysDbTempFile(void) // ��ʼ��ϵͳ���ݿ���ʱ�ļ�
{
	if (m_strUtfSysDbPath.empty())
	{
		CString strTemp;
		if (0 == GetModuleFileName(NULL, strTemp.GetBuffer(1024), 1023))
			return;

		strTemp.ReleaseBuffer();
		if (strTemp.IsEmpty())
			return;

		strTemp = strTemp.Left (strTemp.ReverseFind ('\\'));
		std::string strCurrentPath = CW2A(strTemp);

		m_strGuestTemplete = strTemp + L"\\Config\\Guest.dat";
		m_strSysDbPath = strTemp + L"\\Config\\SysDB.dat";

		//::WideCharToMultiByte(CP_UTF8, 0, strTemp, strTemp.GetLength(), szDbPath, 1024, NULL,FALSE);

		WCHAR tempPath[MAX_PATH] = { 0 };
		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tempPath);
		wcscat_s(tempPath, L"\\MoneyHub");
		::CreateDirectoryW(tempPath, NULL);

		m_strDataDbPath = tempPath;
		m_strDataDbPath += L"\\DataDB.dat";
	
		char szDbPath[1024] = { 0 };
		int srcCount = wcslen(m_strDataDbPath.c_str());
		srcCount = (srcCount > MAX_PATH) ? MAX_PATH : srcCount;
		::WideCharToMultiByte(CP_UTF8, 0, m_strDataDbPath.c_str(), srcCount, szDbPath, 1024, NULL,FALSE);

		m_strUtfDataDbPath = szDbPath;

		memset(szDbPath, 0, 256);
		srcCount = wcslen(m_strSysDbPath.c_str());
		srcCount = (srcCount > MAX_PATH) ? MAX_PATH : srcCount;
		::WideCharToMultiByte(CP_UTF8, 0, m_strSysDbPath.c_str(), srcCount, szDbPath, 1024, NULL,FALSE);

		m_strUtfSysDbPath = szDbPath;

		// �����ʽ���ݿⲻ����
		if (!IsFileExist((LPWSTR)m_strSysDbPath.c_str()))
		{
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, L"SysDB is not exist, exit progrma.");
			exit(0);
		}
		

		if (!IsFileExist ((LPWSTR)m_strDataDbPath.c_str()))
		{
			HANDLE hFile = CreateFileW(m_strDataDbPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
			if (INVALID_HANDLE_VALUE == hFile)
				return;

			CloseHandle(hFile);
			// û�оʹ���һ��
			CreateDataDBFile((LPSTR)m_strUtfDataDbPath.c_str());
		}
	}

	GetDataDbObject();
	GetSysDbObject();
}

int CBankData::GetDBVersion(CppSQLite3DB& db)
{
	CppSQLite3Query q = db.execQuery("SELECT schema_version FROM tbDBInfo;");
	int nVer = q.getIntField("schema_version");
	q.finalize();

	return nVer;
}

bool CBankData::CopyDataFromOldVersoion(CppSQLite3DB& dbSour, CppSQLite3DB& dbDesc)
{
	if (dbSour.tableExists("tbFav"))
	{
		CpyDbTb2OtherDbTb(dbSour, dbDesc,"tbFav", "tbFav");
		CpyDbTb2OtherDbTb(dbSour, dbDesc,"tbEvent", "tbEvent"); // ���������ǲ���һģһ��?
	}
	return true;
}
bool CBankData::UpdateDB7to8(wstring wFilePath, string sUtfFilePath)
{
	try{

		m_dbDataDB.close();
		//Ӧ��������data.db�⣡��
		if(IsFileExist((LPWSTR)m_strDataDbPath.c_str()))
			DeleteFileW(m_strDataDbPath.c_str());
		int i = GetLastError();

		CreateDataDBFile((LPSTR)m_strUtfDataDbPath.c_str());
		CppSQLite3DB dbData;
		dbData.open(m_strUtfDataDbPath.c_str());

		// ����guest.dat��
		std::string sUTFGuestDB = sUtfFilePath + "Guest.dat"; // ��6������7�汾���ݿ�Ĳ���
		std::wstring sGuestDB = wFilePath + L"Guest.dat";
		CppSQLite3DB dbDesc;
		dbDesc.open(sUTFGuestDB.c_str());//Դ

		std::wstring sTempDB = wFilePath + L"~~~~tempGuest.dat";
		std::string sUTFTempDB = sUtfFilePath + "~~~~tempGuest.dat";

		CopyFileW(m_strGuestTemplete.c_str(), sTempDB.c_str(), FALSE);

		CppSQLite3DB dbTemp;
		dbTemp.open(sUTFTempDB.c_str());
		SetGetIDMode(egBatch);

		//1������tbBank��
		CppSQLite3Query q1;
		q1 = dbDesc.execQuery("SELECT * FROM tbBank;");
		dbTemp.execDML("DELETE FROM tbBank;");

		std::map<int, INT64> maptbBank;
		std::map<int, INT64> maptbCategory1;
		/*`id` INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,\
			`name` VARCHAR(256)  NOT NULL,\
			`classId` INTEGER NOT NULL,\
			`BankID` VARCHAR(4)  NULL,\
			`Phone` VARCHAR(256)  NULL,\
			`Website` vARCHAR(256)  NULL);");*/
		char BufSQL[2560] = {0};
		int seq = 1;
		while(!q1.eof())
		{
			int id = q1.getIntField("id");
			INT64 nID = id;
			INT64 nUT = 0;
			if( id >= 10000)//��bank��С��10000�ľ�ΪĬ��ֵ
			{
				nID = GetNewID();
				nUT = GetNewUT();
			}
			string name = q1.getStringField("name", "");
			int classId = q1.getIntField("classId");
			string BankID = q1.getStringField("BankID");
			string Phone = q1.getStringField("Phone");
			string Website = q1.getStringField("Website");
			int order = seq;//����ô������Ĭ��ֵ���ټӽ���
			seq ++;

			maptbBank.insert(std::make_pair(id, nID));
			sprintf_s(BufSQL, 2560, "INSERT INTO tbBank(id, UT, mark, name, classId, BankID, Phone, Website) VALUES(%I64d, %I64d, 0, '%s', %d, '%s','%s','%s');",
				nID, nUT, name.c_str(), classId, BankID.c_str(), Phone.c_str(), Website.c_str());
			dbTemp.execDML(BufSQL);

			q1.nextRow();
		}
		q1.finalize();

		// 2 tbCategory1��
		/* `id` INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT ,\
		`Name` VARCHAR(256) NULL ,\
		`Type` TINYINT NULL);");*/
		dbDesc.execDML("DELETE FROM tbCategory1 WHERE id=15;");
		dbDesc.execDML("INSERT INTO tbCategory1 (id, Name, Type) VALUES (15, \"��������\", 0);");
		q1 = dbDesc.execQuery("SELECT * FROM tbCategory1;");
		//dbTemp.execDML("DELETE FROM tbCategory1;");
		std::vector<int>  m_mapDelete;

		for(int i = 1; i <= 15; i ++)
			m_mapDelete.push_back(i);		

		seq = 1;
		while(!q1.eof())
		{
			int id = q1.getIntField("id");
			std::vector<int>::iterator mite = std::find(m_mapDelete.begin(),m_mapDelete.end(), id);
			if(mite != m_mapDelete.end())
				m_mapDelete.erase(mite);

			INT64 nID = id;
			INT64 nUT = 0;
			if( id > 10025)//��tbCategory�е�ΪĬ��ֵ
			{
				nID = GetNewID();
				nUT = GetNewUT();
			}
			if( id < 10000)//��tbCategory�е�ΪĬ��ֵ
			{				
				nUT = GetNewUT();
			}
			string Name = q1.getStringField("Name", "");
			int Type = q1.getIntField("Type");
			int order = 1000;//����ô������Ĭ��ֵ���ټӽ���
			switch(id)
			{
				case 1: order = 10;break;
				case 2: order = 30;break;
				case 3: order = 40;break;
				case 4: order = 50;break;
				case 5: order = 60;break;
				case 6: order = 70;break;
				case 7: order = 80;break;
				case 8: order = 90;break;
				case 9: order = 100;break;
				case 10: order = 110;break;
				case 11: order = 120;break;
				case 12: order = 130;break;
				case 13: order = 140;break;
				case 14: order = 150;break;
				case 15: order = 20;break;
				default:order = 1000;break;
			}

			maptbCategory1.insert(std::make_pair(id, nID));
			if(id > 10025)
			{
				sprintf_s(BufSQL, 2560, "INSERT INTO tbCategory1(id, UT, mark, Name, Type, seq) VALUES(%I64d, %I64d, 0, '%s', %d, %d);",
					nID, nUT, Name.c_str(), Type, order);
				dbTemp.execDML(BufSQL);

			}
			else if(id < 10000)
			{
				sprintf_s(BufSQL, 2560, "Update tbCategory1 SET UT=%I64d, Name='%s' Where id =%d;",
					nUT, Name.c_str(), nID);
				dbTemp.execDML(BufSQL);
			}			

			q1.nextRow();
		}
		q1.finalize();

		std::vector<int>::iterator itemapDelete = m_mapDelete.begin();
		for(; itemapDelete != m_mapDelete.end(); itemapDelete ++)
		{
			INT64 nUT =  GetNewUT();
			sprintf_s(BufSQL, 2560, "Update tbCategory1 SET UT=%I64d, mark=1 Where id =%d;",
				nUT, (*itemapDelete));
			dbTemp.execDML(BufSQL);

			sprintf_s(BufSQL, 2560, "Update tbCategory2 SET UT=%I64d, mark=1 Where tbCategory1_id =%d;",
				nUT, (*itemapDelete));
			dbTemp.execDML(BufSQL);
		}

		
		// 3 tbAccountType�� ���ö�

		// 4 tbAccount��
		/*`id` INTeger  NOT NULL PRIMARY KEY AUTOINCREMENT ,\
		`Name` VARCHAR(256) NOT NULL ,\
		`tbBank_id` INT(11)  NULL ,\
		`tbAccountType_id` INT(11)  NOT NULL ,\
		`AccountNumber` VARCHAR(256) NULL ,\
		`Comment` VARCHAR(256) NULL,\
		`keyInfo` varchar(255));*/
		std::map<int, INT64> maptbAccount;
		q1 = dbDesc.execQuery("SELECT * FROM tbAccount;");
		dbTemp.execDML("DELETE FROM tbAccount;");

		seq = 1;
		while(!q1.eof())
		{
			int id = q1.getIntField("id");
			INT64 nID = 0;
			INT64 nUT = 0;
			nID = GetNewID();
			nUT = GetNewUT();

			string Name = q1.getStringField("Name");
			int Bankid = q1.getIntField("tbBank_id");
			std::map<int, INT64>::iterator ite = maptbBank.find(Bankid);
			INT64 tbBank_id = 0;
			if(ite != maptbBank.end())
				tbBank_id = (*ite).second;

			int tbAccountType_id = q1.getIntField("tbAccountType_id");
			string AccountNumber = q1.getStringField("AccountNumber");
			string Comment = q1.getStringField("Comment");
			int order = seq;
			seq ++;
			string keyInfo = q1.getStringField("keyInfo");

			// EndDate������tbEvent��ʱ��ͳһ����			

			maptbAccount.insert(std::make_pair(id, nID));
			if(tbAccountType_id == 2)//���ÿ�EndDateĬ�ϸ�Ϊzdhk
				sprintf_s(BufSQL, 2560, "INSERT INTO tbAccount(id, UT, mark, Name, tbBank_id, tbAccountType_id, AccountNumber, Comment, EndDate, keyInfo) VALUES(%I64d, %I64d, 0, '%s', %I64d, %d, '%s','%s', 'zdhk', '%s');",
				nID, nUT, Name.c_str(), tbBank_id, tbAccountType_id, AccountNumber.c_str(), Comment.c_str(), keyInfo.c_str());
			else
				sprintf_s(BufSQL, 2560, "INSERT INTO tbAccount(id, UT, mark, Name, tbBank_id, tbAccountType_id, AccountNumber, Comment, EndDate, keyInfo) VALUES(%I64d, %I64d, 0, '%s', %I64d, %d, '%s','%s', 0, '%s');",
				nID, nUT, Name.c_str(), tbBank_id, tbAccountType_id, AccountNumber.c_str(), Comment.c_str(), keyInfo.c_str());
			dbTemp.execDML(BufSQL);

			q1.nextRow();
		}
		q1.finalize();

		// 5 tbCategory2��
		/* `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT ,\
		`Name` VARCHAR(256) NULL ,\
		`tbCategory1_id` INT(11)  NOT NULL);*/
		std::map<int, INT64> maptbCategory2;
		dbDesc.execDML("DELETE FROM tbCategory2 WHERE id IN (84,85,86,87,88,89);");
		dbDesc.execDML("INSERT INTO tbCategory2 (id, Name, tbCategory1_id) VALUES (84, \"��װ\", 15);");
		dbDesc.execDML("INSERT INTO tbCategory2 (id, Name, tbCategory1_id) VALUES (85, \"Ьñ���\", 15);");
		dbDesc.execDML("INSERT INTO tbCategory2 (id, Name, tbCategory1_id) VALUES (86, \"��Ʒ\", 15);");
		dbDesc.execDML("INSERT INTO tbCategory2 (id, Name, tbCategory1_id) VALUES (87, \"��ױƷ\", 15);");
		dbDesc.execDML("INSERT INTO tbCategory2 (id, Name, tbCategory1_id) VALUES (88, \"��������\", 15);");
		dbDesc.execDML("INSERT INTO tbCategory2 (id, Name, tbCategory1_id) VALUES (89, \"����\", 15);");
		//dbDesc.execDML("INSERT INTO tbCategory2 (id, name, tbCategory1_id) VALUES (90, \"CATA420\",15);");
		q1 = dbDesc.execQuery("SELECT * FROM tbCategory2;");
		//dbTemp.execDML("DELETE FROM tbCategory2;");
		seq = 1;
		while(!q1.eof())
		{
			int id = q1.getIntField("id");
			INT64 nID = id;
			INT64 nUT = 0;
			if(id > 20080)
			{
				nID = GetNewID();
				nUT = GetNewUT();
			}
			else
				nUT = GetNewUT();

			string Name = q1.getStringField("Name");
			int tbCategory1id = q1.getIntField("tbCategory1_id");
			std::map<int, INT64>::iterator ite = maptbCategory1.find(tbCategory1id);
			INT64 tbCategory1_id = tbCategory1id;
			if(ite != maptbCategory1.end())
				tbCategory1_id = (*ite).second;
			int order = seq;
			seq ++;		

			maptbCategory2.insert(std::make_pair(id, nID));
			if(id > 20080)
			{
				sprintf_s(BufSQL, 2560, "INSERT INTO tbCategory2(id, UT, mark, Name, tbCategory1_id) VALUES(%I64d, %I64d, 0, '%s', %I64d);",
					nID, nUT, Name.c_str(), tbCategory1_id);
				dbTemp.execDML(BufSQL);
			}
			else
			{
				sprintf_s(BufSQL, 2560, "UPDATE tbCategory2 SET UT=%I64d, Name='%s', tbCategory1_id=%I64d where  id=%I64d;",
					nUT, Name.c_str(), tbCategory1_id,nID);
				dbTemp.execDML(BufSQL);
			}
			
			q1.nextRow();
		}
		q1.finalize();

		sprintf_s(BufSQL, 2560, "UPDATE tbCategory2 SET UT=%I64d where id=20081;", GetNewUT());
		dbTemp.execDML(BufSQL);


		// 6 tbPayee��
		/*`id` INTEGER NOT NULL PRIMARY KEY  AUTOINCREMENT,\
		`Name` VARCHAR(256) NOT NULL,\
		`email` VARCHAR(256) ,\
		`tel` VARCHAR(256));*/
		std::map<int, INT64> maptbPayee;
		q1 = dbDesc.execQuery("SELECT * FROM tbPayee;");
		dbTemp.execDML("DELETE FROM tbPayee;");

		seq = 1;
		while(!q1.eof())
		{
			int id = q1.getIntField("id");
			INT64 nID = 0;
			INT64 nUT = 0;
			nID = GetNewID();
			nUT = GetNewUT();

			string Name = q1.getStringField("Name");
			string email = q1.getStringField("email");
			string tel = q1.getStringField("tel");
			int order = seq;
			seq ++;

			maptbPayee.insert(std::make_pair(id, nID));
			sprintf_s(BufSQL, 2560, "INSERT INTO tbPayee(id, UT, mark, Name, email, tel) VALUES(%I64d, %I64d, 0, '%s', '%s', '%s');",
				nID, nUT, Name.c_str(), email.c_str(), tel.c_str());
			dbTemp.execDML(BufSQL);

			q1.nextRow();
		}
		q1.finalize();

		// 7 tbSubAccount��
		/*`id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\
		`tbAccount_id` INT(11)  NOT NULL ,\
		`tbCurrency_id` INT(11)  NOT NULL ,\
		`name` VARCHAR(256) NOT NULL,\
		`OpenBalance` FLOAT NOT NULL ,\
		`Balance` FLOAT NOT NULL ,\
		`Days` INT(11)  NULL ,\
		`EndDate` DATE NULL ,\
		`Comment` VARCHAR(256) NULL ,\
		`tbAccountType_id` INT(11) NULL);*/
		std::map<int, INT64> maptbSubAccount;
		q1 = dbDesc.execQuery("SELECT * FROM tbSubAccount;");
		dbTemp.execDML("DELETE FROM tbSubAccount;");

		seq = 1;
		while(!q1.eof())
		{
			int id = q1.getIntField("id");
			INT64 nID = 0;
			INT64 nUT = 0;
			nID = GetNewID();
			nUT = GetNewUT();
			int tbAccountid = q1.getIntField("tbAccount_id");
			INT64 tbAccount_id = 0;
			std::map<int, INT64>::iterator ite = maptbAccount.find(tbAccountid);
			if(ite != maptbAccount.end())
				tbAccount_id = (*ite).second;
			
			INT64 tbCurrency_id = q1.getIntField("tbCurrency_id");
			string name = q1.getStringField("name");
			float OpenBalance = q1.getFloatField("OpenBalance");
			float Balance = q1.getFloatField("Balance");
			int Days = q1.getIntField("Days");
			string EndDate = q1.getStringField("EndDate");
			string Comment = q1.getStringField("Comment");
			int tbAccountType_id = q1.getIntField("tbAccountType_id");
			int order = seq;
			seq ++;
			// EndDate������tbEvent��ʱ��ͳһ����			

			maptbSubAccount.insert(std::make_pair(id, nID));
			sprintf_s(BufSQL, 2560, "INSERT INTO tbSubAccount(id, UT, mark, tbAccount_id, tbCurrency_id, name, OpenBalance, Balance, Days, EndDate, Comment, tbAccountType_id) VALUES(%I64d, %I64d, 0, %I64d, %I64d, '%s',%.2f, %.2f, %d, '%s', '%s', %d);",
				nID, nUT, tbAccount_id, tbCurrency_id, name.c_str(), OpenBalance, Balance, Days, EndDate.c_str(), Comment.c_str(), tbAccountType_id);
			dbTemp.execDML(BufSQL);

			q1.nextRow();
		}
		q1.finalize();
		
		dbTemp.execDML("UPDATE tbSubAccount SET tbAccountType_id=301 WHERE tbAccount_id IN (SELECT id FROM tbAccount WHERE tbAccountType_id=1);");		
		dbTemp.execDML("UPDATE tbSubAccount SET tbAccountType_id=201 WHERE tbAccount_id IN (SELECT id FROM tbAccount WHERE tbAccountType_id=2);");		
		dbTemp.execDML("UPDATE tbSubAccount SET tbAccountType_id=304 WHERE tbAccount_id IN (SELECT id FROM tbAccount WHERE tbAccountType_id=4);");
		dbTemp.execDML("UPDATE tbSubAccount SET tbAccountType_id=305 WHERE tbAccount_id IN (SELECT id FROM tbAccount WHERE tbAccountType_id=5);");
		dbTemp.execDML("UPDATE tbSubAccount SET tbAccountType_id=306 WHERE tbAccount_id IN (SELECT id FROM tbAccount WHERE tbAccountType_id=6);");
		dbTemp.execDML("UPDATE tbSubAccount SET tbAccountType_id=307 WHERE tbAccount_id IN (SELECT id FROM tbAccount WHERE tbAccountType_id=7);");
		dbTemp.execDML("UPDATE tbSubAccount SET tbAccountType_id=308 WHERE tbAccount_id IN (SELECT id FROM tbAccount WHERE tbAccountType_id=8);");

		// 8 tbTransaction��
		/*CREATE  TABLE IF NOT EXISTS `tbTransaction` (\
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
		`isEdit` tinyint (1)DEFAULT 0 NOT NULL);*/
		std::map<int, INT64> maptbTransactionChange;//��¼ת�˼�¼
		q1 = dbDesc.execQuery("SELECT * FROM tbTransaction;");
		dbTemp.execDML("DELETE FROM tbTransaction;");

		seq = 1;
		while(!q1.eof())
		{
			int id = q1.getIntField("id");
			INT64 nID = 0;
			INT64 nUT = 0;
			nID = GetNewID();
			nUT = GetNewUT();

			string TransDate = q1.getStringField("TransDate");
			int tbPayeeid = q1.getIntField("tbPayee_id");
			INT64 tbPayee_id = 0;
			std::map<int, INT64>::iterator ite = maptbPayee.find(tbPayeeid);
			if(ite != maptbPayee.end())
				tbPayee_id = (*ite).second;

			int tbCategory2id = q1.getIntField("tbCategory2_id");
			INT64 tbCategory2_id = 0;
			std::map<int, INT64>::iterator ite2 = maptbCategory2.find(tbCategory2id);
			if(ite2 != maptbCategory2.end())
				tbCategory2_id = (*ite2).second;

			float Amount = q1.getFloatField("Amount");
			int direction = q1.getIntField("direction");
			if(direction != 0)
				maptbTransactionChange.insert(std::make_pair(id, nID));

			int tbSubAccountid = q1.getIntField("tbSubAccount_id");
			INT64 tbSubAccount_id = 0;
			std::map<int, INT64>::iterator ite3 = maptbSubAccount.find(tbSubAccountid);
			if(ite3 != maptbSubAccount.end())
				tbSubAccount_id = (*ite3).second;

			int tbSubAccountid1 = q1.getIntField("tbSubAccount_id1");
			INT64 tbSubAccount_id1 = 0;
			std::map<int, INT64>::iterator ite4 = maptbSubAccount.find(tbSubAccountid1);
			if(ite4 != maptbSubAccount.end())
				tbSubAccount_id1 = (*ite4).second;

			float ExchangeRate = q1.getFloatField("ExchangeRate");			
			string Comment = q1.getStringField("Comment");
			string sign = q1.getStringField("sign");
			int transactionClasses = q1.getIntField("transactionClasses");
			int order = seq;
			seq ++;

			sprintf_s(BufSQL, 2560, "INSERT INTO tbTransaction(id, UT, mark, TransDate, tbPayee_id, tbCategory2_id, Amount, direction, tbSubAccount_id, tbSubAccount_id1, ExchangeRate, Comment, sign, transactionClasses) VALUES(%I64d, %I64d, 0, '%s', %I64d, %I64d, %.2f, %d, %I64d, %I64d, %.2f, '%s', '%s', %d);",
				nID, nUT, TransDate.c_str(), tbPayee_id, tbCategory2_id, Amount, direction, tbSubAccount_id, tbSubAccount_id1, ExchangeRate, Comment.c_str(), sign.c_str(), transactionClasses);
			dbTemp.execDML(BufSQL);

			q1.nextRow();
		}
		q1.finalize();

		std::map<int, INT64>::iterator mtcIte = maptbTransactionChange.begin();
		for(;mtcIte != maptbTransactionChange.end(); mtcIte ++)
		{
			sprintf_s(BufSQL, 2560, "UPDATE tbTransaction SET direction=%I64d WHERE direction=%d;", mtcIte->second, mtcIte->first);
			dbTemp.execDML(BufSQL);
		}

		// 9 tbEvent��
		/*id INT, event_date INT, description CHAR(256), repeat INT, alarm INT, status INT, datestring CHAR(12));*/
		q1 = dbDesc.execQuery("SELECT * FROM tbEvent;");
		dbTemp.execDML("DELETE FROM tbEvent;");

		while(!q1.eof())
		{
			int id = q1.getIntField("id");
			INT64 nID = 0;
			INT64 nUT = 0;
			nID = GetNewID();
			nUT = GetNewUT();

			string datestring = q1.getStringField("datestring");
			int event_date = q1.getIntField("event_date");
			string description = q1.getStringField("description");
			int alarm = q1.getIntField("alarm");
			int repeat = q1.getIntField("repeat");
			int status = q1.getIntField("status");
			INT64 tbAccount_id = 0;
			INT64 tbSubAccount_id = 0;
			int type = 0;
			/*3.1����
			һ��
				��tbEvent������Ҫͨ��description�ֶν��в���
				description���˻��� + "����" ���
				1.����Ͷ�����˻�:
				description = ���˻��� + " " + "����"
				2.�������ÿ��˻���
				�˵����¼�Ϊ
				description = ���˻��� + " " + "�˵���\t" + "����"
				�������¼�Ϊ
				description = ���˻��� + " " + "�˵����ڻ���\t\t" + "����"
				3.���ڴ��/�����˻�
				description = ���˻��� + " " + ���˻��� + "����"

				����
				1������Ͷ�����˻��ʹ��/�����˻��ĵ����մ洢��tbsubAccount���е�enddate(date)�ֶ���
				tbsubAccount���е�enddate��tbEvent���е�datestring(varchar)��ǰ10λ���Ӧ   
				����enddate = 2012-12-05
				��ӦdatestringΪ 2012-12-05 1������1ָ����һ

				============�¼�>>>>>>>����Ͷ�����˻���typeֵΪ5��
				============�¼�>>>>>>>   ���ڴ��/�����˻���typeֵΪ3

				2���������ÿ��˻������մ洢��tbAccount���е�enddate(char(4))�ֶ�   tbAccount���е�enddate��
				tbEvent����һ���������������Ӧ��enddateǰ��λ��ʾ�˵��գ�����λ��ʾ�����ա�   
				����enddate = '0523'
				��ʾ�˵���Ϊ5�ţ� ������Ϊ23��
				��ô��tbevent���ж�Ӧ��datestring�ֱ�Ϊ��2011-12-05 1��2011-12-23  5
				���������ݣ�һ���˵����¼���һ���������¼����˵����¼���typeֵΪ21���������¼���ֵΪ22, ��ݺ��·�Ĭ��Ϊ��ǰ���ڵ����£�

				enddate = "05hk"
				��ʾ�˵���Ϊ5�ţ�û�����û�����
				��ô��tbevent���еĶ�Ӧ��datestringΪ�� 2011-12-05 1��һ���˵����¼���

				enddate = "zd23"
				��ʾ������Ϊ23�ţ�û�������˵���
				��ô��tbevent���еĶ�Ӧ��datestringΪ�� 2011-12-23 5��һ���������¼���*/
			size_t nPos = description.find("����");
			if(nPos != string::npos && nPos != 0)
			{
				size_t nSpace = description.find(" ");
				if(nSpace != string::npos && nSpace != 0)
				{
					int itype = 0;//Ĭ��ΪͶ����
					string account = description.substr(0, nSpace);//���˺�
					if(description.find("�˵���") != string::npos)
						itype = 1;//���ÿ��˵���
					else if(description.find("�˵����ڻ���") != string::npos)
						itype = 2;//���ÿ�������
					else if(nPos != nSpace + 1)
						itype = 3;//���


					if(itype == 0)
						type = 5;
					else if(itype == 3)
						type = 3;


					if(itype == 0 || itype == 3)
					{
						string EndData = datestring.substr(0, 10);
						if(itype == 0)
						{
							CppSQLite3Query q2;
							sprintf_s(BufSQL, 2560, "SELECT id FROM tbAccount WHERE Name='%s';", account.c_str());
							q2 = dbTemp.execQuery(BufSQL);
							
							if(!q2.eof())
							{
								tbAccount_id = q2.getInt64Field("id");
							}
							q2.finalize();

							sprintf_s(BufSQL, 2560, "SELECT id FROM tbSubAccount WHERE tbAccount_id=%I64d;", tbAccount_id);

							q2 = dbTemp.execQuery(BufSQL);
							if(!q2.eof())
								tbSubAccount_id = q2.getInt64Field("id");
							q2.finalize();

							sprintf_s(BufSQL, 2560, "UPDATE tbSubAccount SET EndDate='%s' WHERE id=%I64d;", EndData.c_str(), tbSubAccount_id);
							dbTemp.execDML(BufSQL);
							
						}
						else
						{
							string subAccount = description.substr(nSpace + 1, nPos - nSpace - 1);
							CppSQLite3Query q2;
							sprintf_s(BufSQL, 2560, "SELECT id FROM tbAccount WHERE Name='%s';", account.c_str());
							q2 = dbTemp.execQuery(BufSQL);

							if(!q2.eof())
							{
								tbAccount_id = q2.getInt64Field("id");
							}
							q2.finalize();

							sprintf_s(BufSQL, 2560, "SELECT id FROM tbSubAccount WHERE tbAccount_id=%I64d AND Name='%s';", tbAccount_id, subAccount.c_str());

							q2 = dbTemp.execQuery(BufSQL);
							if(!q2.eof())
								tbSubAccount_id = q2.getInt64Field("id");
							q2.finalize();

							sprintf_s(BufSQL, 2560, "UPDATE tbSubAccount SET EndDate='%s' WHERE id=%I64d;", EndData.c_str(), tbSubAccount_id);
							dbTemp.execDML(BufSQL);
						}							
					}
					else if(itype == 1 || itype == 2)
					{
						string EndData = datestring.substr(8,2);
						if(itype == 1)
						{
							//enddate = "05hk"
							CppSQLite3Query q2;
							sprintf_s(BufSQL, 2560, "SELECT id FROM tbAccount WHERE Name='%s' AND tbAccountType_id=2;", account.c_str());
							q2 = dbTemp.execQuery(BufSQL);

							if(!q2.eof())
							{
								tbAccount_id = q2.getInt64Field("id");
							}
							q2.finalize();

							type = 21;
							sprintf_s(BufSQL, 2560, "UPDATE tbAccount SET EndDate='%s' || substr(EndDate, 3, 2) WHERE Name='%s' AND tbAccountType_id=2;", EndData.c_str(), account.c_str());
							dbTemp.execDML(BufSQL);
						}
						else
						{
							//enddate = "zd23"
							CppSQLite3Query q2;
							sprintf_s(BufSQL, 2560, "SELECT id FROM tbAccount WHERE Name='%s' AND tbAccountType_id=2;", account.c_str());
							q2 = dbTemp.execQuery(BufSQL);

							if(!q2.eof())
							{
								tbAccount_id = q2.getInt64Field("id");
							}
							q2.finalize();

							type = 22;
							sprintf_s(BufSQL, 2560, "UPDATE tbAccount SET EndDate=substr(EndDate, 1, 2) || '%s' WHERE Name='%s' AND tbAccountType_id=2;", EndData.c_str(), account.c_str());
							dbTemp.execDML(BufSQL);
						}
					}
				}
			}		

			sprintf_s(BufSQL, 2560, "INSERT INTO tbEvent(id, UT, mark, event_date, description, repeat, alarm, status, datestring, tbAccount_id, tbSubAccount_id, type) VALUES(%I64d, %I64d, 0, %d, '%s', %d, %d, %d, '%s', %I64d, %I64d, %d);",
				nID, nUT, event_date, description.c_str(), repeat, alarm, status, datestring.c_str(), tbAccount_id, tbSubAccount_id, type);
			dbTemp.execDML(BufSQL);

			sprintf_s(BufSQL, 2560, "INSERT INTO tbTotalEvent(id, mark, event_date, repeat, alarm, status, datestring, type, datUserInfo_userid) VALUES(%I64d, 0, %d, %d, %d, %d, '%s', %d, 'Guest');",
				nID, event_date, repeat, alarm, status, datestring.c_str(), type);
			dbData.execDML(BufSQL);

			q1.nextRow();
		}
		q1.finalize();


		// 10 tbAccountGetBillMonth��
		/*`'id' INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
		'tbaccount_id' INTEGER  NOT NULL,\
		'tbmonth' VARCHAR(6)  NOT NULL,\
		'tbKeyInfo' VARCHAR(100)  NOT NULL*/
		q1 = dbDesc.execQuery("SELECT * FROM tbAccountGetBillMonth;");
		dbTemp.execDML("DELETE FROM tbAccountGetBillMonth;");

		while(!q1.eof())
		{
			int id = q1.getIntField("id");
			INT64 nID = 0;
			INT64 nUT = 0;
			nID = GetNewID();
			nUT = GetNewUT();

			int tbAccountid = q1.getIntField("tbaccount_id");
			INT64 tbAccount_id = 0;
			std::map<int, INT64>::iterator ite = maptbAccount.find(tbAccountid);
			if(ite != maptbAccount.end())
				tbAccount_id = (*ite).second;
			
			string tbmonth = q1.getStringField("tbmonth");
			string tbKeyInfo = q1.getStringField("tbKeyInfo");

			sprintf_s(BufSQL, 2560, "INSERT INTO tbAccountGetBillMonth(id, UT, mark, tbmonth, tbKeyInfo, tbAccount_id) VALUES(%I64d, %I64d, 0, '%s', '%s', %d);",
				nID, nUT, tbmonth.c_str(), tbKeyInfo.c_str(), tbAccount_id);
			dbTemp.execDML(BufSQL);

			q1.nextRow();
		}
		q1.finalize();

		// 11 tbCurrency�� ���ö�
		// 12 tbFav��
		/*`'id' INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
		'tbaccount_id' INTEGER  NOT NULL,\
		'tbmonth' VARCHAR(6)  NOT NULL,\
		'tbKeyInfo' VARCHAR(100)  NOT NULL*/
		q1 = dbDesc.execQuery("SELECT * FROM tbFav;");

		while(!q1.eof())
		{
			string favinfo = q1.getStringField("favinfo");
			int status = q1.getIntField("status");
			int favorder = q1.getIntField("favorder");
			int deleted = q1.getIntField("deleted");

			sprintf_s(BufSQL, 2560, "INSERT INTO datFav(favinfo, status, favorder, deleted) VALUES('%s', %d, %d, %d);",
				favinfo.c_str(), status, favorder, deleted);
			dbData.execDML(BufSQL);

			q1.nextRow();
		}
		q1.finalize();

		// 13 tbProductChoice��
		/*`'id' bigint  NOT NULL,\
		'interestchoice' varchar(256)  NULL,\
		'durationchoice' varchar(256)  NULL,\
		'currencychoice' varchar(256)  NULL,\
		'bankchoice' varchar(256)  NULL,\
		'userId' bigint  NOT NULL,\
		'UT' bigint  NOT NULL*/
		q1 = dbDesc.execQuery("SELECT * FROM tbProductChoice;");
		dbTemp.execDML("DELETE FROM tbProductChoice;");

		while(!q1.eof())
		{
			INT64 nID = 0;
			INT64 nUT = 0;
			nID = GetNewID();
			nUT = GetNewUT();

			string interestchoice = q1.getStringField("interestchoice");
			string durationchoice = q1.getStringField("durationchoice");
			string currencychoice = q1.getStringField("currencychoice");
			string bankchoice = q1.getStringField("bankchoice");

			INT64 userId = q1.getInt64Field("userId");


			sprintf_s(BufSQL, 2560, "INSERT INTO tbProductChoice(id, UT, mark, interestchoice, durationchoice, currencychoice, bankchoice, userId) VALUES(%I64d, %I64d, 0, '%s', '%s', '%s', '%s', %I64d);",
				nID, nUT, interestchoice.c_str(), durationchoice.c_str(), currencychoice.c_str(), bankchoice.c_str(), userId);
			dbTemp.execDML(BufSQL);

			q1.nextRow();
		}
		q1.finalize();

		dbTemp.execDML("UPDATE tbDBInfo SET schema_version=8;");
		dbData.close();
		dbDesc.close();
		dbTemp.close();
		::DeleteFileW(sGuestDB.c_str());//������֮��ɾ��ԭʼ�ļ�
		USES_CONVERSION;
		rename(W2A(sTempDB.c_str()), W2A(sGuestDB.c_str()));
		

	}
	catch(CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"Guest.dat������8�汾ʧ�� error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
	}

	return true;


}
bool CBankData::InstallUpdateDB()
{
	CRecordProgram::GetInstance()->RecordCommonInfo(L"BankData", 1000, L"��ʼ�������ݿ�");
	GetDbPath();
	WCHAR tempPath[MAX_PATH] = { 0 };

	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tempPath);
	wcscat_s(tempPath, L"\\MoneyHub");
	::CreateDirectoryW(tempPath, NULL);
	wcscat_s(tempPath, L"\\Data\\");
	::CreateDirectoryW(tempPath, NULL);

	std::wstring wstrPath = tempPath;

	char szDbPath[1024] = { 0 };
	int srcCount = wcslen(tempPath);
	srcCount = (srcCount > MAX_PATH) ? MAX_PATH : srcCount;
	::WideCharToMultiByte(CP_UTF8, 0, tempPath, srcCount, szDbPath, 1024, NULL,FALSE);

	std::string utfstrPath = szDbPath;

	std::string strutfMoneyhub = utfstrPath + "moneyhub.dat"; // ��¼�µ�ǰ�û����ݿ�·��
	std::wstring strMoneyhub = wstrPath + L"moneyhub.dat";//utf-8ת��������
	// ��4��5�����汾�У����ݿ�����Ϊmoneyhub.dat
	if (IsFileExist((LPWSTR)strMoneyhub.c_str()))
	{
		try{
			CRecordProgram::GetInstance()->RecordCommonInfo(L"BankData", 1000, L"��ʼ�������ݿ�moneyhub.dat");
			CppSQLite3DB dbSour;
			dbSour.openWithNoKey(strutfMoneyhub.c_str());

			CppSQLite3DB dbDesc;
			dbDesc.open(m_strUtfUserDbPath.c_str());

			// �����ǲ���4��5�����汾�����ݿ�
			int nVer = GetDBVersion(dbSour);
			if (nVer == 4 || nVer == 5)
			{
				dbSour.execDML("delete from tbFav where favinfo='b001';");//3.0�汾��ʱ��֧��
				dbSour.execDML("delete from tbFav where favinfo='c001';");
				dbSour.execDML("delete from tbFav where favinfo='f001';");
				dbSour.execDML("delete from tbFav where favinfo='f002';");
				// ��ԭ�������ݿ�����
				CopyDataFromOldVersoion(dbSour, dbDesc);

				CppSQLite3DB dbDataDB;
				dbDataDB.open(m_strUtfDataDbPath.c_str());
				// ��usbnfo�е����ݿ�����DataDB��
				CpyDbTb2OtherDbTb(dbSour, dbDataDB,"tbUSBInfo", "datUSBKeyInfo"); 
				dbDataDB.close();
			}

			dbSour.close();
			dbDesc.close();
			// ɾ������ǰ�汾�����ݿ�
			DeleteFileW(strMoneyhub.c_str());//������utf8����
			CRecordProgram::GetInstance()->RecordCommonInfo(L"BankData", 1000, L"����������ݿ�moneyhub.dat");
		}
		catch(CppSQLite3Exception& ex)
		{
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"moenyhub.dat����ʧ�� error:%d", ex.errorCode()));
			if(ex.errorMessage() != NULL)
				CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		}

	}
	std::string strutfGuestDB = utfstrPath + "Guest.dat"; // ��6������7�汾���ݿ�Ĳ���
	std::wstring strGuestDB = wstrPath + L"Guest.dat"; // ��6������7�汾���ݿ�Ĳ���
	// ��6�����汾�У����ݿ�����Ϊmoneyhub.dat
	if (IsFileExist((LPWSTR)strGuestDB.c_str()))
	{
		try{
			CRecordProgram::GetInstance()->RecordCommonInfo(L"BankData", 1000, L"��ʼ�������ݿ�Guest.dat");
			CppSQLite3DB dbDesc;
			dbDesc.open(strutfGuestDB.c_str());

			// �����ǲ���4��5�����汾�����ݿ�
			int nVer = GetDBVersion(dbDesc);
			if(nVer == 8)
			{
				dbDesc.close();
				return true;
			}
			if (nVer == 6)
			{
				CRecordProgram::GetInstance()->RecordCommonInfo(L"BankData", 1000,  L"��ʼ�������ݿ�6");
				// BUG 2361
				dbDesc.execDML("INSERT INTO tbCategory1(id, Name, Type) VALUES(10025, \"������\", 0);");
				dbDesc.execDML("INSERT INTO tbCategory2(id, Name, tbCategory1_id) VALUES(10067, \"CATA420\", 10025);");
				dbDesc.execDML("UPDATE tbCategory2 SET Name=\"CATA420\" WHERE id=10065;");
				dbDesc.execDML("UPDATE tbCategory2 SET Name=\"CATA420\" WHERE id=10066;");

				dbDesc.execDML("UPDATE tbCategory2 SET Name=\"CATA420\" Where id=10059;");//bug 2063
				dbDesc.execDML("UPDATE tbCategory2 SET Name=\"CATA420\" Where id=10060;");
				dbDesc.execDML("CREATE TABLE IF NOT EXISTS 'tbAccountGetBillMonth' (\
							   'id' INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
							   'tbaccount_id' INTEGER  NOT NULL,\
							   'tbmonth' VARCHAR(6)  NOT NULL,\
							   'tbKeyInfo' VARCHAR(100)  NOT NULL\
							   );");
				dbDesc.execDML("CREATE TABLE IF NOT EXISTS 'tbProductChoice' (\
							   'id' bigint  NOT NULL,\
							   'interestchoice' varchar(256)  NULL,\
							   'durationchoice' varchar(256)  NULL,\
							   'currencychoice' varchar(256)  NULL,\
							   'bankchoice' varchar(256)  NULL,\
							   'userId' bigint  NOT NULL,\
							   'UT' bigint  NOT NULL\
							   );");
				dbDesc.execDML("insert into tbProductChoice (id, interestchoice,durationchoice,currencychoice,bankchoice,userId,ut) values(0,'1|2|3|4','1|2|3|4|5|6','1|2','1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19',0,0);");
				dbDesc.execDML("ALTER TABLE tbTransaction ADD COLUMN `transactionClasses` tinyint (1)DEFAULT 0 NOT NULL;");
				dbDesc.execDML("ALTER TABLE tbTransaction ADD COLUMN `isEdit` tinyint (1)DEFAULT 0 NOT NULL;");//3.1�汾�´������ֶ�
				dbDesc.execDML("ALTER TABLE tbAccount ADD `keyInfo` varchar(255);");
				dbDesc.execDML("UPDATE tbSubAccount SET tbAccountType_id=201 WHERE tbAccountType_id=0;"); // BUG2351
				dbDesc.execDML("UPDATE tbDBInfo SET schema_version=7;");
				CRecordProgram::GetInstance()->RecordCommonInfo(L"BankData", 1000,  L"����������ݿ�6");
				nVer = 7;
			}

			if(nVer == 7)
			{
				//nres > 0// ˵����3.1�汾���˻���������һ����ʼ��Ϊ0

				CppSQLite3Buffer bufSQL;
				bufSQL.format("update tbtransaction set transdate =	substr(transdate, 1, 8)||'0'||substr(transdate, 9, 1) where length(transdate) = 9;");
				//nres > 0// ˵����3.1�汾���˻���������һ����ʼ��Ϊ0
				dbDesc.execDML(bufSQL);

				bufSQL.format("select a.id as aid,date(min(transdate),'-1 day') as handleDate,openbalance from tbsubaccount a left join tbtransaction b on a.id = b.tbsubaccount_id group by a.id;");
				/*
				bufSQL.format("select a.id as aid,date(min(transdate),'-1 day') as handleDate,openbalance, \
							  num as num from tbsubaccount a left join tbtransaction b on a.id = b.tbsubaccount_id left join (select count(a.id) as num, \
							  b.id as sid from tbsubaccount b left join tbtransaction a on a.tbsubaccount_id = b.id \
							  where tbcategory2_id = 10067 group by b.id) c on a.id = c.sid group by a.id;");
				*/
				CppSQLite3Query q = dbDesc.execQuery(bufSQL);
				while(!q.eof())
				{
					int iaid = q.getIntField(0);
					char* handleDate = (char *)q.getStringField(1);
					int iopenbalance = q.getIntField(2);

					bool bTag = false;
					bufSQL.format("select amount from tbtransaction a, tbsubaccount b where a.tbsubaccount_id = b.id and b.id = %d and a.tbcategory2_id = 10067 order by a.transdate,a.id asc limit 0, 1", iaid);
					CppSQLite3Query q2 = dbDesc.execQuery(bufSQL);

					if(!q2.eof())
					{
						int iamount = q2.getIntField(0);
						if( iamount == iopenbalance) bTag = true;
					}
					q2.finalize();

					if(iopenbalance != 0 && (!bTag))
					{
						if(handleDate != NULL && strlen(handleDate) > 0)
						{
							bufSQL.format("INSERT INTO tbTransaction(transdate, tbPayee_id, tbCategory2_id, amount, direction, tbSubaccount_id, exchangerate, comment, tbSubaccount_id1,transactionClasses ) values \
								('%s', 0, 10067, %d, 0, %d, 0, '', 0, 0);", handleDate, iopenbalance, iaid);
						}
						else
						{
							bufSQL.format("INSERT INTO tbTransaction(transdate, tbPayee_id, tbCategory2_id, amount, direction, tbSubaccount_id, exchangerate, comment, tbSubaccount_id1,transactionClasses ) values \
								(date(), 0, 10067, %d, 0, %d, 0, '', 0, 0);", iopenbalance, iaid);
						}
						dbDesc.execDML(bufSQL);
						bufSQL.format("update tbsubaccount set openbalance=0 where id=%d; ", iaid);
						dbDesc.execDML(bufSQL);
					}
					else
					{
						bufSQL.format("update tbsubaccount set openbalance=0 where id=%d; ", iaid);
						dbDesc.execDML(bufSQL);
					}
					//�������˻����
					//�����Ǵ�3.1.0.58������3.1.0.65�õ�
					int changeBalanceClasses = 10067;
					//bufSQL.format("SELECT id, date(transdate,'0 day'), Amount FROM tbtransaction WHERE tbsubaccount_id=%d AND tbcategory2_id=10067 ORDER BY transdate DESC, id DESC LIMIT 1;", iaid);
					bufSQL.format("SELECT id, strftime('%s',TransDate) AS Date, Amount FROM tbtransaction WHERE tbsubaccount_id=%d AND tbcategory2_id=10067 ORDER BY transdate DESC, id DESC LIMIT 1;", "%Y-%m-%d",iaid);
					CppSQLite3Query q3 = dbDesc.execQuery(bufSQL);

					int lastBalanceId = 0;
					string  lastBalanceDate = "1900-01-01";
					float lastBalanceAmount = 0;
					if(!q3.eof()){
						lastBalanceId = q3.getIntField(0);
						lastBalanceDate = (char *)q3.getStringField(1);
						lastBalanceAmount = (float)q3.getFloatField(2);
					}
					q3.finalize();				

					bufSQL.format("SELECT Type, SUM(Amount) sumamount FROM tbtransaction t, tbcategory2, tbcategory1 \
						WHERE tbsubaccount_id=%d AND tbcategory2_id=tbcategory2.id AND tbcategory1_id=tbcategory1.id \
						AND ((t.transdate>'%s') OR ((t.transdate='%s') AND (t.id>%d))) AND tbcategory2_id<>10067 GROUP BY Type", 
						iaid, lastBalanceDate.c_str(), lastBalanceDate.c_str() , lastBalanceId);

					CppSQLite3Query q4 = dbDesc.execQuery(bufSQL);
					
					float totalSpend = 0;
					float totalIncome = 0;
					while(!q4.eof())
					{
						int q4res = q4.getIntField(0);

						if( q4res == 0)
							totalSpend = (float)q4.getFloatField(1);
						else if( q4res == 1)
							totalIncome = (float)q4.getFloatField(1);

						q4.nextRow();
					}
					q4.finalize();

					lastBalanceAmount += totalIncome - totalSpend;

					bufSQL.format("UPDATE tbSubAccount SET balance=%.2f WHERE id=%d",lastBalanceAmount, iaid);
					dbDesc.execDML(bufSQL);				
					q.nextRow();
				}

				q.finalize();
				//֧�����������˻��������ˣ���Ϊ

				bufSQL.format("UPDATE tbSubAccount SET tbCurrency_id=1, name='�����' WHERE tbCurrency_id<>1 AND id IN (Select a.id as id From tbSubAccount a, tbAccount b Where a.tbAccount_id = b.id And b.tbBank_id=31);");
				dbDesc.execDML(bufSQL);

				bufSQL.format("UPDATE tbAccount SET tbAccountType_id=1 WHERE tbAccountType_id=9;");
				dbDesc.execDML(bufSQL);
				
			}

			dbDesc.close();

			if(nVer == 7)
			{
				UpdateDB7to8(wstrPath, utfstrPath);
			}
		}
		catch(CppSQLite3Exception& ex)
		{
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"Guest.dat����ʧ�� error:%d", ex.errorCode()));
			if(ex.errorMessage() != NULL)
				CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		}

	}
	CRecordProgram::GetInstance()->RecordCommonInfo(L"BankData", 1000, L"�������ݿ����");
	return true;
}

const char* CBankData::GetDbPath(std::string strUsID)
{
	if (strUsID == "SysDB")
		return m_strUtfSysDbPath.c_str();

	if (strUsID == "DataDB")
	{
		if (!m_strUtfDataDbPath.empty())
			return m_strUtfDataDbPath.c_str();
	}

	// �����û����ݿ�·��
	if (strUsID.empty())
	{
		if (!m_strUtfUserDbPath.empty())
			return m_strUtfUserDbPath.c_str();

		if (m_strUtfUserDbPath.empty())
			strUsID = MONHUB_GUEST_USERID;
	}

	WCHAR tempPath[MAX_PATH] = { 0 };
	
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tempPath);
	wcscat_s(tempPath, L"\\MoneyHub");
	::CreateDirectoryW(tempPath, NULL);
	wcscat_s(tempPath, L"\\Data\\");
	::CreateDirectoryW(tempPath, NULL);
	USES_CONVERSION;
	std::wstring wstrUsID = A2W(strUsID.c_str());

	m_strUserDbPath = tempPath + wstrUsID + L".dat"; // ��¼�µ�ǰ�û����ݿ�·��

	char szDbPath[1024] = { 0 };
	int srcCount = wcslen(m_strUserDbPath.c_str());
	srcCount = (srcCount > MAX_PATH) ? MAX_PATH : srcCount;
	::WideCharToMultiByte(CP_UTF8, 0, m_strUserDbPath.c_str(), srcCount, szDbPath, 1024, NULL,FALSE);

	m_strUtfUserDbPath = szDbPath; //Ϊ��sqlite3���洢utf8����

	m_strDataPath = tempPath; // ��¼�û����ݿ��·��

	memset(szDbPath, 0, 1024);
	srcCount = wcslen(m_strDataPath.c_str());
	srcCount = (srcCount > MAX_PATH) ? MAX_PATH : srcCount;
	::WideCharToMultiByte(CP_UTF8, 0, m_strDataPath.c_str(), srcCount, szDbPath, 1024, NULL,FALSE);
	m_strUtfDataPath = szDbPath;

	if (!IsFileExist ((LPWSTR)m_strUserDbPath.c_str()))
	{
		// ��ϵͳ���ݿ��п���
		CopyFileW(m_strGuestTemplete.c_str(), m_strUserDbPath.c_str(), false);//�����������治����utf8
	}

	return m_strUtfUserDbPath.c_str();
}

void CBankData::RemoveDatabase()
{
	ObjectLock lock(this);

	try
	{
		DeleteFileA(GetDbPath());
	}
	catch (...)
	{
	}
}

//�ж��Ƿ��Ѿ�����
bool CBankData::IsFavExist(std::string& strFav)
{
	ObjectLock lock(this);

	if(!CheckUserDB())
		return false;
	
	try
	{
		bool isExist = false;
		CppSQLite3Buffer bufSQL;
		bufSQL.format("SELECT favinfo FROM datFav WHERE favinfo='%s';",strFav.c_str());
		CppSQLite3Query q = m_dbDataDB.execQuery(bufSQL);
		if(!q.eof())
			isExist = true;

		q.finalize();
		return isExist;
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"IsFavExist error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}
}

// ɾ���ղؼ�
int CBankData::DeleteFav(std::string strFav)
{
	ObjectLock lock(this);
	return InternalDeleteFav(strFav) ? 1 : 0;
	
}
// ����ղؼ�
int CBankData::SaveFav(std::string strFav, int status)
{
	ObjectLock lock(this);
	return InternalSaveFav(strFav, status) ? 1 : 0;
}

int CBankData::GetFav(std::list<std::string>& strFav,bool bNoInstall) // �õ�û�а�װ���Ѿ��ղص����пؼ�ID
{
	strFav.clear ();
	ObjectLock lock(this);

	if(!CheckUserDB())
		return false;

	try
	{
		CppSQLite3Binary blob;
		CppSQLite3Query q;
		if(bNoInstall)
			q = m_dbDataDB.execQuery("SELECT * FROM datFav;");
		else
			q = m_dbDataDB.execQuery("SELECT * FROM datFav WHERE status<200;");
		while (!q.eof()) {
			strFav.push_back(ToSQLUnsafeString(q.getStringField("favinfo")));
			q.nextRow();
		}

		q.finalize();
		return true;		
	}

	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"GetFav error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}
}
int CBankData::GetFavState(std::string& strFav)
{
	ObjectLock lock(this);

	if(!CheckUserDB())
		return false;

	try
	{
		int status = -1;

		CppSQLite3Buffer bufSQL;
		bufSQL.format("SELECT * FROM datFav Where favinfo='%s';",strFav.c_str());

		CppSQLite3Query q = m_dbDataDB.execQuery(bufSQL);
		if (!q.eof()) {
			status = q.getIntField("status");
		}

		q.finalize();
		return status;		
	}

	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalGetFav error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return -1;
	}

}
int CBankData::GetFav(std::string& strFav)
{

	ObjectLock lock(this);

	strFav = "[";

	std::vector<LPFAVRECORD> vec;

	bool bOK = InternalGetFav(vec);
	if (bOK)
	{
		char szVal[1024];
		for (size_t i = 0; i < vec.size(); i++)
		{
			LPFAVRECORD pFav = vec[i];

			sprintf_s(szVal, sizeof(szVal), "{\"id\":\"%s\",\"status\":\"%d\"}", pFav->fav, pFav->status);

			strFav += szVal;

			if (i != (vec.size() - 1))
				strFav += ",";

			delete pFav;
		}
		vec.clear();
	}

	strFav += "]";

	return bOK;
}


int CBankData::AddUSB(int vid, int pid, DWORD mid, std::string fav, std::string xml,std::string bkurl, std::string ver,int status)
{
	ObjectLock lock(this);

	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("INSERT INTO datUSBKeyInfo (vid, pid, mid, fav, xml, bkurl, ver, status) VALUES(%d, %d, %d, '%s', '%s', '%s', '%s', %d);", vid, pid, mid, fav.c_str(), xml.c_str(), bkurl.c_str(),ver.c_str(), status);

		GetDataDbObject()->execDML(bufSQL);
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"AddUSB error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return 0;
	}
	return 1;
}
bool CBankData::DeleteUSB(int vid, int pid, DWORD mid)
{
	ObjectLock lock(this);

	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("DELETE FROM datUSBKeyInfo WHERE vid=%d AND pid=%d AND mid = %d", vid, pid, mid);

		GetDataDbObject()->execDML(bufSQL);
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"DeleteUSB error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}
	return true;
}
int CBankData::UpdateUSB(int vid, int pid, DWORD mid, std::string fav, std::string xml)
{
	ObjectLock lock(this);

	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("UPDATE datUSBKeyInfo SET fav='%s', xml='%s' WHERE vid=%d AND pid=%d AND mid=%d;", fav.c_str(), xml.c_str(), vid, pid, mid);

		GetDataDbObject()->execDML(bufSQL);
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"UpdateUSB0 error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return 0;
	}
	return 1;

}
int CBankData::UpdateUSB(int vid, int pid, DWORD mid, int status)
{
	ObjectLock lock(this);

	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("UPDATE datUSBKeyInfo SET status=%d WHERE vid=%d AND pid=%d AND mid=%d;", status, vid, pid, mid);

		GetDataDbObject()->execDML(bufSQL);
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"UpdateUSB1 error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return 0;
	}
	return 1;

}

int CBankData::UpdateUSB(int vid, int pid, DWORD mid, const std::string& strVer)
{
	ObjectLock lock(this);

	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("UPDATE datUSBKeyInfo SET ver=%s WHERE vid=%d AND pid=%d AND mid=%d;", strVer, vid, pid, mid);

		GetDataDbObject()->execDML(bufSQL);
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"UpdateUSB2 error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return 0;
	}
	return 1;
}

bool CBankData::IsUsbExist(int vid, int pid, DWORD mid)
{
	ObjectLock lock(this);

	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("SELECT * FROM datUSBKeyInfo WHERE vid=%d AND pid=%d AND mid=%d;", vid, pid, mid);

		CppSQLite3Binary blob;

		bool bExist = false;
		CppSQLite3Query q = GetDataDbObject()->execQuery(bufSQL);
		if (!q.eof()) {
			bExist = true;
		}
		q.finalize();
		return bExist;
	}
	catch (CppSQLite3Exception&)
	{
		return false;
	}

}
bool CBankData::GetAllUsb(std::vector<LPUSBRECORD>& vec)
{
	ObjectLock lock(this);

	try
	{
		CppSQLite3Binary blob;

		CppSQLite3Query q = GetDataDbObject()->execQuery("SELECT * FROM datUSBKeyInfo");
		while (!q.eof()) {
			LPUSBRECORD pusb = new USBRECORD;
			pusb->vid = q.getIntField("vid");
			pusb->pid = q.getIntField("pid");
			pusb->mid = q.getIntField("mid");
			strcpy_s(pusb->fav, sizeof(pusb->fav), ToSQLUnsafeString(q.getStringField("fav")).c_str());
			strcpy_s(pusb->xml, sizeof(pusb->xml), ToSQLUnsafeString(q.getStringField("xml")).c_str());
			strcpy_s(pusb->bkurl, sizeof(pusb->bkurl), ToSQLUnsafeString(q.getStringField("bkurl")).c_str());
			strcpy_s(pusb->ver, sizeof(pusb->ver), ToSQLUnsafeString(q.getStringField("ver")).c_str());
			pusb->status = q.getIntField("status");

			vec.push_back(pusb);
			q.nextRow();
		}
		q.finalize();
		return true;
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"GetAllUsb error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

}

// ͨ��������vid, pid, mid��ȡ��Ӧ�ļ�¼,û���ҵ���Ӧ�ļ�¼�ͷ��� false
bool CBankData::GetAUSBRecord(int vid, int pid, DWORD mid, USBRECORD& record)
{
	memset (&record, sizeof (USBRECORD), 0);
	ObjectLock lock(this);
	bool bRet = false;

	try
	{
		CppSQLite3Binary blob;

		CppSQLite3Buffer bufSQL;
		bufSQL.format("SELECT * FROM datUSBKeyInfo WHERE vid=%d AND pid=%d AND mid=%d;", vid, pid, mid);

		CppSQLite3Query q = GetDataDbObject()->execQuery(bufSQL);
		if (!q.eof())
		{
			record.vid = vid;
			record.pid = pid;
			record.mid = mid;
			strcpy_s(record.fav, sizeof(record.fav), ToSQLUnsafeString(q.getStringField("fav")).c_str());
			strcpy_s(record.xml, sizeof(record.xml), ToSQLUnsafeString(q.getStringField("xml")).c_str());
			strcpy_s(record.bkurl, sizeof(record.bkurl), ToSQLUnsafeString(q.getStringField("bkurl")).c_str());
			strcpy_s(record.ver, sizeof(record.ver), ToSQLUnsafeString(q.getStringField("ver")).c_str());
			record.status = q.getIntField("status");
			bRet = true;
		}
		q.finalize();
		return bRet;
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"GetAUSBRecord error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return bRet;
	}
}

void CBankData::SavePwd(std::string strPwd)
{
	ObjectLock lock(this);

	try
	{
		CppSQLite3DB db;
		db.open(GetDbPath());

		db.execDML("DELETE FROM tbAuthen;");

		CppSQLite3Binary blob;
		blob.setBinary((const unsigned char *)strPwd.c_str(), strPwd.length() + 1);

		CppSQLite3Buffer bufSQL;
		bufSQL.format("INSERT INTO tbAuthen VALUES(%Q);", blob.getEncoded());

		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"SavePwd error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
	}
}

std::string CBankData::GetPwd()
{
	ObjectLock lock(this);

	std::string strPwd = "";

	try
	{
		CppSQLite3DB db;
		db.open(GetDbPath());

		CppSQLite3Binary blob;

		CppSQLite3Query q = db.execQuery("SELECT pwd FROM tbAuthen;");
		if (!q.eof()) {
			blob.setEncoded((unsigned char*)q.fieldValue("pwd"));
			strPwd = (const char *)blob.getBinary();
		} else {
			strPwd = "";
		}
		q.finalize();
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"GetPwd error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
	}

	return strPwd;
}
void CBankData::SetGetIDMode(eGetIdMode eMode)
{
	m_eGetIDMode = eMode;
	m_initID = 0;
}

INT64 CBankData::GetNewID()
{
	INT64 tBase = 129383136000000000;//s2��100����Ϊ��λ����������ʱ��2011.1.1�ź�1601��֮��Ĳ�ֵ
	FILETIME tCurrent;
	GetSystemTimeAsFileTime(&tCurrent);
	INT64 tIntCur = tCurrent.dwHighDateTime * 0x100000000 + tCurrent.dwLowDateTime;//

	INT64 tIntTimeDif = tIntCur - tBase;

	INT64 tMilTimeDif = tIntTimeDif - tIntTimeDif % 10000;//�õ�ʵ��ʱ��ĺ�����
		//2011-01-01 00:00:00 000 �ĺ�����
	INT64 nid = 0;
	if(m_eGetIDMode == egSingle)
	{
		srand((unsigned int)time(NULL));	
		int randomNum = 0;
		int randomLength = 4;

		for(int i = 0;i < randomLength;i ++)
		{
			randomNum = randomNum * 10 + (rand() % 10);	
		}

		nid = tMilTimeDif + randomNum;
	}
	else
	{
		nid = tMilTimeDif + m_initID;
		m_initID ++;
		if(m_initID >= 10000)
			m_initID = 0;
	}
	return nid; 
}
//���ص�ǰ�ĺ�����
INT64 CBankData::GetNewUT()
{
	INT64 tBase = 129383136000000000;//s2��100����Ϊ��λ����������ʱ��2011.1.1�ź�1601��֮��Ĳ�ֵ
	FILETIME tCurrent;
	GetSystemTimeAsFileTime(&tCurrent);
	INT64 tIntCur = tCurrent.dwHighDateTime * 0x100000000 + tCurrent.dwLowDateTime;//

	INT64 tIntTimeDif = tIntCur - tBase;

	INT64 tMilTimeDif = tIntTimeDif/10000;//�õ�ʵ��ʱ��ĺ�����
	if(tMilTimeDif < 0)
		return 0;
	return tMilTimeDif;
}
//
// event ��ʽ
// {"id":"0","event_date":"2010-09-01","description":"�¼����飨������30�������֣�","repeat":2,"alarm":3,"status":1,"tbAccount_id": "121212121212","tbSubAccount_id": "121212313132", "type": 1 }
//
INT64 CBankData::AddEvent(std::string event)
{
	ObjectLock lock(this);

	try
	{
		Json::Value root;
		Json::Reader reader;

		bool parsingSuccessful = reader.parse(event, root);
		if (!parsingSuccessful)
			return 0;

		if (!IsValidEventJson(root))
			return 0;

		EVENTRECORD rec;
		string sid = root.get("id", "0").asString();
		if(sid != "")
			rec.id = _atoi64(sid.c_str());
		else
			rec.id = 0;
		rec.event_date = ToIntTime(root.get("event_date", "").asString());
		strncpy_s(rec.description, sizeof(rec.description), ToSQLSafeString(root.get("description", "").asString()).c_str(), 255);
		rec.repeat = root.get("repeat", 0).asInt();
		rec.alarm = root.get("alarm", 0).asInt();
		rec.status = root.get("status", 0).asInt();
		rec.tbAccountid = root.get("tbAccount_id","0").asString();
		rec.tbSubaccountid = root.get("tbSubAccount_id","0").asString();
		rec.type = root.get("type", 0).asInt();

		return InternalAddEvent(&rec);
	}
	catch (std::runtime_error& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_RUNTIME_ERROR, AToW(ex.what()));
		return 0;		
	}
}

int CBankData::DeleteEvent(INT64 id)
{
	ObjectLock lock(this);
	
	return InternalDeleteEvent(id) ? 1 : 0;
}

int CBankData::GetEvents(int year, int month, std::string& events)
{
	ObjectLock lock(this);

	events = "[";

	std::vector<LPEVENTRECORD> vec;
	bool bOK = InternalGetEvents(year, month, vec);
	if (bOK)
	{
		char szVal[1024];
		for (size_t i = 0; i < vec.size(); i++)
		{
			LPEVENTRECORD pEventRec = vec[i];
			
			sprintf_s(szVal, sizeof(szVal), "{\"id\":\"%I64d\",\"UT\":\"%I64d\",\"event_date\":\"%s\",\"start_date\":\"%s\",\"description\":\"%s\",\"repeat\":%d,\"alarm\":%d,\"status\":%d, \"tbAccount_id\":\"%s\",\"tbSubAccount_id\":\"%s\",\"type\":%d}",
				pEventRec->id,pEventRec->UT,
				ToStrTime(pEventRec->event_date).c_str(),
				ToStrTime(pEventRec->start_date).c_str(),
				pEventRec->description,
				pEventRec->repeat,
				pEventRec->alarm,
				pEventRec->status,
				pEventRec->tbAccountid.c_str(),pEventRec->tbSubaccountid.c_str(),pEventRec->type);

			events += szVal;

			if (i != (vec.size() - 1))
				events += ",";

			delete pEventRec;
		}

		vec.clear();
	}
	
	events += "]";

	return bOK ? 1 : 0;
}

int CBankData::GetEventsOneDay(int year, int month, int day, std::string& events)
{
	ObjectLock lock(this);

	events = "[";

	std::vector<LPEVENTRECORD> vec;
	bool bOK = InternalGetEventsOneDay(year, month, day, vec);
	if (bOK)
	{
		char szVal[1024];
		for (size_t i = 0; i < vec.size(); i++)
		{
			LPEVENTRECORD pEventRec = vec[i];

			sprintf_s(szVal, sizeof(szVal), "{\"id\":\"%I64d\",\"UT\":\"%I64d\",\"event_date\":\"%s\",\"start_date\":\"%s\",\"description\":\"%s\",\"repeat\":%d,\"alarm\":%d,\"status\":%d, \"tbAccount_id\":\"%s\",\"tbSubAccount_id\":\"%s\",\"type\":%d}",
				pEventRec->id,pEventRec->UT,
				ToStrTime(pEventRec->event_date).c_str(),
				ToStrTime(pEventRec->start_date).c_str(),
				pEventRec->description,
				pEventRec->repeat,
				pEventRec->alarm,
				pEventRec->status,
				pEventRec->tbAccountid.c_str(),pEventRec->tbSubaccountid.c_str(),pEventRec->type);

			events += szVal;

			if (i != (vec.size() - 1))
				events += ",";

			delete pEventRec;
		}

		vec.clear();
	}

	events += "]";

	return bOK ? 1 : 0;
}

int CBankData::SetAlarm(std::string alarm)
{
	ObjectLock lock(this);

	try
	{
		Json::Value root;
		Json::Reader reader;

		bool parsingSuccessful = reader.parse(alarm, root);
		if (!parsingSuccessful)
			return 0;

		if (!IsValidAlarmJson(root))
			return 0;

		INT64 nId;
		string sid = root.get("id", "0").asString();
		if(sid != "")
			nId = _atoi64(sid.c_str());
		else
			nId = 0;
		int status = root.get("status", 0).asInt();
		std::string alarmtype = root.get("type", 0).asString();

		return InternalSetAlarm(nId, status, alarmtype) ? 1 : 0;
	}
	catch (std::runtime_error& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_RUNTIME_ERROR, AToW(ex.what()));
		return 0;
	}
}

bool DateSortCallback(const LPALARMRECORD& m1, const LPALARMRECORD& m2)
{
	return m1->event_date < m2->event_date;
}

std::string CBankData::GetTodayAlarms()
{
	ObjectLock lock(this);

	std::string result = "[";
	
	std::vector<LPALARMRECORD> vec;
	if (InternalGetTodayAlarms(vec))
	{

		sort(vec.begin(), vec.end(), DateSortCallback);

		char szVal[1024];
		for (size_t i = 0; i < vec.size(); i++)
		{
			LPALARMRECORD pAlarmRec = vec[i];

			if (pAlarmRec->type == 0)
			{
				sprintf_s(szVal, sizeof(szVal), "{\"id\":\"%I64d\",\"UT\":\"%I64d\",\"event_date\":\"%s\",\"description\":\"%s\",\"status\":%d,\"type\":\"%s\",\"tbAccount_id\":\"%s\",\"tbSubAccount_id\":\"%s\",\"ntype\":%d}",
					pAlarmRec->id,pAlarmRec->UT,
					ToStrTime(pAlarmRec->event_date).c_str(),
					pAlarmRec->description,
					pAlarmRec->status,
					"event",
					pAlarmRec->tbAccountid.c_str(),pAlarmRec->tbSubaccountid.c_str(),pAlarmRec->type);

			}
			result += szVal;

			if (i != (vec.size() - 1))
				result += ",";

			delete pAlarmRec;
		}

		vec.clear();
	}

	result += "]";

	return result;
}


//////////////////////////////////////////////////////////////////////////
// �ڲ�����


bool CBankData::InternalGetFav(std::vector<LPFAVRECORD>& vec)
{
	if(!CheckUserDB())
		return false;

	try
	{
		CppSQLite3Binary blob;
		CppSQLite3Query q = m_dbDataDB.execQuery("SELECT * FROM datFav order by favorder desc;");
		while (!q.eof()) {
			LPFAVRECORD pFav = new FAVRECORD;
			pFav->status = q.getIntField("status");
			strcpy_s(pFav->fav, sizeof(pFav->fav), ToSQLUnsafeString(q.getStringField("favinfo")).c_str());

			vec.push_back(pFav);
			q.nextRow();
		}

		q.finalize();
		return true;		
	}

	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalGetFav error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}
}
bool CBankData::InternalDeleteFav(std::string strFav)
{
	if(!CheckUserDB())
		return false;

	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("DELETE FROM datFav WHERE favinfo='%s';", strFav.c_str());
		m_dbDataDB.execDML(bufSQL);		
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalDeleteFav error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}
	return true;

}
bool CBankData::InternalSaveFav(std::string strFav, int status)
{
	if(!CheckUserDB())
		return false;

	try
	{
		bool isExist = false;
		CppSQLite3Buffer bufSQL;
		bufSQL.format("SELECT favinfo FROM datFav WHERE favinfo='%s';",strFav.c_str());
		CppSQLite3Query q = m_dbDataDB.execQuery(bufSQL);

		// �ҳ�favorder�����ֵ
		bufSQL.format ("select max(favorder) from datFav;");
		CppSQLite3Query p = m_dbDataDB.execQuery(bufSQL);
		int nMaxValue = p.getIntField ("max(favorder)");
		if (nMaxValue <= 0)
			nMaxValue = 1;
		else
			nMaxValue ++;

		if(!q.eof())
			isExist = true;

		q.finalize();
		p.finalize ();

		// ��ȡid
		int id = nMaxValue;
		if(!isExist)// ����ò��������ڣ��ǾͲ���
		{
			CppSQLite3Buffer bufSQL;
			bufSQL.format("INSERT INTO datFav VALUES('%s', %d, %d, 0);", strFav.c_str(), status, nMaxValue);
			m_dbDataDB.execDML(bufSQL);
		}
		else
		{	
			CppSQLite3Buffer bufSQL;
			bufSQL.format("UPDATE datFav SET status=%d WHERE favinfo='%s';", status, strFav.c_str());
			m_dbDataDB.execDML(bufSQL);
		}
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalSaveFav error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}
	return true;
}


INT64 CBankData::InternalAddEvent(LPEVENTRECORD lpEventRec)
{
	INT64 nId = GetNewID();
	INT64 nUT = GetNewUT();

	//ת��ʱ���ʽΪ������
	struct tm newtime;
	_localtime32_s(&newtime, &(lpEventRec->event_date));
	char datestring[128];
	strftime(datestring, 128, "%Y-%m-%d %w", &newtime);

	if(!CheckUserDB())
		return false;

	try
	{
		//CppSQLite3Buffer bufSQL;
		if (lpEventRec->id == 0)
		{
			char bufSQL[2560] = {0};
			sprintf_s(bufSQL, 2560, "INSERT INTO tbEvent (id, UT, mark, event_date, description, repeat, alarm, status, datestring, tbAccount_id, tbSubAccount_id, type)\
								 VALUES(%I64d, %I64d, 0, %d, '%s', %d, %d, %d, '%s', %s, %s, %d);",nId, nUT,
								 (int)lpEventRec->event_date,
								 lpEventRec->description,
								 lpEventRec->repeat,
								 lpEventRec->alarm,
								 lpEventRec->status,
								 datestring, lpEventRec->tbAccountid.c_str(),
								 lpEventRec->tbSubaccountid.c_str(), lpEventRec->type);

			m_dbUser.execDML(bufSQL);

			sprintf_s(bufSQL, 2560, "INSERT INTO tbTotalEvent (id, mark, event_date, repeat, alarm, status, datestring, type,datUserInfo_userid)\
									VALUES(%I64d, 0, %d, %d, %d, %d, '%s',%d, '%s');", 
									nId, 
									(int)lpEventRec->event_date,
									lpEventRec->repeat,
									lpEventRec->alarm,
									lpEventRec->status,
									datestring, lpEventRec->type,
									m_CurUserInfo.struserid.c_str());

			GetDataDbObject()->execDML(bufSQL);
		}
		else
		{
			char bufSQL[2560] = {0};
			nId = lpEventRec->id;

			sprintf_s(bufSQL, 2560, "UPDATE tbEvent SET UT=%I64d, event_date=%d, description='%s', repeat=%d, alarm=%d, status=%d, datestring='%s' ,tbAccount_id=%s, tbSubAccount_id=%s, type=%d WHERE id=%I64d;",
				nUT,(int)lpEventRec->event_date,
				lpEventRec->description,
				lpEventRec->repeat,
				lpEventRec->alarm,
				lpEventRec->status,
				datestring,
				lpEventRec->tbAccountid.c_str(),lpEventRec->tbSubaccountid.c_str(),lpEventRec->type,
				lpEventRec->id);

			m_dbUser.execDML(bufSQL);

			sprintf_s(bufSQL, 2560, "UPDATE tbTotalEvent SET event_date=%d, repeat=%d, alarm=%d, status=%d, datestring='%s' ,type=%d WHERE id=%I64d AND datUserInfo_userid='%s';",
						  (int)lpEventRec->event_date,
						  lpEventRec->repeat,
						  lpEventRec->alarm,
						  lpEventRec->status,
						  datestring, lpEventRec->type,
						  nId,
						  m_CurUserInfo.struserid.c_str());

			GetDataDbObject()->execDML(bufSQL);
		}
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalAddEvent error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return 0;
	}

	return nId;
}

int CBankData::DeleteAllEvent()
{
	ObjectLock lock(this);

	if(!CheckUserDB())
		return 0;

	try
	{
		INT64 nUT = GetNewUT();
		char bufSQL[2560] = {0};
		sprintf_s(bufSQL, 2560, "UPDATE tbEvent SET UT=%I64d, mark=1", nUT);

		m_dbUser.execDML(bufSQL);

		sprintf_s(bufSQL, 2560, "UPDATE tbTotalEvent SET mark=1 WHERE datUserInfo_userid='%s';", m_CurUserInfo.struserid.c_str());

		GetDataDbObject()->execDML(bufSQL);

	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalDeleteEvent error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return 0;
	}

	return 1;
}
bool CBankData::InternalDeleteEvent(INT64 id)
{
	if(!CheckUserDB())
		return false;
	try
	{
		if (!m_dbUser.tableExists("tbEvent"))
			return true;

		INT64 nUT = GetNewUT();
		char bufSQL[2560] = {0};
		sprintf_s(bufSQL, 2560, "UPDATE tbEvent SET UT=%I64d, mark=1 WHERE id=%I64d;", nUT, id);

		m_dbUser.execDML(bufSQL);

		sprintf_s(bufSQL, 2560, "UPDATE tbTotalEvent SET mark=1 WHERE id=%I64d AND datUserInfo_userid='%s';", id, m_CurUserInfo.struserid.c_str());

		GetDataDbObject()->execDML(bufSQL);

	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalDeleteEvent error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

	return true;
}

bool CBankData::InternalGetEvents(int year, int month, std::vector<LPEVENTRECORD>& vecEventRec)
{
	assert(vecEventRec.empty());

	if(!CheckUserDB())
		return false;
	try
	{
		if (!m_dbUser.tableExists("tbEvent"))
			return true;

		// ��ʼʱ��
		struct tm tmVal;
		memset(&tmVal, 0, sizeof(struct tm));
		tmVal.tm_mday = 1;
		tmVal.tm_year = year - 1900;
		tmVal.tm_mon = month - 1;
		
		__time32_t tHead = _mktime32(&tmVal);

		// ����ʱ��
		memset(&tmVal, 0, sizeof(struct tm));
		tmVal.tm_mday = 1;
		if (month < 12)
		{
			tmVal.tm_year = year - 1900;
			tmVal.tm_mon = month;
		}
		else
		{
			tmVal.tm_year = year - 1900 + 1;
			tmVal.tm_mon = 0;
		}

		__time32_t tTail = _mktime32(&tmVal);


/*
		// Query
		CppSQLite3Buffer bufSQL;
		bufSQL.format("SELECT * FROM tbEvent WHERE event_date>=%d AND event_date<%d;", tHead, tTail);

		CppSQLite3Query q = db.execQuery(bufSQL);
		while (!q.eof())
		{
			LPEVENTRECORD pEventRec = new EVENTRECORD;
			
			pEventRec->id = q.getIntField("id");
			pEventRec->event_date = q.getIntField("event_date");
			strcpy_s(pEventRec->description, sizeof(pEventRec->description), ToSQLUnsafeString(q.getStringField("description")).c_str());
			pEventRec->repeat = q.getIntField("repeat");
			pEventRec->alarm = q.getIntField("alarm");
			pEventRec->status = q.getIntField("status");

			vecEventRec.push_back(pEventRec);

			q.nextRow();
		}
		q.finalize();
*/

		// Query
		CppSQLite3Buffer bufSQL;
		bufSQL.format("SELECT * FROM tbEvent WHERE event_date<%d AND mark=0;", tTail);

		CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
		while (!q.eof())
		{
			__time32_t event_date = q.getIntField("event_date");
			int repeat1 = q.getIntField("repeat");

			std::vector<__time32_t> vecShowDates;
			if (IsMonthShowEvent(tHead, tTail, event_date, repeat1, &vecShowDates))
			{
				for (size_t i = 0; i < vecShowDates.size(); i++)
				{
					LPEVENTRECORD pEventRec = new EVENTRECORD;

					pEventRec->id = q.getInt64Field("id");
					pEventRec->event_date = vecShowDates[i];
					pEventRec->start_date = event_date;
					strcpy_s(pEventRec->description, sizeof(pEventRec->description), ToSQLUnsafeString(q.getStringField("description")).c_str());
					pEventRec->repeat = q.getIntField("repeat");
					pEventRec->alarm = q.getIntField("alarm");
					pEventRec->status = q.getIntField("status");
					pEventRec->UT = q.getInt64Field("UT");
					char accountid[256] = {0};
					sprintf_s(accountid, 256,"%I64d", q.getInt64Field("tbAccount_id"));
					pEventRec->tbAccountid = accountid; 

					char subaccountid[256] = {0};
					sprintf_s(subaccountid, 256,"%I64d", q.getInt64Field("tbSubAccount_id"));
					pEventRec->tbSubaccountid = subaccountid; 
					pEventRec->type = q.getIntField("type");

					vecEventRec.push_back(pEventRec);
				}
			}
			
			q.nextRow();
		}
		q.finalize();
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalGetEvents error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

	return true;
}

bool CBankData::InternalGetEventsOneDay(int year, int month, int day, std::vector<LPEVENTRECORD>& vecEventRec)
{
	assert(vecEventRec.empty());

	if(!CheckUserDB())
		return false;

	try
	{
		if (!m_dbUser.tableExists("tbEvent"))
			return true;

		// ��ʼʱ��
		struct tm tmVal;
		memset(&tmVal, 0, sizeof(struct tm));
		tmVal.tm_year = year - 1900;
		tmVal.tm_mon = month - 1;
		tmVal.tm_mday = day;

		__time32_t tHead = _mktime32(&tmVal);

		//�������ڼ�
		struct tm newtime;
		_localtime32_s(&newtime, &tHead);
		int wday = newtime.tm_wday;

		// ����ʱ��
		__time32_t tTail = tHead + 24 * 60 * 60;

		// Query
		CppSQLite3Buffer bufSQL;
		bufSQL.format("SELECT * FROM tbEvent WHERE mark=0 AND event_date<%d AND ((repeat=0 AND event_date>=%d) OR (repeat=1 AND SUBSTR(datestring,12)='%d') OR (repeat=2 AND SUBSTR(datestring,9,2)='%02d') OR (repeat=3 AND SUBSTR(datestring,6,2)%%3=%d AND SUBSTR(datestring,9,2)='%02d') OR (repeat=4 AND SUBSTR(datestring,6,2)='%02d' AND SUBSTR(datestring,9,2)='%02d'));", tTail, tHead, wday, day, month%3, day, month, day);

		CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
		while (!q.eof())
		{
			LPEVENTRECORD pEventRec = new EVENTRECORD;

			pEventRec->id = q.getInt64Field("id");
			pEventRec->UT = q.getInt64Field("UT");
			pEventRec->event_date = q.getIntField("event_date");
			pEventRec->start_date = q.getIntField("event_date");
			strcpy_s(pEventRec->description, sizeof(pEventRec->description), ToSQLUnsafeString(q.getStringField("description")).c_str());
			pEventRec->repeat = q.getIntField("repeat");
			pEventRec->alarm = q.getIntField("alarm");
			pEventRec->status = q.getIntField("status");
			char sAccountid[256] = {0};
			sprintf_s(sAccountid, 256, "%I64d", q.getInt64Field("tbAccount_id"));
			pEventRec->tbAccountid = sAccountid;
			char sSubAccountid[256] = {0};
			sprintf_s(sSubAccountid, 256, "%I64d", q.getInt64Field("tbSubAccount_id"));
			pEventRec->tbSubaccountid = sSubAccountid;
			pEventRec->type = q.getIntField("type");

			vecEventRec.push_back(pEventRec);

			q.nextRow();
		}
		q.finalize();
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalGetEvents error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

	return true;
}

bool CBankData::InternalSetAlarm(INT64 id, int status, std::string alarmtype)
{
	if(!CheckUserDB())
		return false;

	try
	{
		if (!m_dbUser.tableExists("tbEvent"))
			return true;

		char bufSQL[2560] = {0};
		if (alarmtype == "event") {
			sprintf_s(bufSQL, 2560, "UPDATE tbEvent SET status=%d WHERE id=%I64d;", status, id);
			m_dbUser.execDML(bufSQL);

			sprintf_s(bufSQL, 2560, "UPDATE tbTotalEvent SET status=%d WHERE id=%I64d AND datUserInfo_userid='%s';", status, id, m_CurUserInfo.struserid.c_str());
			GetDataDbObject()->execDML(bufSQL);
		}

		

	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalSetAlarm error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

	return true;
}

//
// ���� status = 1 & status = 2 �������¼�
//
bool CBankData::InternalGetTodayAlarms(std::vector<LPALARMRECORD>& vec)
{
	if(!CheckUserDB())
		return false;

	try
	{
		//////////////////////////////////////////////////////////////////////////
		// ��ý����ʱ��

		SYSTEMTIME systime;
		GetSystemTime(&systime);

		struct tm tmVal;
		memset(&tmVal, 0, sizeof(struct tm));
		tmVal.tm_year = systime.wYear - 1900;
		tmVal.tm_mon = systime.wMonth - 1;
		tmVal.tm_mday = systime.wDay;

		__time32_t tToday = _mktime32(&tmVal);

		//////////////////////////////////////////////////////////////////////////
		// From tbEvent

		if (m_dbUser.tableExists("tbEvent"))
		{
			CppSQLite3Buffer bufSQL;
			bufSQL.format("SELECT * FROM tbEvent WHERE mark=0 ORDER BY event_date;");
			
			CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
			while (!q.eof())
			{
				INT64 id = q.getInt64Field("id");
				__time32_t event_date = q.getIntField("event_date");
				int repeat1 = q.getIntField("repeat");
				int alarm = q.getIntField("alarm");

				__time32_t alarm_date;

				if (IsAlarmEvent(tToday, event_date, repeat1, alarm, &alarm_date))
				{
					LPALARMRECORD pAlarmRec = new ALARMRECORD;
					
					pAlarmRec->type = 0;
					pAlarmRec->id = q.getInt64Field("id");
					
					// ���ص��Ǿ���ʱ�䣬���Ǵ���ʱ��
					//pAlarmRec->event_date = q.getIntField("event_date");
					pAlarmRec->event_date = alarm_date;

					strcpy_s(pAlarmRec->description, sizeof(pAlarmRec->description), ToSQLUnsafeString(q.getStringField("description")).c_str());
					pAlarmRec->status = q.getIntField("status");
					pAlarmRec->UT = q.getInt64Field("UT");
					char accountid[256] = {0};
					sprintf_s(accountid, 256,"%I64d", q.getInt64Field("tbAccount_id"));
					pAlarmRec->tbAccountid = accountid;
					char subaccountid[256] = {0};
					sprintf_s(subaccountid, 256,"%I64d", q.getInt64Field("tbSubAccount_id"));
					pAlarmRec->tbSubaccountid = subaccountid; 
					pAlarmRec->ntype = q.getIntField("type");
					
					vec.push_back(pAlarmRec);
				}

				q.nextRow();
			}
			q.finalize();
		}
		
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalGetTodayAlarms error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

	return true;
}


std::string CBankData::ToSQLSafeString(std::string strQuery)
{
	std::string strOut;
	for (size_t i = 0; i < strQuery.size(); i++)
	{
		if (strQuery[i] == '"')
			strOut += "\\\"";
		else
			strOut += strQuery[i];
	}

	return strOut;
}

std::string CBankData::ToSQLUnsafeString(std::string strQuery)
{
	std::string strOut;
	for (size_t i = 0; i < strQuery.size(); i++)
	{
		if (i < (strQuery.size() - 1) && strQuery[i] == '\\' && strQuery[i + 1] == '"')
		{
			strOut += "\"";
			i++;
		}
		else
			strOut += strQuery[i];
	}

	return strOut;
}

std::string CBankData::ToStrTime(__time32_t t)
{
	std::string result;
	struct tm tmVal;

	if (_localtime32_s(&tmVal, &t) == 0)
	{
		char szTime[32];
		sprintf_s(szTime, "%04d-%02d-%02d", tmVal.tm_year + 1900, tmVal.tm_mon + 1, tmVal.tm_mday);
		result = szTime;
	}

	return result;
}

__time32_t CBankData::ToIntTime(std::string t)
{
	int year, month, day;
	if (3 == sscanf_s(t.c_str(), "%d-%d-%d", &year, &month, &day))
	{
		struct tm tmVal;
		memset(&tmVal, 0, sizeof(struct tm));
		tmVal.tm_year = year - 1900;
		tmVal.tm_mon = month - 1;
		tmVal.tm_mday = day;

		return _mktime32(&tmVal);
	}

	return -1;
}

bool CBankData::IsAlarmEvent(__time32_t today, __time32_t event_date, int repeat1, int alarm, __time32_t* pAlarmDate)
{
	const int one_day = 24 * 60 * 60;

	struct tm tmToday;
	struct tm tmEvent;

	if ((today + alarm * one_day) < event_date)
		return false;

	if (repeat1 == 0)	// ���ظ�
	{
		if (event_date >= today && event_date < (today + (alarm + 1) * one_day))
		{
			if (pAlarmDate)
				*pAlarmDate = event_date;

			return true;
		}
	}
	else if (repeat1 == 1) // ���ظ�
	{
		if (_localtime32_s(&tmToday, &today) == 0 && _localtime32_s(&tmEvent, &event_date) == 0)
		{
			int diff_day = tmEvent.tm_wday - tmToday.tm_wday;
			if (diff_day < 0) diff_day += 7;

			event_date = today + diff_day * one_day;
			if ((today + alarm * one_day) >= event_date)
			{
				if (pAlarmDate)
					*pAlarmDate = event_date;

				return true;
			}
		}
	}
	else if (repeat1 == 2) // ���ظ�
	{
		if (_localtime32_s(&tmToday, &today) == 0 && _localtime32_s(&tmEvent, &event_date) == 0)
		{
			tmEvent.tm_year = tmToday.tm_year;
			tmEvent.tm_mon = tmToday.tm_mon;

			int diff_day = tmEvent.tm_mday - tmToday.tm_mday;
			if (diff_day < 0)
			{
				tmEvent.tm_mon++;
				if (tmEvent.tm_mon == 12)
				{
					tmEvent.tm_year++;
					tmEvent.tm_mon = 0;
				}
			}

			AdjustTimeValue(&tmEvent);
			event_date = _mktime32(&tmEvent);

			if ((today + alarm * one_day) >= event_date)
			{
				if (pAlarmDate)
					*pAlarmDate = event_date;

				return true;
			}
		}
	}
	else if (repeat1 == 3) // �����ظ�
	{
		if (_localtime32_s(&tmToday, &today) == 0 && _localtime32_s(&tmEvent, &event_date) == 0)
		{
			tmEvent.tm_year = tmToday.tm_year;
						
			if (tmEvent.tm_mon < tmToday.tm_mon ||
				((tmEvent.tm_mon == tmToday.tm_mon) && (tmEvent.tm_mday < tmToday.tm_mday)))
			{
				int nEventQuarter = (tmEvent.tm_mon / 3 + 1);
				int nTodayQuarter = (tmToday.tm_mon / 3 + 1);			
				
				tmEvent.tm_mon += (nTodayQuarter - nEventQuarter) * 3;
				if (tmEvent.tm_mon < tmToday.tm_mon || (tmEvent.tm_mon == tmToday.tm_mon && tmEvent.tm_mday < tmToday.tm_mday))
					tmEvent.tm_mon++;

				if (tmEvent.tm_mon >= 12)
				{
					tmEvent.tm_mon %= 12;
					tmEvent.tm_year++;
				}
			}

			AdjustTimeValue(&tmEvent);
			event_date = _mktime32(&tmEvent);

			if ((today + alarm * one_day) >= event_date)
			{
				if (pAlarmDate)
					*pAlarmDate = event_date;

				return true;
			}
		}		
	}
	else if (repeat1 == 4) // ���ظ�
	{
		if (_localtime32_s(&tmToday, &today) == 0 && _localtime32_s(&tmEvent, &event_date) == 0)
		{
			tmEvent.tm_year = tmToday.tm_year;

			if (tmEvent.tm_mon < tmToday.tm_mon || 
				((tmEvent.tm_mon == tmToday.tm_mon) && (tmEvent.tm_mday < tmToday.tm_mday)))
			{
				tmEvent.tm_year++;
			}

			AdjustTimeValue(&tmEvent);
			event_date = _mktime32(&tmEvent);

			if ((today + alarm * one_day) >= event_date)
			{
				if (pAlarmDate)
					*pAlarmDate = event_date;

				return true;
			}
		}	
	}

	return false;
}

bool CBankData::NotLeapYear(int year)
{
	return !(year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

void CBankData::AdjustTimeValue(struct tm* _tm)
{
	if (_tm->tm_mon == 3 ||			// 4��
		_tm->tm_mon == 5 ||			// 6��
		_tm->tm_mon == 8 ||			// 9��
		_tm->tm_mon == 10)			// 11��
	{
		if (_tm->tm_mday == 31)
			_tm->tm_mday = 30;
	}

	if (_tm->tm_mon == 1)			// 2��
	{
		if (_tm->tm_mday > 28)
		{
			if (NotLeapYear(_tm->tm_year + 1900))
				_tm->tm_mday = 28;
			else
				_tm->tm_mday = 29;
		}
	}
}

bool CBankData::IsMonthShowEvent(__time32_t tHead, __time32_t tTail, __time32_t event_date, int repeat1, std::vector<__time32_t>* pvecShowDates)
{
	const int one_day = 24 * 60 * 60;

	struct tm tmHead;
	struct tm tmTail;
	_localtime32_s(&tmHead, &tHead);
	_localtime32_s(&tmTail, &tTail);


	int nCount = 0;
	if (repeat1 == 0)	// ���ظ�
	{
		if (event_date >= tHead && event_date < tTail)
		{
			nCount++;
			if (pvecShowDates)
				pvecShowDates->push_back(event_date);
		}
	}
	else if (repeat1 == 1)	// ���ظ�
	{
		struct tm tmEvent;
		if (_localtime32_s(&tmEvent, &event_date) == 0)
		{
			int nDiff = tmEvent.tm_wday - tmHead.tm_wday;
			if (nDiff < 0) nDiff += 7;

			__time32_t tNow = tHead + nDiff * one_day;
			while (true)
			{
				if (tNow >= event_date && tNow < tTail)
				{
					nCount++;
					if (pvecShowDates)
						pvecShowDates->push_back(tNow);
				}

				tNow += 7 * one_day;

				if (tNow >= tTail)
					break;
			}
		}
	}
	else if (repeat1 == 2)	// ���ظ�
	{
		struct tm tmEvent;
		if (_localtime32_s(&tmEvent, &event_date) == 0)
		{
			tmEvent.tm_mon = tmHead.tm_mon;
			tmEvent.tm_year = tmHead.tm_year;
			AdjustTimeValue(&tmEvent);
			
			__time32_t tNow = _mktime32(&tmEvent);

			nCount++;
			if (pvecShowDates)
				pvecShowDates->push_back(tNow);
		}

	}
	else if (repeat1 == 3)	// ���ظ�
	{
		struct tm tmEvent;
		if (_localtime32_s(&tmEvent, &event_date) == 0)
		{
			if ((tmEvent.tm_mon - tmHead.tm_mon) % 3 == 0)
			{
				tmEvent.tm_mon = tmHead.tm_mon;
				tmEvent.tm_year = tmHead.tm_year;
				AdjustTimeValue(&tmEvent);

				__time32_t tNow = _mktime32(&tmEvent);

				nCount++;
				if (pvecShowDates)
					pvecShowDates->push_back(tNow);
			}
		}		
	}
	else if (repeat1 == 4)	// ���ظ�
	{
		struct tm tmEvent;
		if (_localtime32_s(&tmEvent, &event_date) == 0)
		{
			if (tmEvent.tm_mon == tmHead.tm_mon)
			{
				tmEvent.tm_year = tmHead.tm_year;
				AdjustTimeValue(&tmEvent);

				__time32_t tNow = _mktime32(&tmEvent);

				nCount++;
				if (pvecShowDates)
					pvecShowDates->push_back(tNow);
			}
		}		
	}

	return nCount > 0;
}

bool CBankData::IsValidEventJson(Json::Value& root)
{
	return root.isMember("id") && 
		root.isMember("event_date") && 
		root.isMember("description") &&
		root.isMember("repeat") && 
		root.isMember("alarm") && 
		root.isMember("status");
}

bool CBankData::IsValidAlarmJson(Json::Value& root)
{
	return root.isMember("id") &&
		root.isMember("status");

	return true;
}



// �������ݿ�����
std::string CBankData::QuerySQL(std::string strSQL, std::string strDBName)
{
	ObjectLock lock(this);
	return InternalQuerySQL(strSQL, strDBName);
}

int CBankData::ExecuteSQL(std::string strSQL, std::string strDBName)
{
	ObjectLock lock(this);
	return InternalExecuteSQL(strSQL, strDBName);
}

std::string CBankData::GetXMLData(const std::string& strStartDate, const std::string& strEndDate, const std::string& dataseries, const std::string& charttype)
{
	ObjectLock lock(this);
	if ("Pie" == charttype)
		return InternalGetPieXMLData(strStartDate, strEndDate); // �õ�PIEͼ���ݣ�����Pieͼ��SQL��䲻һ����
	else if("newPie" == charttype)
		return InternalGetNewPieXMLData(strStartDate, strEndDate); // �õ���Pieͼ������
	else
		return InternalGetBarXMLData(strStartDate, strEndDate, dataseries); // �õ�Barͼ����
}

std::string CBankData::InternalQuerySQL(std::string strSQL, std::string strDBName)
{
	//assert(vecEventRec.empty());
	std::string strSQLRec = "";

	try
	{
		CppSQLite3DB* pdb = NULL;
		if(strDBName == "DataDB")
		{
			pdb = GetDataDbObject();
		}
		else if(strDBName == "SysDB")
		{
			pdb = GetSysDbObject();
		}
		else
		{
			if(!CheckUserDB())
				return "";
			pdb = &m_dbUser;
		}

		if(pdb == NULL)
			return "";
		// Query
		CppSQLite3Query q = pdb->execQuery(strSQL.c_str());

		strSQLRec = "[";
		while (!q.eof())
		{
			strSQLRec += "{";
			for (int i=0; i<q.numFields(); i++) {
				char szVal[1024];
				switch (q.fieldDataType(i)) {
					case 1:
						{
							__int64 i64Val = q.getInt64Field(q.fieldName(i));
							//������
							sprintf_s(szVal, sizeof(szVal), "\"%s\":%I64d,", q.fieldName(i), i64Val);
							strSQLRec += szVal;
							break;
						}

					case 2:
						//������
						sprintf_s(szVal, sizeof(szVal), "\"%s\":\"%.2f\",", q.fieldName(i), q.getFloatField(q.fieldName(i)));
						strSQLRec += szVal;
						break;

					case 3:
						{
							std::string strVal = ToSQLUnsafeString(q.getStringField(q.fieldName(i))).c_str();
							ReplaceCharInString(strVal, "\\", "\\\\");
							ReplaceCharInString(strVal, "\"", "\\\"");
							//ReplaceCharInString(strVal, "'", "\\'");

							//�ַ���
							sprintf_s(szVal, sizeof(szVal), "\"%s\":\"%s\",", q.fieldName(i), strVal.c_str());
							strSQLRec += szVal;
							break;
						}

					default:
						break;
				}
			}
			strSQLRec = strSQLRec.substr(0, strSQLRec.length() - 1);
			strSQLRec += "},";
			
			q.nextRow();
		}
		if (strSQLRec != "[") strSQLRec = strSQLRec.substr(0, strSQLRec.length() - 1);
		strSQLRec += "]";
		q.finalize();
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"Query sql error:%d", ex.errorCode()));
#ifndef OFFICIAL_VERSION // ��ʽ�汾�����
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"Query sql:%s", CA2W(strSQL.c_str())));
#endif
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return "";
	}
	return strSQLRec;
}

int CBankData::InternalExecuteSQL(std::string strSQL, std::string strDBName)
{
	std::string strSQLRec = "";

	// ת���ɴ�д
	std::string strTp = strSQL;
	transform(strTp.begin(), strTp.end(), strTp.begin(), towupper);

	CppSQLite3DB* pdb = NULL;
	if(strDBName == "DataDB")
	{
		pdb = GetDataDbObject();
	}
	else if(strDBName == "SysDB")
	{
		pdb = GetSysDbObject();
	}
	else
	{
		if(!CheckUserDB())
			return -1;
		pdb = &m_dbUser;
	}

	if(pdb == NULL)
		return -1;

	
	if(!CheckUserDB())
		return false;

	try
	{
		// ִ��SQL���
		pdb->execDML(strSQL.c_str());
		if (strTp.find("INSERT", 0) != string::npos)
		{
			//�����INSERT��䣬��ȡ�����²��������ֵ�����ء�
			CppSQLite3Buffer bufSQL;
			bufSQL.format("SELECT last_insert_rowid();");
			CppSQLite3Query q = pdb->execQuery(bufSQL);
			int iNewId = 0;
			if (!q.eof())
			{
				iNewId = q.getIntField(0);
			}
			q.finalize();
			return iNewId;
		}
		else
		{
			//�����UPDATE����DELETE��䣬�򷵻�1��ʾ�ɹ���
			return 1;
		}
	}
	catch (CppSQLite3Exception& ex)
	{
		//SQL����д��󣬷���0
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"Excute sql error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return 0;
	}
}

bool CBankData::IsFileExist(LPWSTR lpPath)
{
	ATLASSERT(NULL != lpPath);
	if (NULL == lpPath)
		return false;

	HANDLE hFile = CreateFileW(lpPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DWORD dwErr = GetLastError();
		// 2��ʾ���ʵ��ļ�������
		if(2 == dwErr)
			return false;
	}
	else
	{
		DWORD dwLength = GetFileSize(hFile, NULL);
		if(dwLength < 10)
		{
			CloseHandle(hFile);
			DeleteFileW(lpPath);
			return false;
		}
			
	}

	CloseHandle(hFile);

	return true;
}

// �õ�������ص���һ����SQL���
bool CBankData::GetExchangePartSql(LPCSTR lpStrConditon, std::string& strExchange)
{
	strExchange.clear();

	CppSQLite3Buffer bufSQL;
	bufSQL.format("select sysCurrency_id1 as ID, ExchangeRate from datExchangeRate where sysCurrency_id = 1");
	try
	{
		//CppSQLite3DB db;
		//db.open(GetDbPath("DataDB"));
		CppSQLite3Query q = GetDataDbObject()->execQuery(bufSQL);
		while(!q.eof())
		{
			strExchange += "when ";
			strExchange += q.getStringField("ID");
			strExchange += " then ";
			strExchange += lpStrConditon;
			strExchange += "/100* ";
			strExchange += q.getStringField("ExchangeRate");
			strExchange += " ";

			q.nextRow();

		}
		q.finalize();
	}
	catch (CppSQLite3Exception&)
	{
		return false;
	}
	return true;
}

std::string CBankData::InternalGetBarXMLData(const std::string& strStartDate, const std::string& strEndDate, const std::string& strKind)
{
	CACULATE_TYPE emType;
	std::string strType;
	std::string strFormat;

	// ����ͼ�ķ�������
	if ("year" == strKind) // �����
	{
		emType = emCACULATEYEAR;
		strFormat = "%Y��";
	}
	else if("month" == strKind) // ���·�
	{
		emType = emCACULATEMONTH;
		strFormat = "%Y��%m��";
	}
	else if("season" == strKind) // ������
	{
		emType = emCACULATESEASON;
	}
	else if("week" == strKind) // ���ܷ�
	{
		emType = emCACULATEWEEK;
		strFormat = "%Y��%W��";
	}
	else
		return "<chart></chart>";

	int nSize = 0;

	// �õ�������ز��ֵ�SQL���
	std::string strExPart;
	if (!GetExchangePartSql("t.amount", strExPart))
		return "<chart></chart>";

	TiXmlDocument xmldoc;
	TiXmlElement *pRootPart = new TiXmlElement("chart");
	xmldoc.LinkEndChild(pRootPart);

	try
	{
		CppSQLite3Buffer bufSQL;

		if (!strFormat.empty()) // ���꣬�£��ܵ�SQL���
			bufSQL.format("select sum(SumAmount),Date,Type, Name from (SELECT \
							strftime('%s',t.TransDate) AS Date,\
							c1.Type AS Type,c1.name AS Name, c1.id as myID,round(SUM((case d.tbcurrency_id  %s \
							end )), 2) AS SumAmount FROM tbtransaction AS t, tbcategory2 AS c2, \
							tbcategory1 AS c1, tbsubAccount  d\
							WHERE t.mark = 0 and c1.mark = 0 and c2.mark = 0 and d.mark = 0 and  t.tbcategory2_id=c2.id AND c2.tbcategory1_id=c1.id  \
							and t.tbsubaccount_id= d.id and tbCategory1_id not in (10018,10019, 10025) and t.TransDate >= '%s' and t.TransDate <= '%s' \
							GROUP BY c1.id,Date ORDER BY c1.id,Date ) as temp group by Date,Type, myID;", strFormat.c_str(), strExPart.c_str(), strStartDate.c_str(), strEndDate.c_str());
		else // �����ֵ�SQL���
			bufSQL.format("select sum(SumAmount),Date,Type, Name from (SELECT \
							(case  substr(transdate,6,2)\
							when '01'  then substr(transdate,0,5)||'��1��' when '02' then substr(transdate,0,5)||'��1��' when '03' \
							then substr(transdate,0,5)||'��1��'\
							when '06'  then substr(transdate,0,5)||'��2��' when '05' then substr(transdate,0,5)||'��2��' when '06' \
							then substr(transdate,0,5)||'��2��'\
							when '07'  then substr(transdate,0,5)||'��3��' when '08' then substr(transdate,0,5)||'��3��' when '09' \
							then substr(transdate,0,5)||'��3��'\
							when '10'  then substr(transdate,0,5)||'��4��' when '11' then substr(transdate,0,5)||'��4��' when '12' \
							then substr(transdate,0,5)||'��4��'\
							 end )  as Date,\
							c1.Type AS Type,c1.name AS Name, c1.id as myID,round(SUM((case d.tbcurrency_id  %s  \
							end )), 2) AS SumAmount FROM tbtransaction AS t, tbcategory2 AS c2, \
							tbcategory1 AS c1, tbsubAccount  d\
							WHERE t.mark = 0 and c1.mark = 0 and c2.mark = 0 and d.mark = 0 and t.tbcategory2_id=c2.id AND c2.tbcategory1_id=c1.id  \
							and t.tbsubaccount_id= d.id and tbCategory1_id not in (10018,10019,10025) and t.TransDate >= '%s' and t.TransDate <= '%s' \
							GROUP BY c1.id, Date ORDER BY c1.id,Date ) as temp group by Date,Type, myID;", strExPart.c_str(), strStartDate.c_str(), strEndDate.c_str());

		// �������ݿ������״��
		if(!CheckUserDB())
			return "<chart></chart>";


		CppSQLite3Query q = m_dbUser.execQuery(bufSQL);

		typedef std::map<std::string, double> myFirstMap; // ���������洢ʱ��������ƣ���һ�������洢ֵ
		std::map<std::string, myFirstMap> myMap; // ���������洢ʱ�䣨�ꡢ�¡������ܣ�
		std::map<std::string, myFirstMap>::iterator myMapIt;
		std::map<std::string, double>::const_iterator myFMapIt;
		std::map<std::string, bool> myNameMap; //  ������¼���з��õ�����

		while(!q.eof())
		{
			std::string strDate = q.getStringField("Date");
			if (strDate.empty())
			{
				q.nextRow();
				continue;
			}

			if (myMap.end() == myMap.find(strDate))
			{
				myFirstMap tempNode;
				myMap.insert(make_pair(strDate, tempNode));
			}

			myMapIt = myMap.find(strDate);
			std::string strName = q.getStringField("Name");
			
			myFMapIt = (*myMapIt).second.find(strName);
			
			if ((*myMapIt).second.end() == myFMapIt)
			{
				myNameMap.insert(make_pair(strName, false)); // ��¼��������
				double dVal = q.getFloatField("sum(SumAmount)");
				if(dVal < 0) // ֧�������ݿ��ܻ�С��0
					dVal *= -1;

				if (0 == q.getIntField("Type"))
					dVal *= -1;

				(*myMapIt).second.insert(make_pair(strName, dVal));
			}
			q.nextRow();
		}
		q.finalize();


		TiXmlElement *pCategories = new TiXmlElement("categories"); // ��ͼ�ǰ�ʱ�����
		pRootPart->LinkEndChild(pCategories);


		std::map<std::string, TiXmlElement *> readMap;
		std::map<std::string, TiXmlElement *>::const_iterator cstSubIt;

		// ��ĳһʱ��εķ��úͳ��ֵ����еķ��ý��бȶԣ�����ʱ���û�г��ֵķ����������һ��Ϊ0�ķ���
		int nSize = myMap.size();
		std::map<std::string, bool>::const_iterator cstNameMap;
		for (myMapIt = myMap.begin(); myMapIt != myMap.end(); myMapIt ++)
		{
			for(cstNameMap = myNameMap.begin(); cstNameMap != myNameMap.end(); cstNameMap ++)
			{
				std::string strName = (*cstNameMap).first;
				if ((*myMapIt).second.find(strName) == (*myMapIt).second.end())
				{
					(*myMapIt).second.insert(make_pair(strName, 0));
				}
			}
			
		}

			/*<?xml version="1.0" encoding="UTF-8"?>
			<chart>
			<categories>
			<category label='һ��' />
			<category label='����' />
			</categories>
			<dataset seriesName='��ʳ'>
			<set value='200.00' />
			<set value='150.00'/>
			</dataset>
			<dataset seriesName='��ͨ'>
			<set value='300.00'/>
			<set value='320.00'/>
			</dataset>
			</chart>*/


		// ��ȡ���ݣ�����������ʽ�洢
		for (myMapIt = myMap.begin(); myMapIt != myMap.end(); myMapIt ++)
		{
			TiXmlElement *pSet = new TiXmlElement("category");
			
			pSet->SetAttribute("Name", (*myMapIt).first.c_str());
			pCategories->LinkEndChild(pSet);

			
			for (myFMapIt = (*myMapIt).second.begin(); myFMapIt != (*myMapIt).second.end(); myFMapIt ++)
			{
				std::string strTpName = (*myFMapIt).first;
				TiXmlElement *pDataset = NULL;
				cstSubIt = readMap.find(strTpName);
				if (readMap.end() == cstSubIt)
				{
					pDataset = new TiXmlElement("dataset");
					pDataset->SetAttribute("seriesName", (*myFMapIt).first.c_str());
					pRootPart->LinkEndChild(pDataset);
					readMap.insert(make_pair((*myFMapIt).first, pDataset));
				}
				else
					pDataset = (*cstSubIt).second;

				if (NULL == pDataset)
					continue;

				TiXmlElement *pSubSet = new TiXmlElement("set");
				SetIndexDoubleAttribute(pSubSet, "value", (*myFMapIt).second);
				pDataset->LinkEndChild(pSubSet);

			}
		}

	}
	catch (CppSQLite3Exception&)
	{
		return "<chart></chart>";
	}

	TiXmlPrinter printer;
	printer.SetIndent( "    " );

	pRootPart->Accept(&printer);
	std::string strBack;
	strBack = printer.CStr();

	// ���ַ��������е�<chart />��<chart></chart>�����滻
	ReplaceCharInString(strBack, "<chart />", "<chart></chart>");

	return strBack;
}

std::string CBankData::InternalGetNewPieXMLData(std::string strStartDate, std::string strEndDate)
{
	// �ַ���������6��Ƭ��
	// ��һ��Ƭ���ǣ���ʾ����ĸ����ü�ֵ
	// �ڶ���Ƭ���ǣ���ʾ֧���ĸ����ü�ֵ
	// ������Ƭ���ǣ���ʾ�����֧����%
	// ������Ƭ���ǣ���ʾ�����֧����ֵ�����ģ�������Ƭ��û�ã�
	std::string strSQLRec = "";
	TiXmlPrinter printer1, printer2, printer3;
	TiXmlPrinter printer4, printer5, printer6;
	printer1.SetIndent( "    " );
	printer2.SetIndent( "    " );
	printer3.SetIndent( "    " );
	printer4.SetIndent( "    " );
	printer5.SetIndent( "    " );
	printer6.SetIndent( "    " );

	if(!CheckUserDB())
		return "<chart></chart>";

	// �õ������ǲ��ֵ�SQL���
	std::string strExPart;
	if (!GetExchangePartSql("balance", strExPart))
		return "<chart></chart>";

	try
	{
		// ȡ�������·�
		CppSQLite3Buffer bufSQL;
		
		bufSQL.format("select Sum(SumAmount) as SumAmountAll,id,Name from (select round(SUM((case tbcurrency_id  %s end )), 2) as SumAmount,\
					  a.tbAccount_id as id, b.name as Name from tbsubAccount as a, tbAccount as b where a.mark = 0 and a.tbAccount_id = b.id group by a.id ) temp group by id;", strExPart.c_str());
		
		CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
		CppSQLite3Query tempQuery = m_dbUser.execQuery(bufSQL);
		double fSumIn = 0.0;
		double fSumOut = 0.0;
		double fSumAll = 0.0;

		
		TiXmlDocument DocPart1;
		TiXmlDocument DocPart2;
		TiXmlDocument DocPart3;
		TiXmlDocument DocPart4;
		TiXmlDocument DocPart5;
		TiXmlDocument DocPart6;//*pDocPart6 = new TiXmlDocument
		TiXmlElement *pRootPart1 = new TiXmlElement("chart");
		TiXmlElement *pRootPart2 = new TiXmlElement("chart");
		TiXmlElement *pRootPart3 = new TiXmlElement("chart");
		TiXmlElement *pRootPart4 = new TiXmlElement("chart");
		TiXmlElement *pRootPart5 = new TiXmlElement("chart");
		TiXmlElement *pRootPart6 = new TiXmlElement("chart");
		DocPart1.LinkEndChild(pRootPart1);
		DocPart2.LinkEndChild(pRootPart2);
		DocPart3.LinkEndChild(pRootPart3);
		DocPart4.LinkEndChild(pRootPart4);
		DocPart5.LinkEndChild(pRootPart5);
		DocPart6.LinkEndChild(pRootPart6);

		while(!tempQuery.eof())
		{
			double dVal = tempQuery.getFloatField("SumAmountAll");
			if (0 > dVal)
			{
				dVal *= -1; // ֧���Ǹ���
				fSumOut += dVal; // ֧��
			}
			else if(0 < dVal)
			{
				fSumIn += dVal; // ����
			}

			tempQuery.nextRow();
		}

		// �õ�����֧���ı���
		fSumAll = fSumOut + fSumIn;

		TiXmlElement *pSet = new TiXmlElement("set");
		pSet->SetAttribute("seriesName", "����");
		double dwPercent = 0;
		if (0 == fSumAll)
		dwPercent = 0;
		else
		{
			dwPercent = fSumIn / fSumAll * 100;
			//dwPercent = (int)(100 * dwPercent + 0.5)*0.01; // ������С��������λ
		}
		SetIndexDoubleAttribute(pSet, "value", dwPercent);
		pRootPart3->LinkEndChild(pSet);


		pSet = new TiXmlElement("set");
		pSet->SetAttribute("seriesName", "֧��");
		dwPercent = 0;
		if (0 == fSumAll)
			dwPercent = 0;
		else
		{
			dwPercent = fSumOut / fSumAll * 100;
			//dwPercent = (int)(100 * dwPercent + 0.5)*0.01; // ������С��������λ
		}
		SetIndexDoubleAttribute(pSet, "value", dwPercent);
		pRootPart3->LinkEndChild(pSet);
		
		pSet = new TiXmlElement("set");
		pSet->SetAttribute("seriesName", "����");
		//fSumIn = (int)(100 * fSumIn + 0.5)*0.01; // ������С��������λ
		SetIndexDoubleAttribute(pSet, "value", fSumIn);
		pRootPart6->LinkEndChild(pSet);
		pSet = new TiXmlElement("set");
		pSet->SetAttribute("seriesName", "֧��");
		//fSumOut = (int)(100 * fSumOut + 0.5)*0.01; // ������С��������λ
		SetIndexDoubleAttribute(pSet, "value", fSumOut);
		pRootPart6->LinkEndChild(pSet);
		
		tempQuery.finalize();
		while (!q.eof())
		{
			//��ͼ����
			double dVal = q.getFloatField("SumAmountAll");

			if (dVal > 0)
			{
				pSet = new TiXmlElement("set");
				// Ӧ����Ҫ��,�����ĺ��岿�ֵ�ֵ�ڵ�һ�����ֵ����ֺ��� Begin
				//pSet->SetAttribute("seriesName", ToSQLUnsafeString(q.getStringField("Name")).c_str());
				std::string strNewVal = ToSQLUnsafeString(q.getStringField("Name"));
				strNewVal += " : ";
				CString strTp;
				strTp.Format(L"%0.2fԪ", dVal);
				strNewVal += CW2A(strTp);
				pSet->SetAttribute("seriesName", strNewVal.c_str());
				// End

				//pSet->SetAttribute("seriesName", ToSQLUnsafeString(q.getStringField("Name")).c_str());
				SetIndexDoubleAttribute(pSet, "value", dVal);
				//pRootPart4->LinkEndChild(pSet);
				pRootPart1->LinkEndChild(pSet); // ������������ƺ�ֵ
			}
			else if(dVal < 0)
			{
				dVal *= -1;
				pSet = new TiXmlElement("set");
				//pSet->SetAttribute("seriesName", ToSQLUnsafeString(q.getStringField("Name")).c_str());
				// Ӧ����Ҫ��,�����ĺ��岿�ֵ�ֵ�ڵ�һ�����ֵ����ֺ��� Begin
				//pSet->SetAttribute("seriesName", ToSQLUnsafeString(q.getStringField("Name")).c_str());
				std::string strNewVal = ToSQLUnsafeString(q.getStringField("Name"));
				strNewVal += " : ";
				CString strTp;
				strTp.Format(L"%0.2fԪ", dVal);
				strNewVal += CW2A(strTp);
				pSet->SetAttribute("seriesName", strNewVal.c_str());
				// End

				SetIndexDoubleAttribute(pSet, "value", dVal);
				//pRootPart4->LinkEndChild(pSet);
				pRootPart2->LinkEndChild(pSet); // ����֧�������ƺ�ֵ
			}

			q.nextRow();
		}
		q.finalize();

		// ������������ƴ�ӳ�һ���ַ���
		pRootPart1->Accept(&printer1);
		strSQLRec += printer1.CStr();

		pRootPart2->Accept(&printer2);
		strSQLRec += printer2.CStr();

		pRootPart3->Accept(&printer3);
		strSQLRec += printer3.CStr();

		pRootPart4->Accept(&printer4);
		strSQLRec += printer4.CStr();

		pRootPart5->Accept(&printer5);
		strSQLRec += printer5.CStr();

		pRootPart6->Accept(&printer6);
		strSQLRec += printer6.CStr();
	}
	catch (CppSQLite3Exception&)
	{
		return "<chart></chart>";
	}

	ReplaceCharInString(strSQLRec, "<chart />", "<chart></chart>");
	return strSQLRec;
}

// ��д��XML��double����ΪС���������λ
void CBankData::SetIndexDoubleAttribute(TiXmlElement * pSet, const char * name, double val )
{
	ATLASSERT(NULL != pSet);
	if (NULL == pSet)
		return;

	char buf[256];
	#if defined(TIXML_SNPRINTF)		
		TIXML_SNPRINTF( buf, sizeof(buf), "%.2f", val);
	#else
		sprintf( buf, "%f", val );
	#endif
	pSet->SetAttribute( name, buf );
}

std::string CBankData::InternalGetPieXMLData(std::string strStartDate, std::string strEndDate)
{
	std::string strSQLRec = "";
	TiXmlPrinter printer1, printer2, printer3;
	TiXmlPrinter printer4, printer5, printer6;
	printer1.SetIndent( "    " );
	printer2.SetIndent( "    " );
	printer3.SetIndent( "    " );
	printer4.SetIndent( "    " );
	printer5.SetIndent( "    " );
	printer6.SetIndent( "    " );

	// �õ������ǲ��ֵ�SQL���
	std::string strExPart;
	if (!GetExchangePartSql("t.amount", strExPart))
		return "<chart></chart>";

	if(!CheckUserDB())
		return "<chart></chart>";

	try
	{
		// ȡ�������·�

		CppSQLite3Buffer bufSQL;
		
		if (strStartDate.empty() || strEndDate.empty())//t.ExchangeRate AS Rate,
		{
			bufSQL.format("SELECT c1.Type AS Type,c1.name AS Name, round(SUM((case d.tbcurrency_id %s  end ) \
							), 2) AS SumAmount FROM tbtransaction AS t, tbcategory2 AS c2, tbcategory1 AS c1, tbsubAccount  d \
							WHERE t.mark=0 and t.tbcategory2_id not in (10059,10060,10067) and t.tbcategory2_id=c2.id AND c2.tbcategory1_id=c1.id and t.tbsubaccount_id= d.id GROUP BY c1.id ORDER BY c1.id ;", strExPart.c_str());
		}
		else
		{
			bufSQL.format("SELECT c1.Type AS Type,c1.name AS Name, round(SUM((case d.tbcurrency_id %s end ) \
							), 2) AS SumAmount FROM tbtransaction AS t, tbcategory2 AS c2, tbcategory1 AS c1, tbsubAccount  d \
							WHERE t.mark=0 and t.tbcategory2_id not in (10059,10060,10067) and t.tbcategory2_id=c2.id AND c2.tbcategory1_id=c1.id AND TransDate>='%s' AND TransDate<='%s' and t.tbsubaccount_id= d.id GROUP BY c1.id ORDER BY c1.id ;", strExPart.c_str(), strStartDate.c_str(), strEndDate.c_str());
		}
		
		CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
		CppSQLite3Query tempQuery = m_dbUser.execQuery(bufSQL);
		double fSumIn = 0.0;
		double fSumOut = 0.0;
		double fSumAll = 0.0;

		
		TiXmlDocument DocPart1;
		TiXmlDocument DocPart2;
		TiXmlDocument DocPart3;
		TiXmlDocument DocPart4;
		TiXmlDocument DocPart5;
		TiXmlDocument DocPart6;//*pDocPart6 = new TiXmlDocument
		TiXmlElement *pRootPart1 = new TiXmlElement("chart");
		TiXmlElement *pRootPart2 = new TiXmlElement("chart");
		TiXmlElement *pRootPart3 = new TiXmlElement("chart");
		TiXmlElement *pRootPart4 = new TiXmlElement("chart");
		TiXmlElement *pRootPart5 = new TiXmlElement("chart");
		TiXmlElement *pRootPart6 = new TiXmlElement("chart");
		DocPart1.LinkEndChild(pRootPart1);
		DocPart2.LinkEndChild(pRootPart2);
		DocPart3.LinkEndChild(pRootPart3);
		DocPart4.LinkEndChild(pRootPart4);
		DocPart5.LinkEndChild(pRootPart5);
		DocPart6.LinkEndChild(pRootPart6);

		while(!tempQuery.eof())
		{
			if (0 == tempQuery.getIntField("Type"))
			{
				fSumOut += tempQuery.getFloatField("SumAmount"); // ֧��
			}
			else
			{
				fSumIn += tempQuery.getFloatField("SumAmount"); // ����
			}

			tempQuery.nextRow();
		}

		// �õ�����֧���ı���
		fSumAll = fSumOut + fSumIn;
		TiXmlElement *pSet = new TiXmlElement("set");
		pSet->SetAttribute("seriesName", "֧��");
		double dwPercent = 0;
		if (0 == fSumAll)
			dwPercent = 0;
		else
		{
			dwPercent = fSumOut / fSumAll * 100;
			//dwPercent = (int)(100 * dwPercent + 0.5)*0.01; // ������С��������λ
		}
		SetIndexDoubleAttribute(pSet, "value", dwPercent);
		pRootPart3->LinkEndChild(pSet);
		pSet = new TiXmlElement("set");
		pSet->SetAttribute("seriesName", "����");
		if (0 == fSumAll)
			dwPercent = 0;
		else
		{
			dwPercent = fSumIn / fSumAll * 100;
			//dwPercent = (int)(100 * dwPercent + 0.5)*0.01; // ������С��������λ
		}
		SetIndexDoubleAttribute(pSet, "value", dwPercent);
		pRootPart3->LinkEndChild(pSet);

		pSet = new TiXmlElement("set");
		pSet->SetAttribute("seriesName", "֧��");
		//fSumOut = (int)(100 * fSumOut + 0.5)*0.01; // ������С��������λ
		SetIndexDoubleAttribute(pSet, "value", fSumOut);
		pRootPart6->LinkEndChild(pSet);
		pSet = new TiXmlElement("set");
		pSet->SetAttribute("seriesName", "����");
		//fSumIn = (int)(100 * fSumIn + 0.5)*0.01; // ������С��������λ
		SetIndexDoubleAttribute(pSet, "value", fSumIn);
		pRootPart6->LinkEndChild(pSet);

		tempQuery.finalize();
		while (!q.eof())
		{
			//��ͼ����
			//double dRateVal = tempQuery.getFloatField("Rate");
			//dRateVal *= tempQuery.getFloatField("SumAmount");
			double dRateVal = q.getFloatField("SumAmount");

			pSet = new TiXmlElement("set");
			TiXmlElement *pPercentSet = new TiXmlElement("set");
			// Ӧ����Ҫ��,�����ĺ��岿�ֵ�ֵ�ڵ�һ�����ֵ����ֺ��� Begin
			//pSet->SetAttribute("seriesName", ToSQLUnsafeString(q.getStringField("Name")).c_str());


			pPercentSet->SetAttribute("seriesName", ToSQLUnsafeString(q.getStringField("Name")).c_str());
			//dRateVal = (int)(100 * dRateVal + 0.5)*0.01; // ������С��������λ

			
			// Ӧ����Ҫ��,�����ĺ��岿�ֵ�ֵ�ڵ�һ�����ֵ����ֺ��� Begin
			//pSet->SetAttribute("seriesName", ToSQLUnsafeString(q.getStringField("Name")).c_str());
			std::string strNewVal = ToSQLUnsafeString(q.getStringField("Name"));
			strNewVal += " : ";
			CString strTp;
			strTp.Format(L"%0.2fԪ", dRateVal);
			strNewVal += CW2A(strTp);
			pSet->SetAttribute("seriesName", strNewVal.c_str());
			// End

			SetIndexDoubleAttribute(pPercentSet, "value", dRateVal);
			if (0 == q.getIntField("Type"))
			{
				// ռ֧���İٷֱ�
				double dPercent = 0;
				if (fSumOut == 0)
					dPercent = 0;
				else
				{
					dPercent = dRateVal / fSumOut * 100;
					//dPercent = (int)(100 * dPercent + 0.5)*0.01; // ������С��������λ
				}
				SetIndexDoubleAttribute(pSet, "value", dPercent);
				pRootPart4->LinkEndChild(pPercentSet);
				pRootPart1->LinkEndChild(pSet);
			}
			else
			{
				// ռ����İٷֱ�
				double dPercent = 0;
				if (fSumIn == 0)
					dPercent = 0;
				else
				{
					dPercent = dRateVal / fSumIn * 100;
					//dPercent = (int)(100 * dPercent + 0.5)*0.01; // ������С��������λ
				}

				SetIndexDoubleAttribute(pSet, "value", dPercent);
				pRootPart2->LinkEndChild(pSet);
				pRootPart5->LinkEndChild(pPercentSet);

			}

			q.nextRow();
		}
		q.finalize();

		// ������������ƴ�ӳ�һ���ַ���
		pRootPart1->Accept(&printer1);
		strSQLRec += printer1.CStr();

		pRootPart2->Accept(&printer2);
		strSQLRec += printer2.CStr();

		pRootPart3->Accept(&printer3);
		strSQLRec += printer3.CStr();

		pRootPart4->Accept(&printer4);
		strSQLRec += printer4.CStr();

		pRootPart5->Accept(&printer5);
		strSQLRec += printer5.CStr();

		pRootPart6->Accept(&printer6);
		strSQLRec += printer6.CStr();
	}
	catch (CppSQLite3Exception&)
	{
		return "<chart></chart>";
	}

	ReplaceCharInString(strSQLRec, "<chart />", "<chart></chart>");
	
	return strSQLRec;
}

// ��strdst�滻�ַ����������ܺ�strsrcƥ����ַ���
void CBankData::ReplaceCharInString(std::string& strBig, const std::string & strsrc, const std::string &strdst)  
{  
    std::string::size_type pos = 0;
     while( (pos = strBig.find(strsrc, pos)) != string::npos)

     {
         strBig.replace(pos, strsrc.length(), strdst);
         pos += strdst.length();
     }
}  
  
// �����ݿ�ı�������һ�����ݿ���
bool CBankData::CpyDbTb2OtherDbTb(CppSQLite3DB& dbSour, CppSQLite3DB& dbDesc, LPSTR lpTbName, LPSTR lpDesTbName, LPSTR lpCondition)
{
	ATLASSERT (NULL != lpDesTbName && NULL != lpTbName);
	if (NULL == lpTbName || NULL == lpDesTbName)
		return false;


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
	CppSQLite3Table  tempTb = dbSour.getTable(strSQL.c_str());

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

		// Ϊÿһ����¼����һ��SQL���
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
		try{
			dbDesc.execDML(strExSQL.c_str());
		}
		catch(CppSQLite3Exception&)
		{
			continue;
		}

		
	}

	tempTb.finalize ();

	return true;
	
}

//// �����������Ľṹ��ͬ��ָ�����������п�����
//bool CBankData::CpyDbTb2OtherDbTb(CppSQLite3DB& dbSour, CppSQLite3DB& dbDesc, PTBCPYNODE pTabName, std::vector<PTBCPYNODE>* pColName,  LPSTR lpCondition)
//{
//	ATLASSERT (NULL != pTabName && NULL != pColName);
//	//ATLASSERT (NULL != lpDesDbPath && NULL != lpDesTbName);
//	if (NULL == pTabName || NULL == pColName)
//		return false;
//
////	if (!IsFileExist((LPSTR)pPath->strSour.c_str()) || !IsFileExist((LPSTR)pPath->strDes.c_str())) 
////		return false;
//
//	//CppSQLite3DB dbSour;
//	//dbSour.open(pPath->strSour.c_str());
//
//	//CppSQLite3DB dbDes;
//	//dbDes.open(pPath->strDes.c_str());
//
//	std::string strSQL ;//= "delete from ";
//	//strSQL += lpDesTbName;
//	// ɾ�������ݿ�ԭ�е�����
//	//dbDes.execDML(strSQL.c_str());
//
//	int nSize = 0;
//	std::string strColsSour, strColsDes;
//	std::vector<PTBCPYNODE>::const_iterator iter;
//	for (iter = pColName->begin(); iter != pColName->end(); iter ++)
//	{
//		PTBCPYNODE pTemp = *(iter);
//		if (pTemp == NULL)
//			continue;
//
//		strColsSour += pTemp->strSour;
//		strColsDes += pTemp->strDes;
//
//		nSize ++;
//
//		if (nSize != pColName->size())
//		{
//			strColsSour += ", ";
//			strColsDes += ", ";
//		}
//	}
//
//
//	// �õ�Ҫ���������
//	strSQL = "select ";
//	strSQL += strColsSour;
//	strSQL += " from ";
//	strSQL += pTabName->strSour;
//	if (NULL != lpCondition)
//	{
//		strSQL += lpCondition;
//	}
//
//	// �õ�Ҫ�����ļ�¼
//	CppSQLite3Table  tempTb = dbSour.getTable(strSQL.c_str());
//
//	// �õ������SQL���
//	std::string strSqlFront = "Insert into ";
//	strSqlFront += pTabName->strDes;
//	strSqlFront += '(';
//	strSqlFront += strColsDes;
//	strSqlFront += ") Values ";
//
//	std::string strSQLInsert;
//	for (int i = 0; i < tempTb.numRows(); i ++)
//	{
//		tempTb.setRow (i);
//
//		std::string strColVals;
//		strColVals += '(';
//		for (int j = 0; j < tempTb.numFields(); j ++)
//		{
//			strColVals += '\'';
//			strColVals += tempTb.fieldValue(j);
//			strColVals += '\'';
//			if (j != tempTb.numFields() - 1)
//			{
//				strColVals += ',';
//			}
//		}
//
//		strColVals += ')';
//
//		std::string strExSQL = strSqlFront + strColVals;
//
//		// ִ�в���
//		dbDesc.execDML(strExSQL.c_str());
//
//		
//	}
//
//	return true;
//}

bool CBankData::CreateDataDBFile(LPSTR lpPath)
{
	try
	{
		CppSQLite3DB db;
		db.open(lpPath);

		db.execDML("CREATE  TABLE IF NOT EXISTS `datExchangeRate` ( \
				   `sysCurrency_id` INTEGER NOT NULL, \
				   `sysCurrency_id1` INTEGER NOT NULL, \
				   `ExchangeRate` FLOAT NOT NULL, \
				   PRIMARY KEY(sysCurrency_id, sysCurrency_id1));");

		// ֻ�ǲ���
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 1, 100.00);");
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 2, 637.90);");
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 3, 916.69);");
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 4, 7.88);");
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 5, 1053.90);");
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 6, 83.24);");
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 7, 661.42);");
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 8, 685.01);");
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 9, 742.88);");
		db.execDML("INSERT INTO datExchangeRate (sysCurrency_id, sysCurrency_id1, ExchangeRate) VALUES (1, 10, 502.15);");


		
		db.execDML("CREATE  TABLE IF NOT EXISTS `datUserInfo` (\
				   `userid` CHAR(32) NOT NULL  PRIMARY KEY,\
				   `mail` CHAR(256),\
				   `edek` CHAR(256), \
				   `etk`CHAR(256), \
				   `autoload` int default(0), \
				   `autoinfo` int default(1), \
				   `autoremb` int default(0), \
				   `popLoad`  int default(0), \
				   `lastload` int default(0), \
				   `needsynchro` int default(0), \
				   `loadtime` DATE NULL, \
				   `mark` INT NULL DEFAULT 0);"); //����¼������ʶ�� //`stoken` CHAR(256), \//�������ʱ�������û�ʶ��


		db.execDML("CREATE TABLE IF NOT EXISTS 'datFav'( \
				   `favinfo` CHAR(4) NOT NULL , \
				   `status` INTEGER  NULL , \
				   `favorder` INTEGER  NULL , \
				   `deleted` TINYINT(1)  NULL  \
				   );");

		db.execDML("CREATE  TABLE IF NOT EXISTS `datUSBKeyInfo` ( \
				   `vid` INTEGER NOT NULL, \
				   `pid` INTEGER NOT NULL, \
				   `mid` INTEGER NOT NULL , \
				   `fav` CHAR(4), `xml` CHAR(512), `bkurl` CHAR(256), `ver` CHAR(16), `status` INT);");//, PRIMARYKEY KEY(vid, pid)

		db.execDML("CREATE  TABLE IF NOT EXISTS tbTotalEvent( \
				   `id` BIGINT NOT NULL DEFAULT 0 , \
				   `mark` INT(2) NULL DEFAULT 0, \
				   `event_date` INTEGER  NULL , \
				   `repeat` INTEGER  NULL , \
				   `alarm` INTEGER  NULL , \
				   `status` INTEGER  NULL , \
				   `datestring` CHAR(12) NULL , \
				   `type` INT NULL , \
				   `datUserInfo_userid` CHAR(32) NOT NULL \
				   );");

		db.execDML("CREATE  TABLE IF NOT EXISTS datDBInfo( \
				   `schema_version` INTEGER  NOT NULL DEFAULT 8 \
				   );");

		db.execDML("INSERT INTO datDBInfo (schema_version) VALUES(8);");

		// JSҪ���ʼ��Guest�����ݿ�
		db.execDML("insert into datUserInfo values ('Guest', '�ÿ�', '', '', 0, 1, 0, 0, 0, 0, 0, 0);");
		return true;
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalAddEvent error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return 0;
	}
}

CppSQLite3DB* CBankData::GetDataDbObject()
{
	try
	{
		m_dbDataDB.checkDB();
	}
	catch(...)
	{
		m_dbDataDB.open(m_strUtfDataDbPath.c_str());
	}
	return &m_dbDataDB;
}

CppSQLite3DB* CBankData::GetSysDbObject()
{
	try
	{
		m_dbSysDB.checkDB();
	}
	catch(...)
	{
		m_dbSysDB.open(m_strUtfSysDbPath.c_str());
	}
	return &m_dbSysDB;
}

bool CBankData::IsMonthImport(const char* pKeyInfo, const char* pMonth, INT64 nAccountID)
{
	if (NULL == pKeyInfo || nAccountID <= 0 || pMonth == NULL)
		return false;

	if(!CheckUserDB())
		return false;

	char bufSQL[256] = {0};

	string strMonth = pMonth;
	strMonth = FilterStringNumber(strMonth);
	if(strMonth.size() > 8)
		strMonth = strMonth.substr(0, 6);

	sprintf_s(bufSQL, 256, "SELECT tbmonth from tbAccountGetBillMonth where tbKeyInfo = '%s' and tbmonth = '%s'and tbaccount_id = %I64d;", pKeyInfo, strMonth.c_str(), nAccountID);



	CppSQLite3Query q = m_dbUser.execQuery(bufSQL);

	bool bBack = false;
	if(!q.eof())
	{
		bBack = true;
	}

	q.finalize ();

	return bBack;
}

std::string CBankData::strDBVer()
{
	ObjectLock lock(this);
	std::string strDBVer = "";
	try
	{
		//CppSQLite3DB dbDesc;
		//dbDesc.open(m_strUtfUserDbPath.c_str());
		int nVer = GetDBVersion(m_dbUser);
		char temp[ 256 ] = {0};
		sprintf_s(temp, 256, "%d", nVer);

        strDBVer = temp;
		//dbDesc.close();
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"GetstrDBVer error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
	}
	return strDBVer;
}

int  CBankData::TableStr(std::string &strtable)
{
	if(strtable == "1")
		strtable = "tbBank";
	else if(strtable == "2")
		strtable = "tbAccount";
	else if(strtable == "3")
		strtable = "tbsubAccount";
	else if(strtable == "4")
		strtable = "tbCategory1";
	else if(strtable == "5")
		strtable = "tbCategory2";
	else if(strtable == "6")
		strtable = "tbPayee";
	else if(strtable == "7")
		strtable = "tbEvent";
	else if(strtable == "8")
		strtable = "tbFav";
	else if(strtable == "9")
		strtable = "tbTransaction";
	else if(strtable == "10")
		strtable = "tbProductChoice";

	else if(strtable == "11")
		strtable = "tbAccountGetBillMonth";
	else if(strtable == "12")
		strtable = "tbTotalEvent";
	return 1;
}

string  CBankData::itostr(int num)
{
	char temp[ 256 ] = {0};
	sprintf_s(temp, 256, "%d", num);

	string st = temp;
	return st;
}

string CBankData::Filterchar(string scr)
{
	//scr = "3,3'33������#";
	string result;
	for(int i = 0;i < scr.size();i ++)
	{		
		size_t dpos = 0;
		dpos = scr.find(",");
		if(dpos != std::string::npos)
		    scr.replace(dpos ,1, "��");
		else
			break;
	}
	for(int i = 0;i < scr.size();i ++)
	{		
		size_t dpos = 0;
		dpos = scr.find(":");
		if(dpos != std::string::npos)
		    scr.replace(dpos ,1, "��");
		else
			break;
	}
	for(int i = 0;i < scr.size();i ++)
	{		
		size_t dpos = 0;
		dpos = scr.find("#");
		if(dpos != std::string::npos)
		    scr.replace(dpos ,1, "��");
		else
			break;
	}
	result = scr;
	return result;
}

void  CBankData::GfieldName ()
{
	// �������������SQL��Ĳ���ʱ���������
	std::string strColNames = "";
	ObjectLock lock(this);
	if(!CheckUserDB())
		return ;

	for(int i = 1 ; i< 10 ; i ++)
	{
		std::string strtb = itostr( i);
		std::string strtable = strtb;
		TableStr(strtable);
		std::string  strd1 = "SELECT * FROM ";
		strd1 += strtable;  	
		strd1 += " LIMIT 1;";
		try
		{
			CppSQLite3Query q = m_dbUser.execQuery(strd1.c_str());
			int kk = q.numFields();
			//strd1 = itostr( kk);
			tfield utf ;
			utf.ifieldNum = kk;
         	for(int j = 0 ; j< kk ; j ++)
			{
		    	strd1 = q.fieldName(j);
				utf.m_field.insert(std::make_pair( j , strd1));
			}
			GetTablefield()->insert(std::make_pair(strtb, utf));
		}
		catch (CppSQLite3Exception& ex)
		{
			//CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalAddEvent error:%d", ex.errorCode()));
			//if(ex.errorMessage() != NULL)
			//	CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
			//return;
		}
	}
		//std::string strtb = itostr( 12);
		//std::string strtable = strtb;
		//TableStr(strtable);
		//std::string  strd1 = "SELECT * FROM ";		strd1 += strtable;  			strd1 += " LIMIT 1;";
		//try
		//{
		//	CppSQLite3Query q = m_dbDataDB.execQuery(strd1.c_str());
		//	int kk = q.numFields();
		//	//strd1 = itostr( kk);
		//	tfield utf ;
		//	utf.ifieldNum = kk;
  //       	for(int j = 0 ; j< kk ; j ++)
		//	{
		//    	strd1 = q.fieldName(j);
		//		utf.m_field.insert(std::make_pair( j , strd1));
		//	}
		//	GetTablefield()->insert(std::make_pair(strtb, utf));
		//}
		//catch (CppSQLite3Exception& ex)
		//{
		//}
}



int CBankData::GetList(INT64 i64Big30,int &m_total) // 
{
	ObjectLock lock(this);
	if(!CheckUserDB())
		return 0;

	int isynnum = 0;
	for(int i = 1 ; i < 10 ; i ++)
	{
		std::string strtb = itostr( i);					std::string stridt = itostr( 100+i);
		std::string strtable = strtb;					TableStr(strtable);
		std::string  strd1 = "SELECT  * FROM ";		strd1 += strtable;  			strd1 += ";";
		try
		{			
			CppSQLite3Query q;
			q = m_dbUser.execQuery(strd1.c_str());
			while (!q.eof()) 
			{
				std::string  strid = q.getStringField("id");			std::string  stridtable = strid + stridt;
				TableField::iterator Tfmap = GetTablefield()->find(strtb);
				UserIDMap::iterator Uidmap = GetUserIDMap()->find(stridtable);  //q.getStringField("id")
				INT64 i64id = _atoi64(strid.c_str());//q.getStringField("id");
				if(i64id < 1234567890123 && i == 1) {}
				else
				//if(i64id > 1234567890123)   
				{
		           	INT64 icut = _atoi64(q.getStringField("UT"));//q.getInt64Field("UT");
					if (Uidmap != GetUserIDMap()->end())
					{
						INT64 isut = _atoi64(Uidmap->second.lUT.c_str());
						if(icut > isut )
						{
								isynnum ++;
								Uidmap->second.iupdown = 1;
								Uidmap->second.iTableNum = strtb;
								if( icut - i64Big30   > 1800000)
								{
									char   sID[64] ={0}; 					_i64toa_s(i64Big30,sID , 64, 10);
									Uidmap->second.lUT = sID;
									CppSQLite3Buffer bufSQL;
									bufSQL.format("UPDATE %s SET UT=%s WHERE id=%s;",strtable.c_str() ,Uidmap->second.lUT.c_str(), strid.c_str());
									m_dbUser.execDML(bufSQL);
								}
								else
									Uidmap->second.lUT = q.getStringField("UT");
								Uidmap->second.idbver = strDBVer();
								Uidmap->second.imark = q.getStringField("mark") ;
								Uidmap->second.strdata = "" ;								strd1 = "";
								for(int k = 3 ;k < Tfmap->second.ifieldNum  ; k ++)
								{
									strd1 += Filterchar(q.getStringField(k));            strd1 += ":";
								}
								Uidmap->second.strdata += strd1 ;
						}
						else if(icut < isut)
						{
							isynnum++;
								//Uidmap->second.iupdown = 4;  //client have
							Uidmap->second.iupdown = 2;
						}
						else{
							Uidmap->second.iupdown = 3;			m_total--;}
					}
					//		UDData uddata ={ strtb ,strUT ,strDBVer(),q.getStringField("mark") ,99 ,strtable};
					else if(strtb == "11" || strtb == "8" || strtb == "10"|| strtb == "12"){}
					else //������,server�� 4    ������,server�� 0
					{
						std::string strtbtemp = q.getStringField("mark");
						if(strtbtemp == "0")
						{
							isynnum ++;			m_total++;
							strd1 = "";
							for(int k = 3 ;k < Tfmap->second.ifieldNum  ; k ++)
							{
								std::string strtdt = q.getStringField(k);
								strd1 += Filterchar(strtdt);                strd1 += ":";
							}
							std::string strUT;
							if(icut - i64Big30   > 1800000)
							{
								char   sID[64] ={0}; 					_i64toa_s(i64Big30,sID ,64, 10);
								strUT = sID;
								CppSQLite3Buffer bufSQL;
								bufSQL.format("UPDATE %s SET UT='%s' WHERE id='%s';",strtable.c_str() , strUT.c_str(), strid.c_str());
								m_dbUser.execDML(bufSQL);
							}
							else
								strUT = q.getStringField("UT");
							UDData uddata ={ strtb ,strUT ,strDBVer(),strtbtemp ,4 ,strd1};
	    					m_UserIDMap.insert(std::make_pair( stridtable , uddata));//q.getStringField("id")
						}
					}
				}
				q.nextRow();
			}
			q.finalize();
		}
		catch (CppSQLite3Exception& ex)
		{
			//CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"GettList error:%d", ex.errorCode()));
			//if(ex.errorMessage() != NULL)
			//	CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		}
	}
	return isynnum;		

}


string CBankData::FilterStringNumber(string& scr)
{
	string result;
	char *p = (char*)scr.c_str();
	char temp[2] = { 0 };
	for(unsigned char i = 0;i < scr.size();i ++)
	{		
		if((((*p) >= '0')&&((*p) <= '9'))) 
		{
			memcpy(&temp[0], p, 1);
			result += temp; 
		}
		p ++;
	}
	return result;
}

bool CBankData::ReadNeedAutoLoadUser(string& strUserID, string& strMail, string& strStoken)
{
	strUserID.clear();
	strMail.clear();
	strStoken.clear();

	//if(!m_dbDataDB.checkDB())
	//	return false;

	try
	{
		m_dbDataDB.checkDB();
	}
	catch(...)
	{		
		m_dbDataDB.open(GetDbPath("DataDB"));
	}


	CppSQLite3Buffer bufSQL;
	bufSQL.format("SELECT userid, mail, etk from datUserInfo where autoload = 1");

	try
	{
		CppSQLite3Query q = m_dbDataDB.execQuery(bufSQL);

		if(!q.eof())
		{
			strUserID = ToSQLUnsafeString(q.getStringField("userid"));
			strMail = ToSQLUnsafeString(q.getStringField("mail"));
			strStoken = ToSQLUnsafeString(q.getStringField("etk"));
		}

		q.finalize ();
	}
	catch(...)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, L"��ȡ�Զ���¼����ʧ�ܣ�");
		return false;
	}

	return true;
}

bool CBankData::IsTableEmptyInGuestDB(const char* pTbName)
{
	ATLASSERT(NULL != pTbName);
	if (NULL == pTbName)
		return false;

	CppSQLite3Buffer bufSQL;
	bufSQL.format("SELECT count(*) from %s", pTbName);
	if(!CheckUserDB())
		return 0;


	try
	{
		string strGuestPath = m_strUtfDataPath + MONHUB_GUEST_USERID;
		strGuestPath += ".dat";
		CppSQLite3DB dbDesc;
		dbDesc.open(strGuestPath.c_str());

		CppSQLite3Query q = dbDesc.execQuery(bufSQL);
		int nCount = 0;
		if(!q.eof())
		{
			nCount = q.getIntField("count(*)");

		}

		q.finalize ();
		dbDesc.close();

		if (nCount > 0)
			return true;
	}
	catch(...)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, L"��ȡ�Զ���¼����ʧ�ܣ�");
		return false;
	}

	return false;
}

bool CBankData::DeleteUserDbByID(const char* pUserID, bool bCurUser)
{
	ATLASSERT(NULL != pUserID);
	if (NULL == pUserID)
		return false;

	wstring strPath = m_strDataPath;
	strPath += CA2W(pUserID);
	strPath += L".dat";

	// �ж��ǲ��ǵ�ǰ�û�
	if (bCurUser)
		m_dbUser.close();

	return DeleteFile(strPath.c_str());
}

// ���鵱ǰ�û��Ƿ���Ҫͬ��
bool CBankData::IsCurrentDbNeedSynchro()
{
	string strUserID = m_CurUserInfo.struserid;
	if (strUserID.length() < 0)
		return false;

	string strSQL = "select count(*) from datUserInfo where userid = '";
	strSQL += strUserID;
	strSQL += "' and needsynchro = 1";
	
	try
	{
		CppSQLite3DB* pDbObj = GetDataDbObject();
		CppSQLite3Query q = pDbObj->execQuery(strSQL.c_str());
		int nCount = 0;
		if(!q.eof())
		{
			nCount = q.getIntField("count(*)");
		}

		q.finalize ();

		if (nCount > 0)
			return true;
	}
	catch(...)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, L"��ȡͬ�����ʧ�ܣ�");
		return false;
	}

	return false;
}