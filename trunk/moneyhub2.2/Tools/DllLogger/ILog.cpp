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

ILogger *g_logger = NULL;

/**
* ��ʼ���ļ���־��
* ��ʹ�ø���־ģ��ʱ��д��־֮ǰҪ���øú������г�ʼ�������м��κεط������Զ���־�ļ�����д����־��Ϣ����д����־�󣬲��ٵ��øú���д��־����ôҪ����LogUninitialize��������
*/
void LogInitialize(int level,wchar_t* logdirectory,wchar_t* logfilename,long size,int strategy)
{
	if(strategy == 1)
		g_logger = new COneFileStrategyLogger(logdirectory,logfilename,size);
	else
		g_logger = new CTwoFileStrategyLogger(logdirectory,logfilename,size);

	if(g_logger)
		g_logger -> SetLevel(level);
}

/**
* �����־����
* �޲���
*/
void LogUninitialize()
{
	if(g_logger)
		delete g_logger;
	g_logger = NULL;
}

/**
* д��־����
* д��־��Ϣ�ӿڣ�д��־��Ϣ��С�벻Ҫ�����涨���ȵĴ�С�������ʱΪ256�ֽڣ���
*/
void WriteSysLog(int iLevel, char *format,... )
{ 

	CLogInfo msg;
	msg.type = iLevel;
	GetLocalTime(&msg.time);
	msg.pid = ::GetCurrentProcessId();
	switch(iLevel)
	{
		case LOG_TYPE_ERROR:	msg.typeinfo = "ERROR";break;
		case LOG_TYPE_WARN:		msg.typeinfo = "WARNING";break;
		case LOG_TYPE_INFO:		msg.typeinfo = "INFO";break;
		default:				msg.typeinfo = "DEBUG";break;
	}
    char strTemp[MAX_INFO_LENGTH];
    memset(strTemp, 0, sizeof(strTemp));
    char *pTemp = strTemp;
	//�ϳ���Ϣ
 	va_list args; 
    va_start(args,format); 
    vsprintf(pTemp,format,args); 
    va_end(args); 

	string stemp(strTemp);
	msg.info = stemp;

    if (g_logger) 
    { 
		g_logger->Write(msg);
    } 

}

/**
* ����ϵͳ����־��¼�ȼ�
* @param loglevel:int,��־�ܹ���¼��Ϣ�����ȼ���
*/
void SetLogLevel(int level)
{
	if(g_logger)
	{
		g_logger->SetLevel(level);
	}
}

/**
* ���ϵͳ����־��¼�ȼ�
*/
int GetLogLevel()
{
	if(g_logger)
	{
		return g_logger->GetLevel();
	}
	else
		return -1;
}