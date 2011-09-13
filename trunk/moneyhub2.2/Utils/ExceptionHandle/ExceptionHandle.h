#pragma once
#include <Windows.h>
using namespace std;

const int nThread_Name_Len = 100;
class CExceptionHandle
{
public :
	static void MapSEtoCE();

	operator DWORD();

	bool RecordException(); // ��¼�쳣

	void SetThreadName(const char* pName); // �����̵߳�����

private:

	bool TranslateExceptionAndSend(std::string& strErr); // ���쳣ת��������
	CExceptionHandle(PEXCEPTION_POINTERS pep);

	static void __cdecl TranslateSEtoCE(UINT dwEC,PEXCEPTION_POINTERS pep);

private:
	static bool ms_bIsSetTanslator;
	EXCEPTION_RECORD m_er;
	CONTEXT m_context;
	char m_pThreadName[nThread_Name_Len];
};