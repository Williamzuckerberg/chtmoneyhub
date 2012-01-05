/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  ILog.h
 *      ˵����  ��־�ӿ������ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.08.06	������	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */
#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#endif
/**
* ��ʼ���ļ���־��
* @param level:int ��־�ܹ���¼��Ϣ�����ȼ�����ο�LogConst.h�ļ��ڵȼ���
* @param logdirectory:wchar_t* ��־ȫ����·��������·���������õ����ļ�������ǰ�����ã������ﲻ����������ᵼ�¼���־����
* @param logfilename:wchar_t* ��־�ļ�����
* @param size:long,����һ����־�ļ�����󳤶ȡ�
* @param strategy:int,��־���ԣ�1Ϊ���ļ���2Ϊ˫�ļ�������¼�����Ϊ���ļ�����ô��¼����־�����������û��������Ϊ˫�ļ�����ô��־�ļ������Զ�+00��01��
* ��ʹ�ø���־ģ��ʱ��д��־֮ǰҪ���øú������г�ʼ�������м��κεط������Զ���־�ļ�����д����־��Ϣ����д����־�󣬲��ٵ��øú���д��־����ôҪ����LogUninitialize��������
*/
LOG_API void LogInitialize(int level,wchar_t* logdirectory = L".\\Logs",wchar_t* logfilename = L"log.d",long size = 10000000,int strategy = 2);

/**
* д��־����
* @param iLevel:int,Ҫд����־��Ϣ�ĵȼ�����ο�LogConst.h�ļ��ڵȼ���
* @param format:wchar_t*,������Ϣ�ĸ�ʽ���˸�ʽ�ο�printf��
* @param ...:���,�ο�printf��Ҫд�����Ϣ��ΪUnicode��ʽ��
* д��־��Ϣ�ӿڣ�д��־��Ϣ��С�벻Ҫ�����涨���ȵĴ�С�������ʱΪ256�ַ�����
*/
LOG_API void WriteSysLog(int iLevel, wchar_t *format,... );

/**
* �����־����
* �޲���
*/
LOG_API void LogUninitialize();

/**
* ����ϵͳ����־��¼�ȼ�
* @param loglevel:int,��־�ܹ���¼��Ϣ�����ȼ���
*/
LOG_API void SetLogLevel(int level);

/**
* ���ϵͳ����־��¼�ȼ�
* @return int ���������ܹ���¼��Ϣ�����ȼ������û�г�ʼ����־����ô����-1
*/
LOG_API int GetLogLevel();
