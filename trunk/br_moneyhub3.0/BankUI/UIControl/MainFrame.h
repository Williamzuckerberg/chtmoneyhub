#pragma once
#include "TabCtrl/TabCtrl.h"
#include "TabCtrl/TabItem.h"
#include "CategoryCtrl/CategoryCtrl.h"
#include "StatusBar/BrowserStatusBar.h"
#include "FrameBorder/MainFrameBorder.h"
#include "FrameBorder/SizingBorder.h"
#include "FrameBorder/TitleBar.h"
#include "FrameBorder/SystemButtonBar.h"
#include "BrowserToolbar.h"
#include "BigButton.h"
//#include "SSLLockButton.h"
#include "MDIClient.h"
#include "ShowInfoBar.h"
#include "MenuButton.h"
#include "LogoButton.h"
#include "../Resource/Resource.h"

#include "BackButton.h"
#include "ForwardButton.h"
#include "RefreshButton.h"
#include "HelpButton.h"
#include "LoadButton.h"
#include "LoginButton.h"
#include "SepButton.h"

#include "SettingButton.h"
#include <Message.h>
#include "MenuDlg.h"



extern const UINT WM_BROADCAST_QUIT;
extern const UINT WM_UPDATERETRY;
extern const UINT WM_SWITCHTOPPAGE;
#define SHOW_INFO_LENGTH	40

class CMainFrame : public CMainFrameBorder<CMainFrame>, public CSizingBorder<CMainFrame>, public CFSMUtil
{

public:
	DECLARE_FRAME_WND_CLASS(_T("MONEYHUB_MAINFRAME"), IDR_MAINFRAME)

	CMainFrame();
	~CMainFrame();

	void RecalcClientSize(int x, int y);
	void OnFinalMessage(HWND hWnd);
	void UpdateTitle();
	void UpdateSSLState();

private:
	// message

	BEGIN_MSG_MAP_EX(CMainFrame)
		HANDLE_TUO_COPYDATA()

		CHAIN_MSG_MAP(CMainFrameBorder<CMainFrame>)
		CHAIN_MSG_MAP(CSizingBorder<CMainFrame>)
		//CHAIN_MSG_MAP(CCoolMenuClass<CMainFrame>)

		MSG_WM_CREATE(OnCreate)
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_SIZE(OnSize)
		
		MESSAGE_HANDLER_EX(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_MULTI_PROCESS_NOTIFY_AXUI_CREATED, OnNotifyAxUICreated)
		MESSAGE_HANDLER(WM_SHOWHELPMENU, OnShowHelpMenu)

		MESSAGE_HANDLER(WM_GETTING_BILL, OnGetBill)

		MESSAGE_HANDLER(WM_CLOUDALARM ,OnShowCloudMessage)
		MESSAGE_HANDLER(WM_CLOUDCLEAR ,OnClearCloudMessage)
		MESSAGE_HANDLER(WM_CLOUDCHECK ,OnShowCloudStatus)
		MESSAGE_HANDLER(WM_CLOUDNCHECK ,OnNoShowCloudStatus)
		MESSAGE_HANDLER(WM_CLOUDNDESTORY ,OnCloudDestory)
		MESSAGE_HANDLER(WM_MY_MENU_CLICKED, OnMyMenuClicked)
//		MESSAGE_HANDLER(WM_FAV_BANK_CHANGE, OnFavBankChange)

		MESSAGE_HANDLER(WM_CANCEL_ADDFAV, OnCancelAddFav)

		MESSAGE_HANDLER(WM_BROADCAST_QUIT, OnBroadcastQuit)
		MESSAGE_HANDLER(WM_UPDATERETRY, OnUpdateRetry)
		MESSAGE_HANDLER(WM_SWITCHTOPPAGE, OnSwitchTopPage)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_MYDANGEROUS, OnShowCheckMessage)

		COMMAND_ID_HANDLER(ID_HELP_SETTING, OnHelpSetting) // gao �Ӳ˵���ȥ������
		COMMAND_ID_HANDLER(ID_HELP_TIPS, OnHelpTips)
		COMMAND_ID_HANDLER(ID_HELP_ABOUT, OnHelpAbout)
		COMMAND_ID_HANDLER(ID_HELP_UPDATE, OnHelpUpdate)
		COMMAND_ID_HANDLER(ID_HELP_FEEDBACK, OnHelpFeedback)

		FORWARD_MSG_TO_MEMBER(WM_GLOBAL_CREATE_NEW_WEB_PAGE, m_wndMDIClient)
		FORWARD_MSG_TO_MEMBER(WM_GLOBAL_GET_EXIST_WEB_PAGE, m_wndMDIClient)
	END_MSG_MAP()

	// message handler

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnGetMinMaxInfo(LPMINMAXINFO lpMMI);
	BOOL OnEraseBkgnd(CDCHandle dc);
	void OnSize(UINT nType, CSize size);

	bool LoadFromFile(LPCTSTR lpPath);

	LRESULT OnClose(UINT /* uMsg */, WPARAM wParam, LPARAM lParam);
	LRESULT OnGetBill(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnNotifyAxUICreated(UINT /* uMsg */, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnShowHelpMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);

	LRESULT OnShowCloudMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);
	LRESULT OnClearCloudMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);
	LRESULT OnShowCloudStatus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);
	LRESULT OnNoShowCloudStatus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);
	LRESULT OnCloudDestory(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/);
//	LRESULT OnFavBankChange(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/);
	LRESULT OnShowCheckMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);
	LRESULT OnMyMenuClicked(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL &/*bHandled*/);

	LRESULT OnCancelAddFav(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/);

	LRESULT OnBroadcastQuit(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);
	LRESULT OnUpdateRetry(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);
	LRESULT OnSwitchTopPage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/);

	LRESULT OnHelpTips(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelpAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelpUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelpSetting(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelpFeedback(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//LRESULT OnRunUAC(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam/**/, BOOL &/*bHandled*/);

	//void RunUAC();
	
	HWND GetFrameWnd(){ return m_hWnd; }

private:
	FrameStorageStruct m_tsm;
	// ͼ�괰��
	CLogoButton m_LogoButton;
	// 
	CMenuButton m_MenuButton;
	// �ڱ�����������
	CBigButton m_BigButton;

	CShowInfoBar m_ShowInfo;
	CTuotuoTabCtrl m_TabCtrl;
	CTuotuoCategoryCtrl m_CatetoryCtrl;
	CBrowserStatusBarCtrl m_StatusBar;
	CSystemButtonBarCtrl m_SysBtnBar;
	CTitleBarCtrl m_TitleBar;
	CBrowserToolBarCtrl m_Toolbar;
	// ���˰�ť
	CBackButton m_BackButton;
	// ǰ����ť
	CForwardButton m_ForwardButton;
	// ˢ�°�ť
	CRefreshButton m_RefreshButton;
	// ������ť
	CHelpButton m_HelpButton;

	// ��½��ť
	//CLoadButton m_LoadButton;
	
	// ע�ᰴť
	//CLoginButton m_LoginButton;

	// �ָ���
	//CSepButton m_SepButton;

	// ���ð�ť
//	CSettingButton m_SettingButton;
	
	// ����
	CMDIClient m_wndMDIClient;

	CMenuDlg* m_MenuDlg;



private:
	HDWP m_hDWP;

private:
	bool m_IsShowCloudMessage;
	bool m_IsShowCloudStatus;
	CFont m_TextFont;
	HWND m_IeWnd;
};
