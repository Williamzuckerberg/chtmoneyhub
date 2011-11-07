/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  FileLog.h
 *      ˵����  ������־�������ӿڿ������ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.08.06	������	��ʼ�汾

 *  ����������
 *  Visual Studio 2008+WinDDK\7600.16385.1
 *	 Include��:WinDDK\7600.16385.1\inc\ddk;D:\WinDDK\7600.16385.1\inc\crt;WinDDK\7600.16385.1\inc\api
 *	 Lib��:WinDDK\7600.16385.1\lib\wxp\i386\
 *			hal.lib int64.lib ntoskrnl.lib ntstrsafe.lib
 *
 *-----------------------------------------------------------*
 */
#pragma once
#include <ntddk.h>
#include <ntstrsafe.h>


/**
* ������־�������ӿڿ⣬��Ҫ��������ӿ�
* LogInitialize:��ʼ������
* WriteSysLog:д��־�ӿ�
* LogUninitialize:�����־����
* SetLogLevel:������־�ȼ�
* GetLogLevel:��ȡ��־�ȼ�

* �����ṩ�˺���������:
* GetLocalTime():��õ�ǰϵͳʱ��

* ����ʾ����
* LogInitialize(LOG_TYPE_DEBUG);

* WriteSysLog(LOG_TYPE_DEBUG, "%d%s",12,"test");
* WriteSysLog(LOG_TYPE_DEBUG, "%s","test");

* LogUninitialize();
*/



/**
* ��ʼ���ļ���־��
* @param loglevel:ULONG,��־�ܹ���¼��Ϣ�����ȼ�����ο�LogConst.h�ļ��ڵȼ���
* @param logfilename:WCHAR*,��¼ȫ����·��:������·���������õ����ļ�������ǰ�����ã������ﲻ����������ᵼ�¼���־����
* @param filesize:LONGLONG,����һ����־�ļ�����󳤶ȡ�
* @param strategy:USHORT,��־���ԣ�1Ϊ���ļ���2Ϊ˫�ļ�������¼�����Ϊ���ļ�����ô��¼����־�����������û��������Ϊ˫�ļ�����ô��־�ļ������Զ�+00��01��
* ��ʹ�ø���־ģ��ʱ��д��־֮ǰҪ���øú������г�ʼ�������м��κεط������Զ���־�ļ�����д����־��Ϣ����д����־�󣬲��ٵ��øú���д��־����ôҪ����LogUninitialize��������
*/

void LogInitialize(ULONG loglevel,WCHAR* logfilename = L"\\Device\\HarddiskVolume1\\MoneyHubDriver.log",LONGLONG filesize = 10000000,USHORT strategy = 1);
/**
* д��־����
* @param iLevel:ULONG,Ҫд����־��Ϣ�ĵȼ�����ο�LogConst.h�ļ��ڵȼ���
* @param format:NTSTRSAFE_PSTR*,������Ϣ�ĸ�ʽ���˸�ʽ�ο�printf��
* @param ...:���,�ο�printf��Ҫд�����Ϣ��
* д��־��Ϣ�ӿڣ�д��־��Ϣ��С�벻Ҫ�����涨���ȵĴ�С�������ʱΪ256�ֽڣ���
*/
void _cdecl WriteSysLog(ULONG iLevel, NTSTRSAFE_PSTR format,...);

/**
* �����־����
* �޲���
*/
void LogUninitialize();

/**
* ����ϵͳ����־��¼�ȼ�
* @param loglevel:ULONG,��־�ܹ���¼��Ϣ�����ȼ���
*/
void SetLogLevel(ULONG loglevel);

/**
* ���ϵͳ����־��¼�ȼ�
* @return ULONG ���������ܹ���¼��Ϣ�����ȼ���
*/
ULONG GetLogLevel();

/**
* ��ȡ��ǰϵͳʱ��
* @return TIME_FIELDS ���ص�ǰϵͳ��ʱ�䡣
*/
TIME_FIELDS GetLocalTime();


//   ����Ϊ������˽�к������ⲿ���ܵ��ã���

/**
* ��ȡ��ǰ�ļ�����
* @param pfactfilename:PUNICODE_STRING,��Ҫ��ǰ���壬�ڱ������ڽ����˿ռ����룬�������ɵ������ͷţ�����
* @return USHORT ����ת���Ľ��:1��ʾ�ɹ���0��ʾʧ�ܡ�
* ����g_CurrentFileNumber��g_logFile��ͬ�������µ��ļ�����
*/
USHORT GetLogFileName(OUT PUNICODE_STRING pfactfilename);

/**
* ����ļ���С������������ļ������涨��С���´����ļ�
* @param pRecreateFilename:PUNICODE_STRING,���ļ���С�����涨֮��Ҫ�õ��ļ��������´������ļ�����
* @param strategy:USHORT �ؽ��ķ��룬1ΪRepalce��2ΪOpen��
* @return NTSTATUS ִ�иú����Ľ�����ɹ�����STATUS_SUCCESS
* ������Ҫ�����ؽ������ѡ������־ϵͳ��ʼ��ʱһ�����1��2����д����־�ļ����жϴ�С����1.
*/
NTSTATUS CheckLogFileSizeAndReCreateLogFile(PUNICODE_STRING pRecreateFilename,USHORT strategy = 1);

