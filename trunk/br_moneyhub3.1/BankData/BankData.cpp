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
#include "DownloadFile.h"
#include "../Utils/RecordProgram/RecordProgram.h"
#include "../include/ConvertBase.h"
#include "../Utils/SecurityCache/comm.h"

CRITICAL_SECTION CBankData::m_cs;

CBankData::CBankData()
{
	InitializeCriticalSection(&m_cs);
	InitSysDbTempFile();
}

CBankData::~CBankData()
{
	DeleteCriticalSection(&m_cs);
	/*TranslateSourceFile2DestinationFile ((LPSTR)m_strUserTpDbPath.c_str(), (LPSTR)m_strUserSourDbPath.c_str());
	DeleteFileA (m_strUserTpDbPath.c_str());
	DeleteFileA(m_strSysTpDbPath.c_str());*/
}

int CBankData::GetTodayAlarmsNumber()
{
	ObjectLock lock(this);

	std::vector<LPALARMRECORD> vec;
	int size = 0;
	if (InternalGetTodayAlarms(vec))
	{
		for (size_t i = 0; i < vec.size(); i++)
		{
			LPALARMRECORD pAlarmRec = vec[i];
			if ((pAlarmRec->type == 0 && pAlarmRec->status == 1) || (pAlarmRec->type == 1 && pAlarmRec->status == 1))
				size ++;
			delete pAlarmRec;
		}
		vec.clear();
	}
	m_dbUser.close();//�ýӿ����⣬Ҫ�ӹر����ݿ�Ľӿ�
	return size;
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
		CppSQLite3Query q = m_dbUser.execQuery("SELECT * FROM tbFav order by favorder desc;");
		
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
		bufSQL1.format("update tbFav set favorder = favorder + 1 where favorder between %d and %d;", nTo, nFrom);
		bufSQL2.format ("update tbFav set favorder = %d where favinfo = '%s';", nTo, pBankID); 
	}
	else
	{
		bufSQL1.format("update tbFav set favorder = favorder - 1 where favorder between '%d' and '%d';", nFrom, nTo);
		bufSQL2.format ("update tbFav set favorder = %d where favinfo = '%s';", nTo, pBankID); 
	}

	try
	{

		m_dbUser.execDML(bufSQL1);
		m_dbUser.execDML(bufSQL2);
		
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

		m_strGuestTemplete = strTemp + L"\\config\\Guest.dat";
		m_strSysDbPath = strTemp + L"\\config\\SysDB.dat";

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
		CpyDbTb2OtherDbTb(dbSour, dbDesc,"tbToday", "tbToday"); 
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
			}

			/*CppSQLite3Buffer bufSQL;
			bufSQL.format("select count(id) as num from tbtransaction where tbcategory2_id = 10067");
			CppSQLite3Query q2 = dbDesc.execQuery(bufSQL);

			int nres = 0;
			if(!q2.eof())
			{
				nres = q2.getIntField(0);				
			}
			q2.finalize();*/


			//nres > 0// ˵����3.1�汾���˻���������һ����ʼ��Ϊ0
			CppSQLite3Buffer bufSQL;
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
						dbDesc.execDML(bufSQL);
					}
					bufSQL.format("update tbsubaccount set openbalance=0 where id=%d; ", iaid);
					dbDesc.execDML(bufSQL);
				}
				else
				{
					bufSQL.format("update tbsubaccount set openbalance=0 where id=%d; ", iaid);
					dbDesc.execDML(bufSQL);
				}
				//�������˻����
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
					lastBalanceAmount = q3.getFloatField(2);
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
						totalSpend = q4.getFloatField(1);
					else if( q4res == 1)
						totalIncome = q4.getFloatField(1);

					q4.nextRow();
				}
				q4.finalize();

				lastBalanceAmount += totalIncome - totalSpend;

				bufSQL.format("UPDATE tbSubAccount SET balance=%.2f WHERE id=%d",lastBalanceAmount, iaid);
				dbDesc.execDML(bufSQL);				
				q.nextRow();
			}

			q.finalize();



			dbDesc.close();
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
			strUsID = "Guest";
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

	if (!IsFileExist ((LPWSTR)m_strUserDbPath.c_str()))
	{
		// ��ϵͳ���ݿ��п���
		CopyFileW(m_strGuestTemplete.c_str(), m_strUserDbPath.c_str(), false);//�����������治����utf8
	}

	


	//CppSQLite3DB db;
	//db.open(szDbPath);


	//if (!db.tableExists("tbFav")) {
	//	db.execDML("CREATE TABLE tbFav(favinfo CHAR(4), status INT, favorder INT, deleted BOOL);");
	//
	//	db.execDML("CREATE TABLE tbAuthen(pwd BLOB);");
	//	db.execDML("CREATE TABLE tbEvent(id INT, event_date INT, description CHAR(256), repeat INT, alarm INT, status INT, datestring CHAR(12));");
	//	db.execDML("CREATE TABLE tbCoupon(id INT, expire INT, status INT, name CHAR(256), sn CHAR(256), typeid INT);");
	//	db.execDML("CREATE TABLE tbToday(current INT);");
	//	db.execDML("CREATE TABLE tbDBInfo(schema_version INT);");
	//	db.execDML("CREATE TABLE tbUSBInfo(vid INT, pid INT, mid INTEGER, fav CHAR(4), xml CHAR(512), bkurl CHAR(256), ver CHAR(16), status INT);"); // ��usbinfo���д洢�ı�

	//	CppSQLite3Buffer bufSQL;
	//	bufSQL.format("INSERT INTO tbDBInfo VALUES(%d);", 5);
	//	db.execDML(bufSQL);		

	//	//��������������Ϊ��һ����¼
	//	SYSTEMTIME systime;
	//	GetSystemTime(&systime);

	//	struct tm tmVal;
	//	memset(&tmVal, 0, sizeof(struct tm));
	//	tmVal.tm_year = systime.wYear - 1900;
	//	tmVal.tm_mon = systime.wMonth - 1;
	//	tmVal.tm_mday = systime.wDay;

	//	__time32_t tToday = _mktime32(&tmVal);
	//	bufSQL.format("INSERT INTO tbToday VALUES(%d);", tToday);
	//	db.execDML(bufSQL);
	//}
	//if(!db.tableExists("tbDBInfo")) {
	//	//�������ݿ�汾
	//	db.execDML("CREATE TABLE tbDBInfo(schema_version INT);");
	//	CppSQLite3Buffer bufSQL;
	//	bufSQL.format("INSERT INTO tbDBInfo VALUES(%d);", 4);
	//	db.execDML(bufSQL);		
	//	db.execDML("ALTER TABLE tbEvent ADD COLUMN datestring CHAR(12);");
	//	db.execDML("DROP TABLE IF EXISTS tbCoupon;");
	//	db.execDML("CREATE TABLE tbCoupon(id INT, expire INT, status INT, name CHAR(256), sn CHAR(256), typeid INT);");
	////	db.execDML("CREATE TABLE tbUSBInfo(vid INT, pid INT, mid INTEGER, fav CHAR(4), xml CHAR(512), bkurl CHAR(256), ver CHAR(16), status INT);"); // ��usbinfo���д洢�ı�
	//} else {
	//	CppSQLite3Query q = db.execQuery("SELECT schema_version FROM tbDBInfo;");
	//	int s_version = q.getIntField("schema_version");
	//	q.finalize();
	//	if (s_version < 4) {
	//		//�������ݿ��ṹ
	//		db.execDML("UPDATE tbDBInfo SET schema_version=4;");
	//		db.execDML("DROP TABLE IF EXISTS tbCoupon;");
	//		db.execDML("CREATE TABLE tbCoupon(id INT, expire INT, status INT, name CHAR(256), sn CHAR(256), typeid INT);");
	//	}
	//	if (s_version < 5) {
	//		db.execDML("UPDATE tbDBInfo SET schema_version=5;");
	//		//ȡ�����ղأ�ɾ���ɱ����±������±�
	//		std::string strFav; //= "a001&b001";
	//		try
	//		{
	//			CppSQLite3Binary blob;
	//			CppSQLite3Query q1 = db.execQuery("SELECT favinfo FROM tbFav;");
	//			if (!q1.eof()) {
	//				blob.setEncoded((unsigned char*)q1.fieldValue("favinfo"));
	//				strFav = (const char *)blob.getBinary();
	//			} else {
	//				strFav = "";
	//			}
	//			q1.finalize();
	//		}
	//		catch (CppSQLite3Exception&)
	//		{
	//			strFav = "";
	//		}
	//		std::vector<std::string> vFav;
	//		if (strFav != "") {
	//			std::string splitter = "&";
	//			GetInstance()->split(strFav, splitter, &vFav);
	//		}
	//		db.execDML("DROP TABLE IF EXISTS tbFav;");
	//		db.execDML("CREATE TABLE tbFav(favinfo CHAR(4), status INT, favorder INT, deleted BOOL);");
	//		for (size_t i = 0; i < vFav.size(); i++)
	//		{
	//			CppSQLite3Buffer bufSQL;
	//			bufSQL.format("INSERT INTO tbFav VALUES('%s',%d, %d, %d);", vFav[i].c_str(), 200, (i + 1), 0);//���ϰ汾������Ϊ״̬200����ʾ��װ��
	//			db.execDML(bufSQL);		
	//		}
	//		vFav.clear();

	//		//db.execDML("CREATE TABLE tbUSBInfo(vid INT, pid INT, mid INTEGER, fav CHAR(4), xml CHAR(512), bkurl CHAR(256), ver CHAR(16), status INT);"); // ��usbinfo���д洢�ı�

	//	}
	//	if (s_version < 6)  // ���˵��İ汾
	//	{
	//		CreateAccountTables(db);
	//		if (!m_strSysDbPath.empty())
	//		{
	//			CpyDbTb2OtherDbTb((LPSTR)m_strSysDbPath.c_str(), "sysAccountType", (LPSTR)m_strUtfUserDbPath.c_str(), "tbAccountType");
	//			CpyDbTb2OtherDbTb((LPSTR)m_strSysDbPath.c_str(), "sysCurrency", (LPSTR)m_strUtfUserDbPath.c_str(), "tbCurrency");
	//			CpyDbTb2OtherDbTb((LPSTR)m_strSysDbPath.c_str(), "sysCategory1", (LPSTR)m_strUtfUserDbPath.c_str(), "tbCategory1");
	//			CpyDbTb2OtherDbTb((LPSTR)m_strSysDbPath.c_str(), "sysCategory2", (LPSTR)m_strUtfUserDbPath.c_str(), "tbCategory2");

	//			////
	//			TBCPYNODE path;
	//			path.strSour = m_strSysDbPath;
	//			path.strDes = m_strUtfUserDbPath;
	//			TBCPYNODE tabNode;
	//			tabNode.strSour = "sysBank";
	//			tabNode.strDes = "tbBank";
	//			std::vector<PTBCPYNODE> colVect;
	//			TBCPYNODE colNode1, colNode2, colNode3;
	//			colNode1.strSour = "id";
	//			colNode1.strDes = "BankID";
	//			colNode2.strSour = "Name";
	//			colNode2.strDes = "Name";
	//			colNode3.strSour = "classId";
	//			colNode3.strDes = "classId";
	//			colVect.push_back(&colNode1);
	//			colVect.push_back(&colNode2);
	//			colVect.push_back(&colNode3);
	//			CpyDbTb2OtherDbTb(&path, &tabNode, &colVect, " where classId in(0, 1, 2)");//, " where sysAppType_id = '2'"

	//			
	//			// �ڴ�ϵͳ���п�������ʱ�Ὣ��Ӧ����ԭ�е���������������²��������Ӧ���ڿ���֮��
	//			db.execDML("INSERT INTO tbAccountType (id, Name) VALUES (100, \"���ڴ��\");");
	//			db.execDML("INSERT INTO tbAccountType (id, Name) VALUES (101, \"���ڴ��\");");
	//			db.execDML("INSERT INTO tbAccountType (id, Name) VALUES (102, \"��Ʋ�Ʒ\");");

	//			db.execDML("INSERT INTO tbAccount (id, Name, tbAccountType_id) VALUES (1, \"�ҵ�Ǯ��\", 1);");
	//			db.execDML("INSERT INTO tbAccount (id, Name, tbAccountType_id) VALUES (2, \"���п�\", 3);");
	//			db.execDML("INSERT INTO tbAccount (id, Name, tbAccountType_id) VALUES (3, \"���ÿ�\", 2);");

	//			db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance, tbAccountType_id) VALUES (1, \"���ڴ��\", 2, 1, 0, 0, 100);");
	//			//db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance) VALUES (2, \"���ڴ��\", 2, 1, 0, 0);");
	//			db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance) VALUES (2, \"�����\", 3, 1, 0, 0);");
	//			db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance) VALUES (3, \"��Ԫ\", 3, 2, 0, 0);");
	//			db.execDML("INSERT INTO tbSubAccount (id, Name, tbAccount_id, tbCurrency_id, OpenBalance, Balance) VALUES (4, \"�����\", 1, 1, 0, 0);");

	//		}
	//		db.execDML("UPDATE tbDBInfo SET schema_version=6;");

	//	}
	//}

	return m_strUtfUserDbPath.c_str();
}

const char* CBankData::GetCouponPath()
{
	static char szCouponPath[1024] = { 0 };

	if (szCouponPath[0] == 0)
	{
		SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szCouponPath);
		strcat_s(szCouponPath, "\\MoneyHub");
		::CreateDirectoryA(szCouponPath, NULL);
		strcat_s(szCouponPath, "\\Coupons");
		::CreateDirectoryA(szCouponPath, NULL);
	}

	return szCouponPath;
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
		bufSQL.format("SELECT favinfo FROM tbFav WHERE favinfo='%s';",strFav.c_str());
		CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
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
			q = m_dbUser.execQuery("SELECT * FROM tbFav;");
		else
			q = m_dbUser.execQuery("SELECT * FROM tbFav WHERE status<200;");
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
		bufSQL.format("SELECT * FROM tbFav Where favinfo='%s';",strFav.c_str());

		CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
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

//
// event ��ʽ
// {"id":0,"event_date":"2010-09-01","description":"�¼����飨������30�������֣�","repeat":2,"alarm":3,"status":1}
//
int CBankData::AddEvent(std::string event)
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
		rec.id = root.get("id", 0).asInt();
		rec.event_date = ToIntTime(root.get("event_date", "").asString());
		strncpy_s(rec.description, sizeof(rec.description), ToSQLSafeString(root.get("description", "").asString()).c_str(), 255);
		rec.repeat = root.get("repeat", 0).asInt();
		rec.alarm = root.get("alarm", 0).asInt();
		rec.status = root.get("status", 0).asInt();

		return InternalAddEvent(&rec);
	}
	catch (std::runtime_error& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_RUNTIME_ERROR, AToW(ex.what()));
		return 0;		
	}
}

int CBankData::DeleteEvent(int id)
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
			
			sprintf_s(szVal, sizeof(szVal), "{\"id\":%d,\"event_date\":\"%s\",\"start_date\":\"%s\",\"description\":\"%s\",\"repeat\":%d,\"alarm\":%d,\"status\":%d}",
				pEventRec->id,
				ToStrTime(pEventRec->event_date).c_str(),
				ToStrTime(pEventRec->start_date).c_str(),
				pEventRec->description,
				pEventRec->repeat,
				pEventRec->alarm,
				pEventRec->status);

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

			sprintf_s(szVal, sizeof(szVal), "{\"id\":%d,\"event_date\":\"%s\",\"start_date\":\"%s\",\"description\":\"%s\",\"repeat\":%d,\"alarm\":%d,\"status\":%d}",
				pEventRec->id,
				ToStrTime(pEventRec->event_date).c_str(),
				ToStrTime(pEventRec->start_date).c_str(),
				pEventRec->description,
				pEventRec->repeat,
				pEventRec->alarm,
				pEventRec->status);

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

		int id = root.get("id", 0).asInt();
		int status = root.get("status", 0).asInt();
		std::string alarmtype = root.get("type", 0).asString();

		return InternalSetAlarm(id, status, alarmtype) ? 1 : 0;
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
				sprintf_s(szVal, sizeof(szVal), "{\"id\":%d,\"event_date\":\"%s\",\"description\":\"%s\",\"status\":%d,\"type\":\"%s\"}",
					pAlarmRec->id,
					ToStrTime(pAlarmRec->event_date).c_str(),
					pAlarmRec->description,
					pAlarmRec->status,
					"event");

			}
			else
			{
				sprintf_s(szVal, sizeof(szVal), "{\"id\":%d,\"event_date\":\"%s\",\"description\":\"%s%s\",\"status\":%d,\"type\":\"%s\"}",
					pAlarmRec->id,
					ToStrTime(pAlarmRec->event_date).c_str(),
					pAlarmRec->description,
					"�Ż�ȯ����",
					pAlarmRec->status,
					"coupon");
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

int CBankData::SaveCoupon(int style, std::string id, std::string expire, std::string couponname)
{
	ObjectLock lock(this);

	return InternalSaveCoupon(style, id, ToIntTime(expire), couponname) ? 1 : 0;
}

int CBankData::DeleteCoupon(int id)
{
	ObjectLock lock(this);

	return InternalDeleteCoupon(id) ? 1 : 0;
}

std::string CBankData::GetCoupons()
{
	ObjectLock lock(this);

	std::string result = "[";

	std::vector<LPCOUPONRECORD> vec;

	if (InternalGetCoupons(vec))
	{
		char szVal[1024];
		for (size_t i = 0; i < vec.size(); i++)
		{
			LPCOUPONRECORD pCouponRec = vec[i];

			CStringA strLargeImage, strMiddleImage, strSmallImage;
			strLargeImage.Format("file:///%s\\%dL.jpg", GetCouponPath(), pCouponRec->id);
			strMiddleImage.Format("file:///%s\\%dM.jpg", GetCouponPath(), pCouponRec->id);
			strSmallImage.Format("file:///%s\\%dS.jpg", GetCouponPath(), pCouponRec->id);

			strLargeImage.Replace('\\', '/');
			strMiddleImage.Replace('\\', '/');
			strSmallImage.Replace('\\', '/');

			sprintf_s(szVal, sizeof(szVal), "{\"id\":%d,\"expire\":\"%s\",\"large-image\":\"%s\",\"middle-image\":\"%s\",\"small-image\":\"%s\"}",
				pCouponRec->id, ToStrTime(pCouponRec->expire).c_str(), strLargeImage, strMiddleImage, strSmallImage);
			result += szVal;

			if (i != (vec.size() - 1))
				result += ",";

			delete pCouponRec;
		}
		
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
		CppSQLite3Query q = m_dbUser.execQuery("SELECT * FROM tbFav order by favorder desc;");
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
		bufSQL.format("DELETE FROM tbFav WHERE favinfo='%s';", strFav.c_str());
		m_dbUser.execDML(bufSQL);		
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
		bufSQL.format("SELECT favinfo FROM tbFav WHERE favinfo='%s';",strFav.c_str());
		CppSQLite3Query q = m_dbUser.execQuery(bufSQL);

		// �ҳ�favorder�����ֵ
		bufSQL.format ("select max(favorder) from tbFav;");
		CppSQLite3Query p = m_dbUser.execQuery(bufSQL);
		int nMaxValue = p.getIntField ("max(favorder)");
		if (nMaxValue <= 0)
			nMaxValue = 1;
		else
			nMaxValue ++;

		if(!q.eof())
			isExist = true;

		q.finalize();
		p.finalize ();

		if(!isExist)// ����ò��������ڣ��ǾͲ���
		{
			CppSQLite3Buffer bufSQL;
			bufSQL.format("INSERT INTO tbFav VALUES('%s', %d, %d, 0);", strFav.c_str(), status, nMaxValue);
			m_dbUser.execDML(bufSQL);
		}
		else
		{	
			CppSQLite3Buffer bufSQL;
			bufSQL.format("UPDATE tbFav SET status=%d WHERE favinfo='%s';", status, strFav.c_str());
			m_dbUser.execDML(bufSQL);
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


int CBankData::InternalAddEvent(LPEVENTRECORD lpEventRec)
{
	int nId = 1;

	//ת��ʱ���ʽΪ������
	struct tm newtime;
	_localtime32_s(&newtime, &(lpEventRec->event_date));
	char datestring[128];
	strftime(datestring, 128, "%Y-%m-%d %w", &newtime);

	if(!CheckUserDB())
		return false;

	try
	{
		CppSQLite3Buffer bufSQL;
		if (lpEventRec->id == 0)
		{
			bufSQL.format("SELECT MAX(id) FROM tbEvent");
			CppSQLite3Query q = m_dbUser.execQuery(bufSQL);

			if (!q.fieldIsNull(0))
				nId = atoi(q.fieldValue(0)) + 1;

			bufSQL.format("INSERT INTO tbEvent VALUES(%d, %d, '%s', %d, %d, %d, '%s');", 
				nId,
				(int)lpEventRec->event_date,
				lpEventRec->description,
				lpEventRec->repeat,
				lpEventRec->alarm,
				lpEventRec->status,
				datestring);

			m_dbUser.execDML(bufSQL);		
		}
		else
		{
			nId = lpEventRec->id;

			bufSQL.format("UPDATE tbEvent SET event_date=%d, description='%s', repeat=%d, alarm=%d, status=%d, datestring='%s' WHERE id=%d;",
				(int)lpEventRec->event_date,
				lpEventRec->description,
				lpEventRec->repeat,
				lpEventRec->alarm,
				lpEventRec->status,
				datestring,
				lpEventRec->id);

			m_dbUser.execDML(bufSQL);
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

bool CBankData::InternalDeleteEvent(int id)
{
	if(!CheckUserDB())
		return false;
	try
	{
		if (!m_dbUser.tableExists("tbEvent"))
			return true;

		CppSQLite3Buffer bufSQL;
		bufSQL.format("DELETE FROM tbEvent WHERE id=%d;", id);

		m_dbUser.execDML(bufSQL);
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
		bufSQL.format("SELECT * FROM tbEvent WHERE event_date<%d;", tTail);

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

					pEventRec->id = q.getIntField("id");
					pEventRec->event_date = vecShowDates[i];
					pEventRec->start_date = event_date;
					strcpy_s(pEventRec->description, sizeof(pEventRec->description), ToSQLUnsafeString(q.getStringField("description")).c_str());
					pEventRec->repeat = q.getIntField("repeat");
					pEventRec->alarm = q.getIntField("alarm");
					pEventRec->status = q.getIntField("status");

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
		bufSQL.format("SELECT * FROM tbEvent WHERE event_date<%d AND ((repeat=0 AND event_date>=%d) OR (repeat=1 AND SUBSTR(datestring,12)='%d') OR (repeat=2 AND SUBSTR(datestring,9,2)='%02d') OR (repeat=3 AND SUBSTR(datestring,6,2)%%3=%d AND SUBSTR(datestring,9,2)='%02d') OR (repeat=4 AND SUBSTR(datestring,6,2)='%02d' AND SUBSTR(datestring,9,2)='%02d'));", tTail, tHead, wday, day, month%3, day, month, day);

		CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
		while (!q.eof())
		{
			LPEVENTRECORD pEventRec = new EVENTRECORD;

			pEventRec->id = q.getIntField("id");
			pEventRec->event_date = q.getIntField("event_date");
			pEventRec->start_date = q.getIntField("event_date");
			strcpy_s(pEventRec->description, sizeof(pEventRec->description), ToSQLUnsafeString(q.getStringField("description")).c_str());
			pEventRec->repeat = q.getIntField("repeat");
			pEventRec->alarm = q.getIntField("alarm");
			pEventRec->status = q.getIntField("status");

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

bool CBankData::InternalSetAlarm(int id, int status, std::string alarmtype)
{
	if(!CheckUserDB())
		return false;

	try
	{
		if (!m_dbUser.tableExists("tbEvent"))
			return true;

		CppSQLite3Buffer bufSQL;
		if (alarmtype == "event") {
			bufSQL.format("UPDATE tbEvent SET status=%d WHERE id=%d;", status, id);
		} else {
			bufSQL.format("UPDATE tbCoupon SET status=%d WHERE id=%d;", status, id);
		}

		m_dbUser.execDML(bufSQL);
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

		TryResetAlarmStatus(tToday);

		//////////////////////////////////////////////////////////////////////////
		// From tbEvent

		if (m_dbUser.tableExists("tbEvent"))
		{
			CppSQLite3Buffer bufSQL;
			bufSQL.format("SELECT * FROM tbEvent ORDER BY event_date;");
			
			CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
			while (!q.eof())
			{
				int id = q.getIntField("id");
				__time32_t event_date = q.getIntField("event_date");
				int repeat1 = q.getIntField("repeat");
				int alarm = q.getIntField("alarm");

				__time32_t alarm_date;

				if (IsAlarmEvent(tToday, event_date, repeat1, alarm, &alarm_date))
				{
					LPALARMRECORD pAlarmRec = new ALARMRECORD;
					
					pAlarmRec->type = 0;
					pAlarmRec->id = q.getIntField("id");
					
					// ���ص��Ǿ���ʱ�䣬���Ǵ���ʱ��
					//pAlarmRec->event_date = q.getIntField("event_date");
					pAlarmRec->event_date = alarm_date;

					strcpy_s(pAlarmRec->description, sizeof(pAlarmRec->description), ToSQLUnsafeString(q.getStringField("description")).c_str());
					pAlarmRec->status = q.getIntField("status");
					
					vec.push_back(pAlarmRec);
				}

				q.nextRow();
			}
			q.finalize();
		}

		//////////////////////////////////////////////////////////////////////////
		// From tbCoupon

		if (m_dbUser.tableExists("tbCoupon"))
		{
			//__time32_t tAlarm = tToday + 11 * 24 * 60 * 60;
			__time32_t tAlarm = tToday + 4 * 24 * 60 * 60;

			CppSQLite3Buffer bufSQL;
			bufSQL.format("SELECT * FROM tbCoupon WHERE expire>=%d AND expire<%d;", tToday, tAlarm);

			CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
			while (!q.eof())
			{
				LPALARMRECORD pAlarmRec = new ALARMRECORD;

				pAlarmRec->type = 1;
				pAlarmRec->id = q.getIntField("id");
				pAlarmRec->event_date = q.getIntField("expire");
				try {
					strcpy_s(pAlarmRec->description, sizeof(pAlarmRec->description), ToSQLUnsafeString(q.getStringField("name")).c_str());
				} catch(CppSQLite3Exception&) {
					strcpy_s(pAlarmRec->description, sizeof(pAlarmRec->description), ToSQLUnsafeString("").c_str());
				}
				try {
					pAlarmRec->status = q.getIntField("status");
				} catch(CppSQLite3Exception&) {
					pAlarmRec->status = 1;
				}

				vec.push_back(pAlarmRec);

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

bool CBankData::InternalSaveCoupon(int style, std::string id, __time32_t expire, std::string couponname)
{
	int nId = 1;

	try
	{
		CppSQLite3DB db;
		db.open(GetDbPath());
		CppSQLite3Buffer bufSQL;

		bufSQL.format("SELECT MAX(id) FROM tbCoupon");
		CppSQLite3Query q = db.execQuery(bufSQL);

		if (!q.fieldIsNull(0))
			nId = atoi(q.fieldValue(0)) + 1;

		if (id == "-1") {
			//���ڿ��ظ��Ż�ȯ������ֻ����һ��
			bufSQL.format("DELETE FROM tbCoupon WHERE typeid=%d;", style);
			db.execDML(bufSQL);	
		}

		bufSQL.format("INSERT INTO tbCoupon(id, expire, status, name, sn, typeid) VALUES(%d, %d, %d, '%s', '%s', %d);", nId, (int)expire, 1, couponname.c_str(), id.c_str(), style);
		db.execDML(bufSQL);	
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalSaveCoupon error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

	if(!DownloadCouponImages(style, id, nId))// �������ʧ�ܣ���ôɾ���Ѿ���ȡ�����Ż�ȯ
	{
		InternalDeleteCoupon(nId);
		return false;
	}
	return true;
}

bool CBankData::InternalDeleteCoupon(int id)
{
	try
	{
		CppSQLite3DB db;
		db.open(GetDbPath());

		if (!db.tableExists("tbCoupon"))
			return true;

		CppSQLite3Buffer bufSQL;
		bufSQL.format("DELETE FROM tbCoupon WHERE id=%d;", id);

		db.execDML(bufSQL);	
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalDeleteCoupon error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

	DeleteCouponImages(id);

	return true;
}

bool CBankData::InternalGetCoupons(std::vector<LPCOUPONRECORD>& vecCouponRec)
{
	assert(vecCouponRec.empty());

	try
	{
		CppSQLite3DB db;
		db.open(GetDbPath());

		if (!db.tableExists("tbCoupon"))
			return true;

		CppSQLite3Buffer bufSQL;
		bufSQL.format("SELECT * FROM tbCoupon ORDER BY 1;");

		CppSQLite3Query q = db.execQuery(bufSQL);

		while (!q.eof())
		{
			LPCOUPONRECORD pCouponRec = new COUPONRECORD;
			pCouponRec->id = q.getIntField(0);
			pCouponRec->expire = q.getIntField(1);

			vecCouponRec.push_back(pCouponRec);

			q.nextRow();
		}

		q.finalize();
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"InternalGetCoupons error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return false;
	}

	return true;
}

void CBankData::TryResetAlarmStatus(__time32_t tToday)
{
	if(!CheckUserDB())
		return;

	CppSQLite3Query q = m_dbUser.execQuery("SELECT * FROM tbToday;");
	if (!q.eof())
	{
		int tOldTime = q.getIntField("current");
		if (tOldTime != tToday)
		{
			m_dbUser.execDML("UPDATE tbEvent SET alarm=1 WHERE alarm=0;");
			
			CppSQLite3Buffer bufSQL;
			bufSQL.format("UPDATE tbToday SET current=%d;", tToday);
			m_dbUser.execDML(bufSQL);
		}
	}
	else
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("INSERT INTO tbToday VALUES(%d);", tToday);
		m_dbUser.execDML(bufSQL);		
	}

	q.finalize();
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
			if (NotLeapYear(_tm->tm_year))
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

bool CBankData::DownloadCouponImages(int style, std::string sn, int id)
{
	CDownloadFile file(style, sn, id);
	return file.Start();
}

bool CBankData::DeleteCouponImages(int id)
{
	CDownloadFile file(0, "", id);
	return file.Delete();
}

// �������ݿ�����
std::string CBankData::QuerySQL(std::string strSQL, std::string strDBName)
{
	ObjectLock lock(this);
	return InternalQuerySQL(strSQL, strDBName);
}

int CBankData::ExecuteSQL(std::string strSQL)
{
	ObjectLock lock(this);
	return InternalExecuteSQL(strSQL);
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



// ��ԭ�еİ汾������������Ҫ��һЩ����
void CBankData::CreateAccountTables(CppSQLite3DB& db)
{
	try
	{
		db.execDML("CREATE  TABLE IF NOT EXISTS `tbAccount` (\
				   `id` INTeger  NOT NULL PRIMARY KEY AUTOINCREMENT ,\
				   `Name` VARCHAR(256) NOT NULL ,\
				   `tbBank_id` INT(11)  NULL ,\
				   `tbAccountType_id` INT(11)  NOT NULL ,\
				   `AccountNumber` VARCHAR(256) NULL ,\
				   `Comment` VARCHAR(256) NULL,\
				   `keyInfo` varchar(255));");
		/*CONSTRAINT `fk_tbAccount_tbBank`\
		FOREIGN KEY (`tbBank_id` )\
		REFERENCES `tbBank` (`id` )\
		ON DELETE SET NULL\
		ON UPDATE CASCADE,\
		CONSTRAINT `fk_tbAccount_tbAccountType1`\
		FOREIGN KEY (`tbAccountType_id` )\
		REFERENCES `tbAccountType` (`id` )\
		ON DELETE CASCADE\
		ON UPDATE CASCADE*/
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
		/*CONSTRAINT `fk_tbCategory2_tbCategory11`\
		FOREIGN KEY (`tbCategory1_id` )\
		REFERENCES `tbCategory1` (`id` )\
		ON DELETE CASCADE\
		ON UPDATE CASCADE*/
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
		/*CONSTRAINT `fk_tbSubAccount_tbAccount1`\
		FOREIGN KEY (`tbAccount_id` )\
		REFERENCES `tbAccount` (`id` )\
		ON DELETE CASCADE\
		ON UPDATE CASCADE,\
		CONSTRAINT `fk_tbSubAccount_tbCurrency1`\
		FOREIGN KEY (`tbCurrency_id` )\
		REFERENCES `tbCurrency` (`id` )\
		ON DELETE CASCADE\
		ON UPDATE CASCADE,\
		CONSTRAINT `fk_tbSubAccount_tbAccountType1`\
		FOREIGN KEY (`tbAccountType_id` )\
		REFERENCES `tbAccountType` (`id` )\
		ON DELETE CASCADE\
		ON UPDATE CASCADE*/
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
				   `isEdit` tinyint (1)DEFAULT 0 NOT NULL);");
		/*CONSTRAINT `fk_tbTransaction_tbPayee1`\
		FOREIGN KEY (`tbPayee_id` )\
		REFERENCES `tbPayee` (`id` )\
		ON DELETE CASCADE\
		ON UPDATE CASCADE,\
		CONSTRAINT `fk_tbTransaction_tbCategory21`\
		FOREIGN KEY (`tbCategory2_id` )\
		REFERENCES `tbCategory2` (`id` )\
		ON DELETE CASCADE\
		ON UPDATE CASCADE,\
		CONSTRAINT `fk_tbTransaction_tbSubAccount1`\
		FOREIGN KEY (`tbSubAccount_id` )\
		REFERENCES `tbSubAccount` (`id` )\
		ON DELETE CASCADE\
		ON UPDATE CASCADE,\
		CONSTRAINT `fk_tbTransaction_tbSubAccount2`\
		FOREIGN KEY (`tbSubAccount_id1` )\
		REFERENCES `tbSubAccount` (`id` )\
		ON DELETE CASCADE\
		ON UPDATE CASCADE*/

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
	}
	catch (CppSQLite3Exception& ex)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"CreateAccountTables error:%d", ex.errorCode()));
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
	}
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
						//������
						sprintf_s(szVal, sizeof(szVal), "\"%s\":%d,", q.fieldName(i), q.getIntField(q.fieldName(i)));
						strSQLRec += szVal;
						break;

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
		if(ex.errorMessage() != NULL)
			CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR,AToW(ex.errorMessage()));
		return "";
	}
	return strSQLRec;
}

int CBankData::InternalExecuteSQL(std::string strSQL)
{
	std::string strSQLRec = "";

	// ת���ɴ�д
	std::string strTp = strSQL;
	transform(strTp.begin(), strTp.end(), strTp.begin(), towupper);
	
	if(!CheckUserDB())
		return false;

	try
	{
		// ִ��SQL���
		m_dbUser.execDML(strSQL.c_str());
		if (strTp.find("INSERT", 0) != string::npos)
		{
			//�����INSERT��䣬��ȡ�����²��������ֵ�����ء�
			CppSQLite3Buffer bufSQL;
			bufSQL.format("SELECT last_insert_rowid();");
			CppSQLite3Query q = m_dbUser.execQuery(bufSQL);
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
							WHERE t.tbcategory2_id=c2.id AND c2.tbcategory1_id=c1.id  \
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
							WHERE t.tbcategory2_id=c2.id AND c2.tbcategory1_id=c1.id  \
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
					  a.tbAccount_id as id, b.name as Name from tbsubAccount as a, tbAccount as b where a.tbAccount_id = b.id group by a.id ) temp group by id;", strExPart.c_str());
		
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
							WHERE t.tbcategory2_id not in (10059,10060,10067) and t.tbcategory2_id=c2.id AND c2.tbcategory1_id=c1.id and t.tbsubaccount_id= d.id GROUP BY c1.id ORDER BY c1.id ;", strExPart.c_str());
		}
		else
		{
			bufSQL.format("SELECT c1.Type AS Type,c1.name AS Name, round(SUM((case d.tbcurrency_id %s end ) \
							), 2) AS SumAmount FROM tbtransaction AS t, tbcategory2 AS c2, tbcategory1 AS c1, tbsubAccount  d \
							WHERE t.tbcategory2_id not in (10059,10060,10067) and t.tbcategory2_id=c2.id AND c2.tbcategory1_id=c1.id AND TransDate>='%s' AND TransDate<='%s' and t.tbsubaccount_id= d.id GROUP BY c1.id ORDER BY c1.id ;", strExPart.c_str(), strStartDate.c_str(), strEndDate.c_str());
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

// ͨ��������������ȡ�����ֵĻ���
bool CBankData::ReadCurrencyByID(int nID, int nID2, double& dValue)
{
	dValue = -1;
	if (m_strSysDbPath.empty())
		return false;
	CppSQLite3DB db;
	db.open(m_strUtfSysDbPath.c_str());

	CppSQLite3Buffer bufSQL;
	bufSQL.format("SELECT ExchageRate FROM tbFav WHERE sysCurrency_id=%d AND sysCurrency_id1=%d;", nID, nID2);
	CppSQLite3Query q = db.execQuery(bufSQL);
	if(!q.eof())
	{
		dValue = q.getFloatField("ExchangeRate");
	}

	q.finalize ();
	return true;
}

bool CBankData::CreateDataDBFile(LPSTR lpPath)
{
	CppSQLite3DB db;
	db.open(lpPath);

	db.execDML("CREATE  TABLE IF NOT EXISTS `datExchangeRate` (\
			   `sysCurrency_id` INTEGER NOT NULL,\
			   `sysCurrency_id1` INTEGER NOT NULL,\
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

	db.execDML("CREATE  TABLE IF NOT EXISTS `datUSBKeyInfo` (\
			   `vid` INTEGER NOT NULL,\
			   `pid` INTEGER NOT NULL,\
			   `mid` INTEGER NOT NULL ,\
			   `fav` CHAR(4), `xml` CHAR(512), `bkurl` CHAR(256), `ver` CHAR(16), `status` INT);");//, PRIMARYKEY KEY(vid, pid)
	/*db.execDML("CREATE TABLE IF NOT EXISTS `tbUSBInfo` (`vid` INTEGER NOT NULL,\
			   `pid` INTEGER NOT NULL, `mid` INTEGER, `fav` CHAR(4), `xml` CHAR(512), `bkurl` CHAR(256), `ver` CHAR(16), `status` INT, PRIMARYKEY KEY(vid, pid));");*/

	return true;
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

bool CBankData::IsMonthImport(const char* pKeyInfo, const char* pMonth, int nAccountID)
{
	if (NULL == pKeyInfo || nAccountID <= 0 || pMonth == NULL)
		return false;

	if(!CheckUserDB())
		return false;

	CppSQLite3Buffer bufSQL;

	string strMonth = pMonth;
	strMonth = FilterStringNumber(strMonth);
	if(strMonth.size() > 8)
		strMonth = strMonth.substr(0, 6);

	bufSQL.format("SELECT tbmonth from tbAccountGetBillMonth where tbKeyInfo = '%s' and tbmonth = '%s'and tbaccount_id = %d;", pKeyInfo, strMonth.c_str(), nAccountID);



	CppSQLite3Query q = m_dbUser.execQuery(bufSQL);

	bool bBack = false;
	if(!q.eof())
	{
		bBack = true;
	}

	q.finalize ();

	return bBack;
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