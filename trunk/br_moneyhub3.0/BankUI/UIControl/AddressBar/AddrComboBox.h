#pragma once
#include "../BaseClass/EditContainer.h"
#include "AddrBarAdditionTool.h"
#include "AddrDropDownList.h"
#include "AddrComboBoxEdit.h"


struct CWaitSearchResultTimer 
{
	DWORD uTimer;
	DWORD uStart;
	DWORD uWeight;
	CString strWord;
};

class CAddressBarCtrl;

class CAddressComboBoxCtrl : public CEditContainerBase<CAddressComboBoxCtrl>, public CFSMUtil
{

public:

	CAddressComboBoxCtrl(FrameStorageStruct *pFS);

	void CreateComboBox(HWND hParent);
	void AutoCompleteUserInput();

	bool IsInAutoCompletionMode() const		{ return m_bAlreadyAutoComplete; }

	void UpdateIcon(HICON hIcon);
	void UpdateURLText(LPCTSTR lpszURL, bool bForce);
	void UpdateResourceStatus(int iResType, int iCount);
	void UpdateDisplayResourceIcon();

	void DoSelectAddressComboBar();
	void DoShowDropDownList();

	void OnDropDownListHide();
	void GetSearchRecommend(std::wstring& strUrl, std::wstring& strWord);
	BOOL IsSearchComplete(const std::wstring& strWord);

	void StartNavigate(int iComeFromSource, int iShortcutKeyType, LPCTSTR lpszText);
	void OnEditBoxUserChange();
	void AddrEditKillFocus();
	int GetDropDownSelection();
	void IMENotify(bool bStartIME);

	void PrepareQuickComplete(bool bForce);
	void Refresh();

	// callback

	void PaintBackground(CDCHandle dc, RECT rc);
	void GetLeftAndRightMargin(int &left, int &right);
	bool IsTransparentDraw() const;

private:

	void ResetWaitSearch();
	void OpenRecommend();
	void OpenPage(LPCTSTR url, int iWeight);

	// message

	BEGIN_MSG_MAP_EX(CAddressComboBoxCtrl)
		CHAIN_MSG_MAP(CEditContainerBase<CAddressComboBoxCtrl>)

		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_LBUTTONDOWN(OnLButtonDownAndDblClk)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDownAndDblClk)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_CAPTURECHANGED(OnCaptureChanged)

		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER_EX(WM_SETTEXT, OnSetText)
		MESSAGE_HANDLER_EX(WM_TIMER, OnTimer)

		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	// message handler

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnSize(UINT nType, CSize size);
	void OnLButtonDownAndDblClk(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnCaptureChanged(CWindow wnd);

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnSetDropDownContent(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	LRESULT OnAddDropDownContent(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);

	LRESULT OnSetSearchRecommend(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	
	// members

	CAddressComboBoxEditCtrl m_editCtrl;

	// �û��Ƿ��ڲ�ȫ״̬
	// �û����������ҡ�pageup��pagedown����������κ�ȡ����ȫѡȡ�Ĳ��������˳���ȫ״̬
	bool m_bAlreadyAutoComplete;

	// �û��������롢���б���ѡ�񡢲�ȫ֮�࣬��ʱ������browser�����ı��url������
	// ���û����س����url�����߰�esc����ԭʼ��url�����߽��㲻�ڵ�ַ����ʱ�򣬴�ֵΪfalse
	bool m_bUserTyping;

	CString m_strTextBeforeAutoCompletion;		// Ӧ���Զ���ȫ֮ǰ�û����������
	CString m_strCurrentURL;

	CAddrDropDownList m_DropDown;
	CAddrAdditionTool m_tool;

	CString m_strSearchRecommendUrl;
	CString m_strSearchRecommendWord;
	CString m_strSearchResultWord;

	std::vector<std::pair<std::wstring, std::wstring>> m_QuickCompleteStr;

	CWaitSearchResultTimer m_WaitSearch;

public:

	DECLARE_WND_CLASS(_T("MH_TuotuoAddressBarComboBox"))
};
