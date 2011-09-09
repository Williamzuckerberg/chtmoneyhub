#pragma once

//////////////////////////////////////////////////////////////////////////
// CCheckForUpdate class

class CCheckForUpdate
{
protected:
	CCheckForUpdate(HWND hParent);
	bool IsChecking(bool bAuto);

public:
	bool Check(bool bAuto);

	static void ClearInstallFlag();

public:
	static CCheckForUpdate* CreateInstance(HWND hParent);
	static int AddRef();
	static void Release();

protected:
	HWND m_hParentWnd;
	static int m_nAddRef;
};


//////////////////////////////////////////////////////////////////////////
// CInstallUpdatePack class

class CInstallUpdatePack
{
public:
	CInstallUpdatePack();

public:
	/*
	����Ƿ������������Ҫ���µİ�װ��
	*/
	bool Check();
	void Setup();

	/*
	���ע����Ƿ��û�������ģʽ����Ϊÿ�������ƽ������������updata
	*/
	bool setupUpdateWithMH();
protected:
	bool m_bNeedRun;

#ifdef _UNICODE
	std::wstring m_strCmdline;
#else
	std::string m_strCmdline;
#endif

	//static HANDLE m_hUpdateRunMutex;
};
