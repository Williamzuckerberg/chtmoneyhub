#pragma once
#include "Requirement.h"
#include <atlimage.h>
#include <algorithm>
#include "..\..\BankUI\Skin\TuoImage.h"
#include "../FavBankOperator/FavBankOperator.h"
using namespace std;

enum WebsiteType
{
	Website_None,
	Website_Bank
};


typedef std::list<CRequirement*> ReqList;

extern std::vector<std::wstring> uacPopVec;
// ������վ������
class CWebsiteData
{
public:
	~CWebsiteData()
	{
		ReqList::iterator it = m_Requirements.begin();
		for(;it != m_Requirements.end();it ++)
		{
			CRequirement* pReq = (*it);
			if(pReq != NULL)
				delete pReq;
		}
	}
	static void StartUAC(std::wstring appid);

	LPCTSTR GetID() const { return m_strID.c_str(); }
	LPCTSTR GetName() const { return m_strName.c_str(); }
	bool IsNoClose() const { return m_bNoClose; }
	bool IsNoSubTab() const{ return m_bHasSubTab;}
	WebsiteType GetWebsiteType() const { return m_eWebsiteType; }


	CImage& GetLogo() const;
	CTuoImage& GetLogoPng(bool bSelected) const;

public:
	ReqList* GetReqList(){return &m_Requirements;}

	list<wstring> m_urllist;
private:
	// id
	std::wstring m_strID;

	// ����
	std::wstring m_strName;
	mutable bool m_bNoClose;//�Ƿ��йرհ�ť
	mutable bool m_bHasSubTab;//�Ƿ��ж�����ǩ
	WebsiteType m_eWebsiteType;

	// ע������

	ReqList m_Requirements;

	mutable CImage bmpLogo;
	mutable CTuoImage m_bmpLogoPng;
	mutable CTuoImage m_bmpLogoPng_gray;

	friend class CListManager;
	friend class CURLList;
};

typedef std::map<std::wstring, CWebsiteData*> WebDataMap;


//////////////////////////////////////////////////////////////////////////
// DldFilterData

class DldFilterData
{
public:
	void Add(std::string str)
	{
		char delim = ',';
		size_t last = 0;
		size_t index = str.find_first_of(delim, last);
		while (index != std::string::npos)
		{
			m_vecFilter.push_back(str.substr(last, index - last));
			last = index + 1;
			index = str.find_first_of(delim, last);
		}

		if (index - last > 0)
			m_vecFilter.push_back(str.substr(last, index - last));
	}

	bool IsMatch(std::string ext) const
	{
		for (std::vector<std::string>::const_iterator it = m_vecFilter.begin();
			it != m_vecFilter.end(); ++it)
		{
			if (_stricmp(it->c_str(), ext.c_str()) == 0)
				return true;
		}

		return false;
	}

	void Clear()
	{
		m_vecFilter.clear();
	}

protected:
	std::vector<std::string> m_vecFilter;
};

//////////////////////////////////////////////////////////////////////////
// CListManager


class CListManager
{
private:
	enum SystemType
	{
		sAll = 0,
		sUnSupported = 1,
		sWinNT = 2,
		sWin2000 = 3,
		sWinXP = 4,
		sWin2003 = 5,
		sWinVista = 6,
		sWin2008 = 7,
		sWin7 = 8
	};
public:

	static void Initialize(bool bBankCore);// gao 2010-12-13
	static CListManager* _();

	void CalculateHMac(unsigned char* pHMAC) const;
	void CheckHMAC() const;

	void RegQueryLock(void);// gao 2010-12-16

	void RegQueryUnlock(void);// gao 2010-12-16

	// �û��ղ�һ�����У���������xml�ļ��ĸ�·�� gao 2010-12-16
	// ��bBankName = true �������boc, ���������BankID
	bool AddANewFavBank(LPWSTR lpParam, bool bBankName = true);

	// ɾ��һ�����У�����Ҫɾ�����е����� gao 2010-12-16
	bool DeleteAFavBank(string appid);

	void UpdateHMac(void); // �û�����ղأ�Ҳ�ø���MHAC
	
	CFavBankOperator* GetFavBankOper(void);
	
	~CListManager();
public:

	WebDataMap* GetWebDataMap() {return &m_WebsiteData;}
	bool GetResult(){return m_result;};
	DldFilterData* GetDldFilter() { return &m_DldFilterData; }

	bool CheckCom(string appid = "");

public:
	static CString ExtractClearDomain(LPCTSTR lpszUrl);
	static bool IsFinancePage(LPCTSTR lpszUrl);
	static void NavigateWebPage(LPCTSTR lpszUrl);
	//static void NavigateWebPage(BSTR bstrURL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers);
	static void PrintWebPage(LPCTSTR lpszUrl);

	static int GetDefaultPageIndex(LPCTSTR lpszUrl);

protected:
	static CString GetModulePath();
	static CString GetAppData();

private:

	map<wstring,SystemType> m_sysTypeList;

	SystemType m_tSystem;
	int m_tArchite;

	void GetSystemVersion();

	// ���εİ�����

	WebDataMap m_WebsiteData;
	unsigned char m_hmac[20];

	// ��ʱû����ʹ��

	bool m_result;

	// download filter

	DldFilterData m_DldFilterData;

	bool m_bBankCore; // �ü�¼�������Ƿ����ں˽��� // gao 2010-12-16

	HANDLE m_hRegQueryMutex; // �û�ע�����ʵĻ�����

	CFavBankOperator* m_pFavBkOper;

private:
	std::wstring getModulePathForSearch();

	CListManager(bool bBankCore);// gao 2010-12-13

	//���������ļ�����info.mchk
	bool traverseBankInfo(LPWSTR path, LPWSTR parentPath = NULL);

	// ��ȡUI������������� // gao 2010-12-16
	void ReadBankCoreNeedInfo(const TiXmlNode *pNode, CWebsiteData *pWebsiteData);

};
