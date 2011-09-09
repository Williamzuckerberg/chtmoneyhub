/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  RunLog.h
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
#pragma once
#ifndef MY_OWN_LOG_INCLUDE_DEFINE_tag
#define MY_OWN_LOG_INCLUDE_DEFINE_tag
#endif

#include "ILog.h"

#ifndef OFFICIAL_VERSION
#define LOG_WRITE_LEVEL LOG_TYPE_DEBUG		//���԰���
#define LOG_FILE_SIZE 10000000				//�ļ���СΪ10M
#else
#define LOG_WRITE_LEVEL LOG_TYPE_INFO		//��ʽ����
#define LOG_FILE_SIZE 1000000				//�ļ���СΪ10M
#endif
#define LOG_FILE_NAME L"Run.log"			//��־�ļ����·��
#define LOG_WRITE_STRATEGY 1               //д��־����

class CRunLog
{
public:
	//�õ������һ��ʵ��
	static CRunLog* GetInstance();

	//дlog�ļ�
	CMoneyhubLog* GetLog();
private:
	CRunLog();
	~CRunLog();
private:
	CMoneyhubLog m_MoneyHubLog;
	static CRunLog* m_pInsatance;
};
