#pragma once
/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  HookKeyboard
 *      ˵����  һ�����������̲����ࡣ
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	 ˵��
 *	1.0.0	2010.11.11	���ź�ͨ	��ʼ�汾
 *	
 *-----------------------------------------------------------*
 */

 namespace HOOKKEY
 {
	void addNPB(/*std::wstring wcsName*/);	
	bool installHook();
	bool uninstallHook(bool b = false);
	//���ô�ӡ����
	bool    disablePrintKey();
	bool    EnablePrintKey();
};

extern wchar_t  g_noHookfilterUrl[3000];