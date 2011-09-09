#include "stdafx.h"
#include "USBCheckor.h"
#include "USBHardWareDetector.h"
#include "USBSoftWareDetector.h"
#include "..\BankData\BankData.h"
#include <Message.h>

CUSBCheckor* CUSBCheckor::m_Instance = NULL;

CUSBCheckor* CUSBCheckor::GetInstance()
{
	if(m_Instance == NULL)
		m_Instance = new CUSBCheckor();
	return m_Instance;
}

CUSBCheckor::CUSBCheckor():m_task(NULL)
{
}
CUSBCheckor::~CUSBCheckor()
{
}
// ���
bool CUSBCheckor::CheckUSBInfo(USBHardwareInfo& hardinfo)
{
	CRecordProgram* pRecord = CRecordProgram::GetInstance();
	USBKeyInfo* pusb = CUSBHardWareDetector::GetInstance()->CheckUSBHardWare(hardinfo);
	pRecord->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"CheckUSBHardWare");
	if(pusb == NULL)
	{
		pRecord->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"CheckUSBHardWare Pusb = NULL");
		return false;
	}
	
	//bool bAlipay = true;
	//// ֧���ܵ����ӣ���ʱ�Ȳ���
	//if(pusb->hardware.financeid == "e001")//��֧����Ҫ�������⴦��
	//{
	//	//˵��֧���ܰ�װ��ȷ
	//	bAlipay = hardinfo.eig.hasMid;//֧�����������Ļ���bAlipay = false;		
	//}
	
	DWORD mid;
	// �������
	if(pusb->hardware.eig.hasMid == true)
		mid = pusb->hardware.eig.m_midDword;
	else
		mid = 0;

	CBankData* pBankData = CBankData::GetInstance();
	// �ȼ�����ݿ�����û�У�����JSP��ȡһ���¼��������ں˶������Ƿ�װ��
	bool isExist = pBankData->IsUsbExist(pusb->hardware.eig.VendorID, pusb->hardware.eig.ProductID ,mid);
	// ���״̬���	
	// ��ⰲװ״̬,����Ѿ���װ�ˣ���ô�����Ƿ������������Ƿ��޸�
	bool bInstall = CUSBSoftWareDetector::GetInstance()->CheckUSBSoftWare(pusb->software, false);
	pRecord->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"CheckUSBHardWare no repair");

	USES_CONVERSION;
	if(!bInstall)//if(!bInstall && bAlipay)	//û��װ�����ǹؼ�λ�õ�������Ϣ����
	{
		if(RemindUser(pusb))//���λ��Ҫ��ʾ�û��Ƿ�װ
			return true;

		if(isExist)//���ǰ�װ������������,��ɾ����Ϣ
		{
			USBRECORD usbRecord;
			if (pBankData->GetAUSBRecord (pusb->hardware.eig.VendorID, pusb->hardware.eig.ProductID, mid, usbRecord))
			{
				if (usbRecord.status == _SETUP_FINISH)
				{
					pBankData->DeleteUSB(pusb->hardware.eig.VendorID, pusb->hardware.eig.ProductID ,mid);//�ϴΰ�װ���˲�ɾ��
					//����ȷ��
					string version = DEFAULT_INSTALL_VERSION;//"��ʱ���밲װ�汾1.0.0.0",�϶�Ҫ���а�װ����Ϣ
					AddRecord(pusb, version);
				}
			}			
		}
		else
		{
			//����ȷ��
			string version = DEFAULT_INSTALL_VERSION;//"��ʱ���밲װ�汾1.0.0.0",�϶�Ҫ���а�װ����Ϣ
			AddRecord(pusb, version);
		}
	}

	else	//��װ��
	{		
		// ���޸�һ��
		bool rRepair = CUSBSoftWareDetector::GetInstance()->CheckUSBSoftWare(pusb->software, true);
		if(!rRepair)//�޸�ʧ�ܣ���Ҫ���°�װ
		{
			if(RemindUser(pusb))//���λ��Ҫ��ʾ�û��Ƿ�װ
				return true;

			if(isExist)
			{
				//����Ӧ������Դ�����������״̬Ϊ200����Ӧ�ø�Ϊ0������Ӧ�ò��䣬��ʱû��
				USBRECORD usbRecord;
				if (pBankData->GetAUSBRecord (pusb->hardware.eig.VendorID, pusb->hardware.eig.ProductID, mid, usbRecord))
				{
					if (usbRecord.status == _SETUP_FINISH)
					{
						pBankData->DeleteUSB(pusb->hardware.eig.VendorID, pusb->hardware.eig.ProductID ,mid);//�ϴΰ�װ���˲�ɾ��
						//����ȷ��
						string version = DEFAULT_INSTALL_VERSION;//"��ʱ���밲װ�汾1.0.0.0",�϶�Ҫ���а�װ����Ϣ
						AddRecord(pusb, version);
					}
					else
						pRecord->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"CheckUSBHardWare  state isn't _SETUP_FINISH");
				}

			}
			else //������ݿ���û�У���ô
			{
				string version = DEFAULT_INSTALL_VERSION;// ��װ
				AddRecord(pusb, version);
			}
		}
		else
		{
			//�����൱���û�����ok��
			if(pusb->software.bNeedUpdate)
			{
				// ������ڣ������������У������ǲ��ܣ�����ִ�У�ֻ�в����ڣ����Ǽ�¼�¸ð汾,�Ժ����°汾�����ٸ���
				if(!isExist)
				{
					
					pRecord->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, 
						pRecord->GetRecordInfo(L"CheckUSBHardWare  USB���ݸ���:%d__%d", pusb->hardware.eig.VendorID,pusb->hardware.eig.ProductID));

					string version = DEFAULT_BEGIN_VERSION;
					AddRecord(pusb, version, _SETUP_FINISH);//���°�װ
				}
			}// ����Ҫ�������пؼ���������ļ��
			//else
			//{
				//wstring msg = pusb->hardware.finaninstitution + L" USBKey��������ʹ��!";
				//MessageBox(NULL, msg.c_str(), L"�ƽ��", MB_OK);
				//return true;
			//}


		}		
	}

	// 
	// ִ�е����˵����ȻҪ�������ղغ�usbkey�������Ӧ��װ
	// ����ղ�

	if(m_task)
	{
		pRecord->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, 
			pRecord->GetRecordInfo(L"CheckUSBHardWare  ִ������USB����:%d__%d", pusb->hardware.eig.VendorID,pusb->hardware.eig.ProductID));

		// ִ�����أ���������������USBKEY�İ�װ����
		m_task->AddUSBTask(pusb->hardware.eig.VendorID, pusb->hardware.eig.ProductID, mid);
	}
	else
		pRecord->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"m_task = NULL");

	return true;
}
// ���ȷ������false��ȡ������true
bool CUSBCheckor::RemindUser(USBKeyInfo* pusb)
{
	wstring msg;
	msg = L"��⵽" + (pusb->hardware.finaninstitution) + L"��USBKEY����Ҫ��װ���������Ƿ�װ��";
	HWND hFrame = ::FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
	if(IDYES != MessageBox( hFrame, msg.c_str(), L"�ƽ���û���ʾ",MB_YESNO))
		return true;
	return false;
}

// �������ʧ�ܣ�˵�������Լ�Ŀ¼�µ�����ʧ���ˣ���ô��Ҫ���ñ������ص�ַ�������е����ص�ַ����usbkey��������
bool CUSBCheckor::AddRecord(USBKeyInfo* pusb, string ver, int status)
{
	// 
	CBankData* pBankData = CBankData::GetInstance();
	USES_CONVERSION;

	CHAR sys[256] = { 0 };
	switch(m_osType)
	{
	case sWinXP:
		sprintf_s(sys, 256, "WinXP%d",m_osArchi);
		break;
	case sWinVista:
		sprintf_s(sys, 256, "WinVista%d",m_osArchi);
		break;

	case sWin7:
		sprintf_s(sys, 256, "Win7%d",m_osArchi);
		break;
	default:
		sprintf_s(sys, 256, "Normal");
		break;
	}

	// ��������˻��USBkey�İ汾��Ϣ
	char cxml[2560] = {0};
	sprintf_s(cxml, 2560, XMLUSBINFO, (char*)pusb->hardware.financeid.c_str(),  W2A(pusb->hardware.inmodel.c_str()), (char*)sys, ver.c_str());
	string xml(cxml);	

	DWORD mid;
	// �������
	if(pusb->hardware.eig.hasMid == true)
		mid = pusb->hardware.eig.m_midDword;
	else
		mid = 0;

	pBankData->AddUSB(pusb->hardware.eig.VendorID, pusb->hardware.eig.ProductID, mid, 
		pusb->hardware.financeid, xml, W2A(pusb->software.backurl.c_str()), ver, status);

	return true;
}