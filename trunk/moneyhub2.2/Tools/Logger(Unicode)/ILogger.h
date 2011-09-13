/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  ILogger.h
 *      ˵����  ��־����ӿ��������ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.08.06	������	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */

#pragma once
#include "stddef.h"
#include "LogConst.h"
#include "windows.h"
#include <string>
using namespace std;
/**
* ������־�����Ϣ�ӿڲ������õ�����ṹ
* 
*/
class CLogInfo
{
public:
	SYSTEMTIME	time;		//���Ի�ú��뼶��ʱ��
	DWORD		pid;		//��¼����PIDʹ��
	int			type;		//��¼��Ϣ����
	wstring		typeinfo;	//��¼��Ϣ��������
	wstring		info;		//��¼��Ϣ
};

class ILogger
{
public:
	ILogger(void);
	virtual ~ILogger(void);
protected:
	int m_level;			//��¼��ǰ����������־��¼����

public:
	/**
	* д����־��Ϣ������
	* @param CLogInfo& ��Ϣ��ṹ��
	* ����ϵͳ��д����Ϣʱ�����øú���д�롣
	*/
	virtual bool Write(const CLogInfo& info) = 0;

public:
	/**
	* ���ù���������־��¼����
	* @param level ��־����
	* ����ϵͳ�ڵĿɼ�¼�������־��Ϣ����
	*/
	void SetLevel(int level);
	
	/**
	* ��ù���������־��¼����
	* @param level ��־����
	* ���ϵͳ�ڵ���־����
	*/
	int GetLevel();
};
