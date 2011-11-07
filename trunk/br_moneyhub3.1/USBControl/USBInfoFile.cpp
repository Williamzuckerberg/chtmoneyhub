#include "stdafx.h"
#include "USBInfoFile.h"
#include <assert.h>
#include "USBCheckor.h"
#include "../Encryption/CHKFile/CHK.h"
#include "ConvertBase.h"
#include "windows.h"
#pragma comment(lib,"Kernel32.lib")


CUSBInfoFileManager* CUSBInfoFileManager::m_Instance = NULL;

CUSBInfoFileManager* CUSBInfoFileManager::GetInstance()
{
	if(m_Instance == NULL)
		m_Instance = new CUSBInfoFileManager();
	return m_Instance;
}

CUSBInfoFileManager::CUSBInfoFileManager()
{
}
CUSBInfoFileManager::~CUSBInfoFileManager()
{
}

std::string CUSBInfoFileManager::GetFileContent(wstring strPath,bool bCHK)
{	
	HANDLE hFile = ::CreateFile(strPath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		int error = ::GetLastError();
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_USBINIT, CRecordProgram::GetInstance()->GetRecordInfo(L"USBInfoFile Read Error : %d", error));
		return "";
	}

	DWORD dwFileSize = ::GetFileSize(hFile, NULL);
	std::string strFileContent;
	unsigned char *pData = new unsigned char[dwFileSize + 1];
	::ReadFile(hFile, pData, dwFileSize, &dwFileSize, NULL);
	if (bCHK)
	{
		char* content = new char[dwFileSize + 1];
		int contentLength = unPackCHK(pData, dwFileSize, (unsigned char *)content);
		if (contentLength < 0)
		{
			CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_USBINIT, L"USBInfoFile unPackCHK Error");
			delete []content;
			delete []pData;

			CloseHandle(hFile);

			return "";
		}

		content[contentLength] = '\0';
		strFileContent = content;
		delete []content;
	}
	else
	{
		strFileContent = (char *)pData;
	}

	delete []pData;
	::CloseHandle(hFile);

	return strFileContent;
}

void CUSBInfoFileManager::GetHardWareInfo(const TiXmlNode* pHardWare, USBKeyInfo& usbinfo)
{
	if(!pHardWare)
		return;
	//���VID
	string strVid = pHardWare->ToElement()->Attribute("VID");
	USHORT vid = strtol(strVid.c_str(), NULL, 16);
	usbinfo.hardware.eig.VendorID = vid;

	//���PID
	string strPid = pHardWare->ToElement()->Attribute("PID");
	USHORT pid = strtol(strPid.c_str(), NULL, 16);
	usbinfo.hardware.eig.ProductID = pid;
	// ���SN
	usbinfo.hardware.eig.SerierlNumber = pHardWare->ToElement()->Attribute("SN");

	usbinfo.hardware.version = AToW(pHardWare->ToElement()->Attribute("Version"));

	string strbNeedInstall = pHardWare->ToElement()->Attribute("beNeedInstall");
	if(strbNeedInstall == "true")
		usbinfo.hardware.bNeedInstall = true;
	else
		usbinfo.hardware.bNeedInstall = false;

	string strbNeedForbidRun = pHardWare->ToElement()->Attribute("beNeedForbidRun");
	if(strbNeedForbidRun == "true")
		usbinfo.hardware.bNeedForbidRun = true;
	else
		usbinfo.hardware.bNeedForbidRun = false;
	// ���
	if(pHardWare->ToElement()->Attribute("VolumnInfo") != NULL)
		usbinfo.hardware.volumnname = pHardWare->ToElement()->Attribute("VolumnInfo");
	// ���mid�����Ϣ
	const TiXmlNode* pMid = pHardWare->FirstChild("MID");
	if(NULL != pMid)
	{
		string strType = pMid->ToElement()->Attribute("type");
		usbinfo.hardware.eig.hasMid = true;
		if(strType == "number")
		{
			string midinfo = pMid->ToElement()->Attribute("value");
			usbinfo.hardware.eig.m_midDword = strtol(midinfo.c_str(),NULL,16);

			usbinfo.hardware.eig.midDLLName = AToW(pMid->FirstChild("DLLName")->FirstChild()->Value());
			usbinfo.hardware.eig.midFucName = AToW(pMid->FirstChild("Function")->FirstChild()->Value());
		}
		else if(strType == "string")
		{
			//ȡ���˸ò���
		}
		else
			usbinfo.hardware.eig.hasMid = false;

	}
	else//���û�ж�������Ϣ����ôĬ��û��mid
	{
		usbinfo.hardware.eig.hasMid = false;
	}

	const TiXmlNode* pManufacturer = pHardWare->FirstChild("Manufacturer");
	if(NULL != pManufacturer)
	{
		usbinfo.hardware.manufacturer = AToW(pManufacturer->FirstChild()->Value());
	}

	const TiXmlNode* pGoodsName = pHardWare->FirstChild("GoodsName");
	if(NULL != pGoodsName)
	{
		usbinfo.hardware.goodsname = AToW(pGoodsName->FirstChild()->Value());
	}

	const TiXmlNode* pModel = pHardWare->FirstChild("Model");
	if(NULL != pModel)
	{
		//����ڲ���ź�ʵ������
		usbinfo.hardware.inmodel = AToW(pModel->ToElement()->Attribute("inid"));
		usbinfo.hardware.model = AToW(pModel->FirstChild()->Value());
	}

	const TiXmlNode* pFinance = pHardWare->FirstChild("Finance");
	if(NULL != pFinance)
	{
		usbinfo.hardware.financeid = pFinance->ToElement()->Attribute("id");
		assert(usbinfo.hardware.financeid.size() > 0);
		usbinfo.hardware.finaninstitution = AToW(pFinance->FirstChild()->Value());
	}
}

void CUSBInfoFileManager::SplitFileContent(const CStringA& strContent,CCloudFileSelector& cselector)
{
	USES_CONVERSION;
	int curPos = 0;
	CStringA resToken = strContent;
	resToken.Trim();

	if (!resToken.IsEmpty() && resToken.GetAt(0) != ';')
	{
		int nPoundKey = resToken.Find(';');
		if (nPoundKey != -1)
			resToken = resToken.Mid(0, nPoundKey);

		resToken.Replace('/', '\\');

		// (1) Java
		if (resToken.GetAt(0) == '@')
		{
			wstring file = CA2W(resToken.Mid(1));
			cselector.AddWhiteList(1,file);
		}
		// (2) Win7/Vista
		else if (resToken.GetAt(0) == '#')
		{
			wstring file = CA2W(resToken.Mid(1));
			cselector.AddWhiteList(2,file);
		}

		// (3) ����IE����
		else if (resToken.GetAt(0) == '$')
		{
			wstring file = CA2W(resToken.Mid(1));
			cselector.AddWhiteList(3,file);
		}

		else if (resToken.GetAt(0) == '*')
		{
			CStringA restr = resToken.Mid(1);
			int cPos = 0;
			//��õ�ǰ��reg
			wstring reg =  CA2W(restr.Tokenize("+", cPos));
			wstring key = CA2W(restr.Tokenize("+", cPos));
			wstring type = CA2W(restr.Tokenize("+", cPos));
			if(type == L"1")
				cselector.AddRegFolder(reg,key,1);
			else if(type == L"2")
				cselector.AddRegFolder(reg,key,2);
		}

		else if (resToken.GetAt(0) == '&')
		{
			CStringA restr = resToken.Mid(1);
			int cPos = 0;
			//��õ�ǰ��reg
			wstring reg =  CA2W(restr.Tokenize("+", cPos));
			wstring key = CA2W(restr.Tokenize("+", cPos));
			wstring type = CA2W(restr.Tokenize("+", cPos));
			wstring file = CA2W(restr.Tokenize("+",cPos));
			if(type == L"1")
				cselector.AddRegFile(reg,key,file,1);
			else if(type == L"2")
				cselector.AddRegFile(reg,key,file,2);
			else if(type == L"3")
				cselector.AddRegFile(reg,key,file,3);
		}

		else if ( resToken.GetAt(0) == '!')
		{
			wstring file = CA2W(resToken.Mid(1));

			WCHAR expName[MAX_PATH] ={0};
			ExpandEnvironmentStringsW(file.c_str(), expName, MAX_PATH);
			wstring wtp(expName);
			cselector.AddFolder(expName);
		}

		else if (resToken.GetAt(0) == '^')
		{
			CStringA restr = resToken.Mid(1);
			int cPos = 0;
			//��õ�ǰ��reg
			wstring folder =  CA2W(restr.Tokenize("+", cPos));
			wstring externsion = CA2W(restr.Tokenize("+", cPos));
			cselector.AddExtensionsFile(folder,externsion);					
		}
	}
}

bool CUSBInfoFileManager::GetSoftWareInfo(const TiXmlNode* pSoftWare, USBKeyInfo& usbinfo)
{		
	usbinfo.bSupport = false;//�����ڸû�����ϵͳ�в�֧�ָ�usb����İ�װ
	if(!pSoftWare)
		return false;
	
	wstring strOsType =  AToW(pSoftWare->ToElement()->Attribute("osType"));
	string strOsArchi = pSoftWare->ToElement()->Attribute("osArchi");
	string bNeedUpdate = pSoftWare->ToElement()->Attribute("NeedUpdate");
	string bNeedReboot = pSoftWare->ToElement()->Attribute("NeedReboot");

	map<wstring,SystemType>::iterator itr;
	itr = m_sysTypeList.find(strOsType);
	if(itr != m_sysTypeList.end())
	{
		if((*itr).second == m_tSystem || (*itr).second == sAll)//���ϵ�ǰϵͳ����,����ϵͳ���ͽ��а�װ
		{
			int ncpu = 32;
			if(strOsArchi == "0")
				ncpu = m_tArchite;
			else if(strOsArchi == "64")
				ncpu = 64;
			if(ncpu == m_tArchite)
			{
				// �ҵ������ļ�������汾
				usbinfo.bSupport = true;
				if(bNeedUpdate == "false")
					usbinfo.software.bNeedUpdate = false;
				else
					usbinfo.software.bNeedUpdate = true;

				if(bNeedReboot == "true")
					usbinfo.software.bNeedReboot = true;
				else
					usbinfo.software.bNeedReboot = false;

				const TiXmlNode* pUrl = pSoftWare->FirstChild("url");
				if(NULL != pUrl)
				{
					if (NULL == pUrl->FirstChild())
						::MessageBox (NULL, L"url��һ����ֵ", L"��ʾ", MB_OK | MB_ICONINFORMATION);
					else
						usbinfo.software.url = AToW(pUrl->FirstChild()->Value());
				}		

				const TiXmlNode* pbUrl = pSoftWare->FirstChild("backurl");
				if(NULL != pbUrl)
				{
					if (NULL == pbUrl->FirstChild())
						::MessageBox (NULL, L"backurl��һ����ֵ", L"��ʾ", MB_OK | MB_ICONINFORMATION);
					else
						usbinfo.software.backurl = AToW(pbUrl->FirstChild()->Value());
				}

				const TiXmlNode* pVer = pSoftWare->FirstChild("version");
				if(NULL != pVer)
				{
					if (NULL == pVer->FirstChild())
						::MessageBox (NULL, L"version��һ����ֵ", L"��ʾ", MB_OK | MB_ICONINFORMATION);
					else
						usbinfo.software.version = strtol(pVer->FirstChild()->Value(), NULL, 10);
				}

				const TiXmlNode* pafile = pSoftWare->FirstChild("file");
				if(NULL != pafile)
				{
					CCloudFileSelector cselector;//�����﷨��ͬ������ֱ�ӵ���cloud�еĽӿ�

					for(const TiXmlNode* pfile = pafile->FirstChild("name"); pfile != NULL ; pfile = pafile->IterateChildren("name", pfile))
					{
						if (NULL == pfile->FirstChild())
						{
							::MessageBox (NULL, L"file�е�name��һ����ֵ", L"��ʾ", MB_OK | MB_ICONINFORMATION);
							continue;
						}

						wstring strfile= AToW(pfile->FirstChild()->Value());
						CStringA strFname(WToA(strfile , CP_Ansi).c_str());					

						SplitFileContent(strFname,cselector);

					}
					set<wstring> *pfiles =  cselector.GetFiles();

					set<wstring>::iterator ite = pfiles->begin();
					for(; ite != pfiles->end(); ite ++)
					{
						usbinfo.software.files.push_back((*ite));
					}
				}

				const TiXmlNode* pkfiles = pSoftWare->FirstChild("keyfile");
				if(NULL != pkfiles)
				{
					std::wstring name;
					ProgramType ptype;
					for(const TiXmlNode* pkfile = pkfiles->FirstChild("name"); pkfile != NULL ; pkfile = pkfiles->IterateChildren("name", pkfile))
					{
						if (NULL == pkfile->FirstChild())
						{
							::MessageBox (NULL, L"keyfile�е�name��һ����ֵ", L"��ʾ", MB_OK | MB_ICONINFORMATION);
							continue;
						}
						name = AToW(pkfile->FirstChild()->Value());
						string type= pkfile->ToElement()->Attribute("type");
						if(type == "service")
						{
							ptype = pService;

						}
						else if(type == "driver")
						{
							ptype = pDriver;
						}
						else
						{
							CCloudFileSelector cselector;

							CStringA strFname(WToA(name , CP_Ansi).c_str());					

							SplitFileContent(strFname,cselector);

							set<wstring> *pfiles =  cselector.GetFiles();			
							set<wstring>::iterator ite = pfiles->begin();
							if(ite != pfiles->end())
							{
								name = (*ite);
							}
							ptype = pNormal;
						}
						
						usbinfo.software.programinfo.insert(make_pair(name, ptype));
					}

				}

				const TiXmlNode* pRegInfos = pSoftWare->FirstChild("reginfo");
				if(NULL != pRegInfos)
				{
					for(const TiXmlNode* pRegInfo = pRegInfos->FirstChild("name"); pRegInfo != NULL ; pRegInfo = pRegInfos->IterateChildren("name", pRegInfo))
					{
						if (NULL == pRegInfo->FirstChild())
						{
							::MessageBox (NULL, L"reginfo�е�name��һ����ֵ", L"��ʾ", MB_OK | MB_ICONINFORMATION);
							continue;
						}

						wstring name = AToW(pRegInfo->FirstChild()->Value());
						GetRegInfo(name, usbinfo);
					}
				}
			}
		}
	}
	return false;
}


bool CUSBInfoFileManager::InitUSBManegerList(list<USBKeyInfo>& usbinfo)
{
	// ��ȡ��ǰ��ϵͳ�汾��Ϣ
	GetSystemVersion();
	CUSBCheckor::GetInstance()->SetSystem(m_tSystem, m_tArchite);

	wchar_t lpPath[255] = {0};
	GetModuleFileNameW(NULL,lpPath,sizeof(lpPath));
	::PathRemoveFileSpecW(lpPath);

	wstring path(lpPath);

	path += USB_INFO_FILE_NAME;

	string info = GetFileContent(path.c_str(),true);
	if(info == "")
		return false;

	TiXmlDocument xmlDoc;

	xmlDoc.Parse(info.c_str()); 

	if (xmlDoc.Error())
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_USBINIT, L"USBInfoFile TiXmlDocument Parse Error");
		return false;
	}

	const TiXmlNode* pRoot = xmlDoc.FirstChild("main"); // ANSI string 
	if (NULL == pRoot)
		return false;

	// ���usb��Ϣ
	for (const TiXmlNode *pUSB = pRoot->FirstChild("usbinfo"); pUSB != NULL; pUSB = pRoot->IterateChildren("usbinfo", pUSB))
	{
		USBKeyInfo susbinfo;
		const TiXmlNode* pHardWare = pUSB->FirstChild("Hardware");
		if (NULL == pHardWare)
			continue;
		
		GetHardWareInfo(pHardWare, susbinfo);

		for(const TiXmlNode *pSoftWare = pUSB->FirstChild("Software"); pSoftWare != NULL; pSoftWare = pUSB->IterateChildren("Software", pSoftWare))
		{
			// �Ѿ��ҵ������ڱ����ϵ������Ϣ���Ͳ��ٽ��в����ˣ���������һ��usb��Ϣ
			if(GetSoftWareInfo(pSoftWare, susbinfo))
				break;
		}

		usbinfo.push_back(susbinfo);		
	}


	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_USBINIT, L"��ȡUSBInfoFile����");

	return true;
}

void CUSBInfoFileManager::GetSystemVersion()
{ 
	//��ʼ��ϵͳ��Ӧ����
	m_sysTypeList.insert(make_pair(L"All",sAll));
	m_sysTypeList.insert(make_pair(L"UnSupported",sUnSupported));
	m_sysTypeList.insert(make_pair(L"WinNT",sWinNT));
	m_sysTypeList.insert(make_pair(L"Win2000",sWin2000));
	m_sysTypeList.insert(make_pair(L"WinXP",sWinXP));
	m_sysTypeList.insert(make_pair(L"Win2003",sWin2003));
	m_sysTypeList.insert(make_pair(L"WinVista",sWinVista));
	m_sysTypeList.insert(make_pair(L"Win2008",sWin2008));
	m_sysTypeList.insert(make_pair(L"Win7",sWin7));

	m_tArchite = 32;

	SYSTEM_INFO info;                                   //��SYSTEM_INFO�ṹ�ж�64λAMD������ 
    GetSystemInfo(&info);                               //����GetSystemInfo�������ṹ 
    OSVERSIONINFOEXA os; 
    os.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEXA);  /*�ڵ��ú���ǰ������sizeof(OSVERSIONINFOEXA)���dwOSVersionInfoSize�ṹ��Ա*/ 
    if(GetVersionExA((OSVERSIONINFOA *)&os))                  /*����GetVersionEx����OSVERSIONINFOEX�ṹ���뽫ָ������ǿ��ת��*/
    { 
        //������ݰ汾��Ϣ�жϲ���ϵͳ���� 
        switch(os.dwMajorVersion){                        //�ж����汾�� 
            case 4: 
                switch(os.dwMinorVersion){                //�жϴΰ汾�� 
                    case 0: 
                        if(os.dwPlatformId==VER_PLATFORM_WIN32_NT) 
                            m_tSystem = sWinNT;                //1996��7�·��� 
                        else if(os.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) 
                            m_tSystem = sUnSupported; 
                        break; 
                    case 10: 
                        m_tSystem = sUnSupported; 
                        break; 
                    case 90: 
                        m_tSystem = sUnSupported;
                        break; 
                } 
                break; 
            case 5: 
                switch(os.dwMinorVersion){               //�ٱȽ�dwMinorVersion��ֵ 
                    case 0: 
						m_tSystem = sWin2000;                  //1999��12�·��� 
                        break; 
                    case 1: 
						m_tSystem = sWinXP;                  //2001��8�·��� 
                        break; 
                    case 2: 
                        if(os.wProductType==VER_NT_WORKSTATION && 
                           info.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
						{
							m_tSystem = sWinXP; 
						}
                        else if(GetSystemMetrics(SM_SERVERR2)==0) 
							m_tSystem = sWin2003;                                   //2003��3�·��� 
                        else if(GetSystemMetrics(SM_SERVERR2)!=0) 
							m_tSystem = sWin2003;
                        break; 
                } 
                break; 
            case 6: 
                switch(os.dwMinorVersion){ 
                    case 0: 
                        if(os.wProductType==VER_NT_WORKSTATION)/*
                        VER_NT_WORKSTATION������ϵͳ */
							m_tSystem = sWinVista;
                        else
							m_tSystem = sWin2008;                            
                        break; 
                    case 1: 
                        if(os.wProductType==VER_NT_WORKSTATION) 
							m_tSystem = sWin7;
                        else 
							m_tSystem = sWin2008; 
                        break; 
                }
                break; 
            default: 
                m_tSystem = sUnSupported; 
        } 
    }

	BOOL bIsWow64 = FALSE;
	::IsWow64Process(GetCurrentProcess(),&bIsWow64);
	if(bIsWow64 == TRUE)
		m_tArchite = 64;

	return;
}

void CUSBInfoFileManager::GetRegInfo(wstring& szData, USBKeyInfo& usbinfo)
{
	wstring key,valuename;
	HKEY rootkey;
	unsigned char* m_value = NULL;
	wstring strLine = szData;
	DWORD iDataLength = 0;

	if (strLine[0] == '[')
	{
		// ��һ����key������
		size_t nEndKeyName = strLine.find_first_of(']', 1);
		std::wstring strKeyName = strLine.substr(1, nEndKeyName - 1);

		size_t nEnd = strKeyName.find_first_of('\\');
		std::wstring strRootKey = strKeyName.substr(0, nEnd);

		if(strRootKey == L"HKEY_CURRENT_USER")
			rootkey = HKEY_CURRENT_USER;
		else if (strRootKey == _T("HKEY_LOCAL_MACHINE"))
			rootkey = HKEY_LOCAL_MACHINE;

		key = strKeyName.substr(nEnd + 1, strKeyName.size());

		RegInfo reg;
		reg.rootkey = rootkey;
		reg.subkey = key;
		usbinfo.software.reginfo.push_back(reg);
	}
	
}