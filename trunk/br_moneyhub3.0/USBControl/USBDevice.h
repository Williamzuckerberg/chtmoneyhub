#include "windows.h"
#include "dbt.h"
//#include "usbioctl.h"
#include <string>
#include "info.h"
using namespace std;
#define WM_SHNOTIFY WM_USER + 0x101f
#pragma once
struct USBKeyCommInfo
{
	USBEigenvalue   eig;
	wstring			ctlername;// ��¼�����������ƣ������Ժ��ͨ��
	ULONG			NumPorts;//  ��¼���豸�ڿ������еĶ˿ں�
};

class CAxUSBControl : public CWindowImpl<CAxUSBControl>
{

public:

	CAxUSBControl();
	~CAxUSBControl();

	DECLARE_WND_CLASS(_T("MH_USBMoneyhubAxUI"))

	BEGIN_MSG_MAP_EX(CAxUSBControl)
		MSG_WM_CREATE(OnCreate)
		MESSAGE_HANDLER_EX(WM_DEVICECHANGE, OnDeviceChange)
		//MESSAGE_HANDLER_EX(WM_SHNOTIFY, OnWin7DeviceChange)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		HANDLE_TUO_COPYDATA()
	END_MSG_MAP()
	
	LRESULT OnWin7DeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam);	
	int OnCreate(LPCREATESTRUCT lpCreateStruct);

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnExit(UINT uMsg, WPARAM wParam, LPARAM lParam);
	list<USBKeyCommInfo*> m_usbcheckinglist; //�������ڽ��в��Ե�usbkey���´ο��̲߳��ԾͲ��ٽ��в����� //��������
	list<USBKeyCommInfo*> m_usbcheckedlist; //�Ѿ������Ĳ������ǹ��������е�usbkey���´β��ٽ��м����
private:
	void TestUSB();//�е�USBKEYע��Ϊ��ϵͳΨһid���ͼ������һ������
	bool GetUSBAtt(wstring& usbpath, USBEigenvalue& eig);

	int m_HIDChangeTime;//��¼ϵͳ������Ϣ�Ĵ�������Ϊ����豸�ᷢ2��7����Ϣ
	HDEVNOTIFY      m_NotifyDevHandle;
	ULONG   m_hNotify; 

	bool m_isInit;//����usb���ö�ٲ��ֵĳ���
	CRITICAL_SECTION m_cs;
	CRITICAL_SECTION m_cs2;

	USBEigenvalue m_nUSBEig;//��¼�²����usb�豸��vid��pid�ȹؼ�ֵ
	list<USBKeyCommInfo*> m_usblist;// �洢usb��Ϣ�������������������ƺͶ˿ں�

	list<USBKeyCommInfo*> m_actusblist;//�洢��һ�μ���usb�豸��������Ϣ
	static DWORD WINAPI _threadUSBDevice(LPVOID lp);
private:
	bool bChecking;
	bool bNeedReCheck;
	VOID EnumerateHostControllers();
	wstring GetHCDDriverKeyName(HANDLE  HCD);
	wstring GetRootHubName (HANDLE HostController);
	wstring GetExternalHubName (HANDLE  Hub, ULONG   ConnectionIndex);

	VOID EnumerateHub(wstring HubName);
	VOID EnumerateHubPorts(HANDLE hHubDevice,ULONG NumPorts, wstring nHubDeviceName);
	wstring GetDriverKeyName(HANDLE Hub,ULONG ConnectionIndex);

	void CheckNewUSBDevice(wstring ndname);
	bool CheckCheckedlist(USBKeyCommInfo& usb);
	bool CheckCheckinglist(USBKeyCommInfo& usb);
};