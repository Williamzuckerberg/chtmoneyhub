#include "stdafx.h"
#include "../Util/ProcessManager.h"
#include "../Util/Util.h"
#include "../Util/SelfUpdate.h"
#include "../Include/Util.h"
#include "TabCtrl/TabItem.h"
#include "CategoryCtrl/CategoryItem.h"
#include "MainFrame.h"
#include "SettingDlg.h"
#include "AboutDlg.h"
#include "ChildFrm.h"
#include "SelectMonthDlg.h"


#include "../../Utils/HardwareID/genhwid.h"
#include "../../Utils/sn/SNManager.h"
#include "Version.h"
#include "CoolMenuHook.h"
#include "../Tips/TipsDlg.h"
#include "../../Utils/Config/HostConfig.h"
#include "../../Include/ConvertBase.h"
#include "../../Utils/UserBehavior/UserBehavior.h"
#include "../../BankData/BankData.h"
#include "../../Utils/SecurityCache/comm.h"
#include "ShowInfoBar.h"
#include "ShowJSFrame.h"

#include "../Synchro/SynchroDlg.h"


#define TIMER_AUTOUPDATE (0xf0)
#define TIMER_BCHECKKERNEL (0xf1)
#define TIMER_CHECKKERNEL (0xf2)

const UINT WM_BROADCAST_QUIT = RegisterWindowMessage(_T("BankQuitBroadcastMsg"));
const UINT WM_UPDATERETRY = RegisterWindowMessage(_T("BankUpdateRetryMsg"));
const UINT WM_SWITCHTOPPAGE = RegisterWindowMessage(_T("BankSwitchTopPageMsg"));
const UINT WM_SVRCHECKUSER = RegisterWindowMessage(_T("BankSvrCheckUser"));


#define USER_LOAD_INFO_KEEP_TIME 10// 10 ��ʾ�ɸð�ť�����Ի�����10��


CMainFrame::CMainFrame() : CFSMUtil(&m_tsm), m_ShowInfo(&m_tsm), m_BigButton(&m_tsm), m_TabCtrl(&m_tsm), m_CatetoryCtrl(&m_tsm), m_StatusBar(&m_tsm),
	m_SysBtnBar(&m_tsm), m_TitleBar(&m_tsm), m_Toolbar(&m_tsm), m_wndMDIClient(m_TabCtrl, m_CatetoryCtrl, &m_tsm), m_hDWP(NULL), m_MenuButton(&m_tsm),//m_LogoButton(&m_tsm), 
	m_BackButton(&m_tsm), m_ForwardButton(&m_tsm), m_RefreshButton(&m_tsm), m_HelpButton(&m_tsm),m_IsShowCloudMessage(false),
	m_IsShowCloudStatus(false), m_MenuDlg(NULL), m_LoadButton(&m_tsm), m_LoginButton(&m_tsm), m_SepButton(&m_tsm), m_pTextButton(NULL),// m_AlarmButton(&m_tsm),
	m_SynchroButton(&m_tsm), m_pUserInfoBtn(NULL), m_InfoDlg(NULL), m_pShowDlg(NULL), m_pLoadingButton(NULL), m_UserMenuDlg(NULL)//,m_SettingButton(&m_tsm)
{
	FS()->pMainFrame = this;
}


CMainFrame::~CMainFrame()
{
	if (NULL != m_MenuDlg)
		delete m_MenuDlg;

	if (NULL != m_pTextButton)
		delete m_pTextButton;
	m_pTextButton = NULL;

	if (NULL != m_pUserInfoBtn)
		delete m_pUserInfoBtn;
	m_pUserInfoBtn = NULL;

	if (NULL != m_InfoDlg)
		delete m_InfoDlg;
	m_InfoDlg = NULL;

	if (NULL != m_UserMenuDlg)
		delete m_UserMenuDlg;
	m_UserMenuDlg = NULL;
}

LRESULT CMainFrame::UpdateUserLoadStatus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/)// �����û���¼״̬
{
	
	// �����ڵ�¼��ťȥ��
	if (NULL != m_pLoadingButton)
	{
		delete m_pLoadingButton;
		m_pLoadingButton = NULL;
	}

	// �����ǰ�stoken#mail#userid#���͹���
	string strRead = (LPSTR)lParam;

	// ����UI�еĵ�¼�û���Ϣ(��ͬ��ʹ��)
	string strTag = MY_PARAM_END_TAG;
	// ��ȡstoken
	CBankData::GetInstance()->m_CurUserInfo.strstoken = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

	// ��ȡMail
	strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
	string strMail = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));
	CBankData::GetInstance()->m_CurUserInfo.strmail = strMail;

	// ��ȡuserid
	strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
	CBankData::GetInstance()->m_CurUserInfo.struserid = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

	// ��ȡ ������֤��Ϣ
	strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
	string strMailVerif = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

	// ��ȡ �û���¼״̬
	strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
	string strUserStatus = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

	USERSTATUS emStatus = (USERSTATUS)atoi(strUserStatus.c_str());
	CBankData::GetInstance()->m_CurUserInfo.emUserStatus = emStatus;

	// �Ѿ���¼
	if (emStatus != emNotLoad)
	{
		// ��½��ť
		m_LoadButton.ShowWindow(false);
		// �ָ���
		m_SepButton.ShowWindow(false);
		// ע�ᰴť
		m_LoginButton.ShowWindow(false);
		//m_LogoButton.ShowWindow(false);
	
		RECT rcClient;
		GetClientRect(&rcClient);
		//w = rcClient.right;
		//h = rcClient.bottom;

		POINT pt;
		SIZE sz;

		// ͬ����ť
		pt.x = rcClient.right - m_SynchroButton.GetFullWidth() - 15;
		// ���Ѱ�ť
	//	pt.x = pt.x - m_AlarmButton.GetFullWidth() - 10;

		if (m_pTextButton != NULL)
		{
			wstring strText;
			m_pTextButton->ReadButtonText(strText);
			string strTpText = CW2A(strText.c_str());
			if (strTpText != strMail)
			{
				delete m_pTextButton;
				m_pTextButton = NULL;
			}
		}
		if (m_pTextButton == NULL)
		{
			m_pTextButton = new CTextButton(&m_tsm);
			m_pTextButton->CreateButton(m_hWnd, CA2W(strMail.c_str()), TEXT_BTN_USER_NAME);
		}
		// �ı���ť
		pt.x = pt.x - m_pTextButton->GetTextButtonWidth() - 20;
		pt.y = 49;
		sz.cx = m_pTextButton->GetTextButtonWidth();
		sz.cy = m_pTextButton->GetTextButtonHeight();
		m_pTextButton->SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);

		if (strMailVerif == MY_MAIL_VERIFY_ERROR)
		{
			// ��ɫ̾��
			if (NULL == m_pUserInfoBtn)
			{
				m_pUserInfoBtn = new CSepButton(&m_tsm);
				m_pUserInfoBtn->CreateButton(m_hWnd, _T("UserInfo.png"), WM_USER_INFO_CLICKED);

			}
			// ��λ��ɫ̾��
			pt.x = pt.x - m_pUserInfoBtn->GetButtonWidth() - 4;
			pt.y = 48;
			sz.cx = m_pUserInfoBtn->GetButtonWidth();
			sz.cy = m_pUserInfoBtn->GetButtonHeight();
			m_pUserInfoBtn->SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);

			m_pUserInfoBtn->ShowWindow(true);
			// �ú�ɫ̾����ʾ����ʾ��ͣ��10��
			PostMessage(WM_USER_INFO_CLICKED, USER_LOAD_INFO_KEEP_TIME, SW_SHOW);
		}
		
		// ��ת����ҳ
		//m_wndMDIClient.ActiveCategoryByIndex(0);

		// �ı���ť
	//	m_LoginButton.ShowWindow(true);
		// ͬ����ť
		//m_SynchroButton.ShowWindow(true);
		// ���Ѱ�ť
		//m_AlarmButton.ShowWindow(true);
	}
	else
	{
		// ͬ����ť
		//m_SynchroButton.ShowWindow(false);
		// ���Ѱ�ť
		//m_AlarmButton.ShowWindow(false);

		// �ı���ť
		if (NULL != m_pTextButton)
		{
			delete m_pTextButton;
			m_pTextButton = NULL;
		}

		// ��ɫ̾��
		if (NULL != m_pUserInfoBtn)
		{
			m_pUserInfoBtn->ShowWindow(false);
		}

		// ��ɫ̾����ʾ��
		if (NULL != m_InfoDlg)
		{
			m_InfoDlg->ShowWindow(FALSE);
			m_InfoDlg->InitSendMailTime();
		}

		// ��½��ť
		m_LoadButton.ShowWindow(true);
		// �ָ���
		m_SepButton.ShowWindow(true);
		// ע�ᰴť
		m_LoginButton.ShowWindow(true);
		//m_LogoButton.ShowWindow(true);
	}

	// ����JSȥ������ҳ����ʾ���� 
	::PostMessage(FS()->TabItem()->GetChildFrame()->GetItem()->GetAxControl(), WM_CHANGE_FIRST_PAGE_SHOW, 0, emStatus);

	// ���µ�ǰHTMLҳ��
	FS()->TabItem()->GetChildFrame()->DoNavigateRefresh();

	// ��������Ҫͬ��
	if (emStatus == emUserNet && CBankData::GetInstance() ->IsCurrentDbNeedSynchro())
	{
		// �ܸ�Ҫ��BUG2823
		//CSynchroDlg dlg(3);
		//dlg.DoModal();
	}
	return 0;
}

void CMainFrame::RecalcClientSize(int w, int h)
{
	int iTop = 0;
	if (w == -1)
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		w = rcClient.right;
		h = rcClient.bottom;
	}

	POINT pt;
	SIZE sz;

	bool bIsWindowMax = (GetStyle() & WS_MAXIMIZE) != 0;

	RECT rcSysBtn;
	m_SysBtnBar.GetClientRect(&rcSysBtn);
	m_SysBtnBar.SetWindowPos(NULL, w - rcSysBtn.right - 2, 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	m_SysBtnBar.Invalidate();
	
	pt.x = w - m_SysBtnBar.GetFullWidth() - m_MenuButton.GetFullWidth() - 1;
	pt.y = 1;
	m_MenuButton.SetWindowPos(NULL, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	m_MenuButton.Invalidate();

	m_TitleBar.SetWindowPos(NULL, 0, 0, pt.x, 30/*rcSysBtn.bottom*/, SWP_NOZORDER);

	// ͼ�갴ť
	//pt.x = w - 105;
	//pt.y = 34;// m_BigButton.GetHeight() + 6;
	//sz.cx = m_LogoButton.GetFullWidth();
	//sz.cy = m_LogoButton.GetHeight();
	//m_LogoButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);

	// ͬ����ť
	pt.x = w - m_SynchroButton.GetFullWidth() - 15;
	pt.y = 40;
	sz.cx = m_SynchroButton.GetFullWidth();
	sz.cy = m_SynchroButton.GetHeight();
	m_SynchroButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);

#define LOAD_BUTTON_HEIGHT 49
	// ��ӵ�½��ť
	pt.x = pt.x - m_LoadButton.GetFullWidth()- 15;
	pt.y = LOAD_BUTTON_HEIGHT;
	sz.cx = m_LoadButton.GetFullWidth();
	sz.cy = m_LoadButton.GetHeight();
	m_LoadButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);
	m_LoadButton.Invalidate();

	// ��ӷָ�ͼ��
	pt.x = pt.x - m_SepButton.GetFullWidth() - 10;
	pt.y = LOAD_BUTTON_HEIGHT;
	sz.cx = m_SepButton.GetFullWidth();
	sz.cy = m_SepButton.GetHeight();
	m_SepButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);

	// ���ע�ᰴť
	pt.x = pt.x - m_LoginButton.GetFullWidth() - 10;
	pt.y = LOAD_BUTTON_HEIGHT;
	sz.cx = m_LoginButton.GetFullWidth();
	sz.cy = m_LoginButton.GetHeight();
	m_LoginButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);
	m_LoginButton.Invalidate();


	// ͬ����ť
	pt.x = w - m_SynchroButton.GetFullWidth() - 15;
	// ���Ѱ�ť
	/*pt.x = pt.x - m_AlarmButton.GetFullWidth() - 10;
	pt.y = 40;
	sz.cx = m_AlarmButton.GetFullWidth();
	sz.cy = m_AlarmButton.GetHeight();
	m_AlarmButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);*/

	if (NULL != m_pTextButton)
	{
		// �ı���ť
		pt.x = pt.x - m_pTextButton->GetTextButtonWidth() - 20;
		pt.y = 49;
		sz.cx = m_pTextButton->GetTextButtonWidth();
		sz.cy = m_pTextButton->GetTextButtonHeight();
		m_pTextButton->SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);
		m_pTextButton->Invalidate();
	}

	if (NULL != m_pUserInfoBtn)
	{
		// ��ɫ̾��
		pt.x = pt.x - m_pUserInfoBtn->GetButtonWidth() - 4;
		pt.y = 48;
		sz.cx = m_pUserInfoBtn->GetButtonWidth();
		sz.cy = m_pUserInfoBtn->GetButtonHeight();
		m_pUserInfoBtn->SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);
		CRect rect(0, 0, sz.cx, sz.cy);
		::InvalidateRect(m_pUserInfoBtn->m_hWnd, &rect, false);
		//Invalidate();
	}

	if (NULL != m_InfoDlg)
	{
		if (m_InfoDlg->IsWindowShow())
		{
			// �ƶ���ɫ̾����ʾ��
			TPMPARAMS tps;
			tps.cbSize = sizeof(TPMPARAMS);
			m_pUserInfoBtn->GetWindowRect(&tps.rcExclude);

			CPoint pt;
			pt.x = tps.rcExclude.left - 32;
			pt.y = tps.rcExclude.bottom;

			m_InfoDlg->ShowMenuWindow(pt);
		}
	}



	m_Toolbar.SetWindowPos(NULL, 0, 30/*rcSysBtn.bottom*/, w - s()->LogoButton()->GetWidth(), m_Toolbar.GetHeight(), SWP_NOZORDER);

	// (B-1)
	// Back Button
	pt.x = 9; //s()->MainFrame()->GetBorderWidth() + 2;
	pt.y = 30 + 3;// m_BigButton.GetHeight() + 3;
	sz.cx = m_BackButton.GetFullWidth();
	sz.cy = m_BackButton.GetHeight();
	m_BackButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOZORDER);

	// (B-2)
	// Forward Button
	pt.x = 40;
	pt.y = 30 + 8;//m_BigButton.GetHeight() + 8;
	sz.cx = m_ForwardButton.GetFullWidth();
	sz.cy = m_ForwardButton.GetHeight();
	m_ForwardButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOZORDER);

	// (B-3)
	// Reload Button
	pt.x = 40 + 6 + m_ForwardButton.GetFullWidth();
	pt.y = 30 + 8;//m_BigButton.GetHeight() + 8;
	sz.cx = m_RefreshButton.GetFullWidth();
	sz.cy = m_RefreshButton.GetHeight();
	m_RefreshButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOZORDER);

	// (B-4)
	// setting button
	/*pt.x = 40 + 12 + m_ForwardButton.GetFullWidth() + m_RefreshButton.GetFullWidth();
	pt.y = 30 + 8;
	sz.cx = m_SettingButton.GetFullWidth ();
	sz.cy = m_SettingButton.GetHeight ();
	m_SettingButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOZORDER);*/

	// (B-5)
	// Help Button
	pt.x = 40 + 12 + m_ForwardButton.GetFullWidth() + m_RefreshButton.GetFullWidth();//40 + 18 + m_ForwardButton.GetFullWidth() + m_RefreshButton.GetFullWidth() + m_SettingButton.GetFullWidth ();
	pt.y = 30 + 8;//m_BigButton.GetHeight() + 8;
	sz.cx = m_HelpButton.GetFullWidth();
	sz.cy = m_HelpButton.GetHeight();
	m_HelpButton.SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOZORDER);

	

	// ʣ��߶�
	int iTopHeight = 79;	
	int iLessHeight = h - iTopHeight - s()->StatusBar()->GetStatusBarHeight(false);
		
	if (bIsWindowMax)
		m_CatetoryCtrl.SetWindowPos(NULL, 0, iTopHeight, w, s()->Category()->GetCategoryHeight(), SWP_NOZORDER);
	else
	{
		if (iLessHeight > s()->Category()->GetCategoryHeight())
		{
			RECT rect = { s()->MainFrame()->GetBorderWidth(), iTopHeight, w - s()->MainFrame()->GetBorderWidth(), iTopHeight + s()->Category()->GetCategoryHeight() };
			m_CatetoryCtrl.SetWindowPos(NULL,s()->MainFrame()->GetBorderWidth(), iTopHeight, w - 2 * s()->MainFrame()->GetBorderWidth(), s()->Category()->GetCategoryHeight(), SWP_NOZORDER);
		}
		else
			m_CatetoryCtrl.SetWindowPos(NULL, s()->MainFrame()->GetBorderWidth(), iTopHeight, w - 2 * s()->MainFrame()->GetBorderWidth(), iLessHeight, SWP_NOZORDER);
	}

	iTopHeight += s()->Category()->GetCategoryHeight();
	iLessHeight = h - iTopHeight - s()->StatusBar()->GetStatusBarHeight(false);

	// ������ʾ��Ϣ���Ƿ���ʾ�����״̬Ϊ0����ô��������ʾ����Ϣ��
	if(FS()->TabItem() != NULL && FS()->TabItem()->m_pCategory->GetShowInfoState() != 0)//���Ʋ���ʾ��Ϣ��
	{
		// ���ø���ʾ�Ļ�������
		m_ShowInfo.SetFinanceName(FS()->TabItem()->m_pCategory->GetShowInfo());
		// ��ȡ��ʾ״̬������ʾ���ֻ���ֱ����ʾ����Ϊ��ɫ
		if(FS()->TabItem()->m_pCategory->GetShowInfoState() == 1)
			m_ShowInfo.SetState(eShowInfo);
		else if(FS()->TabItem()->m_pCategory->GetShowInfoState() == 2)
			m_ShowInfo.SetState(eHideOthers);
		if(m_ShowInfo.GetShowState() == eShowInfo)
		{
			// ����ʾ����ʱ�� ���ÿ�ȴ�С�����ݵ�ǰ��С���ò�ͬ����
			if (bIsWindowMax)
				m_ShowInfo.SetWindowPos(NULL, 0, iTopHeight, w, SHOW_INFO_LENGTH, SWP_NOZORDER | SWP_SHOWWINDOW);
			else
			{
				if (iLessHeight > SHOW_INFO_LENGTH)
				{
					RECT rect = { s()->MainFrame()->GetBorderWidth(), iTopHeight, w - s()->MainFrame()->GetBorderWidth(), iTopHeight + SHOW_INFO_LENGTH };
					m_ShowInfo.SetWindowPos(NULL, &rect, SWP_NOZORDER | SWP_SHOWWINDOW);
				}
				else
					m_ShowInfo.SetWindowPos(NULL, s()->MainFrame()->GetBorderWidth(), iTopHeight, w - 2 * s()->MainFrame()->GetBorderWidth(), iLessHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

			}
			iTopHeight += SHOW_INFO_LENGTH;
			iLessHeight = h - iTopHeight - s()->StatusBar()->GetStatusBarHeight(false);
		}
		// ��ʾ����ɫʱ������ʣ����ʾ���沿��Ϊ1,������ظò��ִ���
		else
		{
			m_ShowInfo.SetWindowPos(NULL, s()->MainFrame()->GetBorderWidth(), iTopHeight, w - 2 * s()->MainFrame()->GetBorderWidth(), h - s()->StatusBar()->GetStatusBarHeight(false) - iTopHeight - 1, SWP_NOZORDER | SWP_SHOWWINDOW);
			iTopHeight = h - s()->StatusBar()->GetStatusBarHeight(false) - 1;
			iLessHeight = 1;
		}
	}
	// ֻ�е����˵�ʱ����ʾ��Ϣ���������������
	else
		m_ShowInfo.ShowWindow(SW_HIDE);
		
	if (FS()->TabItem() == NULL || FS()->TabItem()->m_pCategory->GetWebData()->IsNoSubTab() == true)
		m_TabCtrl.ShowWindow(SW_HIDE);
	else
	{
		if (bIsWindowMax)
			m_TabCtrl.SetWindowPos(NULL, 0, iTopHeight, w, s()->Tab()->GetTabHeight(), SWP_NOZORDER | SWP_SHOWWINDOW);
		else
		{
			if (iLessHeight > s()->Tab()->GetTabHeight())
				m_TabCtrl.SetWindowPos(NULL, s()->MainFrame()->GetBorderWidth(), iTopHeight, w - 2 * s()->MainFrame()->GetBorderWidth(), s()->Tab()->GetTabHeight(), SWP_NOZORDER | SWP_SHOWWINDOW);
			else
				m_TabCtrl.SetWindowPos(NULL, s()->MainFrame()->GetBorderWidth(), iTopHeight, w - 2 * s()->MainFrame()->GetBorderWidth(), iLessHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
		}

		iTopHeight += s()->Tab()->GetTabHeight() + 1;
	}
	// ��ʾie childframe���ֵĴ���
	int xMdi = bIsWindowMax ? 0 : s()->MainFrame()->GetBorderWidth();
	int yMdi = iTopHeight;
	int cxMdi = w - (bIsWindowMax ? 0 : s()->MainFrame()->GetBorderWidth() * 2);
	int cyMdi = h - iTopHeight - s()->StatusBar()->GetStatusBarHeight(bIsWindowMax);
	m_wndMDIClient.SetWindowPos(NULL, xMdi, yMdi, cxMdi, cyMdi, SWP_NOZORDER | SWP_NOSENDCHANGING);
	m_wndMDIClient.PostMessage(WM_SIZE, 0, MAKELPARAM(cxMdi, cyMdi));
	
	// ��ʾ״̬��
	m_StatusBar.SetWindowPos(NULL, 0, h - s()->StatusBar()->GetStatusBarHeight(bIsWindowMax), w, s()->StatusBar()->GetStatusBarHeight(false), SWP_NOZORDER);

	CClientDC dc(m_hWnd);
	OnEraseBkgnd(dc);
}

void CMainFrame::OnFinalMessage(HWND hWnd)
{
	delete this;
}

void CMainFrame::UpdateTitle()
{
	std::wstring str = _T("�ƽ��");
	if (FS()->TabItem())
	{
		str += _T(" - ");
		if (FS()->TabItem()->m_pCategory->GetWebData())
			str += std::wstring(FS()->TabItem()->m_pCategory->GetWebData()->GetName()) + _T(" - ") + FS()->TabItem()->GetText();
	}

	SetWindowText(str.c_str());
	m_TitleBar.Invalidate();
}

void CMainFrame::UpdateSSLState()
{
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
// message handler

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	::SetActiveWindow(m_hWnd);
	//SetWindowPos(HWND_TOP, 0, 0, 0, 0,SWP_NOSIZE);
	SetClassLongPtr(m_hWnd, GCL_STYLE, GetClassLong(m_hWnd, GCL_STYLE) | CS_DROPSHADOW);
	ModifyStyle(WS_CAPTION, 0);

	// icons
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	HICON hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	//
	m_TextFont.CreateFont(13, 0, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, _T("Tahoma"));

	// use data
	SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)&m_tsm);
	FS()->hMainFrame = m_hWnd;
	s()->CreateTheme(m_hWnd, false);

	RECT rc = { lpCreateStruct->x, lpCreateStruct->y, lpCreateStruct->x + lpCreateStruct->cx, lpCreateStruct->y + lpCreateStruct->cy };
	SetRestoreRectAndMaxStatus(rc, (lpCreateStruct->style & WS_MAXIMIZE) != 0);

	m_MenuButton.CreateButton(m_hWnd);

	m_Toolbar.CreateBrowserToolbar(m_hWnd);
	m_SysBtnBar.CreateSystemButtonBar(m_hWnd);
	m_TitleBar.CreateTitleBar(m_hWnd);
	//m_LogoButton.CreateLogoButton(m_hWnd);

	m_BackButton.CreateButton(m_hWnd);
	m_ForwardButton.CreateButton(m_hWnd);
	m_RefreshButton.CreateButton(m_hWnd);
	m_HelpButton.CreateButton(m_hWnd);

	m_LoadButton.CreateButton(m_hWnd);
	m_LoginButton.CreateButton(m_hWnd);
	m_SepButton.CreateButton(m_hWnd, _T("Sep_button.png"));

	m_SynchroButton.CreateButton(m_hWnd);
	//m_AlarmButton.CreateButton(m_hWnd);
	//m_SynchroButton.ShowWindow(false);
	//m_AlarmButton.ShowWindow(false);
//	m_SettingButton.CreateButton(m_hWnd);


	m_TabCtrl.Create(m_hWnd, NULL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE);
	m_CatetoryCtrl.Create(m_hWnd, NULL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE);
	m_ShowInfo.CreateShowInfoBar(m_hWnd);
	m_wndMDIClient.CreateTabMDIClient(m_hWnd);
	m_StatusBar.CreateBrowserStatusBar(m_hWnd);

	GetClientRect(&rc);
	SetRgnForWindow(lpCreateStruct->cx, lpCreateStruct->cy);
	//RecalcClientSize(rc.right, rc.bottom);

/*

	m_Rebar.CreateBrowserReBarCtrl(m_hWnd, lpCreateStruct->cx);
	m_CmdBar.CreateCommandBarCtrl(m_Rebar);
	m_AddressBar.CreateAddressBar(m_Rebar);
	m_SearchBar.CreateSearchBar(m_Rebar);
	m_FavorBar.CreateFavoriteBar(m_Rebar);
	m_RecentBar.CreateRecentBar(m_Rebar);

	m_Rebar.InitBand(&m_TitleBar, &m_SysBtnBar, &m_CmdBar, &m_Toolbar, &m_AddressBar, &m_SearchBar, &m_FavorBar, &m_RecentBar);

	m_FindBar.CreateFindTextBar(m_hWnd);

	// �����tab�ڶ��ϣ���tab��Ҫ����rebar���棨����titlebar��
*/

	CCheckForUpdate::ClearInstallFlag();

	//InitCustomDrawMenu();
//	CCoolMenuHook::InstallHook(m_hWnd);
	//InitCoolMenuSettings();

	m_RefreshButton.EnableWindow(FALSE);
	m_ForwardButton.EnableWindow(FALSE);
	m_BackButton.EnableWindow(FALSE);
	return 0;
}


void CMainFrame::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
{
	lpMMI->ptMinTrackSize.x = 540;
	lpMMI->ptMinTrackSize.y = 110;
}

BOOL CMainFrame::OnEraseBkgnd(CDCHandle dc)
{
	bool bIsSecure = false;
	if (m_CatetoryCtrl.GetCurrentSelection() && m_CatetoryCtrl.GetCurrentSelection()->ShowSecureColor())
		bIsSecure = true;

	RECT rect;
	GetClientRect(&rect);
	rect.bottom = 139 + SHOW_INFO_LENGTH; //s()->Toolbar()->GetBigButtonHeight() + s()->Toolbar()->GetToolbarHeight() + s()->Tab()->GetTabHeight();
	s()->Toolbar()->DrawRebarBack(dc, rect, (GetStyle() & WS_MAXIMIZE) != 0, bIsSecure);
	FS()->pCate->Invalidate();

	// ���߿�
	if ((GetStyle() & (WS_MAXIMIZE | WS_MINIMIZE)) == 0) 
	{
		GetClientRect(&rect);
		//rect.top += s()->Toolbar()->GetBigButtonHeight() + s()->Toolbar()->GetToolbarHeight() + s()->Tab()->GetTabHeight();
		rect.top += 108;
		s()->MainFrame()->DrawBorder(dc, rect, false, bIsSecure);
	}

	RECT rcSysBtn;
	m_SysBtnBar.GetClientRect(&rcSysBtn);
	GetClientRect(&rect);
	dc.SetPixel(rect.right - 2, rect.top + rcSysBtn.bottom - 1 , RGB(24, 82, 123));


	if(m_IsShowCloudMessage)
	{
		//���λ��Ҫ������������
		HFONT oldFont = dc.SelectFont(m_TextFont);
		dc.SetTextColor(RGB(16, 93, 145));

		SetBkMode(dc,TRANSPARENT); 

		//��ȡ��ʾ���ֵ�λ��
		POINT pt;
		pt.x = 40 + 18 + m_ForwardButton.GetFullWidth() + m_RefreshButton.GetFullWidth() + m_HelpButton.GetFullWidth()+ 45;// + m_SettingButton.GetFullWidth ()
		pt.y = 30 + 8 + 8;
		LPTSTR mes = _T("�޷������ư�ȫ�����������ܴ��ڰ�ȫ����");
		dc.TextOut(pt.x,pt.y,mes,_tcslen(mes));	
		dc.SelectFont(oldFont);
	}
/*
	m_wndMDIClient.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.left -= s()->MainFrame()->GetBorderWidth();
	rect.right += s()->MainFrame()->GetBorderWidth();
	rect.top -= s()->MainFrame()->GetBorderHeight();
	rect.bottom += s()->MainFrame()->GetBorderHeight();
	dc.FillSolidRect(&rect, FS()->TabItem() == NULL || FS()->TabItem()->GetSecure() == false ? 0xcc0000 : 0x00cc00);

*/
	return TRUE;
}


void CMainFrame::OnSize(UINT nType, CSize size)
{
	RecalcClientSize(size.cx, size.cy);
}


LRESULT CMainFrame::OnClose(UINT /* uMsg */, WPARAM wParam, LPARAM lParam)
{

	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"�û��ر�");

	WINDOWPLACEMENT winp = { sizeof(WINDOWPLACEMENT) };
	::GetWindowPlacement(m_hWnd, &winp);

	DWORD dwPos = MAKELONG(winp.rcNormalPosition.left, winp.rcNormalPosition.top);
	DWORD dwSize = MAKELONG(winp.rcNormalPosition.right - winp.rcNormalPosition.left, winp.rcNormalPosition.bottom - winp.rcNormalPosition.top);
	DWORD dwMax = (GetStyle() & WS_MAXIMIZE) ? 1 : 0;
	::SHSetValue(HKEY_CURRENT_USER, _T("Software\\Bank"), _T("wndpos"), REG_DWORD, &dwPos, sizeof(DWORD));
	::SHSetValue(HKEY_CURRENT_USER, _T("Software\\Bank"), _T("wndsize"), REG_DWORD, &dwSize, sizeof(DWORD));
	::SHSetValue(HKEY_CURRENT_USER, _T("Software\\Bank"), _T("wndmax"), REG_DWORD, &dwMax, sizeof(DWORD));

	// �˳�ʱɾ��Software\\Bank\\VirtualReg
	::SHDeleteKeyW(HKEY_CURRENT_USER, _T("Software\\Bank\\VirtualReg"));

	::SendMessageTimeoutW(m_IeWnd, WM_MAINFRAME_CLOSE,  0,  0, SMTO_NORMAL, 2000, 0);
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"֪ͨ�ں��˳�");
	DestroyWindow();
	::PostQuitMessage(0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnNotifyAxUICreated(UINT /* uMsg */, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	SwitchToThisWindow(m_hWnd, TRUE);
	m_IeWnd = (HWND)lParam;
	CProcessManager::_()->OnAxUICreated((HWND)lParam);
	AccessDefaultPage(m_hWnd); // ����Ĭ�ϵ�ҳ�沢��ʾ�û�ָ���ı�ǩҳ

	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"�ں˴����˹���ҳ");

	m_RefreshButton.EnableWindow(TRUE);
	m_ForwardButton.EnableWindow(TRUE);
	m_BackButton.EnableWindow(TRUE);

	// �����Ƿ�����Զ���½�û�
	UserAutoLoad();

#ifndef SINGLE_PROCESS
	SetTimer(TIMER_CHECKKERNEL, 5000, NULL);
#endif

	return 0;
}

LRESULT CMainFrame::OnMyMenuClicked(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL & bHandled )
{
	int nVal = wParam;
	switch(nVal)
	{
	case 1:
		//OnHelpSetting(0, 0, 0, bHandled);
		PostMessage(WM_SHOW_USER_DLG, 0, MY_TAG_SETTING_DLG);
		break;
	case 2:
		OnHelpTips(0, 0, 0, bHandled);
		break;
	case 3:
		OnHelpFeedback(0, 0, 0, bHandled);
		break;
	case 4:
		OnHelpAbout(0, 0, 0, bHandled);
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CMainFrame::OnShowUserMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/) // �����û���¼��Ĳ˵�
{
	TPMPARAMS tps;
	tps.cbSize = sizeof(TPMPARAMS);
	m_pTextButton->GetWindowRect(&tps.rcExclude);

	CPoint pt1;
	pt1.x = tps.rcExclude.right - 48;
	pt1.y = tps.rcExclude.bottom;

	if (NULL == m_UserMenuDlg)
	{
		m_UserMenuDlg = new CMenuDlg(m_hWnd, _T("Menu2.png"), WM_USER_INFO_MENU_CLICKED, 3, 33, 7); // 3��ʾ��ť����33��ťѡ��ʱ�ĸ߶ȣ�7��ʾ��ť��ⲿ�ֵĸ߶�
		m_UserMenuDlg->Create(NULL,IDD_DLG_MENU);	
	}

	m_UserMenuDlg->ShowMenuWindow(pt1);

	return 0;
}

LRESULT CMainFrame::OnUserMenuClick(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/) // �û���¼��Ĳ˵�����Ӧ�¼�
{
	int nVal = wParam;
	switch(nVal)
	{
	case MY_USER_INFO_MENU_CLICK_MAILCHANGE: // ��������
		{
			// ����JSȥ������ҳ����ʾ���� 
			::SendMessage(FS()->TabItem()->GetChildFrame()->GetItem()->GetAxControl(), WM_AX_CHANGE_SETTINT_STATUS, 0, emUserChangeMail);
			PostMessage(WM_SHOW_USER_DLG, 0, MY_TAG_SETTING_DLG);
			break;
		}
	case MY_USER_INFO_MENU_CLICK_PWDCHANGE: // ��������
		{
			// ����JSȥ������ҳ����ʾ���� 
			::SendMessage(FS()->TabItem()->GetChildFrame()->GetItem()->GetAxControl(), WM_AX_CHANGE_SETTINT_STATUS, 0, emUserChangePwd);
			PostMessage(WM_SHOW_USER_DLG, 0, MY_TAG_SETTING_DLG);
			break;
		}
	case MY_USER_INFO_MENU_CLICK_QUIT: // �˳�
		{
			CBankData::GetInstance()->CloseDB();
			// ��ת����ҳ
			//m_wndMDIClient.ActiveCategoryByIndex(0);
			::PostMessage(m_IeWnd, WM_AX_LOAD_USER_QUIT, 0, 0);

			// ˢ�µ�ǰҳ
			FS()->TabItem()->GetChildFrame()->DoNavigateRefresh();
			break;
		}
	default:
		break;
	}
	return 0;
}

LRESULT CMainFrame::OnShowHelpMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/)
{	
	TPMPARAMS tps;
	tps.cbSize = sizeof(TPMPARAMS);
	m_MenuButton.GetWindowRect(&tps.rcExclude);

	CPoint pt;
	pt.x = tps.rcExclude.left;
	pt.y = tps.rcExclude.bottom;

	if (NULL == m_MenuDlg)
	{
		m_MenuDlg = new CMenuDlg(m_hWnd, _T("NoSel.png"), WM_MY_MENU_CLICKED, 4, 25, 14);
		m_MenuDlg->Create(NULL,IDD_DLG_MENU);	
	}

	m_MenuDlg->ShowMenuWindow(pt);
	return 0;
}

LRESULT CMainFrame::OnHelpSetting(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	/*CSettingDlg dlg;
	dlg.DoModal();*/

	return 0;
}

LRESULT CMainFrame::OnHelpTips(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CTipsDlg tip(1);
	tip.DoModal();

	return 0;
}

LRESULT CMainFrame::OnHelpAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();

	return 0;
}

LRESULT CMainFrame::OnHelpUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	KillTimer(TIMER_AUTOUPDATE);

	CCheckForUpdate* pCheck = CCheckForUpdate::CreateInstance(m_hWnd);
	//pCheck->Check(false);
	pCheck->Release();

	return 0;
}

LRESULT CMainFrame::OnBroadcastQuit(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/)
{
	static bool bIsQuit = false;
	if (!bIsQuit)
	{
		bIsQuit = true;
		PostMessage(WM_CLOSE);
	}
	
	return 0;
}

LRESULT CMainFrame::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL &bHandled)
{
	if(TIMER_BCHECKKERNEL == wParam)
	{
		KillTimer(TIMER_BCHECKKERNEL);
	}
	else if(TIMER_CHECKKERNEL == wParam)
	{
		LPCTSTR lpszProcessMutex = _T("_Moneyhub_{878B413D-D8FF-49e7-808D-9A9E6DDCF2B7}");
		HANDLE _hIEcoreProcessMutex  = CreateMutex(NULL, TRUE, lpszProcessMutex);

		DWORD err = GetLastError();
		if(_hIEcoreProcessMutex != NULL)
			::CloseHandle(_hIEcoreProcessMutex);

		// ���ں˶����Ѿ����ڻ��߽�ֹ����ʱ��˵���Ѿ������ý���
		if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED)
		{
			return 0;
		}
		else
		{
			KillTimer(TIMER_CHECKKERNEL);//�رն�ʱ��
			mhMessageBox(NULL, L"������д���Ϊ���������ʻ���ȫ���ƽ�㽫����", L"�ƽ����",MB_OK);

			CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_COMMON_ERROR, L"��⵽�ں�����Ӧ");

				// ���������ƽ��
			WCHAR szPath[MAX_PATH] ={0};
			::GetModuleFileName(NULL, szPath, _countof(szPath));
			::PathRemoveFileSpecW(szPath);

			std::wstring path(szPath);
			path += L"\\Moneyhub.exe -wait";

			STARTUPINFO si;	
			PROCESS_INFORMATION pi;	
			ZeroMemory( &pi, sizeof(pi) );	
			ZeroMemory( &si, sizeof(si) );	
			si.cb = sizeof(si);	
			//��������	
			if(CreateProcessW(NULL, (LPWSTR)path.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
			{		
				CloseHandle( pi.hProcess );		
				CloseHandle( pi.hThread );		
			}

			CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"�����ƽ��");

			exit(0);
		}

		return 0;
	}
	else
		bHandled = FALSE;

	return 0;
}

LRESULT CMainFrame::OnUpdateRetry(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/)
{
	//SetTimer(TIMER_AUTOUPDATE, 10 * 60 * 1000);

	return 0;
}

LRESULT CMainFrame::OnSwitchTopPage(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/)
{
	enum TopPageStruct tps = (TopPageStruct)wParam;
	SwitchTopPage(m_hWnd, tps);

	return 0;
}

LRESULT CMainFrame::OnSvrCheckUser(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/)
{
	wstring mail = (wchar_t *)lParam;

	USES_CONVERSION;
	string smail = W2A(mail.c_str());
	if((CBankData::GetInstance()->m_CurUserInfo.struserid == MONHUB_GUEST_USERID && smail == "�ÿ�") || (CBankData::GetInstance()->m_CurUserInfo.strmail == smail))
	{
		enum TopPageStruct tps = kToolsPage;
		SwitchTopPage(m_hWnd, tps);
	}
	else if(CBankData::GetInstance()->m_CurUserInfo.struserid == MONHUB_GUEST_USERID)
		mhMessageBox(m_hWnd, L"������½�û��˺Ž��в鿴", L"�ƽ��", MB_OK);
	else
		mhMessageBox(m_hWnd, L"Ҫ�鿴���˺��뵱ǰ��¼�˺Ų�һ�£�����Ҫ���˳���ǰ��¼�˺�", L"�ƽ��", MB_OK);

	return 0;
}

LRESULT CMainFrame::OnHelpFeedback(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DWORD dwValueNameLength = 100;        // ֵ���ַ�������   
	WCHAR ptszValueName[100] = L"";    // ֵ���ַ���
	wstring ieversion;

	//���ie�汾��Ϣ
	DWORD dwType, dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0, dwMax = 0;
	if( ERROR_SUCCESS == ::SHGetValueW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Internet Explorer", L"Version", &dwType, &ptszValueName, &dwValueNameLength))
	{
		ieversion = ptszValueName;
	}

	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, CRecordProgram::GetInstance()->GetRecordInfo(L"IE�汾:%s",ieversion.c_str()));
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	USES_CONVERSION;
	sprintf_s(szBuf, sizeof(szBuf), "%s?Moneyhubuid=%s&V=%s&Sn=%s",W2CA(CHostContainer::GetInstance()->GetHostName(kFeedback).c_str()), GenHWID2().c_str(), ProductVersion_All, CSNManager::GetInstance()->GetSN().c_str());

	HWND hPop = FindWindowW(NULL, L"MoneyHub_Svr_Mainframe");
	if(hPop != NULL)
		::PostMessage(hPop, WM_MONEYHUB_FEEDBACK, 0, 0);
	//mhMessageBox(NULL, L"лл����֧�֣�",L"�ƽ��", MB_OK);
	::CreateNewPage_0(m_hWnd, A2CT(szBuf),TRUE);

	return 0;
}

LRESULT CMainFrame::OnShowCloudMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/)
{
	m_IsShowCloudMessage = true;
	Invalidate();
	return 0;
}
LRESULT CMainFrame::OnClearCloudMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/)
{
	m_IsShowCloudMessage = false;
	Invalidate();
	return 0;
}
LRESULT CMainFrame::OnShowCloudStatus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/)
{
	FS()->Status()->SetStatusText(L"���������ư�ȫɨ�衭��", true);
	Invalidate();
	return 0;
}

LRESULT CMainFrame::OnNoShowCloudStatus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/)
{
	FS()->Status()->SetStatusText(L"", true);
	Invalidate();
	return 0;
}

LRESULT CMainFrame::OnCloudDestory(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/)
{
	mhMessageBox(NULL,L"�ƽ�㷢�ְ�ȫ��в������ϣ�Ϊ��֤��ȫ�������������ƽ��",L"�ƽ��",MB_OK);
	
	CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_COMMON_ERROR, L"��⵽��ȫ��в");

	::SendMessageTimeoutW(m_IeWnd, WM_MAINFRAME_CLOSE,  0,  0, SMTO_NORMAL, 2000, 0);
	DestroyWindow();

	// ���������ƽ��
	WCHAR szPath[MAX_PATH] ={0};
	::GetModuleFileName(NULL, szPath, _countof(szPath));
	::PathRemoveFileSpecW(szPath);

	std::wstring path(szPath);
	path += L"\\Moneyhub.exe -wait";

	STARTUPINFO si;	
	PROCESS_INFORMATION pi;	
	ZeroMemory( &pi, sizeof(pi) );	
	ZeroMemory( &si, sizeof(si) );	
	si.cb = sizeof(si);	
	//��������	
	if(CreateProcessW(NULL, (LPWSTR)path.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{		
		CloseHandle( pi.hProcess );		
		CloseHandle( pi.hThread );		
	}
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"�����ƽ��");

	::PostQuitMessage(0);
	return 0;
}

LRESULT CMainFrame::OnShowCheckMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &/*bHandled*/)
{
	::MessageBoxW(NULL, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��", L"�ƽ�㰲ȫ��ʾ", MB_OK);
	return 0;
}
// ȡ���ղ�
LRESULT CMainFrame::OnCancelAddFav(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/)
{
	if(m_IeWnd)
		::PostMessageW(m_IeWnd, WM_CANCEL_ADDFAV,  wParam,  lParam);
	return 0;
}

LRESULT CMainFrame::OnAutoUserDlg(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/)
{
	if (MY_STATUE_CLOSE_DLG == lParam) // ��ʾ�ر�
	{
		if (NULL != m_pShowDlg && NULL != m_pShowDlg->m_hWnd) // �����Ի���
		{
			::SendMessage(m_pShowDlg->m_hWnd, END_SHOW_DIALOG, 0, 0);
		}
	}
	else if(MY_STATUE_SHOW_DLG == lParam) // ��ʾ��
	{
		BOOL bTag = FALSE;
		if (NULL != m_pShowDlg)
		{
			m_pShowDlg->ShowWindow(true);
		}
		else
			OnShowUserDlg(0, 0, wParam, bTag);
	}
	else if(MY_STATUS_HIDE_DLG == lParam)
	{
		if (NULL != m_pShowDlg)
			m_pShowDlg->ShowWindow(false);
	}
	return 0;
}

LRESULT CMainFrame::OnShowUserDlg(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/)
{
	wstring strText, strPath;
	int nErrCode = 0;
	if(wParam !=3 )//caohaitao
	{
		if (MY_TAG_LOAD_DLG == lParam)
		{
			// ������ڵ�¼���򲻵���¼��
			if (IsLoading())
				return 0;

			strText = L"��¼�ƽ��";
			strPath = L"login.html";
			nErrCode = wParam; // ��ʱֻ�е�¼������д�����
		}
		else if (MY_TAG_REGISTER_DLG == lParam)
		{
			strText = L"ע��ƽ���˻�";
			strPath = L"register.html";
		}
		else if (MY_TAG_SETTING_DLG == lParam)
		{
			strText = L"����";
			strPath = L"setOption.html";
		}
		else if (MY_TAG_REGISTER_GUIDE == lParam)
		{
			strText = L"ע����";
			strPath = L"registerGuide.html";
		}
		else if(MY_TAG_INIT_PWD == lParam)
		{
			strText = L"��������";
			strPath = L"findPwd.html";
		}

		CShowJSFrameDlg dlg(strText.c_str(), strPath.c_str(), nErrCode);

		m_pShowDlg = &dlg;


		dlg.DoModal(m_hWnd);

		m_pShowDlg = NULL;
	}
	else//caohaitao
		// ֪ͨ�ں˽��е�¼
		::PostMessage(m_IeWnd, WM_AX_USER_AUTO_LOAD, 3, 0);
	return 0;
}

LRESULT CMainFrame::UserInfoBtnClicked(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/)
{
	if (SW_SHOW == lParam)
	{
		TPMPARAMS tps;
		tps.cbSize = sizeof(TPMPARAMS);
		m_pUserInfoBtn->GetWindowRect(&tps.rcExclude);

		CPoint pt;
		pt.x = tps.rcExclude.left - 32;
		pt.y = tps.rcExclude.bottom;

		if (NULL == m_InfoDlg)
		{
			m_InfoDlg = new CUserLoadInfoDlg(m_hWnd, _T("UserLoadInfo.png"));
			m_InfoDlg->Create(m_hWnd,IDD_DLG_MENU);	
		}

		// ���öԻ�������ʱ�䣬��ʱ��������ĵط��öԻ�������
		if (0 != wParam)
			m_InfoDlg->KeepWindowForSeconds(wParam);


		m_InfoDlg->ShowMenuWindow(pt);
	}
	else if (SW_HIDE == lParam)
	{
		if (NULL != m_InfoDlg)
			m_InfoDlg->ShowWindow(FALSE);
	}

	return 0;
}

LRESULT CMainFrame::ChangeUserDlgName(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &/*bHandled*/)
{
	string strTp = (char*)lParam;
	if (NULL != m_pShowDlg)
	{
		m_pShowDlg->SetWindowText(CA2W(strTp.c_str()));
		/*CRect rcClient;
		::GetClientRect(m_pShowDlg->m_hWnd, &rcClient);
		CRect rect(0, 0, rcClient.right, 20);
		::InvalidateRect(m_pShowDlg->m_hWnd, &rcClient, false);*/
	}

	return 0;
}

BOOL CMainFrame::UserAutoLoad()
{
	string strUserID, strMail, strStoken;
	bool bBack = CBankData::GetInstance()->ReadNeedAutoLoadUser(strUserID, strMail, strStoken);

	bool bAutoLoading = false;
	if (strUserID.length() > 0 && bBack)
		bAutoLoading = true;

	if (bAutoLoading)
	{
		if (NULL == m_pLoadingButton)
		{
			// ���ذ�ť
			// ��½��ť
			m_LoadButton.ShowWindow(false);
			// �ָ���
			m_SepButton.ShowWindow(false);
			// ע�ᰴť
			m_LoginButton.ShowWindow(false);

			m_pLoadingButton = new CTextButton(&m_tsm);
			m_pLoadingButton->CreateButton(m_hWnd, L"���ڵ�¼...", TEXT_BTN_USER_LOADINT);


			RECT rcClient;
			GetClientRect(&rcClient);
			POINT pt;
			SIZE sz;

			// ͬ����ť
			pt.x = rcClient.right - m_SynchroButton.GetFullWidth() - 15;

			// ��ӵ�½��ť
			pt.x = pt.x - m_pLoadingButton->GetFullWidth()- 15;
			pt.y = 49;
			sz.cx = m_pLoadingButton->GetFullWidth();
			sz.cy = m_pLoadingButton->GetHeight();
			m_pLoadingButton->SetWindowPos(NULL, pt.x, pt.y, sz.cx, sz.cy, SWP_NOSIZE);

			m_pLoadingButton->ShowWindow(true);

			// ֪ͨ�ں˽��е�¼
			::PostMessage(m_IeWnd, WM_AX_USER_AUTO_LOAD, 0, 0);
		}
	}
	return true;
}

// �Ƿ����û����ڵ�¼
bool CMainFrame::IsLoading()
{
	// ����ͨ�����ڵ�¼�İ�ť���ж�(��ȻҲ�������һ���������б��)
	if (NULL != m_pLoadingButton)
		return true;
	return false;
}

LRESULT CMainFrame::OnCloseCurUserDB(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &/*bHandled*/)
{
	CBankData::GetInstance()->CloseDB();
	return 0;
}

LRESULT CMainFrame::OnUpdateCurUserDB(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &/*bHandled*/) // UI���µ�ǰ�û���
{
	string strRead = (LPSTR)lParam;
	string strTag = MY_PARAM_END_TAG;

	
	// ��ȡstrPath
	string strPath = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

	// ��ȡPWD
	strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
	string strPwd = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));
	char chPwd[33] = {0};
	int nBack = 0;
	// ת����ʵ�ʵ�����
	FormatDecVal(strPwd.c_str(), chPwd, nBack);

#ifndef OFFICIAL_VERSION
	CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, L"BankUI SetCurrentUserDB" );
	wstring strw = CA2W(strPwd.c_str());
	CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, strw);
#endif

	if (!CBankData::GetInstance()->SetCurrentUserDB((LPSTR)strPath.c_str(), (LPSTR)chPwd, 32))
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_BANK_NAME, MY_ERROR_SQL_ERROR, L"BankUI SetCurrentUserDB error" );
	}

	return 0;
}

LRESULT CMainFrame::OnResendVerifyMail(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/)
{
	LRESULT lr = ::SendMessage(m_IeWnd, WM_RESEND_VERIFY_MAIL, wParam, lParam);

	if (S_OK == lr)
	{
		CString strMes;
		wstring str = CA2W(CBankData::GetInstance()->m_CurUserInfo.strmail.c_str());
		strMes.Format(L"����%s������֤�ʼ����뼰ʱ���ղ����ʼ�˵�������֤��", str.c_str()); 
		::mhMessageBox(m_hWnd, strMes, L"�ʼ��ط�", MB_OK | MB_ICONINFORMATION);
	}
	else if (S_FALSE == lr)
		::mhMessageBox(m_hWnd, L"��֤�ʼ�����ʧ�ܣ�������������״����", L"�ʼ��ط�", MB_OK | MB_ICONINFORMATION);
	else
	{
		//::mhMessageBox(m_hWnd, L"�Ѿ���֤��", L"�ʼ��ط�", MB_OK | MB_ICONINFORMATION);
		if (NULL != m_pUserInfoBtn)
			m_pUserInfoBtn->ShowWindow(false);
	}

	return 0;
}

LRESULT CMainFrame::OnSynchroBtnChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &/*bHandled*/)
{
	m_SynchroButton.KeepOnSelect(true);
	return 0;
}

LRESULT CMainFrame::OnInitSynchroBtnStatus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &/*bHandled*/)
{
	::PostMessage(FS()->TabItem()->GetChildFrame()->GetItem()->GetAxControl(), WM_AX_INIT_SYNCHRO_BTN, 0, 0);
	return 0;
}

LRESULT CMainFrame::OnChangeCurUserStoken(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &/*bHandled*/)
{
	string strParam = (char*)lParam;

	if (0 != strParam.length())
		CBankData::GetInstance()->m_CurUserInfo.strstoken = strParam;
	return 0;
}