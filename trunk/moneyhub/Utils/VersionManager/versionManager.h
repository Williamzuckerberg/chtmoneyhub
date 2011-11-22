#pragma once

typedef std::map<std::wstring, std::wstring>  VMMAPDEF;

enum MONEYHUBVERSION
{
	ALLVERSION =0,
	MAINVERSION =1,
	BANKVERSION,
	OTHERVERSION,
};

class versionManager
{
public:
	versionManager(void);
	~versionManager(void);

public:
	static versionManager * m_instance;
	static versionManager * getHinstance();
	/**
	*    ��ȡ����汾�ţ�index��ʾ��Ҫ��ȡ�ĺ���ģ��İ汾��Ϣ��
	*/
	 bool getAllVersion(VMMAPDEF& mapVersion, MONEYHUBVERSION index, bool bEnName = true);

private:
	 void getMainModuleVersion(VMMAPDEF& mapVersion);
	 void getBankModuleVersion(VMMAPDEF& mapVersion);
	 void getOtherModuleVersion(VMMAPDEF& mapVersion);
private:
	 std::wstring getModulePath();

	 bool  traverseFile(LPWSTR path, LPWSTR wName, VMMAPDEF& mapVersion );
	 bool getVersionFromChk(std::wstring& vs , LPCSTR lpCHKFileName, std::wstring& szBankName);
	 bool chkToXml(LPCSTR lpChkFileName , LPSTR  lpContentXml, UINT *pLen);

private:
	 bool m_bEnName;
	 VMMAPDEF m_mapEnChName;
};
