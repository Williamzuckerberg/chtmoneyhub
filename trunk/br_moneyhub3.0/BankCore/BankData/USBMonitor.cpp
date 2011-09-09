#include "stdafx.h"
#include "USBMonitor.h"
#include "BkInfoDownload.h"
#include "../stdafx.h"
#include "../../BankData/BankData.h"
#include "../../USBControl/USBHardWareDetector.h"
#include "../../USBControl/USBSoftWareDetector.h"
#include "../../BankUI/UIControl/CoolMessageBox.h"

bool CBankDownInterface::CheckServerXmlFile(bool bUpdate,  USBRECORD& ur)
{
	USBKeyInfo* pusb = CUSBHardWareDetector::GetInstance()->CheckUSBHardWare(ur.vid ,ur.pid, ur.mid);//���ڴ��ļ��л�������Ϣ
	if(!pusb)
		return false;
	//����������ʾ�����⣬���ݰ汾�����ж��Ƿ��ٴ���ʾ
	if(bUpdate)
	{
		wstring msg;
		msg = L"��⵽" + pusb->hardware.finaninstitution + L"��USBKEY��Ҫ���£��Ƿ���и��£�";
		HWND hFrame = ::FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
		if(IDYES != MessageBox( hFrame, msg.c_str(), L"�ƽ����ʾ",MB_YESNO))
			return false;
	}


	return true;
}

bool CBankDownInterface::USBFinalTest(int vid, int pid, DWORD mid, bool& bSetup)
{
	USBKeyInfo* pusb = CUSBHardWareDetector::GetInstance()->CheckUSBHardWare(vid ,pid, mid);//���ڴ��ļ��л�������Ϣ

	if(!pusb)
	{
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_USBKEY_CHECK, L"USBFinalTest pusb = NULL");
		return true;
	}

	bool bInstall = CUSBSoftWareDetector::GetInstance()->CheckUSBSoftWare(pusb->software, false);//���汾�Ƿ�����
	if(bInstall)
	{			
		CUSBSoftWareDetector::GetInstance()->CheckUSBSoftWare(pusb->software, true);
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_USBKEY_CHECK, L"USBFinalTest��ʼ�޸�");
		
		if(pusb->hardware.financeid == "e001")
		{
			bSetup = true;
			CBankData::GetInstance ()->UpdateUSB (pusb->hardware.eig.VendorID, pusb->hardware.eig.ProductID, 0, 200);
			wstring alipaymsg;
			alipaymsg = L"��γ�֧���ܣ�Ȼ�����²��룬����ɰ�װ����";
			HWND hFrame = ::FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
			MessageBox( hFrame, alipaymsg.c_str(), L"�ƽ����ʾ",MB_OK );
		}

		wstring msg;
		msg = pusb->hardware.goodsname + L"�����װ����������ʹ��";
		
		mhShowMessage( GetActiveWindow(), msg.c_str());
			return true;
	}
	else
	{
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_USBKEY_CHECK, L"USBFinalTestAxUI��װʧ��");
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_USBKEY_CHECK, pusb->hardware.goodsname.c_str());
		wstring msg;
		msg = pusb->hardware.goodsname + L"�����װʧ�ܣ��Ƿ����ԣ�";
		HWND hFrame = ::FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
		if(IDYES == MessageBox( hFrame, msg.c_str(), L"�ƽ����ʾ",MB_YESNO))
			return false;
		else
			return true;
	}
}

CUSBMonitor* CUSBMonitor::m_Instance = NULL;

CUSBMonitor* CUSBMonitor::GetInstance()
{
	if(m_Instance == NULL)
		m_Instance = new CUSBMonitor();
	return m_Instance;
}

CUSBMonitor::CUSBMonitor()
{
}
CUSBMonitor::~CUSBMonitor()
{
}
void CUSBMonitor::InitParam()
{
	//����ӿڶ����������
	IBankDownInterface* pBdi = new CBankDownInterface;
	CUSBCheckor::GetInstance()->SetUSBTask(this);
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_USBKEY_CHECK, L"USBMonitor::InitParam");
	CBkInfoDownloadManager::GetInstance()->SetCheckFun(pBdi);
}

bool CUSBMonitor::StartUSBMonitor()
{
	// InitParam();
	CUSBControl::GetInstance()->BeginUSBControl();
	return true;
}

bool CUSBMonitor::AddUSBTask(int vid, int pid, DWORD mid)
{
	CBankData* pBankData = CBankData::GetInstance();
	

	USBRECORD usbrecord;
	// ��ǰ��װ��
	if(pBankData->GetAUSBRecord(vid, pid, mid, usbrecord))
	{
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_USBKEY_CHECK, CRecordProgram::GetInstance()->GetRecordInfo(L"����usb��������vid %d pid %d",vid,pid));
		CBkInfoDownloadManager::GetInstance()->MyBankUsbKeyDownload(&usbrecord);//����usb��������
		
	}
	else
		return false;

	return true;
}

