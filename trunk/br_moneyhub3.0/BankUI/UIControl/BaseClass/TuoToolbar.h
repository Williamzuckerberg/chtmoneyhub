#pragma once


enum ToolType
{
	ToolType_Normal,			// ��ͨ��ť
	ToolType_Dropdown,			// Dropdown��ť
	ToolType_WithDropdown,		// ��ͨ��ť+DropDown
	ToolType_Toggle,			// ��״̬�İ�ť
	ToolType_Null,				// û��ť�����Ǹ�ռλ��
	ToolType_Sep				// �ָ���
};



struct ToolData
{
	ToolType eType;
	bool bEnabled;
	DWORD_PTR dwData;
	int iButtonWidth;
	int iDropDownPartWidth;		// һ����������ã�ֻ����TypeΪToolType_WithDropdownʱ����

	// ���³�ԱΪ�ڲ�ʹ��
	struct
	{
		int iXPos;
		bool bChecked;
	} inner;
};

typedef std::vector<ToolData> ToolVector;

#define WM_TUO_COMMAND				(WM_APP + 0x0111)
#define WM_TUO_CHEVRON_COMMAND		(WM_APP + 0x0112)

#define BTN_CHEVRON			-2

#define BTN_SEP_WIDTH		5


class CTuoToolBarCtrlBase
{

public:

	CTuoToolBarCtrlBase() : m_iLineNumberInRebar(0) {}

	void AddButton(ToolData *pToolData)
	{
		ToolData *pLastTool = m_toolVec.size() == 0 ? NULL : &m_toolVec[m_toolVec.size() - 1];
		m_toolVec.push_back(*pToolData);
	}

	void AddButtons(ToolData *pToolData, int iCount)
	{
		ToolData *pLastTool = m_toolVec.size() == 0 ? NULL : &m_toolVec[m_toolVec.size() - 1];
		int iCurrentPos = pLastTool ? pLastTool->inner.iXPos + pLastTool->iButtonWidth : 0;
		for (int i = 0; i < iCount; i++)
		{
			pToolData[i].inner.iXPos = iCurrentPos;
			if (pToolData[i].eType == ToolType_Sep)
				pToolData[i].iButtonWidth = BTN_SEP_WIDTH;
			iCurrentPos += pToolData[i].iButtonWidth;
			m_toolVec.push_back(pToolData[i]);
		}
	}

	void ClearAllButtons()
	{
		m_toolVec.clear();
	}

	int GetButtonCount() const { return m_toolVec.size(); }

	bool GetEnable(int iIndex) const { return m_toolVec[iIndex].bEnabled; }
	bool GetChecked(int iIndex) const { return m_toolVec[iIndex].inner.bChecked; }

	DWORD_PTR GetData(int iIndex) const { return m_toolVec[iIndex].dwData; }
	void SetData(int iIndex, DWORD_PTR dwData) { m_toolVec[iIndex].dwData = dwData; }

	void GetButtonRect(int iIndex, RECT *prect) const
	{
		::GetClientRect(m_hToolWnd, prect);
		prect->left = m_toolVec[iIndex].inner.iXPos;
		prect->right = prect->left + m_toolVec[iIndex].iButtonWidth;
	}

	int GetFullWidth() const
	{
		if (m_toolVec.size() == 0)
			return 0;
		else
		{
			const ToolData *pTool = &m_toolVec[m_toolVec.size() - 1];
			return pTool->inner.iXPos + pTool->iButtonWidth;
		}
	}
	int GetHeight() const { return m_iDefaultHeight; }
	void SetHeight(int iHeight) { m_iDefaultHeight = iHeight; }

	int GetButtonWidth(int iIndex) const { return m_toolVec[iIndex].iButtonWidth; }

	int GetLineNumberInRebar() const { return m_iLineNumberInRebar; }

	HWND m_hToolWnd;
	int m_iDefaultHeight;
	int m_iLineNumberInRebar;		// ��������rebar���������

protected:

	ToolVector m_toolVec;

};


template <class T>
class ATL_NO_VTABLE CTuoToolBarCtrl : public CWindowImpl<T>, public CTuoToolBarCtrlBase
{

public:

	CTuoToolBarCtrl() : m_bIsMouseInWindow(false), m_iCurrentIndex(INVALID_ITEM), m_iMouseDownIndex(INVALID_ITEM), m_bAnchorHighlight(false),
		m_bUseDropDownButton(false), m_bStartCapturing(false), m_iButtonDisplayIndex(0) {}


	void SetUseChevron(bool bUseChevron)
	{
		m_bUseChevron = bUseChevron;
	}


	void SetEnable(int iIndex, bool bEnable, bool bRedraw = true)
	{
		if (m_toolVec[iIndex].bEnabled == bEnable)			// ��״̬δ�䣬��ˢ��
			return;
		m_toolVec[iIndex].bEnabled = bEnable;
		if (bRedraw)
			RedrawButton(iIndex, true);
	}

	void SetChecked(int iIndex, bool bChecked, bool bRedraw = true)
	{
		ATLASSERT(m_toolVec[iIndex].eType == ToolType_Toggle);		// ֻ��toggle��ť֧�ִ�״̬
		if (m_toolVec[iIndex].inner.bChecked == bChecked)			// ��״̬δ�䣬��ˢ��
			return;
		m_toolVec[iIndex].inner.bChecked = bChecked;
		if (bRedraw)
			RedrawButton(iIndex, true);
	}

	void SetButtonDropDownPartWidth(int iIndex, int iWidth)
	{
		m_toolVec[iIndex].iDropDownPartWidth = iWidth;
	}
	void SetButtonWidth(int iIndex, int iWidth, bool bRedraw = false)
	{
		if (m_toolVec[iIndex].iButtonWidth == iWidth)			// �����δ�䣬��ˢ��
			return;
		m_toolVec[iIndex].iButtonWidth = iWidth;

		// ��ǰ��ť��ȱ��ˣ���Ҫˢ�´˰�ť����İ�ť
		RECT rcClient;
		GetClientRect(&rcClient);
		rcClient.left = m_toolVec[iIndex].inner.iXPos;
		T *pT = static_cast<T*>(this);
		if (bRedraw && rcClient.right > rcClient.left)
		{
			CClientDC dc(m_hWnd);
			CMemoryDC memDC(dc, rcClient);
			pT->DrawBackground(memDC, rcClient);
			for (size_t i = iIndex; i < m_toolVec.size(); i++)
			{
				m_toolVec[i].inner.iXPos = rcClient.left;
				rcClient.right = rcClient.left + m_toolVec[i].iButtonWidth;
				if (m_toolVec[i].eType != ToolType_Null)			// ����ռλ����ť���û���
				{
					if (i < (size_t)m_iButtonDisplayIndex)			// ֻ�а�ť�ڿ���ʾ��Χ�ڲŻ���
					{
						if (m_toolVec[i].eType == ToolType_Sep)
							DrawButtonSep(memDC, rcClient);
						else
							pT->DrawButton(memDC, i, rcClient, GetStatusOfButton(i));
					}
				}
				rcClient.left = rcClient.right;
			}
		}
		else
		{
			T *pT = static_cast<T*>(this);
			for (size_t i = iIndex; i < m_toolVec.size(); i++)
			{
				m_toolVec[i].inner.iXPos = rcClient.left;
				rcClient.right = rcClient.left + m_toolVec[i].iButtonWidth;
				rcClient.left = rcClient.right;
			}
		}

		// ��������С�����仯����Ҫ���¼�����ʾ�Ĺ������ı߽�
		RECT rc;
		GetClientRect(&rc);
		if (m_toolVec.size() > 0 && m_toolVec[m_toolVec.size() - 1].inner.iXPos + m_toolVec[m_toolVec.size() - 1].iButtonWidth > rc.right)
		{
			// �������İ�ť��ʾ���£�����Ҫ�ض�
			int iCurrentWidth = 0;
			for (size_t i = 0; i < m_toolVec.size(); i++)
			{
				iCurrentWidth += m_toolVec[i].iButtonWidth;
				if (iCurrentWidth > rc.right)
				{
					m_iButtonDisplayIndex = i;
					break;
				}
			}
		}
		else
			m_iButtonDisplayIndex = m_toolVec.size();
	}

	// ��ȡ�����߽緶Χ���Ǹ���ť���±�
	int GetDisplayBoundButton() const		{ return m_iButtonDisplayIndex; }

	// ��Ϊtrue�����ʾ��ʹ����뿪�˹���������ť��ʾ�ڡ���������桱��״̬
	void SetAnchorHighlight(bool bAnchorHighlight)
	{
		m_bAnchorHighlight = bAnchorHighlight;
	}

	int HitTest(const POINT &pt) { return GetIndexByMousePosition(pt); }
	int GetHotItem() const { return m_iCurrentIndex; }
	void SetHotItem(int iIndex)
	{
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE | TME_CANCEL, m_hWnd };
		::TrackMouseEvent(&tme);
		m_bIsMouseInWindow = false;
		ChangeCurrentIndex(iIndex, true);
	}
	void PressButton(int iIndex)
	{
		m_iMouseDownIndex = iIndex;
	}


	void RefreshAfterAddButtonOrSizeChange(bool bForceRedraw = false)
	{
		int iOldButtonDisplayIndex = m_iButtonDisplayIndex;
		// ����������С�����仯ʱ����Ҫ���¼�����ʾ�Ĺ������ı߽�
		RECT rc;
		GetClientRect(&rc);
		if (m_toolVec.size() > 0 && m_toolVec[m_toolVec.size() - 1].inner.iXPos + m_toolVec[m_toolVec.size() - 1].iButtonWidth > rc.right)
		{
			// �������İ�ť��ʾ���£�����Ҫ�ض�
			int iCurrentWidth = 0;
			for (size_t i = 0; i < m_toolVec.size(); i++)
			{
				iCurrentWidth += m_toolVec[i].iButtonWidth;
				if (iCurrentWidth > rc.right)
				{
					m_iButtonDisplayIndex = i;
					break;
				}
			}
		}
		else
			m_iButtonDisplayIndex = m_toolVec.size();
return;
		T *pT = static_cast<T*>(this);
		CClientDC dc(m_hWnd);
		GetClientRect(&rc);
		if (!bForceRedraw && iOldButtonDisplayIndex == m_iButtonDisplayIndex)
		{
			rc.left = m_iButtonDisplayIndex == 0 ? 0 : m_toolVec[m_iButtonDisplayIndex - 1].inner.iXPos + m_toolVec[m_iButtonDisplayIndex - 1].iButtonWidth;
			CMemoryDC memDC(dc, rc);
			pT->DrawBackground(memDC, rc);
			return;
		}

		if (iOldButtonDisplayIndex > m_iButtonDisplayIndex)
		{
			// ���˵��toolbar����ˣ���Ҫ����ʾ��һ��İ�ť�ñ���ɫ����
			rc.left = m_iButtonDisplayIndex == 0 ? 0 : m_toolVec[m_iButtonDisplayIndex - 1].inner.iXPos + m_toolVec[m_iButtonDisplayIndex - 1].iButtonWidth;
			CMemoryDC memDC(dc, rc);
			pT->DrawBackground(memDC, rc);
		}
		else
		{
			// ���˵��toolbar�䳤�ˣ���Ҫ����������ʾ�����İ�ť
			rc.left = iOldButtonDisplayIndex == 0 ? 0 : m_toolVec[iOldButtonDisplayIndex - 1].inner.iXPos + m_toolVec[iOldButtonDisplayIndex - 1].iButtonWidth;
			CMemoryDC memDC(dc, rc);
			pT->DrawBackground(memDC, rc);
			for (int i = iOldButtonDisplayIndex; i < m_iButtonDisplayIndex; i++)
			{
				if (m_toolVec[i].eType == ToolType_Null)
					continue;
				rc.left = m_toolVec[i].inner.iXPos;
				rc.right = rc.left + m_toolVec[i].iButtonWidth;
				if (m_toolVec[i].eType == ToolType_Sep)
					DrawButtonSep(memDC, rc);
				else
					pT->DrawButton(memDC, i, rc, GetStatusOfButton(i));
			}
		}
	}


	BEGIN_MSG_MAP_EX(T)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MSG_WM_CAPTURECHANGED(OnCaptureChanged)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		bHandled = FALSE;
		m_hToolWnd = m_hWnd;
		CREATESTRUCT *pCS = (CREATESTRUCT*)lParam;
		m_iDefaultHeight = pCS->cy;
		return 0;
	}

	LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		// ���ﻹ�и��ط��Ƚϻ�ɬ���������WM_WINDOWPOSCHANGED��Ϣ���ڵ���ShowWindowʱ��������һ��
		bHandled = FALSE;
		WINDOWPOS *pPos = (WINDOWPOS*)lParam;
		if ((pPos->flags & SWP_NOSIZE) == 0)
		{
			RefreshAfterAddButtonOrSizeChange();
			Invalidate();
		}
		return 0;
	}


	void OnPaint(CDCHandle dc, RECT rcClient)		// һ���Ի������а�ť
	{
		GetClientRect(&rcClient);
		CMemoryDC memDC(dc, rcClient);
		T *pT = static_cast<T*>(this);
		pT->DrawBackground(memDC, rcClient);
		for (int i = 0; i < m_iButtonDisplayIndex; i++)		// ֻ������Щ��ʾ���µİ�ť
		{
			if (m_toolVec[i].eType == ToolType_Null)
				continue;
			rcClient.left = m_toolVec[i].inner.iXPos;
			rcClient.right = rcClient.left + m_toolVec[i].iButtonWidth;
			if (m_toolVec[i].eType == ToolType_Sep)
				DrawButtonSep(memDC, rcClient);
			else
				pT->DrawButton(memDC, i, rcClient, GetStatusOfButton(i));
		}
	}

	void OnMouseMove(UINT nFlags, CPoint point)
	{
		if (!m_bIsMouseInWindow)		// ���� WM_MOUSELEAVE ����Ϣ
		{
			TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd };
			if (::TrackMouseEvent(&tme))
				m_bIsMouseInWindow = true;
		}

		if (m_iMouseDownIndex >= 0)		// �������ǰ��µģ���ֻ�а��µ��Ǹ���ť״̬�����ı�
		{
			int iCurrentIndex = GetIndexByMousePosition(point);
			if (iCurrentIndex != m_iMouseDownIndex)		// �û�������겻�ţ�ȴ�뿪�˰�ť�����ǾͰѰ�ť�ָ���ԭʼ״̬
			{
				if (m_iCurrentIndex != INVALID_ITEM)
				{
					m_iCurrentIndex = INVALID_ITEM;
					RedrawButton(m_iMouseDownIndex, true);
				}
			}
			else									// ����û�����ַŻ��˰�ť����Ѱ�ť״̬��Ϊ����
			{
				if (m_iCurrentIndex != m_iMouseDownIndex)
				{
					m_iCurrentIndex = m_iMouseDownIndex;
					RedrawButton(m_iMouseDownIndex, true);
				}
			}
			return;
		}

		int iMousePos = GetIndexByMousePosition(point);
		if (iMousePos != INVALID_ITEM || !m_bAnchorHighlight)			// ���m_bAnchorHighlight=true����ʾ����Ƴ���ҲҪ�������һ���������İ�ť�ĸ���״̬
			ChangeCurrentIndex(iMousePos);
//		if (iMousePos >= 0 && m_toolVec[iMousePos].eType != ToolType_Null && m_toolVec[iMousePos].eType != ToolType_Sep)
//			CSogouToolTip::GetTooltip()->RelayEventSingleton(m_hWnd, iMousePos);
//		else
//			CSogouToolTip::GetTooltip()->RelayEventSingleton(m_hWnd, INVALID_ITEM);
	}

	void OnMouseLeave()
	{
		m_bIsMouseInWindow = false;
		if (!m_bAnchorHighlight)			// ���m_bAnchorHighlight=true����ʾ����Ƴ���ҲҪ�������һ���������İ�ť�ĸ���״̬
			ChangeCurrentIndex(INVALID_ITEM);
//		CSogouToolTip::GetTooltip()->RelayEventSingleton(m_hWnd, INVALID_ITEM);
	}

	void OnLButtonDown(UINT nFlags, CPoint point)
	{
//		CSogouToolTip::GetTooltip()->RelayEventSingleton(m_hWnd, INVALID_ITEM);

		int iNewIndex = GetIndexByMousePosition(point);
		if (iNewIndex == INVALID_ITEM)			// ��갴�µ�ʱ�����˿հ״�
		{
			ChangeCurrentIndex(INVALID_ITEM);
			return;
		}

		ToolData *pTool = &m_toolVec[iNewIndex];
		if (IsButtonActivate(pTool))
		{
			bool bUseDropDown = false;
			if (pTool->eType == ToolType_WithDropdown)
			{
				if (point.x >= pTool->inner.iXPos + pTool->iDropDownPartWidth)
				{
					bUseDropDown = true;
					m_bUseDropDownButton = true;
				}
			}
			else if (pTool->eType == ToolType_Dropdown)
				bUseDropDown = true;

			if (bUseDropDown)
			{
				m_iMouseDownIndex = iNewIndex;
				ChangeCurrentIndex(iNewIndex, true);

				TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE | TME_CANCEL, m_hWnd };
				::TrackMouseEvent(&tme);

				SendMessage(WM_TUO_COMMAND, MAKELONG(iNewIndex, NM_DROPDOWN), MAKELPARAM(point.x, point.y));

				tme.dwFlags = TME_LEAVE;
				::TrackMouseEvent(&tme);

				m_bUseDropDownButton = false;
				m_iMouseDownIndex = INVALID_ITEM;
				::GetCursorPos(&point);		// ���»�ȡ�������
				ScreenToClient(&point);
				ChangeCurrentIndex(GetIndexByMousePosition(point), true);

				// ��������ڵ�ǰչ���˵��İ�ť���ٵ�һ��ʱ���˵�������ʧ������һ��֮�������ٵ�������
				// Ϊ�˱����������󣬼�����������
				MSG msg;
				RECT rect;
				GetButtonRect(iNewIndex, &rect);
				ClientToScreen(&rect);
				if (::PeekMessage(&msg, m_hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE) && ::PtInRect(&rect, msg.pt))
					::PeekMessage(&msg, m_hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE);
			}
			else
			{
				m_iMouseDownIndex = iNewIndex;
				ChangeCurrentIndex(iNewIndex, true);
				m_bStartCapturing = true;
				SetCapture();
			}
		}
		else			// ��갴��ʱ��������disabled�İ�ť��
			ChangeCurrentIndex(INVALID_ITEM);
	}

	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		if (m_iMouseDownIndex < 0)		// �˴����á�== INVALID_ITEM�������жϵ�ԭ���ǣ�chevron��dropdown������click
			return;
		int iCurrentIndex = GetIndexByMousePosition(point);
		ToolData *pTool = iCurrentIndex == m_iMouseDownIndex ? &m_toolVec[iCurrentIndex] : NULL;
		m_iMouseDownIndex = INVALID_ITEM;
		if (pTool && pTool->eType == ToolType_Toggle)		// �����toggle��ť����Ҫ�ı�״̬
			pTool->inner.bChecked = !pTool->inner.bChecked;
		ChangeCurrentIndex(iCurrentIndex, true);
		m_bStartCapturing = false;
		::ReleaseCapture();

//		if (iCurrentIndex >= 0 && m_toolVec[iCurrentIndex].eType != ToolType_Null && m_toolVec[iCurrentIndex].eType != ToolType_Sep)
//			CSogouToolTip::GetTooltip()->RelayEventSingleton(m_hWnd, iCurrentIndex);
//		else
//			CSogouToolTip::GetTooltip()->RelayEventSingleton(m_hWnd, INVALID_ITEM);

		if (pTool && IsButtonActivate(pTool) && (pTool->eType == ToolType_Normal || pTool->eType == ToolType_Toggle
			|| (pTool->eType == ToolType_WithDropdown && point.x < pTool->inner.iXPos + pTool->iDropDownPartWidth)))
		{
			if (SendMessage(WM_TUO_COMMAND, MAKELONG(iCurrentIndex, NM_CLICK), MAKELPARAM(point.x, point.y)))
				RedrawButton(iCurrentIndex, true);
		}
	}

	LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		int iCurrentIndex = GetIndexByMousePosition(pt);
		if (iCurrentIndex >= 0)			// �˴����á�!= INVALID_ITEM�������жϵ�ԭ���ǣ�chevron��֧���Ҽ�
		{
			ToolData *pTool = &m_toolVec[iCurrentIndex];
			if (IsButtonActivate(pTool) && pTool->eType != ToolType_Null && pTool->eType != ToolType_Sep)
			{
				if (SendMessage(WM_TUO_COMMAND, MAKELONG(iCurrentIndex, NM_RCLICK), lParam) != 0)
					return 0;
			}
		}
		bHandled = FALSE;
		return 0;
	}


	void OnCaptureChanged(CWindow wnd)
	{
		if (m_bStartCapturing)
		{
			// ������õ������ˣ�˵��û��ִ�е�LButtonUp����ȡ�����а�ť��״̬
			m_bStartCapturing = false;
			m_iMouseDownIndex = INVALID_ITEM;
			ChangeCurrentIndex(INVALID_ITEM);
		}
	}


private:

	// �ػ浥����ť
	void RedrawButton(int iIndex, bool bDrawBackground)
	{
		if (iIndex >= m_iButtonDisplayIndex)		// ���Ҫ���Ƶİ�ť�����߽�Ͳ�����
			return;
		T *pT = static_cast<T*>(this);
		if (!pT->IsWindowVisible())					// ������������صģ����Բ��û���
			return;

		RECT rc;
		GetClientRect(&rc);

		ToolData *pToolData = &m_toolVec[iIndex];
		if (pToolData->eType == ToolType_Null)			// ������ͱ�ʾ��ťֻ��һ��ռλ���������л���
			return;
		rc.left = pToolData->inner.iXPos;
		rc.right = rc.left + pToolData->iButtonWidth;
		CClientDC dc(m_hWnd);
		if (bDrawBackground)
		{
			CMemoryDC memDC(dc, rc);
			pT->DrawBackground(memDC, rc);
			if (pToolData->eType == ToolType_Sep)
				DrawButtonSep(memDC, rc);
			else
				pT->DrawButton(memDC, iIndex, rc, GetStatusOfButton(iIndex));
		}
		else
		{
			if (pToolData->eType == ToolType_Sep)
				DrawButtonSep(dc, rc);
			else
				pT->DrawButton(dc, iIndex, rc, GetStatusOfButton(iIndex));
		}
	}

	// ���Ʒָ���
	void DrawButtonSep(HDC hDC, const RECT &rc) const
	{
		s()->Toolbar()->DrawToolBarSep(hDC, rc);
	}

	// ������������ȡ���ڰ�ť���±�
	int GetIndexByMousePosition(POINT pt) const
	{
		if (m_toolVec.empty())
			return INVALID_ITEM;
		RECT rcClient;
		GetClientRect(&rcClient);
		if (pt.x < 0 || pt.x > rcClient.right || pt.y < 0 || pt.y > rcClient.bottom)
			return INVALID_ITEM;
		ATLASSERT(m_iButtonDisplayIndex > 0 && m_iButtonDisplayIndex <= (int)m_toolVec.size());

		// �ж��Ƿ���������һ����ť�ϣ�Ϊ�����۰���ң���Ҫ�Ȱ����һ����ĺ�벿��ȥ����
		const ToolData *pLastToolData = &m_toolVec[m_iButtonDisplayIndex - 1];
		if (pt.x >= pLastToolData->inner.iXPos)
		{
			if (pt.x > pLastToolData->inner.iXPos + pLastToolData->iButtonWidth)
			{
				// ���λ�ڵ�ǰ��ʾ�����һ����ť���ұߣ���������һ���Ƿ�λ��chevron��ť��
				return INVALID_ITEM;
			}
			else
				return m_iButtonDisplayIndex - 1;
		}

		int iBeginLeft = 0, iBeginRight = m_iButtonDisplayIndex - 1;		// ������ұ߽��趨Ϊ�ܹ���ʾ���µİ�ť����Ϊ����ǲ����Ƶ���Щ��ʾ���µİ�ť�ϵģ�
		while (iBeginLeft < iBeginRight - 1)
		{
			int iMid = (iBeginLeft + iBeginRight) / 2;
			if (pt.x > m_toolVec[iMid].inner.iXPos)		// ��껹Ҫ����
				iBeginLeft = iMid;
			else if (pt.x < m_toolVec[iMid].inner.iXPos)	// ��껹Ҫ����
				iBeginRight = iMid;
			else
				return iMid;
		}
		return iBeginLeft;
	}

	// ��ȡָ���İ�ť��ǰ��״̬
	UINT GetStatusOfButton(int iButtonIndex) const
	{
		const ToolData *pTool = &m_toolVec[iButtonIndex];
		if (pTool->bEnabled)
		{
			UINT uCheckBtn = pTool->eType == ToolType_Toggle && pTool->inner.bChecked ? BTN_STATUS_CHECKED : 0;
			if (iButtonIndex == m_iCurrentIndex)
			{
				if (m_iMouseDownIndex == m_iCurrentIndex)
					return uCheckBtn | (m_bUseDropDownButton ? BTN_STATUS_DROPDOWN : 0) | BTN_STATUS_MOUSEDOWN;
				else
					return uCheckBtn | BTN_STATUS_MOUSEOVER;
			}
			else
				return uCheckBtn | BTN_STATUS_MOUSEOUT;
		}
		else
			return BTN_STATUS_DISABLED;
	}

	// ��һ����ť�Ƿ������Ӧ����¼�
	bool IsButtonActivate(ToolData *pTool) const
	{
		if (!pTool->bEnabled)
			return false;
		if (pTool->eType == ToolType_Null || pTool->eType == ToolType_Sep)
			return false;
		return true;
	}

	// �ı䲢���Ƶ�ǰ��ť����Ҫ��ԭ�ȼ���İ�ť�ָ����ڻ����µļ��ť��
	void ChangeCurrentIndex(int iNewIndex, bool bForceRefresh = false)
	{
		if (m_iCurrentIndex == iNewIndex)
		{
			if (bForceRefresh && iNewIndex != INVALID_ITEM)
				RedrawButton(iNewIndex, true);
			return;
		}
		int iOldIndex = m_iCurrentIndex;
		m_iCurrentIndex = iNewIndex;
		if (iOldIndex != INVALID_ITEM)
			RedrawButton(iOldIndex, true);
		if (m_iCurrentIndex != INVALID_ITEM)
			RedrawButton(m_iCurrentIndex, true);
	}


	bool m_bIsMouseInWindow;
	int m_iCurrentIndex;
	int m_iMouseDownIndex;
	bool m_bUseDropDownButton;		// ����갴��ʱ�������˰�ť��dropdown������

	bool m_bAnchorHighlight;		// ���Ϊtrue������ζ�ż�ʹ����뿪�˹���������ťҲ��ʾΪ����������桱��״̬
	bool m_bStartCapturing;			// ִ�й�SetCapture��������û��ִ��ReleaseCapture

	int m_iButtonDisplayIndex;		// �����޵Ĵ�������ʾ�����һ����ť���±꣨��Ϊ�������ڿ�ȵİ�ť����Ҫ�ٻ����ˣ�

protected:

	// overridable methods

	void DrawBackground(HDC hDC, const RECT &rc) { ATLASSERT(0 && "Please override me!"); }
	void DrawButton(HDC hDC, int iIndex, const RECT &rc, UINT uCurrentStatus) { }
};

//////////////////////////////////////////////////////////////////////////

#define TUO_COMMAND_HANDLER_EX(id, code, func) \
	if (uMsg == WM_TUO_COMMAND && (WORD)code == HIWORD(wParam) && (WORD)id == LOWORD(wParam)) \
	{ \
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }; \
		lResult = func(pt); \
		return TRUE; \
	}

#define TUO_COMMAND_CODE_HANDLER_EX(code, func) \
	if (uMsg == WM_TUO_COMMAND && (WORD)code == HIWORD(wParam)) \
	{ \
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }; \
		lResult = func((int)LOWORD(wParam), pt); \
		return TRUE; \
	}

#define TUO_CHEVRON_COMMAND(func) \
	if (uMsg == WM_TUO_CHEVRON_COMMAND) \
	{ \
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }; \
		lResult = func(pt); \
		return TRUE; \
	}
