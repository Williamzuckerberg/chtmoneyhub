#include "stdafx.h"
#include "USBHardWareDetector.h"
#include "USBControl.h"
//#include "../Moneyhub_Agent/Skin/CoolMessageBox.h"
//#include "AliPayTestor.h"
#include <algorithm>
#pragma once

wstring CUSBHardWareDetector::drvName[26] = {L"A:",L"B:",L"C:",L"D:",L"E:",L"F:",L"G:",L"H:",L"I:",L"J:",L"K:",L"L:",L"M:",L"N:",L"O:",
L"P:",L"Q:",L"R:",L"S:",L"T:",L"U:",L"V:",L"W:",L"X:",L"Y:",L"Z:"};

CUSBHardWareDetector* CUSBHardWareDetector::m_Instance = NULL;

CUSBHardWareDetector* CUSBHardWareDetector::GetInstance()
{
	if(m_Instance == NULL)
		m_Instance = new CUSBHardWareDetector();
	return m_Instance;
}

CUSBHardWareDetector::CUSBHardWareDetector()
{
	InitializeCriticalSection(&m_cs);
	InitializeCriticalSection(&m_cs2);
	
}
CUSBHardWareDetector::~CUSBHardWareDetector()
{
	DeleteCriticalSection(&m_cs);
	DeleteCriticalSection(&m_cs2);
}

//���hardware�����
USBKeyInfo* CUSBHardWareDetector::CheckUSBHardWare(USBHardwareInfo& hardinfo)
{
	// ����VID��PID�ҵ�֧�ֵĹؼ�����
	hardinfo.eig.hasMid = false;
	USBKeyInfo* psusb = CheckSupportUSB(hardinfo);
	if(psusb)// �ҵ���֧�ֵ�usb������
	{
		CRecordProgram::GetInstance()->RecordDebugInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"CheckSupportUSB: psusb != NULL");
		if(psusb->bSupport)//����usb�豸�Ƿ�֧�ָ�ϵͳ
		{
			return psusb;
			// ��⵽֧�ָ�usbkey�豸
			// �����Ƿ�
		}
		else
		{			
			std::wstring msg;
			msg = L"��⵽" + (psusb->hardware.finaninstitution) + L"��" + (psusb->hardware.goodsname) + L"�ͺ�USBKEY����������ϵͳ����ʱ��֧��ʹ�ã�����USBKEY�Ƽ�ϵͳʹ�á�";
			MessageBox( NULL, msg.c_str(), L"�ƽ���û���ʾ",MB_OK);
		}

	}
	return NULL;
}

USBKeyInfo* CUSBHardWareDetector::CheckUSBHardWare(int vid, int pid, DWORD mid)//����ʾ�ļ�⣬ֱ�ӻ�ü����
{
	USBKeyInfo* psusb = CheckSupportUSB(vid, pid, mid);
	if(psusb)// �ҵ���֧�ֵ�usb������
	{
		if(psusb->bSupport)//����usb�豸�Ƿ�֧�ָ�ϵͳ
		{
			return psusb;
		}

	}
	return NULL;
}

USBKeyInfo* CUSBHardWareDetector::CheckSupportUSB(int vid, int pid, DWORD mid)
{
	list<USBKeyInfo>* pUsb = CUSBControl::GetInstance()->GetUSBInfo();
	list<USBKeyInfo>::iterator ite = pUsb->begin();
	for(;ite != pUsb->end(); ite ++)
	{
		if((pid == (*ite).hardware.eig.ProductID) && (vid == (*ite).hardware.eig.VendorID))//���VID��PID�������ͬ
		{
			if(mid == 0)
			{
				return (&(*ite));
			}
			else
			{
				if(mid == (*ite).hardware.eig.m_midDword)
				{
					return (&(*ite));
				}
			}
		}
	}
	return NULL;
}

bool CUSBHardWareDetector::InitLogicDriver()
{
	DWORD allDisk = ::GetLogicalDrives();  //����һ��32λ����������ת���ɶ����ƺ󣬱�ʾ����,���λΪA��	
	m_allDisk = allDisk;

	if( allDisk != 0 )
	{
		USES_CONVERSION;
		::EnterCriticalSection(&m_cs);
		for (int i = 0; i < 32 ; i ++)     // ��������
		{
			if ((allDisk & 1) == 1)
			{
				UINT type = ::GetDriveType(drvName[i].c_str());
				if ( type == DRIVE_CDROM )
				{
					DWORD flag;
					WCHAR info[MAX_PATH + 1] = {0};
					int err = 0;
					if( ::GetVolumeInformationW(drvName[i].c_str(), info, MAX_PATH + 1, 0, 0, &flag, 0, 0) != 0)  //�ж������Ƿ�׼������
					{
						if((flag & FILE_READ_ONLY_VOLUME) == FILE_READ_ONLY_VOLUME)//������ֻ����
						{
							string volumnname(W2A(info));
							m_logicdriver.insert(std::make_pair(i, volumnname));
						}
					}
				}
			}

			allDisk = allDisk >> 1;
		}
		::LeaveCriticalSection(&m_cs);
	}
	return true;
}
bool CUSBHardWareDetector::RecheckLogicDriver()//����U��
{
	DWORD allDisk = ::GetLogicalDrives();  //����һ��32λ����������ת���ɶ����ƺ󣬱�ʾ����,���λΪA��
	DWORD tpDisk = m_allDisk;
	m_allDisk = allDisk;
	if(allDisk > tpDisk)//˵���д�������
	{
		allDisk = tpDisk ^ allDisk;//������ӵĴ���
		if( allDisk != 0 )
		{
			USES_CONVERSION;
			for (int i = 0; i < 32 ; i ++)     // ��������
			{
				if ((allDisk & 1) == 1)
				{
					UINT type = ::GetDriveTypeW(drvName[i].c_str());
					if ( type == DRIVE_CDROM )
					{
						DWORD flag;
						WCHAR info[MAX_PATH + 1] = {0};
						if( ::GetVolumeInformationW(drvName[i].c_str(), info, MAX_PATH + 1, 0, 0, &flag, 0, 0) != 0)  //�ж������Ƿ�׼������
						{
							if((flag & FILE_READ_ONLY_VOLUME) == FILE_READ_ONLY_VOLUME)//������ֻ����
							{
								string volumnname(W2A(info));
								::EnterCriticalSection(&m_cs);
								m_logicdriver.insert(std::make_pair(i, volumnname));
								::LeaveCriticalSection(&m_cs);
							}
						}

					}
				}

				allDisk = allDisk >> 1;
			}
		}
	}
	else if(allDisk < tpDisk)//˵���д��̼���
	{
		allDisk = tpDisk ^ allDisk;//������ӵĴ���
		if( allDisk != 0 )
		{
			for (int i = 0; i < 32 ; i ++)     // ��������
			{
				if ((allDisk & 1) == 1)
				{
					::EnterCriticalSection(&m_cs);
					std::map<int , string>::iterator ite = m_logicdriver.find(i);
					if(ite != m_logicdriver.end())
						m_logicdriver.erase(ite);
					::LeaveCriticalSection(&m_cs);
				}

				allDisk = allDisk >> 1;
			}
		}
	}
	if(allDisk == allDisk)
		return false;

	return true;
}
bool CUSBHardWareDetector::CheckLogicDriver(string& vname)//���U��
{
	char diskPath[5] = {0};
	std::map<int ,std::string>::iterator ite = m_logicdriver.begin();
	::EnterCriticalSection(&m_cs);
	for(;ite != m_logicdriver.end(); ite ++)
		if((*ite).second == vname)
		{
			::LeaveCriticalSection(&m_cs);
			return true;
		}

	::LeaveCriticalSection(&m_cs);
	return false;
}
//����ҵ���֧�ֵ�usb����ô����֧�ֵ�usb�����ݣ����û�У�����NULL
USBKeyInfo* CUSBHardWareDetector::CheckSupportUSB(USBHardwareInfo& hardinfo)
{
	list<USBKeyInfo>* pUsb = CUSBControl::GetInstance()->GetUSBInfo();
	list<USBKeyInfo>::iterator ite = pUsb->begin();
	for(;ite != pUsb->end(); ite ++)
	{
		if((hardinfo.eig.ProductID == (*ite).hardware.eig.ProductID) && (hardinfo.eig.VendorID == (*ite).hardware.eig.VendorID))//���VID��PID�������ͬ
		{
			//֧����Ŀǰ�����dll�ᵼ������Ӧ��Ŀǰȥ���ò��ִ���
			// ��ʱ��������Ҫ��֧���ܽ��ж��⴦�����ڸ�Ϊdll
			// ��VID��PID����Ҫmid

			if((*ite).hardware.eig.hasMid == false && ((*ite).hardware.volumnname.size() <= 0))
			{
				return (&(*ite));
			}
			else if((*ite).hardware.eig.hasMid == false && ((*ite).hardware.volumnname.size() > 0))
			{
				//��1.5s���þ��
				Sleep(1500);//�����ʱ����Ҫ�ٸı�
				string vname = (*ite).hardware.volumnname;
				RecheckLogicDriver();//����ɨ�����ӵĴ���
				bool ret = CheckLogicDriver(vname);
				if(ret == true)
				{
					return (&(*ite));
				}
				else
					continue;
			}
			else //���mid�����
			{
				if(hardinfo.eig.hasMid == false)//��һ���������һ��mid
				{
					if(CheckMid(hardinfo.eig, (*ite).hardware.eig))//��������ֱ���ҵ�����ȷ��mid
					{
						return (&(*ite));
					}
					else// ���ڼ��ص�ԭ���������ԭ��û�ҵ�mid����ô����������һ������
					{
						continue;
					}
				}
				else //��һ���Ѿ���ȷ�ҵ���mid,ֻ�Ǻ���һ����һ��
				{
					if(hardinfo.eig.m_midDword == (*ite).hardware.eig.m_midDword)
					{
						return (&(*ite));
					}
				}
			}
		}
	}
	return NULL;
}
// ���mid,�������������е�mid�������dataeig�����ݸ����Ͻ���mid��⣬���ص�mid�洢��acteig�У����ؼ��õ���midֵ����⵽��midΪ�����midֵ����ôֱ�ӷ���true��
// ����������mid

typedef DWORD	(*GetNumberMIDFuc)();
typedef CHAR	(*GetStringMIDFuc)();
bool CUSBHardWareDetector::CheckMid(USBEigenvalue& acteig, USBEigenvalue& dataeig)
{
	acteig.hasMid = false;//��ʼ���ʧ��

	if(dataeig.hasMid == false)
		return false;

	HMODULE hModule;
	hModule = LoadLibraryW(dataeig.midDLLName.c_str());
	if(hModule == NULL)
		return false;

	USES_CONVERSION;
	string fuc = W2A(dataeig.midFucName.c_str()); 
	GetNumberMIDFuc GetNumberMID = NULL;
	GetNumberMID = (GetNumberMIDFuc)::GetProcAddress(hModule, fuc.c_str());//��ȡ����
	if(GetNumberMID != NULL)
	{
		acteig.m_midDword = GetNumberMID();

		if(acteig.m_midDword == 0)// ���û�ҵ���midֵ����0
		{
			acteig.hasMid = true;			//�ɹ���ȡ����mid
			if(acteig.m_midDword == dataeig.m_midDword)//���midһ�£���ô˵���Ѿ��ҵ����豸
			{
				FreeLibrary(hModule);
				return true;
			}
		}
	}
	

	FreeLibrary(hModule);
	return false;
}