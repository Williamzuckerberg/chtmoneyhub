/**
*-----------------------------------------------------------*
*  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
*    �ļ�����  GetOSInfo.cpp
*      ˵����  ��ȡ����ϵͳ��Ϣ��
*    �汾�ţ�  1.0.0
* 
*  �汾��ʷ��
*	�汾��		����	����	˵��
*	1.0.0	2011.1.12	bh
*-----------------------------------------------------------*
*/


#include "StdAfx.h"
#include "getOSInfo.h"


typedef BOOL  (WINAPI* pIsWow64Process)(
						   __in          HANDLE hProcess,
						   __out         PBOOL Wow64Process
						   );


CGetOSInfo* CGetOSInfo::m_hIns =NULL;

CGetOSInfo* CGetOSInfo::getInstance()
{
	if(!m_hIns)
		m_hIns = new CGetOSInfo();

	return m_hIns;
}

CGetOSInfo::CGetOSInfo()
{

}

CGetOSInfo::~CGetOSInfo()
{

}
/**
*  Ĭ��Ϊ32bits
*/
BOOL CGetOSInfo::isX64()
{
	BOOL  bIsWow = false;
	HMODULE hDll = GetModuleHandleW(L"Kernel32.dll");

	if(!hDll)
		hDll = LoadLibraryW(L"Kernel32.dll");

	if( hDll )
	{
		pIsWow64Process pFun = (pIsWow64Process)GetProcAddress(hDll, "IsWow64Process");
		
		if( pFun )
			pFun(GetCurrentProcess(), &bIsWow);
	}

	return bIsWow;
}