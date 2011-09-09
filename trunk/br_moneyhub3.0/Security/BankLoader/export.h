#pragma once

#include "atlstr.h"
#include "windows.h"

#include "..\.\common\DriverDefine.h"

namespace BankLoader
{
	bool LoadProcess(const CString& cmdline, HANDLE& hProcess, DWORD& PID);
	bool IsFinished();

	bool InstallAndStartDriver();
	bool UnInstallDriver();

	//add by bh 8 9
	bool setSecuModuleBR();//���Ͱ�����

	bool SendBlackListToDriver();//���ͺ�����

	int returnDriverStatus(MYDRIVERSTATUS type);

	bool  checkHook();

	bool SendReferenceEvent(HANDLE& ev);//evΪ���������Ϊ����֪ͨӦ�ó����л������ļ�

	bool GetGrayFile(void *pFileData, DWORD size);

	bool CheckDriver();//����Ϊpop����ļ������״̬�ĺ���

	bool SendProtectId(UINT32 id);
	bool SetKernelContinue(bool b);
};