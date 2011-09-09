#pragma once
#include <WTypes.h>

class CServiceControl
{
public:
	
	//���÷��������Լ���������
	CServiceControl(LPCTSTR lpServiceName,LPCTSTR lpServieDescription);
	virtual ~CServiceControl(void);

public:
	// ��װ����
	bool Install(void);
	// ��������
	bool Start(void);
	// ֹͣ����
	bool Stop(void);
	// ж�ط���
	bool UnInstall(void);
	// ��ѯ����״̬
	DWORD QueryStatus(void);


private:
	// �ж��Ƿ�װ�˷���
	TCHAR m_tzServiceName[MAX_PATH];
	TCHAR m_tzLogPathName[MAX_PATH];
	bool IsInstall(void);
private:
	TCHAR m_tzServieDescription[MAX_PATH];
};
