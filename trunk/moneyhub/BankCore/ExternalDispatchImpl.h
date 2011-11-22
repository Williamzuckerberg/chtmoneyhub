/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  ExternalDispatchImpl.h
 *      ˵����  JS�ⲿ���ýӿ��������ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.10.27	���ź�ͨ	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */
#pragma once
class CAxControl;
#include "windows.h"


class CExternalDispatchImpl : public IDispatch
{

public :
	CExternalDispatchImpl(CAxControl *pAxControl);
	~CExternalDispatchImpl();

public :
	STDMETHOD(QueryInterface)(REFIID, void **);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	//IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS  *pDispParams, VARIANT  *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

private:
	// ���б��ص�¼
	int ExcuteLocalLoad(const char* pMail, const unsigned char* pKek, int nLen);

private:
	CAxControl* m_pAxControl;
	void RefreshPage(int nPage);	
	std::string m_strBkID;
	static std::map<std::string, std::string> m_mapParam;

public:
	static DWORD WINAPI DownloadBkCtrlThreadProc(LPVOID lpParam);
	static HWND m_hFrame[3];
	static HWND m_hAxui;
	static HANDLE m_logHandle;
	static CRect	s_rectClient;

	bool IsVista();

	//static CRITICAL_SECTION m_cs;

	static std::list<std::string> m_sstrVerctor;
	// �����Ƿ��Ѿ���ӵ��б���
	static bool IsInUserFavBankList(const std::string& strBkID);
	// ��¼�û��ղص�����
	static bool AddUserFavBank(const std::string& strBkID);
	// �Ƴ��û��ղص�����
	static bool RemoveUserFavBank(const std::string& strBkID);
};
