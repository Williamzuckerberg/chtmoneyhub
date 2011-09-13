#include "stdafx.h"
#include "CloudFileSelector.h"

const long MAX_VALUENAME = 16383;
CCloudFileSelector::CCloudFileSelector(void)
{
	m_sysversion = 0;
}

CCloudFileSelector::~CCloudFileSelector(void)
{
	m_cloudfile.clear();
}

set<wstring>* CCloudFileSelector::GetFiles()
{
	return &m_cloudfile;
}


void CCloudFileSelector::AddWhiteList(int tag,wstring filename)
{
	switch(tag)
	{
		case 1:{
			if(m_java == L"")
				if(GetJavaInstallDirectory() == false)
					break;

			wstring full = m_java + filename;

			WCHAR expName[MAX_PATH] ={0};
			
			ExpandEnvironmentStringsW(full.c_str(), expName, MAX_PATH);
		
			_wcsupr_s(expName, wcslen(expName) + 1);//��дת��
			wstring wtp(expName);
			m_cloudfile.insert(wtp);
			break;
		}//java file
		case 2:{
			if(m_sysversion == 0)
				if(GetWinVer() == false)
					break;
			if(m_sysversion >= 6)
			{
				WCHAR expName[MAX_PATH] ={0};
				ExpandEnvironmentStringsW(filename.c_str(), expName, MAX_PATH);
				_wcsupr_s(expName, wcslen(expName) + 1);//��дת��
				wstring wtp(expName);
				m_cloudfile.insert(wtp);
			}
			break;
		}//win7/vista file
		default:{
			WCHAR expName[MAX_PATH] ={0};
			ExpandEnvironmentStringsW(filename.c_str(), expName, MAX_PATH);
			_wcsupr_s(expName, wcslen(expName) + 1);//��дת��
			wstring wtp(expName);
			m_cloudfile.insert(wtp);
			break;
		}
	}

}
void CCloudFileSelector::AddExtensionsFile(wstring folder,wstring extensions)
{
	if(folder == L"")
		return;
	WCHAR expName[MAX_PATH] ={0};
	ExpandEnvironmentStringsW(folder.c_str(), expName, MAX_PATH);
	wstring infolder(expName);

	wstring findname = infolder + L"\\*." + extensions;
	// file
	WIN32_FIND_DATAW fd;
	memset(&fd, 0, sizeof(WIN32_FIND_DATAW));
	HANDLE hFind = FindFirstFileW(findname.c_str(), &fd);
	if(INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			wstring subname = fd.cFileName;
			if (subname != L"." && subname != L"..")
			{
				wstring fname = infolder +L"\\" + subname;
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					transform(fname.begin(), fname.end(), fname.begin(), towupper); //ת����д
					m_cloudfile.insert(fname);
				}
			}
		} while (FindNextFileW(hFind, &fd) != 0);
	}

	FindClose(hFind);
}

void CCloudFileSelector::AddRegFile(wstring regflag,wstring key,wstring file,int flag)
{
	DWORD dwValueNameLength = MAX_VALUENAME;        // ֵ���ַ�������   
    WCHAR ptszValueName[MAX_VALUENAME] = L"";    // ֵ���ַ���

	//���
	if(flag == 1)
	{
		DWORD dwType, dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;
		if( ERROR_SUCCESS != ::SHGetValueW(HKEY_LOCAL_MACHINE,regflag.c_str(),key.c_str(), &dwType, &ptszValueName, &dwValueNameLength))
		{
			return;
		}

		if(dwValueNameLength > 0)
		{
			WCHAR expName[MAX_PATH] ={0};
			ExpandEnvironmentStringsW(ptszValueName, expName, MAX_PATH);
			PathRemoveFileSpecW(expName);
			wstring full(expName);
			full += L"\\"+file;

			transform(full.begin(), full.end(), full.begin(), towupper); //ת����д
			m_cloudfile.insert(full);
			return;
		}
	}
	else if(flag == 3)
	{
		DWORD dwType, dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;
		if( ERROR_SUCCESS != ::SHGetValueW(HKEY_LOCAL_MACHINE,regflag.c_str(),key.c_str(), &dwType, &ptszValueName, &dwValueNameLength))
		{
			return;
		}

		if(dwValueNameLength > 0)
		{
			WCHAR expName[MAX_PATH] ={0};
			ExpandEnvironmentStringsW(ptszValueName, expName, MAX_PATH);
			wstring full(expName);
			full += L"\\"+file;

			transform(full.begin(), full.end(), full.begin(), towupper); //ת����д
			m_cloudfile.insert(full);
			return;
		}
	}

	else if(flag == 2)
	{
		const int MAX_KEY = 260;
		DWORD dwValueNameLength = MAX_VALUENAME;        // ֵ���ַ�������   
		WCHAR ptszValueName[MAX_VALUENAME] = _T(""); 
		HKEY hKey = NULL;                               // ���������
		DWORD dwSubKeyCount = 0;                        // ��ǰ�Ӽ���   
		DWORD dwValueCount = 0;                         // ��ǰ��ֵ�� 
		WCHAR ptszSubKey[MAX_KEY] = L"";				// �Ӽ��ַ���
		DWORD cbSubKey = MAX_KEY;						// �Ӽ��ַ�������

		DWORD dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;

		//�Ȳ�Environments���������
		if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, regflag.c_str(), 0, KEY_READ, &hKey))   
		{   
			return;
		} 
	
		if( ERROR_SUCCESS != ::SHQueryInfoKeyW(hKey,&dwSubKeyCount,NULL, &dwValueCount,NULL))
		{
			if (NULL != hKey)   
			{   
				::RegCloseKey(hKey);   
				hKey = NULL;   
			}
			return;
		}


		if (0 != dwValueCount)   
		{
			for (DWORD i = 0; i < dwValueCount; i++)   
			{
				dwPos = dwMax =  260;
				if(ERROR_SUCCESS == ::SHEnumValueW(hKey,i,ptszSubKey,&dwPos,&dwSize,ptszValueName,&dwMax))
				{
					PathRemoveFileSpecW(ptszValueName);
					DWORD attr = GetFileAttributesW(ptszValueName);
					if(attr == FILE_ATTRIBUTE_DIRECTORY)
					{
						wstring full(ptszValueName);
						full += L"\\"+file;
						transform(full.begin(), full.end(), full.begin(), towupper); //ת����д
						m_cloudfile.insert(full);
						return;
					}
				}
			}
		}
		::RegCloseKey(hKey); 
	}

}

void CCloudFileSelector::AddRegFolder(wstring regflag,wstring keyname,int flag)
{
	DWORD dwValueNameLength = MAX_VALUENAME;        // ֵ���ַ�������   
    WCHAR ptszValueName[MAX_VALUENAME] = L"";    // ֵ���ַ���

	//���java��װ·��
	if(flag == 1)
	{
		DWORD dwType, dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;
		if( ERROR_SUCCESS != ::SHGetValueW(HKEY_LOCAL_MACHINE,regflag.c_str(),keyname.c_str(), &dwType, &ptszValueName, &dwValueNameLength))
		{
			return;
		}
		if(wcsstr(ptszValueName,L"McAfee") != NULL)
		{
			PathRemoveFileSpecW(ptszValueName);
		}

		if(dwValueNameLength > 0)
		{
			WCHAR expName[MAX_PATH] ={0};
			ExpandEnvironmentStringsW(ptszValueName, expName, MAX_PATH);
			//PathRemoveFileSpecW(expName);
			AddFolder(expName);
		}
	}

	else if(flag == 2)
	{
		const int MAX_KEY = 260;
		DWORD dwValueNameLength = MAX_VALUENAME;        // ֵ���ַ�������   
		WCHAR ptszValueName[MAX_VALUENAME] = _T(""); 
		HKEY hKey = NULL;                               // ���������
		DWORD dwSubKeyCount = 0;                        // ��ǰ�Ӽ���   
		DWORD dwValueCount = 0;                         // ��ǰ��ֵ�� 
		WCHAR ptszSubKey[MAX_KEY] = L"";				// �Ӽ��ַ���
		DWORD cbSubKey = MAX_KEY;						// �Ӽ��ַ�������

		DWORD dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;

		//�Ȳ�Environments���������
		if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, regflag.c_str(), 0, KEY_READ, &hKey))   
		{   
			return;
		} 
	
		if( ERROR_SUCCESS != ::SHQueryInfoKeyW(hKey,&dwSubKeyCount,NULL, &dwValueCount,NULL))
		{
			if (NULL != hKey)   
			{   
				::RegCloseKey(hKey);   
				hKey = NULL;   
			}
			return;
		}


		if (0 != dwValueCount)   
		{
			for (DWORD i = 0; i < dwValueCount; i++)   
			{
				dwPos = dwMax =  260;
				if(ERROR_SUCCESS == ::SHEnumValueW(hKey,i,ptszSubKey,&dwPos,&dwSize,ptszValueName,&dwMax))
				{
					PathRemoveFileSpecW(ptszValueName);
					DWORD attr = GetFileAttributesW(ptszValueName);
					if(attr == FILE_ATTRIBUTE_DIRECTORY)
					{
						AddFolder(ptszValueName);
					}
				}
			}
		}
		::RegCloseKey(hKey); 
	}
}

void CCloudFileSelector::AddFolder(wstring folder)//��Ҫ�к�׺
{
	EnumDir(folder+L"\\");
	return;
}


void CCloudFileSelector::EnumDir(wstring resToken)
{
	if(resToken == L"")
		return;
	wstring findname = resToken + L"*.*";
	// file
	WIN32_FIND_DATAW fd;
	memset(&fd, 0, sizeof(WIN32_FIND_DATAW));
	HANDLE hFind = FindFirstFileW(findname.c_str(), &fd);
	if(INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			wstring subname = fd.cFileName;
			if (subname != L"." && subname != L"..")
			{
				wstring fname = resToken + subname;
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					fname += L"\\";
					EnumDir(fname);
				}
				else
				{
					transform(fname.begin(), fname.end(), fname.begin(), towupper); //ת����д
					m_cloudfile.insert(fname);
				}
			}
		} while (FindNextFileW(hFind, &fd) != 0);
	}

	FindClose(hFind);
}


void CCloudFileSelector::ClearFiles()
{
	m_sysversion = 0;
	m_java.empty();
	m_cloudfile.clear();
}
void CCloudFileSelector::GetAllFiles()
{
	//��ʼ��ģ������ini�ļ�
	//GetModuleFileNameW(NULL, m_cloudcheckfile, MAX_PATH);
	//PathRemoveFileSpecW(m_cloudcheckfile);
	//wcscat(m_cloudcheckfile,L"\\CloudCheck.ini");



	//���Java���е��ļ��б�
	//GetJavaFiles();

	//������뷨dll
	GetInputFiles();

	//��ô�ӡ������
	GetPrintDriver();//���԰�ȡ����ӡ����

	//���Win7��Vista����Ҫ���ļ�
	//GetWinFiles();

	//���IE�ļ������ڲ��������
	//GetIEFiles();

	//��������ļ�
	GetThemeFiles();
}

bool CCloudFileSelector::GetThemeFiles()
{
	DWORD dwValueNameLength = MAX_VALUENAME;        // ֵ���ַ�������   
    WCHAR ptszValueName[MAX_VALUENAME] = L"";    // ֵ���ַ���

	//���java��װ·��
	DWORD dwType, dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;
	if( ERROR_SUCCESS != ::SHGetValueW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager", L"DllName", &dwType, &ptszValueName, &dwValueNameLength))
	{
		return false;
	}

	if(dwValueNameLength > 0)
	{
		WCHAR expName[MAX_PATH] ={0};
		ExpandEnvironmentStringsW(ptszValueName, expName, MAX_PATH);
		_wcsupr_s(expName, wcslen(expName) + 1);//��дת��
		wstring wtp(expName);
		m_cloudfile.insert(wtp);
	}

	return true;
}
bool  CCloudFileSelector::GetIEFiles()
{
	/*unsigned int len;
	WCHAR  strbuf[1256] = {0};
	len = GetPrivateProfileSectionW(L"IE",strbuf,1256,m_cloudcheckfile);
	GetStringFiles(strbuf,len,L"");*/
	return true;
}

bool CCloudFileSelector::GetWinVer()
{
	OSVERSIONINFOEXW os; 
    os.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEXW);
	GetVersionExW((OSVERSIONINFOW *)&os);

	m_sysversion = os.dwMajorVersion;
	//if( >= 6)//Win7����Vista��2008
	//{
	//	
	//	unsigned int len;
	//	WCHAR  strbuf[1256] = {0};
	//
	//	len = GetPrivateProfileSectionW(L"Win7",strbuf,1256,m_cloudcheckfile);

	//	GetStringFiles(strbuf,len,L"");
	//}

	return true;
}


bool CCloudFileSelector::GetStringFiles(const wchar_t* str,int len,const wstring& dir)
{
	if(str == NULL || len <= 0|| len > 2000)
		return false;

	WCHAR  work[MAX_PATH] = {0};
	int   Count = 0; // gao
	int   CountPrivate = 0;

	while(Count < len)
	{
		memset(work, '\0', sizeof(work));      /*work���ڻ�*/
		memcpy(work, &str[Count],wcslen(&str[Count])*sizeof(WCHAR));/**/
		CountPrivate = wcslen(&str[Count]) + 1;/**/
		Count += CountPrivate;/**/
		wstring tp(work);
		if(tp == L"")
			break;

		tp = dir + tp;

		WCHAR expName[MAX_PATH] ={0};
		ExpandEnvironmentStringsW(tp.c_str(), expName, MAX_PATH);
		_wcsupr_s(expName, wcslen(expName) + 1);//��дת��
		wstring wtp(expName);											
		m_cloudfile.insert(wtp);
	}
	return true;

}
bool CCloudFileSelector::GetJavaInstallDirectory()
{
	DWORD dwValueNameLength = MAX_VALUENAME;        // ֵ���ַ�������   
    WCHAR ptszValueName[MAX_VALUENAME] = L"";    // ֵ���ַ���

	//���java��װ·��
	DWORD dwType, dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;
	if( ERROR_SUCCESS != ::SHGetValueW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\JavaSoft\\Java Web Start", L"CurrentVersion", &dwType, &ptszValueName, &dwValueNameLength))
	{
		return false;
	}

	WCHAR ptszNextPath[500]; // 500 ΪԤ����С, �������ʵ��Ӵ�   
    swprintf(ptszNextPath,500,L"SOFTWARE\\JavaSoft\\Java Web Start\\%s", ptszValueName); 

	dwValueNameLength = MAX_VALUENAME;//��ʹ��֮ǰҪ��ֵ���㹻��С
	if( ERROR_SUCCESS != ::SHGetValueW(HKEY_LOCAL_MACHINE,ptszNextPath, L"Home", &dwType, &ptszValueName, &dwValueNameLength))
	{
		return false;
	}

	//unsigned int func_rtn;
	WCHAR  strbuf[1256] = {0};
	

	//func_rtn = GetPrivateProfileSectionW(L"Java",strbuf,1256,m_cloudcheckfile);

	wstring fl(ptszValueName);
	fl += L"\\";

	m_java = fl;

	return true;
}

bool CCloudFileSelector::GetInputFiles()
{
	const int MAX_KEY = 260;
	DWORD dwValueNameLength = MAX_VALUENAME;        // ֵ���ַ�������   
    WCHAR ptszValueName[MAX_VALUENAME] = _T(""); 
    HKEY hKey = NULL;                               // ���������
	DWORD dwSubKeyCount = 0;                        // ��ǰ�Ӽ���   
    DWORD dwValueCount = 0;                         // ��ǰ��ֵ�� 
	WCHAR ptszSubKey[MAX_KEY] = L"";					// �Ӽ��ַ���
	DWORD cbSubKey = MAX_KEY;							// �Ӽ��ַ�������

	DWORD dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;

	if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts", 0, KEY_READ, &hKey))   
    {   
       return false;
    } 
	
	if( ERROR_SUCCESS != ::SHQueryInfoKeyW(hKey,&dwSubKeyCount,NULL, &dwValueCount,NULL))
	{
		if (NULL != hKey)   
        {   
            ::RegCloseKey(hKey);   
            hKey = NULL;   
        }
		return false;
	}


	// �����Ӽ�   
    if (0 != dwSubKeyCount)   
    {
		for (DWORD i = 0; i < dwSubKeyCount; i++)   
        { 
			cbSubKey = MAX_KEY;   
			if (ERROR_SUCCESS == ::SHEnumKeyExW(hKey, i, ptszSubKey, &cbSubKey))   
            {   
				 WCHAR ptszValueName[1024]; // 1024 ΪԤ����С, �������ʵ��Ӵ�
				
				 DWORD dwType;
				 DWORD dwValueNameLength = MAX_VALUENAME;
				 if(ERROR_SUCCESS ==::SHGetValueW(hKey,ptszSubKey,L"Layout File",&dwType,&ptszValueName,&dwValueNameLength))
				 {
					 if(wcscmp(ptszValueName,L"") != 0)
					 {
						 wchar_t* pfile = ptszValueName;
						 _wcslwr_s(pfile,1024);
						 //wchar_t* pfile = _wcslwr(ptszValueName);
						 wstring tp(pfile);
						 tp = L"%SystemRoot%\\system32\\" + tp;
						 WCHAR expName[MAX_PATH] ={0};
						 ExpandEnvironmentStringsW(tp.c_str(), expName, MAX_PATH);
						 _wcsupr_s(expName, wcslen(expName) + 1);//��дת��
						 wstring wtp(expName);
						 m_cloudfile.insert(wtp);
					 }
				 }

				 dwValueNameLength = MAX_VALUENAME;
				 if(ERROR_SUCCESS ==::SHGetValueW(hKey,ptszSubKey,L"IME File",&dwType,&ptszValueName,&dwValueNameLength))
				 {
					 if(wcscmp(ptszValueName,L"") != 0)
					 {
						 wchar_t* pfile = ptszValueName;
						 _wcslwr_s(pfile,1024);
						 //wchar_t* pfile = _wcslwr(ptszValueName);
						 wstring tp(pfile);
						 tp = L"%SystemRoot%\\system32\\" + tp;
						 WCHAR expName[MAX_PATH] ={0};
						 ExpandEnvironmentStringsW(tp.c_str(), expName, MAX_PATH);
						 _wcsupr_s(expName, wcslen(expName) + 1);//��дת��
						 wstring wtp(expName);
						  m_cloudfile.insert(wtp);
					 }

				 }
			}
		}
	}

	::RegCloseKey(hKey);
	return true;
}



bool CCloudFileSelector::GetPrintDriver()
{
	GetPrintEnvironments();
	GetPrintMonitors();
	GetPrintProviders();
	return true;
}

bool CCloudFileSelector::GetPrintEnvironments()
{
	const int MAX_KEY = 260;
	DWORD dwValueNameLength = MAX_VALUENAME;			// ֵ���ַ�������   
    WCHAR ptszValueName[MAX_VALUENAME] = _T(""); 
    HKEY hKey = NULL;									// ���������
	DWORD dwSubKeyCount = 0;							// ��ǰ�Ӽ���   
    DWORD dwValueCount = 0;								// ��ǰ��ֵ�� 
	WCHAR ptszSubKey[MAX_KEY] = L"";					// �Ӽ��ַ���
	DWORD cbSubKey = MAX_KEY;							// �Ӽ��ַ�������

	DWORD dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;

	//�Ȳ�Environments���������
	if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Print\\Environments", 0, KEY_READ, &hKey))   
    {   
       return false;
    } 
	
	if( ERROR_SUCCESS != ::SHQueryInfoKeyW(hKey,&dwSubKeyCount,NULL, &dwValueCount,NULL))
	{
		if (NULL != hKey)   
        {   
            ::RegCloseKey(hKey);   
            hKey = NULL;   
        }
		return false;
	}

	wstring wSubDir;
	HKEY hChildKey = NULL;								//�Ӽ�

	// �����Ӽ�   
    if (0 != dwSubKeyCount)   
    {
		for (DWORD i = 0; i < dwSubKeyCount; i++)   
        { 
			cbSubKey = MAX_KEY;   
			if (ERROR_SUCCESS == ::SHEnumKeyExW(hKey, i, ptszSubKey, &cbSubKey))   
            {   
				 WCHAR ptszValueName[1024]; // 1024 ΪԤ����С, �������ʵ��Ӵ�
				
				 DWORD dwType;
				 DWORD dwValueNameLength = MAX_VALUENAME;
				 //������ļ�������¼
				 if(ERROR_SUCCESS ==::SHGetValueW(hKey,ptszSubKey,L"Directory",&dwType,&ptszValueName,&dwValueNameLength))
				 {
					 if(wcscmp(ptszValueName,L"") != 0)
					 {
						 wstring tp(ptszValueName);
						 wSubDir = tp;
					 }
					 else
						 continue;
				 }

				 wstring wwSub(ptszSubKey);
				 wwSub += L"\\Drivers";

				if (ERROR_SUCCESS != ::RegOpenKeyExW(hKey, wwSub.c_str(), 0, KEY_READ, &hChildKey))   
				{ 
					continue;
				}

				DWORD ndwSubKeyCount = 0 ,ndwValueCount = 0;

				if( ERROR_SUCCESS != ::SHQueryInfoKeyW(hChildKey,&ndwSubKeyCount,NULL, &ndwValueCount,NULL))
				{
					if (NULL != hChildKey)   
					{   
						::RegCloseKey(hChildKey);
						hChildKey = NULL;   
					}
					continue;
				}

				if(ndwSubKeyCount > 0)
				{
					//����Ŀ���ļ��У���ʼ�����ļ�
					WCHAR nptszSubKey[MAX_KEY] = L"";					// �Ӽ��ַ���
					DWORD ncbSubKey = MAX_KEY;							// �Ӽ��ַ�������

					for(DWORD j = 0; j < ndwSubKeyCount; j++)
					{
						if (ERROR_SUCCESS == ::SHEnumKeyExW(hChildKey, j, nptszSubKey, &ncbSubKey))   
						{
							HKEY hActKey = NULL;
							WCHAR nptszValueName[1024];
							DWORD ndwValueNameLength = MAX_VALUENAME;
							if(ERROR_SUCCESS ==::SHGetValueW(hChildKey,nptszSubKey,L"Directory",&dwType,&nptszValueName,&ndwValueNameLength))
							{
								if(wcscmp(ptszValueName,L"") != 0)
								{
									wstring tp(nptszValueName);
									wSubDir += L"\\" + tp + L"\\";
								}
								else
									continue;
							}

							if( ERROR_SUCCESS != ::RegOpenKeyExW(hChildKey, nptszSubKey, 0, KEY_READ, &hActKey))
							{
								continue;
							}

							DWORD keycount,valuecount;
							if( ERROR_SUCCESS != ::SHQueryInfoKeyW(hActKey,&keycount,NULL, &valuecount,NULL))
							{
								if (NULL != hActKey)   
								{   
									::RegCloseKey(hActKey);
									hActKey = NULL;   
								}
								continue;
							}

							if(keycount > 0)
							{
								for(DWORD k = 0; k < keycount; k++)
								{
									WCHAR subkey[MAX_KEY];
									DWORD ncbsub = MAX_KEY;
									if (ERROR_SUCCESS == ::SHEnumKeyExW(hActKey, k, subkey, &ncbsub))   
									{
										//��������ļ�
										ndwValueNameLength = MAX_VALUENAME;
										if(ERROR_SUCCESS ==::SHGetValueW(hActKey,subkey,L"Configuration File",&dwType,&nptszValueName,&ndwValueNameLength))
										{
											wchar_t* pfile = nptszValueName;
											_wcslwr_s(pfile,1024);
											//wchar_t* pfile = _wcslwr();
						 					wstring actfile(pfile);
											actfile = L"%SystemRoot%\\System32\\spool\\drivers\\" + wSubDir + actfile;
											WCHAR expName[MAX_PATH] ={0};
											ExpandEnvironmentStringsW(actfile.c_str(), expName, MAX_PATH);
											_wcsupr_s(expName, wcslen(expName) + 1);//��дת��
											wstring wtp(expName);											
											m_cloudfile.insert(wtp);
										}

										ndwValueNameLength = MAX_VALUENAME;
										memset(nptszValueName,0,sizeof(nptszValueName));
										if(ERROR_SUCCESS ==::SHGetValueW(hActKey,subkey,L"Dependent Files",&dwType,&nptszValueName,&ndwValueNameLength))
										{
											GetStringFiles(nptszValueName,ndwValueNameLength/2,L"%SystemRoot%\\System32\\spool\\drivers\\" + wSubDir);
										}

										ndwValueNameLength = MAX_VALUENAME;
										if(ERROR_SUCCESS ==::SHGetValueW(hActKey,subkey,L"Driver",&dwType,&nptszValueName,&ndwValueNameLength))
										{
											wchar_t* pfile = nptszValueName;
											_wcslwr_s(pfile,1024);
											//wchar_t* pfile = _wcslwr(nptszValueName);
						 					wstring actfile(pfile);
											actfile = L"%SystemRoot%\\System32\\spool\\drivers\\" + wSubDir + actfile;
											WCHAR expName[MAX_PATH] ={0};
											ExpandEnvironmentStringsW(actfile.c_str(), expName, MAX_PATH);
											_wcsupr_s(expName, wcslen(expName) + 1);//��дת��
											wstring wtp(expName);
											m_cloudfile.insert(wtp);
										}
	
									}

								}
							}
							RegCloseKey(hActKey);
						}
						
					}
				}
				RegCloseKey(hChildKey);

			}
		}
	}
	::RegCloseKey(hKey);
	return true;
}
bool CCloudFileSelector::GetPrintMonitors()
{
	return GetValue(L"SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors",L"Driver");
}
bool CCloudFileSelector::GetPrintProviders()
{
	return GetValue(L"SYSTEM\\CurrentControlSet\\Control\\Print\\Providers",L"Name");
}

bool CCloudFileSelector::GetValue(wchar_t* para,wchar_t* value)
{
	const int MAX_KEY = 260;
	DWORD dwValueNameLength = MAX_VALUENAME;			// ֵ���ַ�������   
    WCHAR ptszValueName[MAX_VALUENAME] = _T(""); 
    HKEY hKey = NULL;									// ���������
	DWORD dwSubKeyCount = 0;						// ��ǰ�Ӽ���   
    DWORD dwValueCount = 0;								// ��ǰ��ֵ�� 
	WCHAR ptszSubKey[MAX_KEY] = L"";					// �Ӽ��ַ���
	DWORD cbSubKey = MAX_KEY;							// �Ӽ��ַ�������

	DWORD dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;

	//�Ȳ�Environments���������
	if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, para, 0, KEY_READ, &hKey))   
    {   
       return false;
    } 
	
	if( ERROR_SUCCESS != ::SHQueryInfoKeyW(hKey,&dwSubKeyCount,NULL, &dwValueCount,NULL))
	{
		if (NULL != hKey)   
        {   
            ::RegCloseKey(hKey);   
            hKey = NULL;   
        }
		return false;
	}


	// �����Ӽ�   
    if (0 != dwSubKeyCount)   
    {
		for (DWORD i = 0; i < dwSubKeyCount; i++)   
        { 
			cbSubKey = MAX_KEY;   
			if (ERROR_SUCCESS == ::SHEnumKeyExW(hKey, i, ptszSubKey, &cbSubKey))   
            {   
				 WCHAR ptszValueName[1024]; // 1024 ΪԤ����С, �������ʵ��Ӵ�
				
				 DWORD dwType;
				 DWORD dwValueNameLength = MAX_VALUENAME;
				 if(ERROR_SUCCESS ==::SHGetValueW(hKey,ptszSubKey,value,&dwType,&ptszValueName,&dwValueNameLength))
				 {
					 if(wcscmp(ptszValueName,L"") != 0)
					 {
						 wchar_t* pfile = ptszValueName;
						 _wcslwr_s(pfile,1024);
						 //wchar_t* pfile = _wcslwr(ptszValueName);
						 wstring tp(pfile);
						 tp = L"%SystemRoot%\\system32\\" + tp;
						 WCHAR expName[MAX_PATH] ={0};
						 ExpandEnvironmentStringsW(tp.c_str(), expName, MAX_PATH);
						 _wcsupr_s(expName, wcslen(expName) + 1);//��дת��
						 wstring wtp(expName);						 
						 m_cloudfile.insert(wtp);
					 }
				 }
			}
		}
	}

	::RegCloseKey(hKey);
	return true;

}

bool CCloudFileSelector::GetSharedDLLs()
{
	return GetValue(L"SYSTEM\\CurrentControlSet\\Control\\Print\\Providers",L"Name");
}