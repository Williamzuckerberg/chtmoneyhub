#pragma once
#include "../../Util/ThreadStorageManager.h"
#include "../BaseClass/TuoToolbar.h"


#define WM_MENUBAR_ALT_KEYDOWN			(WM_USER + 10)


class CTuotuoMenuBarCtrl : public CTuoToolBarCtrl<CTuotuoMenuBarCtrl>, public CFSMUtil
{

public:

	CTuotuoMenuBarCtrl(FrameStorageStruct *pFS);

	void Refresh();

	void CreateCommandBarCtrl(HWND hParent);

	// override

	void DrawBackground(HDC hDC, const RECT &rc);
	void DrawButton(HDC hDC, int iIndex, const RECT &rc, UINT uCurrentStatus);

private:

	// Message map and handlers

	BEGIN_MSG_MAP_EX(CTuotuoMenuBarCtrl)
		CHAIN_MSG_MAP(CTuoToolBarCtrl<CTuotuoMenuBarCtrl>)
		MSG_WM_CREATE(OnCreate)
	END_MSG_MAP()

	// message handler

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSysCommand(UINT nID, LPARAM lParam);
	LRESULT OnMenuEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnDropDown(int iIndex, POINT pt);

	// members

	HWND m_hWndFocus;
	bool m_bDropDownWithFirstSelection;
	bool m_bHasFocus;					// ��ǰ�Ƿ����ڴ��ڻ�ȡ�����״̬
	bool m_bVisibleBeforeGetFocus;		// ��ý���֮ǰ�Ƿ�����ʾ״̬
	bool m_bIsAltKeyPressed;			// ̧��alt��֮ǰ���Ƿ��⵽��alt���µ���Ϣ


	DECLARE_WND_CLASS_NODRAW(_T("SE_TuotuoMenuBar"))
};
