#pragma once
#include <tuodwmapi.h>
#include "../../../Utils/ListManager/ListManager.h"
#include "../../Skin/SkinManager.h"
#include "CategoryCtrl.h"

class CTuotuoCategoryCtrl;
class CChildFrame;

class CCategoryItem
{
	friend class CTuotuoCategoryCtrl;

public:
	CCategoryItem(CTuotuoCategoryCtrl *pCateCtrl);

	// ��tab��iDeltaHeight��ʾ��chromeƤ���£���ǩ�����ϲ��ճ����ĸ߶�
	void DoPaint(CDCHandle dc, ButtonStatus eTabStatus, CategoryPosition btnPos, ButtonStatus eBtn, bool bDrawCloseBtn, int iDeltaHeight);

	const CWebsiteData* GetWebData() const		{ return m_pWebData; }

	// ����
	LPCTSTR GetText() const;
	void SetText(LPCTSTR lpszText);

	// ���
	void SetNewWidthImmediately();

	// λ��
	void SetPosImmediately(int iPos);

	// ����
	size_t GetIndex() const				{ return m_nIndex; }
	void SetIndex(size_t nIndex)		{ m_nIndex = nIndex; }

	int GetShowInfoState() const		{ return m_bShowInfo; }//��ʾ
	void SetShowInfo(int bShowInfo)		{ m_bShowInfo = bShowInfo; }//��ʾ
	void SetShowInfo(wstring ShowInfo)	{ m_ShowInfo = ShowInfo; }//��ʾ
	wstring GetShowInfo()				{ return m_ShowInfo; }

	// �Ƿ���ʾ���밲ȫɫ
	bool ShowSecureLock();
	bool ShowSecureColor();

	// �ػ�
	void Redraw()						{ m_pCateCtrl->Invalidate(); }

public:
	CTabItem *m_pSelectedItem;
	std::vector<CTabItem*> m_TabItems;
	wstring m_ShowInfo;

private:
	CTuotuoCategoryCtrl *m_pCateCtrl;
	bool m_bNoClose;//��ʾ��Ϣ

	int m_bShowInfo;

	int m_nIndex;
	std::tstring m_sText;

	int m_iXPos;
	int m_iWidth;

	const CWebsiteData *m_pWebData;
};
