#pragma once
#include <set>
#include <string>
using namespace std;

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

public:
	/**
	*��������Ϣ������������ͨѶǰ��׼������
	*/
	void communicationDriver();

	/**
	*�ж��Ƿ��ѽ����������͸�����
	*/
	REGSTATUS      isSendData()       const;

	bool      isSendDataWithDriver() const;


	/**
	*������ͨ�ţ����ݰ�����
	*/
	void sendData();
	void SendBlackList();

	bool CheckDriver();
	// �����¼����������
	bool SendReferenceEvent(HANDLE& ev);

	// ��û�����
	bool GetGrayFile(set<wstring>& file);//��ʱֻ���ļ���������md5

	static bool m_isOk;

	//������ѯ������ں�
	bool SetKernelContinue(bool b);

};
