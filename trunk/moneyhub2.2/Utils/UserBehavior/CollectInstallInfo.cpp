/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  CollectInstallInfo.cpp
 *      ˵����  �ռ���װ������з�����Ϣ�����ʵ���ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.10.27	������	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */

#include "stdafx.h"
#include "CollectInstallInfo.h"
#include "shlwapi.h"
#include "../sn/SNManager.h"
#pragma comment(lib,"shlwapi.lib")
#include <comdef.h>
#include <Wbemidl.h>
# pragma comment(lib, "wbemuuid.lib")

#include "windows.h"
#pragma comment(lib,"Kernel32.lib")
/*
<?xml version="1.0" encoding="UTF-8"?>
<data>
	<terminalid>yxTKCfGPO2OxBQ89hEc%2F%2BE5mqqRS6MWT</terminalid>
	<action>install</action>
	<systemname>Windows Xp</systemname>
	<systemversion>sp3 1.1.1.1</systemversion>
	<systemlang>CHN</systemlang>
	<ieversion>ie8</ieversion>
	<antivirusname>360��ȫ��ʿ</antivirusname>
	<cpuname>Intel Core 2 Duo</cpuname>
	<productdmi>Dell inc</productdmi>
	<productname>optiplex 780</productname>
	<memorysize>4GB</memorysize>
	<moneyhubversion>2.1.1.1</moneyhubversion>
	<beforeversion>2.0.0.0</beforeversion>
	<afterversion>2.1.1.1</afterversion>
	<starttype>desk_start</starttype>
	<visitedurl>http://www.moneyhub.com/boc.php</visitedurl>
	<errorinfo>���������������󡣡�����������</errorinfo>
</data>
*/


CCollectInstallInfo::CCollectInstallInfo(void)
{
	m_sXmlInfo = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><data><action>install</action>";
}
CCollectInstallInfo::~CCollectInstallInfo(void)
{
}
// ��ð�װʱ��������Ϣ
string CCollectInstallInfo::GetInstallInfo()
{
	GetTerminal_id();
	GetSystem_nameAversion();
	GetSystem_lang();
	GetIe_version();
	GetAntivirus_name();

	GetCpu_name();
	GetProduct_dmiAname();
	GetMemory_size();

	m_sXmlInfo += "</data>";
	return m_sXmlInfo;
}
// ���Ӳ��ID�ĺ�����Ӳ��idΪ��˾���Զ��壬�ο������ĵ���
void CCollectInstallInfo::GetTerminal_id()
{
	m_sXmlInfo += "<terminalid>" + GenHWID2() + "</terminalid>";
	m_sXmlInfo += "<SN>" + CSNManager::GetInstance()->GetSN() +"</SN>";
	return;
}
// ���ϵͳ���Ƽ��汾�ĺ���
void CCollectInstallInfo::GetSystem_nameAversion()
{       
	string vname;
	string sversion;
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
                            vname = "Microsoft Windows NT 4.0";                //1996��7�·��� 
                        else if(os.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) 
                            vname = "Microsoft Windows 95"; 
                        break; 
                    case 10: 
                        vname = "Microsoft Windows 98"; 
                        break; 
                    case 90: 
                        vname = "Microsoft Windows Me"; 
                        break; 
                } 
                break; 
            case 5: 
                switch(os.dwMinorVersion){               //�ٱȽ�dwMinorVersion��ֵ 
                    case 0: 
                        vname = "Microsoft Windows 2000";                    //1999��12�·��� 
                        break; 
                    case 1: 
                        vname = "Microsoft Windows XP";                    //2001��8�·��� 
                        break; 
                    case 2: 
                        if(os.wProductType==VER_NT_WORKSTATION)
                            vname = "Microsoft Windows XP Professional"; 
                        else if(GetSystemMetrics(SM_SERVERR2)==0) 
                            vname = "Microsoft Windows Server 2003";        //2003��3�·��� 
                        else if(GetSystemMetrics(SM_SERVERR2)!=0) 
                            vname = "Microsoft Windows Server 2003 R2"; 
                        break; 
                } 
                break; 
            case 6: 
                switch(os.dwMinorVersion){ 
                    case 0: 
                        if(os.wProductType==VER_NT_WORKSTATION)/*
                        VER_NT_WORKSTATION������ϵͳ */
                            vname = "Microsoft Windows Vista"; 
                        else 
                            vname = "Microsoft Windows Server 2008";          //�������汾 
                        break; 
                    case 1: 
                        if(os.wProductType==VER_NT_WORKSTATION) 
                            vname = "Microsoft Windows 7"; 
                        else 
                            vname = "Microsoft Windows Server 2008 R2"; 
                        break; 
                }
                break; 
            default: 
                vname = "δ֪����ϵͳ"; 
        } 
    }

	BOOL bIsWow64 = FALSE;
	::IsWow64Process(GetCurrentProcess(),&bIsWow64);
	if(bIsWow64 == TRUE)
		vname += "64-bit";
	else
		vname += "32-bit";

	char buf[100] = {0};
    sprintf_s(buf,100,"%s %d.%d.%d.%d",os.szCSDVersion,os.dwMajorVersion,os.dwMinorVersion,os.dwBuildNumber,os.wServicePackMajor<< 0x10 | os.wServicePackMinor);

	sversion = buf;

	m_sXmlInfo += "<systemname>" + vname + "</systemname>";
	m_sXmlInfo += "<systemversion>" + sversion + "</systemversion>";

	return;
}
// ���ϵͳ����
void CCollectInstallInfo::GetSystem_lang()
{
	char buf[10] = {0};
    sprintf_s(buf,10,"%d",GetSystemDefaultLangID());

	string slang = buf;
	m_sXmlInfo += "<systemlang>" + slang + "</systemlang>";
}
// ���IE�汾
void CCollectInstallInfo::GetIe_version()
{
	DWORD dwValueNameLength = 100;        // ֵ���ַ�������   
    char ptszValueName[100] = "";    // ֵ���ַ���

	//���ie�汾��Ϣ
	DWORD dwType, dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;
	if( ERROR_SUCCESS == ::SHGetValueA(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Internet Explorer", "Version", &dwType, &ptszValueName, &dwValueNameLength))
	{
		string ieversion = ptszValueName;
		m_sXmlInfo += "<ieversion>" + ieversion + "</ieversion>";
	}
}
// ���ɱ���������
void CCollectInstallInfo::GetAntivirus_name()
{
	wstring aname;

    HRESULT hres;
    // Initialize COM. ------------------------------------------
    hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if (FAILED(hres))
        return;                  // Program has failed.

	// Step 2: --------------------------------------------------
    hres =  CoInitializeSecurity(NULL,-1,                          // COM authentication
        NULL,NULL,RPC_C_AUTHN_LEVEL_DEFAULT,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE,                   // Additional capabilities 
        NULL);
                      
    if (FAILED(hres))
    {
        CoUninitialize();
        return;                    // Program has failed.
    }
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------
    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,IID_IWbemLocator, (LPVOID *) &pLoc);
    if (FAILED(hres))
    {
        CoUninitialize();
        return;                 // Program has failed.
    }
    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\SecurityCenter"),  // Object path of WMI namespace
         NULL,NULL,0,NULL,0,0,&pSvc);    
    if (FAILED(hres))
    {
        pLoc->Release();     
        CoUninitialize();
        return;                // Program has failed.
    }

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------
    hres = CoSetProxyBlanket(pSvc,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,NULL,RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE );
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return;               // Program has failed.
    }
    // Step 6: --------------------------------------------------

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),bstr_t("SELECT * FROM AntiVirusProduct"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,&pEnumerator);
    
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;               // Program has failed.
    }
    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------
    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
            &pclsObj, &uReturn);

        if(0 == uReturn)
            break;

        VARIANT vtProp;
        // Get the value of the Name property
        hr = pclsObj->Get(L"displayName", 0, &vtProp, 0, 0);
		aname = vtProp.bstrVal;
        VariantClear(&vtProp);
		if(hr == S_OK)
			pclsObj->Release();
    }
    // Cleanup 

	pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();


    CoUninitialize();

	m_sXmlInfo += "<antivirusname>" + ws2s(aname) + "</antivirusname>";
}
// ���CPU����
void CCollectInstallInfo::GetCpu_name()
{
	CCPUID cpuid;
	m_sXmlInfo += "<cpuname>" + cpuid.GetName() + "</cpuname>";
}
// ��ò�Ʒ�������̺��ͺ�����
void CCollectInstallInfo::GetProduct_dmiAname()
{
	SMBiosData bios;
	bios.FetchSMBiosData();
	SMBios_TYPE1 s1;
	bios.GetData(s1);

	string Product_dmi, Product_name;

	if(s1.szManufacturer)
		 Product_dmi = s1.szManufacturer;
	if(s1.szProductName)
		 Product_name = s1.szProductName;

	m_sXmlInfo += "<productdmi>" + Product_dmi + "M</productdmi>";
	m_sXmlInfo += "<productname>" + Product_name + "M</productname>";
}
// ���ϵͳ�ڴ��С
void CCollectInstallInfo::GetMemory_size()
{
	char buf[30] = {0};
	MEMORYSTATUS memory;
	memory.dwLength = sizeof(memory);   //��ʼ�� 
	GlobalMemoryStatus(&memory);
	int size = int(memory.dwTotalPageFile/1024/1024);

	sprintf_s(buf,30,"%d",size);
	string msize = buf;
	m_sXmlInfo += "<memorysize>" + msize + "M</memorysize>";
}
// ��wstring���ͱ���ת��Ϊstring����
string CCollectInstallInfo::ws2s(const wstring& ws)
{
    string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
    setlocale(LC_ALL, "chs");
    const wchar_t* _Source = ws.c_str();
    size_t _Dsize = 2 * ws.size() + 1;
    char *_Dest = new char[_Dsize];
    memset(_Dest,0,_Dsize);
    //wcstombs(_Dest,_Source,_Dsize);
	size_t _CoverSize = 0;
	wcstombs_s (&_CoverSize, _Dest, _Dsize, _Source, _Dsize); // gao
    string result = _Dest;
    delete []_Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}
