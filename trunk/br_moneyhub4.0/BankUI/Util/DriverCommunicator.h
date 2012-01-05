#pragma once


#include "SecurityCheck.h"

enum REGSTATUS
{
	CD_ALLREADYEXISTING = 0,
	CD_SUCESSFUL,
	CD_ERROR,
};


class CDriverCommunicator
{
public:
	CDriverCommunicator(void);
	~CDriverCommunicator(void);

private:
	/**
	*�õ���ǰ·�����������в���
	*/
	void getCurrentPath(LPCWSTR lPath)   const;
public:
	/**
	*��������Ϣ������������ͨѶǰ��׼������
	*/
	void communicationDriver();

	/**
	*ͨ���ص�ʵ�ֶ��߳�ͬ��
	*/
	static void voidFun(CheckStateEvent ev, DWORD dw, LPCTSTR lpszInfo, LPVOID lp);
	/**
	*�ж��Ƿ��ѽ����������͸�����
	*/
	REGSTATUS      isSendData()       const;

	bool      isSendDataWithDriver() const;
	/**
	*�����û�̬������OS����
	*/
	REGSTATUS      setAutoRun()       const;

	bool           deleteAutoRun()    const;

	/**
	*������ͨ�ţ����ݰ�����
	*/
	void sendData();
	void SendBlackList();

	static bool m_isOk;
	/*
	*�Զ��ر���ʾ����
	*/
	void show();

	/*
	*��Ȩ����
	*/
	bool runWithUAC(BYTE index)          const;
};
