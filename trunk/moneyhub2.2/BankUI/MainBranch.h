/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  main.cpp
 *      ˵����  �����̷���ִ�м��������ͷ�ļ�
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
#include "Windows.h"
#include <string>
#include <list>
#include <map>
using namespace std;

class CMainBranch
{
public:
	CMainBranch(void);
	~CMainBranch(void);
private:
	UINT_PTR  m_returnTimer;
public:
	// �ж�MoneyHub�������в��л��������ĺ���
	BOOL	IsAlreadyRunning();
	// �ر���IsAlreadyRunning�п������ں˶���Ҫ�ڵ���IsAlreadyRunning���˳�����ʱ����
	void	CloseHandle();
	// �������������Ȩ�޵ľ��
	bool	CheckToken();
	// ��õ�ǰ������ʾλ��
	bool	GetFramePos(int& nShowWindow,RECT& rcWnd,DWORD& dwMax);
	// ����Ƿ���ʾ��
	void	CheckGuide(HWND& hFrame);	

public:
	// �����ں˽���
	void	RunIECoreProcess(LPCTSTR lpstrCmdLine);
	// ж������
	int		UnInstall();
	// ��װ����
	int		Install();
	// ������ʾ��ҳ
	bool	PopSetPage();
	// ���IE kernel����
	bool	CheckIECoreProcess();
	// �ر�ie���̵ĺ���
	bool	TerminateIECore();

	bool InitManagerList();

	bool RunUAC(LPCTSTR lpstrCmdLine);

	bool Shell(LPCTSTR lpstrCmdLine);

public:
	// �������
	bool	UpdateCheck();
	// ��ȫ���
	bool	SecurityCheck();

	// ��װʱ�ļ��
	int		InstallCheck();

	bool	CheckPop(bool bCehck = true);

	bool	IsPopAlreadyRunning();


public:
	// ���ҹ��õĺ���
	static	VOID CALLBACK CheckHookProc(HWND hwnd , UINT uMsg , UINT_PTR idEvent , DWORD dwTime);
	// ������ʱ���ҹ���ע���ĺ���
	void	StartMonitor();
	// �رռ��
	void	StopMonitor();
private:
	// ��wstringת��Ϊstring���͵ĺ���
	string ws2s(const wstring& ws);

	DWORD GetIEVersion();
};

extern CMainBranch g_AppBranch;
