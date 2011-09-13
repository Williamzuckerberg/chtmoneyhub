#include "stdafx.h"
#include "TabCtrl.h"
#include "TabItem.h"
#include "TabDragDropSource.h"
#include "../MainFrame.h"


#define ID_DRAG_CURSOR			2
#define ID_CANNOT_DROP_CURSOR	1

#define SCROLL_ZONE_WIDTH		20


void CTuotuoTabCtrl::RegisterDragDrop()
{
	m_pDropTarget = new CTabDropTarget(m_hWnd);
	HRESULT hr = ::RegisterDragDrop(m_hWnd, m_pDropTarget);
	ATLASSERT(SUCCEEDED(hr));
}


void CTuotuoTabCtrl::DraggingItem(int iMouseX, int iMouseY)
{
	m_bDraggingSource = true;
	m_iDraggingItemIndex = m_pSelectedItem->GetIndex();
	m_nDragToPos = m_pSelectedItem->GetIndex();
	m_iBeginDragItemOffset = iMouseX - m_pSelectedItem->GetIndex() * m_iCurrentWidth - m_iScrollOffset;

	CIDropSource* pdsrc = new CIDropSource();
	pdsrc->AddRef();
	CIDataObject* pdobj = new CIDataObject(pdsrc);
	pdobj->AddRef();

	HWND hTargetFrame = NULL;
	FORMATETC fmtetc = { CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	CStringA str = m_pSelectedItem->GetURLText();
	STGMEDIUM medium = { TYMED_HGLOBAL };
	int iLen = str.GetLength() + 1 + sizeof(DragGlobalData);
	medium.hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, iLen);
	char *pMem = (char*)::GlobalLock(medium.hGlobal);
	strcpy_s(pMem, iLen - 1, str);
	DragGlobalData *pData = (DragGlobalData*)(pMem + iLen - sizeof(DragGlobalData));
	pData->pDraggingItem = m_pSelectedItem;
	pData->hDragToFrame = &hTargetFrame;
	pData->dwProcessID = ::GetCurrentProcessId();
//	CTabDraggingWindow::_()->SetParam(1);
	::GlobalUnlock(medium.hGlobal);
	pdobj->SetData(&fmtetc, &medium, TRUE);

	float fPercent = (float)m_iBeginDragItemOffset / (float)m_iCurrentWidth;
//	CTabDraggingWindow::_()->SetTabDragging(pData->pDraggingItem->GetText(), (HICON)pData->pDraggingItem->GetIcon(), pData->pDraggingItem->GetChildFrame()->FS()->MDI()->m_hWnd, pData->pDraggingItem->GetAxControl(), fPercent);

	RECT rcClient;
	GetClientRect(&rcClient);
	if (iMouseY > rcClient.bottom || iMouseY < 0)
	{
		m_pDraggingItem = m_pSelectedItem;
		DragItemOut();
	}

	bool bReturnFocusToCurrentPage = false;
	DWORD dwEffect;
	HRESULT hr = ::DoDragDrop(pdobj, pdsrc, DROPEFFECT_MOVE, &dwEffect);
//	CTabDraggingWindow::_()->SetParam(0);
//	CTabDraggingWindow::_()->StopTabDragging();
	pdsrc->Release();
	pdobj->Release();

	m_bDraggingSource = false;
	m_pDraggingItem = NULL;

	if (hr == DRAGDROP_S_DROP)
	{
		if (hTargetFrame)
		{
			DragItemEnd_Source(hTargetFrame);
			if (hTargetFrame == FS()->MainFrame()->m_hWnd)
				bReturnFocusToCurrentPage = true;			// ��������Լ��Ŀ�������϶�����黹����
		}
		else
		{
			DragItemCancel();
		}
	}
	else if (hr == DRAGDROP_S_CANCEL)
	{
		bReturnFocusToCurrentPage = true;			// �����ȡ����ק����黹���㵽iewnd
		DragItemCancel();
	}

	m_nDragToPos = -2;
}

void CTuotuoTabCtrl::DragItemEnter(CTabItem *pItem)
{
	ATLASSERT(m_pDraggingItem == NULL);
	m_pDraggingItem = pItem;
	m_pDraggingItem->ChangeItemParent(this);
	UpdateLayout();
	if (m_pDraggingItem != m_pSelectedItem)
		m_iBeginDragItemOffset = m_iCurrentWidth / 2;
}

void CTuotuoTabCtrl::DragItemOut()
{
	CTabItem *pOldDraggingItem = m_pDraggingItem;
	m_pDraggingItem = NULL;
	UpdateLayout();

	int iLeftPos = 0;
	for (size_t i = 0; i < m_TabItems.size(); i++)
	{
		CTabItem *pItem = m_TabItems[i];
		if (pItem != pOldDraggingItem)
		{
			//pItem->SetPos(iLeftPos);
			pItem->SetPosImmediately(iLeftPos);
			iLeftPos += m_iCurrentWidth;
		}
	}

	if (m_bDraggingSource)		// ��������ǩ�����ϳ���tab���Ǹ���ǩ����һ����tab�ϳ�ȥ����ζ���ϵ�����
		m_nDragToPos = -1;
	else
		m_nDragToPos = -2;
}

void CTuotuoTabCtrl::DragItemMoving(POINT ptCursor)
{
	m_pDraggingItem->SetPosImmediately(ptCursor.x - m_iBeginDragItemOffset + m_rcTabItemArea.left);

	int iMouseX = ptCursor.x;
	if (iMouseX >= m_rcTabItemArea.right)
		iMouseX = m_rcTabItemArea.right - 1;
	else if (iMouseX <= m_rcTabItemArea.left)
		iMouseX = m_rcTabItemArea.left + 1;
	iMouseX += - m_iScrollOffset - m_rcTabItemArea.left;
	if (iMouseX >= 0)
	{
		// ��굱ǰ���ڵڼ���tab��λ����
		size_t nTabIndex = iMouseX / m_iCurrentWidth;
		// ����ǰ�tab�Ƶ����������ϣ�����Ҫ���������϶������tab
		size_t nMaxPos = GetItemCountInDraggingMode();
		if (nTabIndex >= nMaxPos)
			nTabIndex = nMaxPos - 1;
		if (nTabIndex != m_nDragToPos)
		{
			m_nDragToPos = nTabIndex;
			for (size_t i = 0, c = 0; i < m_TabItems.size(); i++)
			{
				CTabItem *pItem = m_TabItems[i];
				if (c == m_nDragToPos)
					c++;
				if (pItem == m_pDraggingItem)
					c--;
				else
					//pItem->SetPos(c * m_iCurrentWidth);
					pItem->SetPosImmediately(c * m_iCurrentWidth);
				c++;
			}
		}
	}

	if (m_bOverflowLeft && ptCursor.x < m_rcTabItemArea.left + SCROLL_ZONE_WIDTH)
	{
		SetScrollOffsetImmediately(GetValidScrollPos(m_iScrollOffset + SCROLL_DELTA_DRAGGING, m_bOverflowLeft, m_bOverflowRight));
		SetScrollDir(-1);
	}
	else if (m_bOverflowRight && ptCursor.x > m_rcTabItemArea.right - SCROLL_ZONE_WIDTH)
	{
		SetScrollOffsetImmediately(GetValidScrollPos(m_iScrollOffset - SCROLL_DELTA_DRAGGING, m_bOverflowLeft, m_bOverflowRight));
		SetScrollDir(1);
	}
	else
		SetScrollDir(0);

	Invalidate();
}

void CTuotuoTabCtrl::DragItemEnd_Source(HWND hTargetFrame)
{
	// restore flags and kill timers
	SetScrollDir(0);

	if (hTargetFrame == FS()->MainFrame()->m_hWnd)		// ���Լ���tab����������ȥ
	{
		if (m_nDragToPos >= 0 && m_nDragToPos < m_TabItems.size())
		{
			if (m_nDragToPos != m_pSelectedItem->GetIndex())
			{
				if (m_pSelectedItem->GetIndex() > m_nDragToPos)
				{
					for (size_t i = m_pSelectedItem->GetIndex(); i > m_nDragToPos; i--)
					{
						m_TabItems[i] = m_TabItems[i - 1];
						m_TabItems[i]->SetIndex(i);
					}
				}
				else
				{
					for (size_t i = m_pSelectedItem->GetIndex(); i < m_nDragToPos; i++)
					{
						m_TabItems[i] = m_TabItems[i + 1];
						m_TabItems[i]->SetIndex(i);
					}
				}
				m_TabItems[m_nDragToPos] = m_pSelectedItem;
				m_pSelectedItem->SetIndex(m_nDragToPos);
			}
		}
		m_pSelectedItem->SetPosImmediately(m_pSelectedItem->GetAniPos() - m_iScrollOffset - m_rcTabItemArea.left);
		//m_pSelectedItem->SetPos(m_pSelectedItem->GetIndex() * m_iCurrentWidth);
		m_pSelectedItem->SetPosImmediately(m_pSelectedItem->GetIndex() * m_iCurrentWidth);
		m_nDragToPos = -2;
		EnsureVisible(m_pSelectedItem);
	}
//	else			// �϶������˵�tab����ȥ��
//		FS()->MDI()->DragTabToAnotherFrame(m_pSelectedItem, hTargetFrame);
}

void CTuotuoTabCtrl::DragItemEnd_Dest()
{
	// ��������Լ���frame�����϶������ǲ�������������洦��
	if (m_bDraggingSource)
		return;

	// ���������жϣ��Է�ֹ��־���Ŀ���
	if (m_nDragToPos > m_TabItems.size())
		m_nDragToPos = m_TabItems.size();

	SetScrollDir(0);
	CTabItem *pItem = m_pDraggingItem;
	m_pDraggingItem = NULL;
	InsertItem(m_nDragToPos, pItem);
	m_nDragToPos = -2;
	EnsureVisible(pItem);
}


void CTuotuoTabCtrl::DragItemCancel()
{
	SetScrollDir(0);
	int iLeftPos = 0;
	if (m_pSelectedItem)
	{
		m_pSelectedItem->ChangeItemParent(this);
		m_pSelectedItem->SetNewWidthImmediately();
		m_pSelectedItem->SetPosImmediately(m_iCurrentWidth * m_pSelectedItem->GetIndex());
	}

	for (size_t i = 0; i < m_TabItems.size(); i++)
	{
		//m_TabItems[i]->SetPos(iLeftPos);
		m_TabItems[i]->SetPosImmediately(iLeftPos);
		iLeftPos += m_iCurrentWidth;
	}
	m_nDragToPos = -2;
	UpdateLayout();
	EnsureVisible(m_pSelectedItem);
}


size_t CTuotuoTabCtrl::GetItemCountInDraggingMode() const
{
	if (m_bDraggingSource)		// �ѱ�ǩ�ϳ�ȥ���Ǹ�tab
	{
		if (m_pDraggingItem == NULL)		// �Լ���ǩ������Ǹ��Ѿ��ϳ�ȥ��
			return m_TabItems.size() - 1;
		else
			return m_TabItems.size();
	}
	else			// �ϵ�Ŀ��tab
	{
		if (m_pDraggingItem != NULL)		// ����������tab�ı�ǩ�ϵ����tab
			return m_TabItems.size() + 1;
		else
			return m_TabItems.size();
	}
}
