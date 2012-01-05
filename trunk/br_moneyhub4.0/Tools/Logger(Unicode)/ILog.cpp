/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  ILog.cpp
 *      ˵����  ��־�ӿ�ʵ���ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.08.06	������	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */
#include "ILogger.h"
#include "FileLogger.h"
#include "ILog.h"
#include "LogConst.h"

CMoneyhubLog::CMoneyhubLog()
{
	//��ʼ�������������Դ ֣�� 2011.2.14 16:34 Add Begin
	m_hFileMutex = NULL;
	//��ʼ�������������Դ ֣�� 2011.2.14 16:34 Add End
}

CMoneyhubLog::~CMoneyhubLog()
{
	if(m_logger != NULL)
		LogUninitialize();

	//�ͷŻ����������Դ ֣�� 2011.2.14 16:34 Add Begin
	if (NULL != m_hFileMutex)
	{
		CloseHandle(m_hFileMutex);
		m_hFileMutex = NULL;
	}
	//�ͷŻ����������Դ ֣�� 2011.2.14 16:34 Add End
}
/**
* ��ʼ���ļ���־��
* ��ʹ�ø���־ģ��ʱ��д��־֮ǰҪ���øú������г�ʼ�������м��κεط������Զ���־�ļ�����д����־��Ϣ����д����־�󣬲��ٵ��øú���д��־����ôҪ����LogUninitialize��������
*/
void CMoneyhubLog::LogInitialize(int level,wchar_t* logdirectory,wchar_t* logfilename,long size,int strategy,bool bSynchronize)
{
	if(strategy == 1)
		m_logger = new COneFileStrategyLogger(logdirectory,logfilename,size);
	else
		m_logger = new CTwoFileStrategyLogger(logdirectory,logfilename,size);

	if(m_logger)
	{
		ILogger * p = (ILogger *)m_logger;
		p -> SetLevel(level);
	}

	//���������������Դ ֣�� 2011.2.14 16:34 Add Begin
	if (bSynchronize)
	{
		m_hFileMutex = CreateMutexW(NULL,false,logfilename);
	}
	if (NULL == m_hFileMutex) //�����������ʧ��
	{
		
	}
	//���������������Դ ֣�� 2011.2.14 16:34 Add End
}

/**
* �����־����
* �޲���
*/
void CMoneyhubLog::LogUninitialize()
{
	if(m_logger)
	{
		ILogger * p = (ILogger *)m_logger;
		delete p;
	}
	m_logger = NULL;
}

/**
* д��־����
* д��־��Ϣ�ӿڣ�д��־��Ϣ��С�벻Ҫ�����涨���ȵĴ�С�������ʱΪ256���ַ�����
*/
void CMoneyhubLog::WriteSysLog(int iLevel, wchar_t *format,... )
{ 
	CLogInfo msg;
	msg.type = iLevel;
	GetLocalTime(&msg.time);
	msg.pid = ::GetCurrentProcessId();
	switch(iLevel)
	{
		case LOG_TYPE_ERROR:	msg.typeinfo = L"ERROR";break;
		case LOG_TYPE_WARN:		msg.typeinfo = L"WARNING";break;
		case LOG_TYPE_INFO:		msg.typeinfo = L"INFO";break;
		default:				msg.typeinfo = L"DEBUG";break;
	}
    wchar_t strTemp[MAX_INFO_LENGTH];
    memset(strTemp, 0, sizeof(strTemp));
    wchar_t *pTemp = strTemp;
	//�ϳ���Ϣ
 	va_list args; 
    va_start(args,format); 
    vswprintf(pTemp,MAX_INFO_LENGTH,format,args); 
    va_end(args); 

	wstring stemp(strTemp);
	msg.info = stemp;

    if (m_logger) 
    { 
		ILogger * p = (ILogger *)m_logger;
		//���ݻ���������ж��Ƿ����ͬ�� ֣�� 2011.2.14 17:00 Add Begin
		if (NULL != m_hFileMutex)
		{
			WaitForSingleObject(m_hFileMutex,INFINITE);
		}
		//���ݻ���������ж��Ƿ����ͬ�� ֣�� 2011.2.14 17:00 Add End

		p->Write(msg);

		//�ͷŻ������ ֣�� 2011.2.14 17:00 Add Begin
		if (NULL != m_hFileMutex)
		{
			ReleaseMutex(m_hFileMutex);
		}
		//�ͷŻ������ ֣�� 2011.2.14 17:00 Add End
    } 

}

/**
* ����ϵͳ����־��¼�ȼ�
* @param loglevel:int,��־�ܹ���¼��Ϣ�����ȼ���
*/
void CMoneyhubLog::SetLogLevel(int level)
{
	if(m_logger)
	{
		ILogger * p = (ILogger *)m_logger;
		p -> SetLevel(level);
	}
}

/**
* ���ϵͳ����־��¼�ȼ�
*/
int CMoneyhubLog::GetLogLevel()
{
	if(m_logger)
	{
		ILogger * p = (ILogger *)m_logger;
		return p->GetLevel();
	}
	else
		return -1;
}