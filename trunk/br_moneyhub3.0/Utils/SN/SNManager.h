#pragma once
#include <string>
using namespace std;


class CSNManager
{
private:
	CSNManager();
	~CSNManager();

	static CSNManager* m_Instance;
public:
	static CSNManager* GetInstance();

public:
	// ��ע���д��SN������SN�Ĺ���
	void MakeSN();
	// ��ע�����SN�Ĺ��̣���������ڣ����ؿ��ַ���
	string GetSN();

private:
	std::string m_sn;
	
	void MakeCheckCode(char *sn);//����У����

	unsigned short GenCRC16(const char *pdata, int size); 
};