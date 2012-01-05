/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  ILogger.cpp
 *      ˵����  ��־����ӿ���ʵ���ļ���
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

ILogger::ILogger(void)
{
	m_level = LOG_TYPE_ALL;
}

ILogger::~ILogger(void)
{
}

void ILogger::SetLevel(int level)
{
	m_level = level;
};

int ILogger::GetLevel()
{
	return m_level;
};
