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
#include "windows.h"
#pragma comment(lib,"Kernel32.lib")


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
	BOOL  bIsWow64 = false;
	::IsWow64Process(GetCurrentProcess(),&bIsWow64);
	return bIsWow64;
}