/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  LogConst.h
 *      ˵����  ������־�������ؼ��ֶζ����ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.08.06	������	��ʼ�汾

 *  ����������
 *  Visual Studio 2008+WinDDK\7600.16385.1
 *	 Include��:WinDDK\7600.16385.1\inc\ddk;D:\WinDDK\7600.16385.1\inc\crt;WinDDK\7600.16385.1\inc\api
 *	 Lib��:	WinDDK\7600.16385.1\lib\wxp\i386\
 *			hal.lib int64.lib ntoskrnl.lib ntstrsafe.lib
 *
 *-----------------------------------------------------------*
 */
#pragma once
// ���¶���Ϊ��Ϣ�ȼ�
#define LOG_TYPE_NONE		0		//����¼�κ���Ϣ
#define LOG_TYPE_ERROR		1		//������Ϣ�ȼ�
#define LOG_TYPE_WARN		2		//�澯��Ϣ�ȼ�
#define LOG_TYPE_INFO		3		//һ����Ϣ�ȼ�
#define LOG_TYPE_DEBUG		4		//����¼��ϢΪDebug
#define LOG_TYPE_ALL		100		//��¼������Ϣ

#define MAX_INFO_LENGTH		256		//�����Ϣ����