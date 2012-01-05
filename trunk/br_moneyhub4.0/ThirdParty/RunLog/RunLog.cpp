/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  RunLog.cpp
 *      ˵����  ��־�ӿ������ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2011.02.21	֣��	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */

#include "stdafx.h"
#include "RunLog.h"

CRunLog* CRunLog::m_pInsatance = NULL;

CRunLog::CRunLog()
{
	//1����ȡ��������·��
	// ��%%��־��·����չ����һ������·��
	WCHAR szAppDataPath[MAX_PATH + 1];
	ExpandEnvironmentStringsW(L"%APPDATA%\\MoneyHub", szAppDataPath, MAX_PATH);
	::CreateDirectoryW(szAppDataPath, NULL);
	m_MoneyHubLog.LogInitialize(LOG_WRITE_LEVEL,szAppDataPath,LOG_FILE_NAME,LOG_FILE_SIZE, LOG_WRITE_STRATEGY, true);
}

CRunLog::~CRunLog()
{
	m_MoneyHubLog.LogUninitialize();
}

CRunLog* CRunLog::GetInstance()
{	
	if(!m_pInsatance)
		m_pInsatance = new CRunLog();

	return m_pInsatance;
}

CMoneyhubLog* CRunLog::GetLog()
{
	return &m_MoneyHubLog;
}

