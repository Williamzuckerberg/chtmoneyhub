#include "stdafx.h"
#include "TuoImage.h"
#include "SkinLoader.h"


CSkinLoader *g_pSkinLoader = NULL;
bool CSkinLoader::sm_bIsSupportAreoEffect = false;
bool CSkinLoader::sm_bIsVista = false;


CSkinLoader::CSkinLoader() : m_eDisplayBorder((BorderEffect)-1), m_iNeedDuplicateFrame(0)
{
	g_pSkinLoader = this;
	RefreshAeroStatus();
	OSVERSIONINFO os = { sizeof(OSVERSIONINFO) };
	::GetVersionEx(&os);
	sm_bIsVista = os.dwMajorVersion >= 6;
}


void CSkinLoader::LoadSkinProperty()
{
	BorderEffect eOldBorder = m_eDisplayBorder;

	m_propertyReader.ReadINIData();

	// ��ȡ����͸���߿������
	switch (CSkinProperty::GetDWORD(Skin_Property_Use_Aero_Effect))
	{
		case 1:	// XP�����Ի棬vista����ʹ��ϵͳ�Դ��߿�
			m_eDisplayBorder = IsSupportAeroEffect() ? (CSkinProperty::GetDWORD(Skin_Property_Aero_Type) != 0 ? Border_AdvanceAero : Border_SystemBorder) : Border_OwnerDrawn;
			break;
		case 2:		// XP��vista���涼ʹ��ϵͳ�Դ��߿�
			m_eDisplayBorder = CSkinProperty::GetDWORD(Skin_Property_Aero_Type) != 0 && IsSupportAeroEffect() ? Border_AdvanceAero : Border_SystemBorder;
			break;
		case 0: // ����ϵͳ���棬�������Ի�߿�
		default:
			m_eDisplayBorder = Border_OwnerDrawn;
			break;
	}

	// ���ڴ�����ģʽ�л���Border_AdvanceAeroģʽ����Ҫ���´���frame������������һ��������¼
	// һ�������������ʹ�ù�һ��Border_AdvanceAeroģʽ���Ժ�Ͳ���Ҫ�����´�����
	if (m_iNeedDuplicateFrame != 2)
	{
		if (eOldBorder == Border_AdvanceAero)
			m_iNeedDuplicateFrame = 2;
		else
		{
			if (m_eDisplayBorder == Border_AdvanceAero && eOldBorder != Border_AdvanceAero && eOldBorder != (BorderEffect)-1)
				m_iNeedDuplicateFrame = 1;
			else
				m_iNeedDuplicateFrame = 0;
		}
	}

	if (m_eDisplayBorder == Border_AdvanceAero)
	{
		m_bStatusbarAero = CSkinProperty::GetDWORD(Skin_Property_Statusbar_Aero) != 0;
		m_iRebarAeroHeight = CSkinProperty::GetDWORD(Skin_Property_Rebar_Aero_Type);
		m_bTabAero = CSkinProperty::GetDWORD(Skin_Property_Tabbar_Aero) != 0;
		if (m_iRebarAeroHeight == 0)
			m_iRebarAeroHeight = 1;
		else if (m_iRebarAeroHeight >= 4)
			m_iRebarAeroHeight = 10;
	}
	else
	{
		m_iRebarAeroHeight = -1;
		m_bStatusbarAero = false;
		m_bTabAero = false;
	}
}

//////////////////////////////////////////////////////////////////////////

void CSkinLoader::Initialize()
{
	ThreadCacheDC::InitializeThreadCacheDC();
	new CSkinLoader();
}

//////////////////////////////////////////////////////////////////////////

void CSkinLoader::RefreshAeroStatus()
{
	sm_bIsSupportAreoEffect = ::IsAeroEffectEnabled();
}
