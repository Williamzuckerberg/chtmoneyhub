#include "stdafx.h"
#include "info.h"
#pragma once

using namespace std;


class CUSBSoftWareDetector
{
private:
	CUSBSoftWareDetector();
	~CUSBSoftWareDetector();


	static CUSBSoftWareDetector* m_Instance;
public:
	static CUSBSoftWareDetector* GetInstance();

	bool CheckUSBSoftWare(USBSoftwareInfo& softinfo, bool bNeedRepair = false, bool bNeedRestart = false);//��Ⲣ�޸�software���������,���û�а�װ����false����װ�˷���true

	//bool CheckVesion();		// ���汾������汾�͵Ļ�����Ҫ���£���Ҫ�ͷ�����ͨ��
private:
	bool CheckFile(list<wstring>& fname);								// ����ļ��Ƿ������Ϊ��׼
	bool CheckRegInfo(list<RegInfo>& reginfo);							// ���ע����Ƿ����
	bool CheckKeyFile(std::map<std::wstring,ProgramType>& programinfo, bool bNeedRepair, bool bNeedRestart = false);

	bool CheckIsDriverInstalled(const wstring& dname);						//����Ƿ�װ������

	bool CheckServiceIsWork(const wstring& sname, bool bNeedRepair = false);	//������Ҫ����������Ϊ������񱻽�ֹ�ˣ���������

	bool GetPriviledge();

	bool CheckProgramIsRunning(const wstring& pname, bool bNeedRestart = false);							//��Ⲣ�����ؼ�����
};
