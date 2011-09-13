#pragma once

#include "atlstr.h"

namespace BankMdrVerifier
{
	/**
	*Init�ǰ�·����ʼ��
	*InitCheck�ǰ����ݳ�ʼ��
	*/
	const bool Init(const char* filename);
	const bool InitCheck(const char * hash,int hashLen);

	bool VerifyModule(CStringW filepath);

	ULONG GetBankModuleNumber();

	bool FillFileHashes(unsigned char* hashes, int maxlen);

	bool CleanUp();
};