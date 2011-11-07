#include "stdafx.h"
#include "info.h"
#include <list>
#include <map>
#pragma once

using namespace std;


class CUSBHardWareDetector
{
private:
	CUSBHardWareDetector();
	~CUSBHardWareDetector();


	static CUSBHardWareDetector* m_Instance;
public:
	static CUSBHardWareDetector* GetInstance();

	USBKeyInfo* CheckUSBHardWare(USBHardwareInfo& hardinfo);//���hardware�����

	USBKeyInfo* CheckUSBHardWare(int vid, int pid, DWORD mid);//����ʾ�ļ�⣬ֱ�ӻ�ü������core���̼��usb�����Ϣ��״̬

private:
	USBKeyInfo* CheckSupportUSB(USBHardwareInfo& hardinfo); 

	USBKeyInfo* CheckSupportUSB(int vid, int pid, DWORD mid);// ��mid�ļ�⣬ֱ�ӷ��ؽ����core���̼��usb�����Ϣ��״̬

	bool CheckMid(USBEigenvalue& acteig, USBEigenvalue& dataeig);		// ���mid


	//list<string> m_checkedlogicdriver;	//�洢�Ѿ�������U���̷�
	map<int, string> m_logicdriver;	//�ı�֮ǰ��
	DWORD m_allDisk;

	CRITICAL_SECTION m_cs;
	CRITICAL_SECTION m_cs2;
public:
	bool InitLogicDriver();
	bool RecheckLogicDriver();//����U��
	bool CheckLogicDriver(string& vname);//����U��
	static wstring drvName[26];
};
