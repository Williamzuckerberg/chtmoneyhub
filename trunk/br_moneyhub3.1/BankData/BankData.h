
#include <string>
#include <vector>
#include <list>
#include <string>
#include <windows.h>

#include "..\Utils\SQLite\CppSQLite3.h"
#include "..\ThirdParty\json\include\json\json.h"
#include "../ThirdParty/tinyxml/tinyxml.h"
#pragma once
#define MY_BANK_NAME					L"MoneyhubBankData"
#define MY_ERROR_SQL_ERROR				3000
#define MY_ERROR_RUNTIME_ERROR			3001

#define BILL_CANCEL_GET_BILL			2000		// ȡ�������˵�
#define BILL_EXCEED_MAX_TIME			2001		// ��ʱ
#define BILL_NORMAL_STATE				0			// login success
#define BILL_COM_ERROR					1000		// ����ʧ��
#define BILL_GET_ACCOUNT_ERROR			1001		// ��ȡ�˻�ʧ��
#define BILL_INNER_CANCEL				3000		// �ڲ�ȡ���ղأ�Ҫ�ر���ʾ���ڵ�½��ͬʱ�ر�ҳ��
#define BILL_LOGIN_SUCC		         	3			// login success
#define BILL_LOAD_DLG					10			// ��ʾ���ڵ�¼�˻�����
#define BILL_SELECT_ACCOUNT				20			// ѡ���˻�
#define BILL_SELECT_ACCOUNT_MONTH2		21			// �����˻�ѡ����ֵ����·�ѡ��
#define BILL_SELECT_MONTH				30			// ѡ���·�
#define BILL_SELECT_MONTH2				31			// �ڶ���ѡ���·ݵķ���
#define BLII_NEED_RESTART				33			// ѡ�����˺���Ҫ����ѡȡ
#define BILL_DOWNLOAD_DLG				40			// ��ʾ�������ؽ���
#define BILL_RECORD_NUMBER				50			// ��ʾ�����ؼ�¼��������
#define BILL_BROWSER_GO		    		81			// continue getbill 
#define BILL_BROWSER_LOOP		    	83			// loop getbill
#define BILL_FINISH_STATE				99			// һ���˵�ץȡ���
#define BILL_ALL_FINISH					100			// �����е��˵�ץȡ���

#define WM_BILL_CHANGE_NOFIFY			WM_USER + 0x100F //���ǿ���̵���Ϣ����
#define WM_BILL_HIDE_NOFIFY				WM_USER + 0x100E //���ضԻ��������
#define WM_BILL_SHOW_NOFIFY				WM_USER + 0x100D //��ʾ�Ի���
using namespace std;

#define CHECKBOX_SHOW_CHECKED			0x00000010 // ��ʾʱ��ѡ�е�״̬��ʾ
#define CHECKBOX_SHOW_UNCHECKED			0x00000100 // ��ʾʱ��û��ѡ�е�״̬��ʾ
#define CHECKBOX_SHOW_CHECKED_BEFORE	0x00001000 // ��ʾʱ��ʾΪ�Ѿ�����

typedef struct SELECTINFONODE // ѡ���˻���ѡ���·ݵ�ʱ����
{
	SELECTINFONODE()
	{
		memset(szNodeInfo, 0, 256);
		dwVal = 0;
	}
	char	szNodeInfo[256];
	DWORD	dwVal;
}SELECTINFONODE, *PSELECTINFONODE;

typedef struct TransactionRecord
{
	TransactionRecord(){
		memset(TransDate,0,256);
		memset(PostDate,0,256);
		memset(Description,0,256);
		memset(Amount,0,256);
		memset(Country,0,256);
		memset(OriginalTransAmount,0,256);
		memset(Payee,0,256);
	}
	char	TransDate[256];				//������
	char    Payee[256];					//֧������
	char	PostDate[256];				//������
	char	Description[256];			//ժҪ
	char	Amount[256];				//���
	int		CardNumber;					//����ĩ4λ
	char	Country[256];				//���׵ص�
	char	OriginalTransAmount[256];	//���׵ؽ�һ��û��	
}TRANRECORD, *LPTRANRECORD;

enum BillType
{
	RMB,
	USD
};

struct BillData
{
	BillData()
	{
		memset(select, 0, 256);
		type = 0;
		accountid = 0;
	}
	string				aid;					// ���ڻ�����aid
	char				select[256];			// ѡ���ʱ������˻�
	string				tag;					// �����˻��ı�ǣ��˺�ĩ��λ
	int					type;					// �����˻����͵�
	int					accountid;				// Ҫ������˻�id
	bool				isSel;					// ��ʼ�������Ƿ��Ѿ�ѡ���˶���	
};
typedef struct MonthBillRecord
{
	string				month;					// ������·�
	bool				m_isSuccess;			// �ж��˵��Ƿ���ɹ�
	list<LPTRANRECORD>	TranList;				// �˵�
}MONTHBILLRECORD,*LPMONTHBILLRECORD;

typedef struct BillRecord
{
	string				balance;				// ��ʧ�ܵĻ�Ϊ��F��
	BillType			type;					// �˻�����		����;��Ԫ
	list<LPMONTHBILLRECORD> bills;				// ���»���ʱ����ص��˵�
}BILLRECORD, *LPBILLRECORD;

typedef struct BillRecords
{
	BillRecords()
	{
		memset(aid, 0, 256);
		memset(tag, 0, 256);
	}
	char				aid[256];					// ����id
	char				tag[256];					// ʵ�ʵõ����˺ű��tag
	int					type;						// �˻������ͣ����ÿ�or��ǿ�
	int					accountid;					// Ҫ������˻������Ϊ��ѡ����ô��ֵ����
	bool				isFinish;					// �˻��Ƿ������
	list<LPBILLRECORD>	BillRecordlist;				// �˵�
	list<SELECTINFONODE> m_mapBack;				// �����˵��м��ȡ������(����˺Ż������·�,boolֵ��ʾ�Ƿ�ѡ��)
	//dll������ڴ�ͳһ�ɸýӿڽ��д����������������ڴ�����
}BILLRECORDS, *LPBILLRECORDS;

typedef struct tagEventRecord
{
	int id;
	__time32_t event_date;
	__time32_t start_date;
	char description[256];
	int repeat;
	int alarm;
	int status;

} EVENTRECORD, *LPEVENTRECORD;

typedef struct tagCouponRecord
{
	int id;
	__time32_t expire;

} COUPONRECORD, *LPCOUPONRECORD;

typedef struct tagAlarmRecord
{
	int type;	// 0 event, 1 coupon
	int id;
	__time32_t event_date;
	char description[256];
	int status;

} ALARMRECORD, *LPALARMRECORD;

typedef struct tagFavRecord
{
	char fav[256];	
	int status; // =0��ʾû��װ���пؼ���=100��ʾ�ɹ���װ�ؼ�
	int favorder;
	bool bdelete;
} FAVRECORD, *LPFAVRECORD;

typedef struct tagUSBKeyRecord
{
	USHORT  vid;		// ���vid
	USHORT  pid;		// ���pid
	DWORD	mid;		// ���û��mid��0
	char	fav[5];		// ���ڻ���id
	char	xml[512];	// ��ѯ�汾��������ص�ַ��xml
	char	bkurl[256]; // �������ص�ַ
	char	ver[16];	// �Ѿ���װ�İ汾�����û��װ����1.0.0.0������ȡ2.0.0.0
	int		status;		// ��ʾ����״̬��0-99��ʾ���ع��̣�100��ʾ���أ�200��ʾ���ذ�װ��
} USBRECORD, *LPUSBRECORD;

class CBankData
{
friend class ObjectLock;

public:
	CBankData();
	~CBankData();

	enum CACULATE_TYPE
	{
		emCACULATEWEEK = 1,
		emCACULATEMONTH,
		emCACULATESEASON,
		emCACULATEYEAR,
	};

public:
	static CBankData* GetInstance();
	const char* GetDbPath(std::string strUsID = "");
	static const char* GetCouponPath();

	void RemoveDatabase();
	string FilterStringNumber(string& scr);

public:
	bool DeleteUSB(int vid, int pid, DWORD mid);
	int AddUSB(int vid, int pid, DWORD mid, std::string fav, std::string xml,std::string bkurl, std::string ver,int status);
	int UpdateUSB(int vid, int pid, DWORD mid, std::string fav, std::string xml);
	int UpdateUSB(int vid, int pid, DWORD mid, int status);
	int UpdateUSB(int vid, int pid, DWORD mid, const std::string& strVer);
	bool IsUsbExist(int vid, int pid, DWORD mid = 0);
	bool GetAllUsb(std::vector<LPUSBRECORD>& vec);
	//bool GetNotSetupUsbKey(std::vector<LPUSBRECORD>& vec);
	bool GetAUSBRecord(int vid, int pid, DWORD mid, USBRECORD& record);
	bool ChangeOrder(const char* pBankID, int nTo, int nFrom); // �û������϶������ղ�˳��

	// ����������c++������õ�
public:
	bool IsFavExist(std::string& strFav);		// �ж��Ƿ����
	bool OpenDB(std::string strUsID = "");		// ���û����ݿ�
	bool ReOpenDB(std::string strUsID = "");	// �ش��û����ݿ�
	void CloseDB(std::string strUsID = "");		// �ر��û����ݿ�
	bool CheckUserDB();							// �����û����ݿ�
	
	//������JS�ű����õĴ���
public:
	
	bool IsMonthImport(const char* pKeyInfo, const char* pMonth, int nAccountID); 
	// �ղؼ����ݲ���
	// ע����øú����������lFav
	//void GetFav(std::list<LPFAVRECORD>& lFav);			//Ҫ�����е�Fav����״̬
	int DeleteFav(std::string strFav);				//ɾ���ղؼ�
	int SaveFav(std::string strFav, int status);	//�޸��ղؼ�
	int GetFav(std::string& strFav);				//�����ʱ����
	int GetFavState(std::string& strFav);
	int GetFav(std::list<std::string>& strFav, bool bNoInstall = true); // �õ�û�а�װ���Ѿ��ղص����пؼ�ID

	// �û��������
	void SavePwd(std::string strPwd);
	std::string GetPwd();

	int AddEvent(std::string event);
	int DeleteEvent(int id);
	int GetEvents(int year, int month, std::string& events);
	int GetEventsOneDay(int year, int month, int day, std::string& events);

	int SetAlarm(std::string alarm);
	std::string GetTodayAlarms();
    int GetTodayAlarmsNumber();
	
	int SaveCoupon(int style, std::string id, std::string expire, std::string couponname);
	int DeleteCoupon(int id);
	std::string GetCoupons();

	std::string QuerySQL(std::string strSQL, std::string strDBName); // �˵��ӿ�
	int ExecuteSQL(std::string strSQL);
	std::string GetXMLData(const std::string& strStartDate, const std::string& strEndDate, const std::string& dataseries, const std::string& charttype);
	
	// ���lpConditon = NULL�������ݿ��������������һ�����ݿ��У�����ֻ�������������ļ�¼(������ṹһ��)
	bool CpyDbTb2OtherDbTb(CppSQLite3DB& dbSour, CppSQLite3DB& dbDesc, LPSTR lpTbName, LPSTR lpDesTbName, LPSTR lpConditon = NULL);

	bool InstallUpdateDB();
	// �����������Ľṹ��ͬ��ָ�����������п�����
	//bool CpyDbTb2OtherDbTb(const CppSQLite3DB& dbSour, const CppSQLite3DB& dbDesc, PTBCPYNODE pTabName, std::vector<PTBCPYNODE>* pColName,  LPSTR lpCondition = NULL);

protected:

	bool InternalGetFav(std::vector<LPFAVRECORD>& vec);
	bool InternalDeleteFav(std::string strFav);
	bool InternalSaveFav(std::string strFav, int status);
	bool ReadCurrencyByID(int nID, int nID2, double& dValue); // ��ȡ�������ּ�Ļ���
	bool CreateDataDBFile(LPSTR lpPath);
	int GetDBVersion(CppSQLite3DB& db); // ��ȡ���ݿ�������İ汾
	bool CopyDataFromOldVersoion(CppSQLite3DB& dbSour, CppSQLite3DB& dbDesc);

	CppSQLite3DB* GetDataDbObject(); // �õ�DataDB���ݿ�����ָ��
	CppSQLite3DB* GetSysDbObject(); // �õ�SysDB���ݿ�����ָ��(Ӧ�ý�3����ȡ���ݿ�ķ���д��һ������)

	int InternalAddEvent(LPEVENTRECORD lpEventRec);
	bool InternalDeleteEvent(int id);
	bool InternalGetEvents(int year, int month, std::vector<LPEVENTRECORD>& vecEventRec);
	bool InternalGetEventsOneDay(int year, int month, int day, std::vector<LPEVENTRECORD>& vecEventRec);

	bool InternalSetAlarm(int id, int status, std::string alarmtype);
	bool InternalGetTodayAlarms(std::vector<LPALARMRECORD>& vec);
	
	bool InternalSaveCoupon(int style, std::string id, __time32_t expire, std::string couponname);
	bool InternalDeleteCoupon(int id);
	bool InternalGetCoupons(std::vector<LPCOUPONRECORD>& vecCouponRec);

	std::string InternalQuerySQL(std::string strSQL, std::string strDBName); // �˵��ӿ�
	int InternalExecuteSQL(std::string strSQL);
	std::string InternalGetPieXMLData(std::string strStartDate, std::string strEndDate);
	std::string InternalGetNewPieXMLData(std::string strStartDate, std::string strEndDate);
	std::string InternalGetBarXMLData(const std::string& strStartDate, const std::string& strEndDate, const std::string& strKind);

	void ReplaceCharInString(std::string& strBig, const std::string & strsrc, const std::string &strdst); 

	bool GetExchangePartSql(LPCSTR lpStrConditon, std::string& strExchange);

	void SetIndexDoubleAttribute(TiXmlElement * pSet, const char * name, double val );

	// �ж������Ƿ����
	bool IsFileExist(LPWSTR lpPath);


	// ��Դ�ļ�ת����Ŀ���ļ�
	//bool TranslateSourceFile2DestinationFile(LPSTR lpSource, LPSTR lpDestination, bool bEncrypt = true);

	void TryResetAlarmStatus(__time32_t tToday);

	std::string ToSQLSafeString(std::string strQuery);
	std::string ToSQLUnsafeString(std::string strQuery);
	std::string ToStrTime(__time32_t t);
	__time32_t  ToIntTime(std::string t);
	
	bool IsAlarmEvent(__time32_t today, __time32_t event_date, int repeat1, int alarm, __time32_t* pAlarmDate = NULL);
	bool NotLeapYear(int year);
	void AdjustTimeValue(struct tm* _tm);

	bool IsMonthShowEvent(__time32_t tHead, __time32_t tTail, __time32_t event_date, int repeat1, std::vector<__time32_t>* pvecShowDates = NULL);

	bool IsValidEventJson(Json::Value& root);
	bool IsValidAlarmJson(Json::Value& root);

	bool DownloadCouponImages(int style, std::string sn, int id);
	bool DeleteCouponImages(int id);
	void split(std::string& s, std::string& delim,std::vector< std::string >* ret);
	void InitSysDbTempFile(void); // ��ʼ��ϵͳ���ݿ���ʱ�ļ�

private:
	static CRITICAL_SECTION m_cs;
	void CreateAccountTables(CppSQLite3DB& db); // �˵��ӿ�
	std::string m_strUtfUserDbPath; // �洢���ǵ�ǰ�û����ݿ��ȫ·�� utf8!!
	std::wstring m_strUserDbPath;//

	//std::string m_strUserSourDbPath; // �洢��ǰ�û������ݿ�Դ�ļ�·��
	std::string m_strUtfSysDbPath; // �洢ϵͳ���ݵ�ȫ·�� uft8!!!
	std::wstring m_strSysDbPath; // �洢ϵͳ���ݵ�ȫ·��

	std::string m_strUtfDataDbPath; // �洢DataDB��ȫ·��  ����utf8
	std::wstring m_strDataDbPath;//
	std::wstring m_strGuestTemplete; // �洢�û�ģ�����ݵ�·��//�������utf�洢

	CppSQLite3DB m_dbUser;
	CppSQLite3DB m_dbDataDB; // DataDB���ݿ����
	CppSQLite3DB m_dbSysDB; // SysDB���ݿ����
};

class ObjectLock
{
public:
	ObjectLock(CBankData* pData)
	{
		m_pBankData = pData;
		EnterCriticalSection(&m_pBankData->m_cs);
	}

	~ObjectLock()
	{
		LeaveCriticalSection(&m_pBankData->m_cs);
	}

private:
	CBankData* m_pBankData;
};