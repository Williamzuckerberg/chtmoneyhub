#pragma once

class CWebsiteData;

class CRegItem
{

public:

	// ע����ֵ��Ľṹ
	CRegItem(LPCTSTR lpszName, LPCTSTR lpszValue, const CWebsiteData *pWebsite);

	void WriteItemToReg(HKEY hKey) const;

private:
	// ֵ����
	std::wstring m_strItemName;
	// ֵ����
	UINT m_uRegValueType;
	// ֵ���ݴ洢λ��
	union
	{
		LPCTSTR m_lpszString;
		BYTE *m_pBinary;
		DWORD m_dwDword;
	};
	// ֵ����
	int m_iDataLength;
};

//////////////////////////////////////////////////////////////////////////

class CRegDir
{

public:

	CRegDir() : m_bForceReplace(false) {}

	CRegDir* CreateRegDirs(LPCTSTR lpszPath);
	const CRegDir* GetRegDirs(LPCTSTR lpszPath) const;

	void CreateRegItem(LPCTSTR lpszKey, LPCTSTR lpszValue, const CWebsiteData *pWebsite);

	void WriteAllItemsToReg(HKEY hKey) const;
	void WriteCurrentKey(HKEY hKey) const;

	void SetForceReplace() { m_bForceReplace = true; }
	bool ForceReplaceSystemValue() const { return m_bForceReplace; }

private:
	// ע�����ļ���ֵ��
	typedef std::map<std::wstring, CRegItem*> RegItemMap;
	RegItemMap m_RegValues;

	// ע���ļ����Ӽ�
	typedef std::map<std::wstring, CRegDir*> RegDirMap;
	RegDirMap m_RegSubDirs;

	bool m_bForceReplace;
};

//////////////////////////////////////////////////////////////////////////

class CRegData
{

public:

	void LoadRegData(const std::wstring &szData, const CWebsiteData *pWebsite);

	const CRegDir* GetRegDirs(LPCTSTR lpszPath, HKEY hRootKey) const;

private:

	CRegDir* CreateRegDirs(LPCTSTR lpszPath);

	typedef std::map<HKEY, CRegDir*> RegRootDirMap;
	RegRootDirMap m_RegRootDirs;
};
