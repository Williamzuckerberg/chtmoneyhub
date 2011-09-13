/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  AxHookManager.h
 *      ˵����  �ؼ����ӹ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.10.22	���ź�ͨ	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */

#pragma once

class CAxHookManager
{

public:

	CAxHookManager();

	static void Initialize();
	static CAxHookManager* Get();
	void Hook();
	void Unhook();

private:

	static LRESULT CALLBACK AxWndProc(int iCode, WPARAM wParam, LPARAM lParam);
	static DWORD sm_dwTLSIndex;
	HHOOK m_hAxHook;
};
