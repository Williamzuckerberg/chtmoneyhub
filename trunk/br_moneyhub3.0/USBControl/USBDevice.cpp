#include "stdafx.h"
#include "USBDevice.h"
#include <assert.h>
#include <basetyps.h>
#include "winioctl.h"
#include <usbioctl.h>
#include "info.h"
#include "usbiodef.h"
#include <algorithm>
#include "USBCheckor.h"
#include "USBHardWareDetector.h"
#define MY_STARTDELAYEVENT (0xff03)

#pragma once
#include "USBInfoFile.h"

#define ALLOC(dwBytes) GlobalAlloc(GPTR,(dwBytes))
#define REALLOC(hMem, dwBytes) GlobalReAlloc((hMem), (dwBytes), (GMEM_MOVEABLE|GMEM_ZEROINIT))
#define FREE(hMem)  GlobalFree((hMem))

#define NUM_HCS_TO_CHECK 10

static /*const*/ GUID MYGUID_DEVINTERFACE_USB_DEVICE = 
{ 0xA5DCBF10L, 0x6530, 0x11D2, {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED}};

DEFINE_GUID( GUID_CLASS_USBHUB,    0xf18a0e88, 0xc30c, 0x11d0, 0x88, 0x15, 0x00, \
			0xa0, 0xc9, 0x06, 0xbe, 0xd8);

CAxUSBControl* pAxControl = NULL;
CAxUSBControl::CAxUSBControl():m_NotifyDevHandle(NULL),m_hNotify(0),bChecking(false),bNeedReCheck(false)
{
	InitializeCriticalSection(&m_cs);
	InitializeCriticalSection(&m_cs2);
	pAxControl = this;
}

CAxUSBControl::~CAxUSBControl()
{
	DeleteCriticalSection(&m_cs);
	DeleteCriticalSection(&m_cs2);
	list<USBKeyCommInfo*>::iterator ite;
	ite = m_usbcheckinglist.begin();
	// ����õ�usb���жԱȣ���������ǵ�key�б��У�����Ƿ���Ҫmid��ͨ�Ż��mid��Ϣ
	for(;ite != m_usbcheckinglist.end(); ite ++)
	{
		if((*ite) != NULL)
			delete (*ite);
	}
	m_usbcheckinglist.clear();
	m_usbcheckedlist.clear();
	if(!m_NotifyDevHandle)
		UnregisterDeviceNotification(m_NotifyDevHandle);
	if(m_hNotify != 0)
		SHChangeNotifyDeregister(m_hNotify); 
}


//////////////////////////////////////////////////////////////////////////
typedef BOOL (WINAPI * pChangeWindowMessageFilter)(UINT message, DWORD dwFlag);

int CAxUSBControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//HMODULE hDLL = ::LoadLibraryW(L"user32.dll");	// ��̬װ�����

	//if(hDLL)
	//{
	//	pChangeWindowMessageFilter lpChangeWindowMessageFilter = (pChangeWindowMessageFilter)::GetProcAddress( hDLL, "ChangeWindowMessageFilter");	
	//	// "ChangeWindowMessageFilter"����ָ��
	//	if(lpChangeWindowMessageFilter)	
	//	{
	//		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"USBControlע����ϢChangeWindowMessageFilter");
	//		lpChangeWindowMessageFilter(WM_DEVICECHANGE, MSGFLT_ADD);
	//	}
	//	FreeLibrary(hDLL);
	//}
	m_usbcheckedlist.clear();
	m_usbcheckinglist.clear();

	//OSVERSIONINFOEX OSVerInfo; 
	//OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX); 
	//if(!GetVersionEx((OSVERSIONINFO *)&OSVerInfo)) 
	//{ 
	//	OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); 
	//	GetVersionEx((OSVERSIONINFO *)&OSVerInfo); 
	//} 

	//if(OSVerInfo.dwMajorVersion < 6) // Vista ���� 
	//{ 
	//	DEV_BROADCAST_DEVICEINTERFACE   broadcastInterface; 
	//	broadcastInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	//	broadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

	//	memcpy( &(broadcastInterface.dbcc_classguid),
	//		&(MYGUID_DEVINTERFACE_USB_DEVICE),
	//		sizeof(struct _GUID));

	//	m_NotifyDevHandle = RegisterDeviceNotification(m_hWnd,
	//		&broadcastInterface,
	//		DEVICE_NOTIFY_WINDOW_HANDLE);
	//	DWORD error = ::GetLastError();

	//	if(m_NotifyDevHandle == NULL)
	//	{
	//		if(error != ERROR_ACCESS_DENIED) 
	//			SetTimer(MY_STARTDELAYEVENT, 60*1000, NULL);//��һ�������ʼ��ʧ�ܣ��ӳٽ��г�ʼ��
	//		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"USBControl��ʼ��RegisterDeviceNotificationʧ��");
	//	}
	//}
	CUSBHardWareDetector::GetInstance()->InitLogicDriver();
	m_isInit = true;
	TestUSB();

	return 0;
}

LRESULT CAxUSBControl::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if(wParam == MY_STARTDELAYEVENT)
	{

		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"USBControl�����ӳٽ��г�ʼ��");

		//�ӳ�2������ʾ
		KillTimer(MY_STARTDELAYEVENT);
		// �ڳ�ʼ����ʱ��Ҫ���һ��USB�豸�Ѿ��������	
		// Register to receive notification when a USB device is plugged in.
		// ����ע��һ��Ҫ��ע�ᣬ������ղ����е�USB�豸������Ϣ	
		DEV_BROADCAST_DEVICEINTERFACE   broadcastInterface; 
		broadcastInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
		broadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

		memcpy( &(broadcastInterface.dbcc_classguid),
			&(MYGUID_DEVINTERFACE_USB_DEVICE),
			sizeof(struct _GUID));

		m_NotifyDevHandle = RegisterDeviceNotification(m_hWnd,
			&broadcastInterface,
			DEVICE_NOTIFY_WINDOW_HANDLE);
		m_isInit = true;
		
		TestUSB();

	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////


LRESULT CAxUSBControl::OnExit(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	::TerminateProcess(::GetCurrentProcess(), 0);
	return 0;
}

//*****************************************************************************
//
// GetHCDDriverKeyName()
//
//*****************************************************************************

wstring CAxUSBControl::GetHCDDriverKeyName(HANDLE  HCD)
{
    BOOL                    success;
    ULONG                   nBytes;
    USB_HCD_DRIVERKEY_NAME  driverKeyName;
    PUSB_HCD_DRIVERKEY_NAME driverKeyNameW;

    driverKeyNameW = NULL;
	// Get the length of the name of the driver key of the HCD
    success = DeviceIoControl(HCD,
							  IOCTL_GET_HCD_DRIVERKEY_NAME,
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &nBytes,
                              NULL);
    if (!success)
        return NULL;
    // Allocate space to hold the driver key name
    nBytes = driverKeyName.ActualLength;

    if (nBytes <= sizeof(driverKeyName))
        return NULL;

    driverKeyNameW = (PUSB_HCD_DRIVERKEY_NAME)ALLOC(nBytes);
	ZeroMemory(driverKeyNameW, nBytes);

    if (driverKeyNameW == NULL)
    {
        return NULL;
    }
	// Get the name of the driver key of the device attached to
    // the specified port.
    success = DeviceIoControl(HCD,
                              IOCTL_GET_HCD_DRIVERKEY_NAME,
                              driverKeyNameW,
                              nBytes,
                              driverKeyNameW,
                              nBytes,
                              &nBytes,
                              NULL);
    if (success)
    {
		wstring temp(driverKeyNameW->DriverKeyName);
		FREE(driverKeyNameW);
		return temp;        
    }
	FREE(driverKeyNameW);
	return NULL;
}

wstring CAxUSBControl::GetRootHubName(HANDLE HostController)
{
	BOOL                success;
	ULONG               nBytes;
	USB_ROOT_HUB_NAME   rootHubName;
	PUSB_ROOT_HUB_NAME  rootHubNameW;

	rootHubNameW = NULL;

	// Get the length of the name of the Root Hub attached to the
	// Host Controller
	success = DeviceIoControl(HostController,
		IOCTL_USB_GET_ROOT_HUB_NAME,
		0,
		0,
		&rootHubName,
		sizeof(rootHubName),
		&nBytes,
		NULL);
	if (!success)
	{
		return NULL;
	}
	// Allocate space to hold the Root Hub name
	nBytes = rootHubName.ActualLength;

	rootHubNameW = (PUSB_ROOT_HUB_NAME)ALLOC(nBytes);
	if (rootHubNameW == NULL)
	{
		return NULL;
	}
	// Get the name of the Root Hub attached to the Host Controller
	//
	success = DeviceIoControl(HostController,
		IOCTL_USB_GET_ROOT_HUB_NAME,
		NULL,
		0,
		rootHubNameW,
		nBytes,
		&nBytes,
		NULL);
	if (success)
	{
		wstring temp(rootHubNameW->RootHubName);
		FREE(rootHubNameW);
		return temp;
	}
	return NULL;
}

VOID CAxUSBControl::EnumerateHostControllers()
{
	WCHAR       HCName[16];
    int         HCNum;
    HANDLE      hHCDev;

    wstring       rootHubName;

    // Iterate over some Host Controller names and try to open them.
    for (HCNum = 0; HCNum < NUM_HCS_TO_CHECK; HCNum++)
    {
        wsprintf(HCName, L"\\\\.\\HCD%d", HCNum);
        hHCDev = CreateFileW(HCName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

        // If the handle is valid, then we've successfully opened a Host
        // Controller.  Enumerate the Root Hub attached to the Host Controller.
        if (hHCDev != INVALID_HANDLE_VALUE)
        {
            wstring driverKeyName, deviceDesc;
			driverKeyName = GetHCDDriverKeyName(hHCDev);
            rootHubName = GetRootHubName(hHCDev);
            if (rootHubName != L"")
            {
               EnumerateHub( rootHubName);
            }
			CloseHandle(hHCDev);
        }        
     }
}

VOID CAxUSBControl::EnumerateHub(wstring HubName)
{
	HANDLE          hHubDevice;
	wstring         deviceName;
	BOOL            success;
	ULONG           nBytes;
	PUSB_NODE_INFORMATION  info;

	// Initialize locals to not allocated state so the error cleanup routine
	// only tries to cleanup things that were successfully allocated.
	info        = NULL;
	hHubDevice  = INVALID_HANDLE_VALUE;

	// Allocate some space for a USB_NODE_INFORMATION structure for this Hub,
	info = (PUSB_NODE_INFORMATION)ALLOC(sizeof(USB_NODE_INFORMATION));
	if (info == NULL)
	{
		return;
	}
	// Allocate a temp buffer for the full hub device name.
	deviceName = L"\\\\.\\" + HubName;
	// Try to hub the open device
	//
	hHubDevice = CreateFile(deviceName.c_str(),	GENERIC_WRITE,	FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hHubDevice == INVALID_HANDLE_VALUE)
	{
		FREE(info);
		return;
	}
	// Now query USBHUB for the USB_NODE_INFORMATION structure for this hub.
	// This will tell us the number of downstream ports to enumerate, among
	// other things.
	success = DeviceIoControl(hHubDevice,
		IOCTL_USB_GET_NODE_INFORMATION,
		info, sizeof(USB_NODE_INFORMATION), 
		info, sizeof(USB_NODE_INFORMATION),
		&nBytes,
		NULL);
	if (!success)
	{
		CloseHandle(hHubDevice);
		FREE(info);
		return;
	}
	// Now recursively enumrate the ports of this hub.
	EnumerateHubPorts(hHubDevice, info->u.HubInformation.HubDescriptor.bNumberOfPorts, deviceName);

	CloseHandle(hHubDevice);
	FREE(info);
	return;
}


VOID CAxUSBControl::EnumerateHubPorts(HANDLE hHubDevice,ULONG NumPorts,wstring nHubDeviceName)
{
	ULONG		index;
	BOOL        success;

	PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;

	// Port indices are 1 based, not 0 based.
	for (index=1; index <= NumPorts; index++)
	{
		ULONG nBytes;
		// Allocate space to hold the connection info for this port.
		nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) + sizeof(USB_PIPE_INFO) * 30;
		connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)ALLOC(nBytes);

		if (connectionInfo == NULL)
			break;
		// Now query USBHUB for the USB_NODE_CONNECTION_INFORMATION structure
		// for this port. 
		connectionInfo->ConnectionIndex = index;
		success = DeviceIoControl(hHubDevice, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
			connectionInfo,	nBytes,	connectionInfo,	nBytes,
			&nBytes, NULL);
		if (!success)
		{
			FREE(connectionInfo);
			continue;
		}

		// ���ҵ���USB�豸��������Ϣ�ŵ�list������
		if (connectionInfo->ConnectionStatus == DeviceConnected)
		{
			// ��������е�nHubDeviceName����ǿ����������ƣ����ڼ�¼���²����usb�豸����
			// ����ǳ�ʼ��ʱ��ö�٣���ô�������豸����ö�٣�������ǣ���ôֻ���ĺ�����usb�豸���������˵�
			// Ŀǰ��˵�õ���Щ��Ϣ�Ѿ������ˣ��Ѿ��õ���VID��PID����������û���Ҫ����õ�
			// ������Ϣ��Ҫ��������IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION��Ϣ
			if(m_isInit)
			{	
				USBKeyCommInfo *pinfo = new USBKeyCommInfo;
				pinfo->eig.VendorID = connectionInfo->DeviceDescriptor.idVendor;
				pinfo->eig.ProductID = connectionInfo->DeviceDescriptor.idProduct;
				//info.eig.SerierlNumber = connectionInfo->DeviceDescriptor.iSerialNumber;
				pinfo->ctlername = nHubDeviceName;
				pinfo->NumPorts = NumPorts;

				//�����������ȽϺã���Ϊ�Ѿ���ͨ���ˣ�����ֱ�ӻ�Ҫ���MID
				m_usblist.push_back(pinfo);
			}
			else
			{	
				//m_nUSBEig���������¼���²�����豸��vid��pid��
				if((connectionInfo->DeviceDescriptor.idVendor == m_nUSBEig.VendorID) && (connectionInfo->DeviceDescriptor.idProduct == m_nUSBEig.ProductID))
				{
					USBKeyCommInfo *pinfo = new USBKeyCommInfo;
					pinfo->eig.VendorID = connectionInfo->DeviceDescriptor.idVendor;
					pinfo->eig.ProductID = connectionInfo->DeviceDescriptor.idProduct;
					//info.eig.SerierlNumber = connectionInfo->DeviceDescriptor.iSerialNumber;
					pinfo->ctlername = nHubDeviceName;
					pinfo->NumPorts = NumPorts;
					// ������usb�豸�Ŀ������������Ϣ
					m_usblist.push_back(pinfo);
				}
			}
		}

		// If the device connected to the port is an external hub, get the
		// name of the external hub and recursively enumerate it.
		//
		if (connectionInfo->DeviceIsHub)
		{
			wstring extHubName;
			extHubName = GetExternalHubName(hHubDevice,index);
			if (extHubName != L"")
			{
				EnumerateHub(extHubName);
				// On to the next port
			}
		}

		FREE(connectionInfo);
	}
}

//*****************************************************************************
//
// GetExternalHubName()
//
//*****************************************************************************

wstring CAxUSBControl::GetExternalHubName(HANDLE  Hub, ULONG   ConnectionIndex)
{
    BOOL                        success;
    ULONG                       nBytes;
    USB_NODE_CONNECTION_NAME    extHubName;
    PUSB_NODE_CONNECTION_NAME   extHubNameW;

    extHubNameW = NULL;

    // Get the length of the name of the external hub attached to the
    // specified port.
    //
    extHubName.ConnectionIndex = ConnectionIndex;
    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              &extHubName,
                              sizeof(extHubName),
                              &extHubName,
                              sizeof(extHubName),
                              &nBytes,
                              NULL);

    if (!success)
    {
		return NULL;
    }
    // Allocate space to hold the external hub name
    //
    nBytes = extHubName.ActualLength;

    if (nBytes <= sizeof(extHubName))
    {
       return NULL;
    }

    extHubNameW = (PUSB_NODE_CONNECTION_NAME)ALLOC(nBytes);

    if (extHubNameW == NULL)
    {
        return NULL;
	}
    extHubNameW->ConnectionIndex = ConnectionIndex;
    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              extHubNameW,
                              nBytes,
                              extHubNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success)
    {
        FREE(extHubNameW);
		return NULL;
    }

	wstring temp(extHubNameW->NodeName);
    FREE(extHubNameW);

    return temp;
}
LRESULT CAxUSBControl::OnWin7DeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"���յ�Svr�ļ���ź�");

	// win7����svr֪ͨusb�豸�仯
	m_isInit = true;//����һ��usb
	TestUSB();


	return 0;
}
LRESULT CAxUSBControl::OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_isInit = true;//����һ��usb
	TestUSB();
	//switch(wParam)
	//{
	//	case DBT_DEVICEARRIVAL://����U��
	//		{
	//		//�е�USBKEYע��Ϊ��ϵͳΨһid���ͼ������һ�����ͣ���Ҫ��������
	//		//��������U��һ���ע��ΪCDROM��MASS STORAGE����Ҳ��û����ʾ�ģ�����������Ҫ���жϣ���ͳһ��USB�ж�
	//			PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
	//			if(pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
	//			{
	//				PDEV_BROADCAST_DEVICEINTERFACE pDevInf;
	//				pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;//�����Ѿ�����˲����USB��Ϣ���Ͳ�����ö����
	//				wstring ndevicename(pDevInf->dbcc_name);

	//				CheckNewUSBDevice(ndevicename);
	//			}
	//		}
	//		break;
	//	case DBT_DEVICEREMOVECOMPLETE://ɾ��U�̲���
	//	default:
	//		break;
	//}
	return 0;
}

DWORD WINAPI CAxUSBControl::_threadUSBDevice(LPVOID lp)
{
	// ���߳�Ҫ��һ��checkinglist�����������ظ���ѯ�Ĺ���
	USBKeyCommInfo* pusb = (USBKeyCommInfo*)lp;
	if(pusb == NULL)
		return 0;

	::EnterCriticalSection(&pAxControl->m_cs2);
	pAxControl->m_usbcheckedlist.push_back(pusb);
	::LeaveCriticalSection (&pAxControl->m_cs2);

	USBHardwareInfo usb;
	usb.eig.ProductID = pusb->eig.ProductID;
	usb.eig.VendorID = pusb->eig.VendorID;
	if(CUSBCheckor::GetInstance()->CheckUSBInfo(usb) == false)
	{
		::EnterCriticalSection(&pAxControl->m_cs);
		pAxControl->m_usbcheckinglist.push_back(pusb);
		::LeaveCriticalSection (&pAxControl->m_cs);
		::EnterCriticalSection(&pAxControl->m_cs2);
		pAxControl->m_usbcheckedlist.remove(pusb);
		::LeaveCriticalSection (&pAxControl->m_cs2);

	}
	else
	{
		::EnterCriticalSection(&pAxControl->m_cs2);
		pAxControl->m_usbcheckedlist.remove(pusb);
		::LeaveCriticalSection (&pAxControl->m_cs2);
		delete pusb;
	}


	return 0;
}

void CAxUSBControl::TestUSB()
{
	m_usblist.clear();
	EnumerateHostControllers();
	if(m_usblist.size() > m_actusblist.size())//˵��������usb�豸
	{
		//CUSBHardWareDetector::GetInstance()->RecheckLogicDriver();
		USBKeyCommInfo *ptemp;
		list<USBKeyCommInfo *>::iterator lite, aite;
		lite = m_usblist.begin();
		for(;lite != m_usblist.end(); )
		{
			bool tflag = true;
			if((*lite)!= NULL)
			{
				for(aite = m_actusblist.begin(); aite != m_actusblist.end(); aite ++)
				{
					if(((*aite)->eig.VendorID == (*lite)->eig.VendorID) && ((*aite)->eig.ProductID == (*lite)->eig.ProductID) &&((*aite)->ctlername == (*lite)->ctlername))//˵������������
					{
						ptemp = (*lite);
						lite = m_usblist.erase(lite);
						delete ptemp;
						tflag = false;
						break;
					}
				}
			}
			if(tflag)
			{	
				USBKeyCommInfo* pinfo = new USBKeyCommInfo;
				pinfo->eig.VendorID = (*lite)->eig.VendorID;
				pinfo->eig.ProductID = (*lite)->eig.ProductID;
				pinfo->ctlername = (*lite)->ctlername;
				pinfo->NumPorts = (*lite)->NumPorts;
				m_actusblist.push_back(pinfo);
				lite ++;
			}
		}
	}
	else if(m_usblist.size() < m_actusblist.size())//˵��usb�豸����
	{
		CUSBHardWareDetector::GetInstance()->RecheckLogicDriver();
		m_actusblist.swap(m_usblist);
		list<USBKeyCommInfo *>::iterator lite = m_usblist.begin();
		for(;lite != m_usblist.end(); lite ++)
		{
			if((*lite) != NULL)
				delete (*lite);
		}
		m_usblist.clear();
		return;		
	}
	else//usb�豸û�б仯
		return;
	// �����е�USB��Ϣ���жԱ�
	list<USBKeyCommInfo *>::iterator ite;
	ite = m_usblist.begin();
	// ����õ�usb���жԱȣ���������ǵ�key�б��У�����Ƿ���Ҫmid��ͨ�Ż��mid��Ϣ
	for(;ite != m_usblist.end(); ite ++)
	{
		// ��ʱֻ��USBKeyCommInfo������ֵ����//���̣߳�
		if((*ite) != NULL)
		{
			USBHardwareInfo usb;
			usb.eig.ProductID = (**ite).eig.ProductID;
			usb.eig.VendorID = (**ite).eig.VendorID;
			::EnterCriticalSection(&m_cs);
			if(CheckCheckedlist((**ite)))
			{
				::LeaveCriticalSection (&m_cs);
				continue;
			}
			::LeaveCriticalSection (&m_cs);

			::EnterCriticalSection(&m_cs2);
			if(CheckCheckinglist((**ite)))
			{
				::LeaveCriticalSection (&m_cs2);
				continue;
			}
			::LeaveCriticalSection (&m_cs2);
		// ���������̲߳�ѯ
			CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, 
				CRecordProgram::GetInstance()->GetRecordInfo(L"��鵽USB�豸VID:%d__PID:%d", usb.eig.VendorID , usb.eig.ProductID));

			DWORD dw;
			USBKeyCommInfo* pinfo = new USBKeyCommInfo;
			pinfo->eig.VendorID = (*ite)->eig.VendorID;
			pinfo->eig.ProductID = (*ite)->eig.ProductID;
			pinfo->ctlername = (*ite)->ctlername;
			pinfo->NumPorts = (*ite)->NumPorts;

			CloseHandle(CreateThread(NULL, 0, _threadUSBDevice, (LPVOID)pinfo, 0, &dw));
		}
	}

	ite = m_usblist.begin();
	// ����õ�usb���жԱȣ���������ǵ�key�б��У�����Ƿ���Ҫmid��ͨ�Ż��mid��Ϣ
	for(;ite != m_usblist.end(); ite ++)
	{
		if((*ite) != NULL)
			delete (*ite);
	}
	m_usblist.clear();
}
bool CAxUSBControl::CheckCheckinglist(USBKeyCommInfo& usb)
{
	list<USBKeyCommInfo*>::iterator ite;
	ite = m_usbcheckedlist.begin();
	// ����õ�usb���жԱȣ���������ǵ�key�б���
	for(;ite != m_usbcheckedlist.end(); ite ++)
	{
		if((usb.eig.ProductID == (**ite).eig.ProductID) && (usb.eig.VendorID == (**ite).eig.VendorID)&& (usb.ctlername == (**ite).ctlername))
			return true;
	}
	return false;
	
}
bool CAxUSBControl::CheckCheckedlist(USBKeyCommInfo& usb)
{
	list<USBKeyCommInfo*>::iterator ite;
	ite = m_usbcheckinglist.begin();
	// ����õ�usb���жԱȣ���������ǵ�key�б���
	for(;ite != m_usbcheckinglist.end(); ite ++)
	{
		if((usb.eig.ProductID == (**ite).eig.ProductID) && (usb.eig.VendorID == (**ite).eig.VendorID) && (usb.ctlername == (**ite).ctlername))
			return true;
	}
	return false;
	
}
void CAxUSBControl::CheckNewUSBDevice(wstring ndname)
{
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"CheckNewUSBDevice:%s",ndname.c_str()));
	// �ڵõ�����豸����֮���������豸ͨ�Ż��vid��pid�أ�
	// ��ʱ����ö������usb�������豸���Ա�VID��PID���usb��������Ȼ������ͨ�Ž��д���
	// �Ժ��ٿ���ʲô����ֱ��ͨ�����ﴫ����������ֱ�ӻ��USB������
	if(!GetUSBAtt(ndname,m_nUSBEig))
	{
		m_isInit = true;//���ͨ������û����ȷ��ã���ô��ȡ���еĴ�ͷ����һ��
	}
	else
	{
		m_isInit = false;
	}

	TestUSB();
}

bool CAxUSBControl::GetUSBAtt(wstring& usbpath, USBEigenvalue& eig)
{
	//�Ȱ�����ת��Ϊȫ��д
	transform(usbpath.begin(), usbpath.end(), usbpath.begin(), towupper);
	size_t nvidName = usbpath.find(L"VID_");
	if(nvidName != wstring::npos)
	{
		std::wstring strVidName = usbpath.substr(nvidName + 4, 4);
		strVidName = L"0x" + strVidName;
		USHORT vid = wcstol(strVidName.c_str(), NULL, 16);
		eig.VendorID = vid;
	}
	else
		return false;

	nvidName = usbpath.find(L"PID_");
	if(nvidName != wstring::npos)
	{
		std::wstring strPidName = usbpath.substr(nvidName + 4, 4);
		strPidName = L"0x" + strPidName;
		USHORT pid = wcstol(strPidName.c_str(), NULL, 16);
		eig.ProductID = pid;
	}
	else
		return false;

	nvidName = usbpath.find(L"#", 10);
	if(nvidName != wstring::npos)
	{
		std::wstring strSN = usbpath.substr(nvidName + 1, 14);
		USES_CONVERSION;
		eig.SerierlNumber = W2A(strSN.c_str());
	}
	else
		return false;

	return true;
}


	/*if(NULL == OldSetupDiGetClassDevsW || NULL == OldSetupDiEnumDeviceInterfaces ||
		NULL == OldSetupDiDestroyDeviceInfoList || NULL == OldSetupDiGetDeviceInterfaceDetailW)
		return;
*/

	//GUID  hidGuid;
	//GUID* guid;
	//if(!isSHIDDevice)
	//	guid = (GUID*)(void*)&GUID_DEVINTERFACE_USB_DEVICE;
	//else
	//{
	//	OldHidD_GetHidGuid (&hidGuid);
	//	guid = (GUID*)&hidGuid; //�����ѧ�����豸���е�usbkey��
	//}

	////   ׼�����ҷ���HID�淶��USB�豸
	//
	//HDEVINFO   hDevInfo = OldSetupDiGetClassDevsW(guid, NULL, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE); 
	//int rest = GetLastError();
	//if(ERROR_INVALID_PARAMETER == rest || ERROR_INVALID_FLAGS == rest)
	//	return;

	////   ����USB�豸�ӿ� 
	//SP_DEVICE_INTERFACE_DATA   strtInterfaceData; 
	//strtInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	//DWORD dwIndex = 0;
	////ZeroMemory(&strtInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
	////strtInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	//BOOL bRet = FALSE;
	//while(TRUE)
	//{
	//	
	//// ����ͬʱ����2��usb��������⣬�����ҵ��Ĳ��ǵ�һ��������Ĵ������ľ��Ǹ�����,����û�ж����������ʱ��һ����
	//// �ҵ���һ�����ϸ�usb�ӿڵ��豸
	//	BOOL   bSuccess = OldSetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex, 
	//		&strtInterfaceData);
	//	if (!bSuccess || strtInterfaceData.Flags != SPINT_ACTIVE)
	//	{
	//		if (GetLastError() == ERROR_NO_MORE_ITEMS)
	//		{
	//			break;
	//		}
	//		continue;
	//	}

	//	dwIndex ++ ;
	//	//   ���ҵ����豸�����ȡ�豸·���� 
	//	PSP_DEVICE_INTERFACE_DETAIL_DATA   strtDetailData; 
	//	DWORD strSize = 0;
	//	DWORD reqSize = 0;
	//	//  �����Ҫ�Ļ�������С
	//	OldSetupDiGetDeviceInterfaceDetailW(hDevInfo,&strtInterfaceData,NULL,0,&strSize,NULL);
	//	//SetupDiGetDeviceInterfaceDetail
	//	reqSize = strSize;
	//	strtDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)new unsigned char[reqSize];
	//	ZeroMemory(strtDetailData, reqSize);
	//	strtDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	//	// ���·������,�����õ���USB���ƣ����Ҫ��ø�ϸ�ڵģ�Ҫ����������USBSTOR��·�����в���
	//	if (!OldSetupDiGetDeviceInterfaceDetailW(hDevInfo,&strtInterfaceData, 
	//		strtDetailData,strSize,&reqSize,NULL)) 
	//	{ 	
	//		// */"�����豸·��ʱ����! "
	//		delete strtDetailData;
	//		continue;

	//	}
	//	//
	//	//  �м�����USB�豸ͨ��
	//	//   �������豸��ͨ��
	//	wstring usbpath(strtDetailData->DevicePath);
	//	// �����豸��ϵͳ��ռ����2��������Ҫ��Ϊ0
	//	HANDLE   hCom;
	//	if(!isSHIDDevice)
	//		hCom = CreateFileW(usbpath.c_str(), GENERIC_READ | GENERIC_WRITE, 
	//			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
	//	else
	//		hCom = CreateFileW(usbpath.c_str(), 0,
	//			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //�����ѧ�豸��Ҫ�����
	//	delete strtDetailData;
	//	if (hCom == INVALID_HANDLE_VALUE) 
	//	{
	//		continue;
	//	} 
	//	// ��ʱ���Զ���
	//	USBEigenvalue eig;
	//	if(!isSHIDDevice)
	//		GetVIDnPID(usbpath, eig);
	//	else
	//	{
	//		HIDD_ATTRIBUTES deviceAttributes;
	//		if (!OldHidD_GetAttributes(hCom, &deviceAttributes)) //������øú���ʧ�ܣ���ô�����ƽ��д���  
	//		{
	//			GetVIDnPID(usbpath, eig);
	//		}
	//		else
	//		{
	//			eig.ProductID = deviceAttributes.ProductID;
	//			eig.VendorID = deviceAttributes.VendorID;
	//			eig.VersionNumber = deviceAttributes.VersionNumber;
	//		}
	//	}
	//	
	//	//�����ֱ��
	//	//// �����
	//	//PSTORAGE_DEVICE_DESCRIPTOR pDeviceDesc;
	//	//pDeviceDesc=(PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1];
	//	//pDeviceDesc->Size = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;
	//	////�������洢���͵��ǲ���ͨ��HidD_GetAttributes���VID��PID�ģ����潫ͨ�����ƻ��
	//	//if(GetDriveProperty(hCom, pDeviceDesc) == FALSE)
	//	//	rest = GetLastError();

	//	//   ��ѯ�豸��ʶ 



	//		
	//			
	//	// ����ط������usb�豸��vid��pid����Ӧ���������strtAttrib
	//	// ����vid��pid����豸�ţ��������Ҫ���mid����ôֱ�����£������Ҫ���mid�����������ͨ��

	//	// �ͷ���Դ 
	//	CloseHandle(hCom); 
	//}
	////
	//OldSetupDiDestroyDeviceInfoList(hDevInfo);



//// ��ȡ�洢�豸���͵ľ��������Ŀǰ����,hDeviceҪ��usbstor���͵Ĵ�
//BOOL CAxUSBControl::GetDriveProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc)
//{
//	STORAGE_PROPERTY_QUERY Query;    // ��ѯ�������
//	DWORD dwOutBytes;                // IOCTL������ݳ���
//	BOOL bResult;                    // IOCTL����ֵ
//	// ָ����ѯ��ʽ
//
//	Query.PropertyId = StorageDeviceProperty;
//	Query.QueryType = PropertyStandardQuery;
//	// ��IOCTL_STORAGE_QUERY_PROPERTYȡ�豸������Ϣ
//	bResult = ::DeviceIoControl(hDevice, // �豸���
//		IOCTL_STORAGE_QUERY_PROPERTY,    // ȡ�豸������Ϣ
//		&Query, sizeof(STORAGE_PROPERTY_QUERY),    // �������ݻ�����
//		pDevDesc, pDevDesc->Size,        // ������ݻ�����
//		&dwOutBytes,                     // ������ݳ���
//		(LPOVERLAPPED)NULL);             // ��ͬ��I/O    
//
//	return bResult;
//}


