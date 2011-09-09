#include "stdafx.h"
#include "DownloadManagerThread.h"
#include "../../BankData/BankData.h"

CDownloadManagerThread::CDownloadManagerThread()
{
	m_lpCS = new CRITICAL_SECTION ();
	// ��ʼ���ٽ���
	InitializeCriticalSection(m_lpCS);

}

CDownloadManagerThread::~CDownloadManagerThread()
{
	// �ͷ��������ص��߳�
	DeleteAllDownLoadThread();

	delete m_lpCS;
	m_lpCS = NULL;
}

bool  CDownloadManagerThread::CreateDownLoadTask(PDOWN_LOAD_PARAM_NODE pNode)// (LPCTSTR lpszHWID, LPCTSTR lpszUrl, LPCTSTR lpszSaveFile, LPVOID lpPostData, DWORD dwPostDataLength, bool bCreateThread)
{/*
	m_UsbNode.nMid = pNode->dwUsbKeyParam.nMid;
	m_UsbNode.nPid = pNode->dwUsbKeyParam.nPid;
	m_UsbNode.nVid = pNode->dwUsbKeyParam.nVid;*/

	if (pNode->bCreateThread)
	{
		CDownloadAndSetupThread* pTempThread = new CDownloadAndSetupThread ();
		ATLASSERT (NULL != pTempThread);
		if (NULL == pTempThread)
			return false;

		::EnterCriticalSection (m_lpCS);
		m_pDLThreadList.insert (std::make_pair (pTempThread, false));
		::LeaveCriticalSection (m_lpCS);
	
		pTempThread->DownLoadAndSetupDlFile (pNode);
	}
	else
	{
		CDownloadAndSetupThread cdl;
		cdl.DownLoadAndSetupDlFile (pNode);
	}
	return true;
}

void CDownloadManagerThread::CancleAllDownload() // �û�ֹͣ����
{
	std::map<CDownloadAndSetupThread*, bool>::const_iterator it;
	::EnterCriticalSection (m_lpCS);
	for (it = m_pDLThreadList.begin (); it != m_pDLThreadList.end (); it ++)
	{
		CDownloadAndSetupThread* pTemp = (*it).first;
		ATLASSERT (NULL != pTemp);
		if (NULL == pTemp)
			continue;

		pTemp->CancleDownloadAndSetup ();
		std::wstring strSave = pTemp->GetBreakFilePath ();
		DeleteFile (strSave.c_str ());
	}
	::LeaveCriticalSection (m_lpCS);
}

void CDownloadManagerThread::PauseAllDownload() // �û���ͣ����
{
	std::map<CDownloadAndSetupThread*, bool>::const_iterator it;
	::EnterCriticalSection (m_lpCS);
	for (it = m_pDLThreadList.begin (); it != m_pDLThreadList.end (); it ++)
	{
		CDownloadAndSetupThread* pTemp = (*it).first;
		ATLASSERT (NULL != pTemp);
		if (NULL == pTemp)
			continue;

		pTemp->CancleDownload ();
	}
	::LeaveCriticalSection (m_lpCS);
}

CDownloadAndSetupThread* CDownloadManagerThread::GetDLThread(LPCTSTR lpszUrl)
{
	ATLASSERT (NULL != lpszUrl);
	if (NULL == lpszUrl)
		return NULL;

	std::map<CDownloadAndSetupThread*, bool>::const_iterator begin;
	::EnterCriticalSection (m_lpCS);
	begin = m_pDLThreadList.begin ();
	for (; begin != m_pDLThreadList.end (); begin ++)
	{
		CDownloadAndSetupThread* pFind = (*begin).first;
		ATLASSERT (NULL != pFind);
		if (NULL == pFind)
			continue;

		if (pFind->GetDownloadURL () == lpszUrl)
			return pFind;
	}
	::LeaveCriticalSection (m_lpCS);
	return NULL;
}

int CDownloadManagerThread::GetDownLoadThreadSize()
{
	return m_pDLThreadList.size ();
}

// ��ȡƽ������
int CDownloadManagerThread::GetAverageDownLoadProcess()
{
	UINT64 i64Read = 0, i64Total = 0;

	std::map<CDownloadAndSetupThread*, bool>::const_iterator begin;
	::EnterCriticalSection (m_lpCS);
	begin = m_pDLThreadList.begin ();
	for (; begin != m_pDLThreadList.end (); begin ++)
	{
		CDownloadAndSetupThread* pTemp = (*begin).first;
		ATLASSERT (NULL != pTemp);
		if (NULL == pTemp)
			continue;

		UINT64 i64TempRead = 0, i64TempTotal = 0;
		pTemp->ReadDownloadPercent (i64TempTotal, i64TempRead);
		i64Read += i64TempRead;
		i64Total += i64TempTotal;
	}
	::LeaveCriticalSection (m_lpCS);

	if (i64Read == 0 || i64Total == 0)
		return 0;

	return i64Read * 100/i64Total;
	
}

// ɾ���Ѿ���ɵ�����
void CDownloadManagerThread::DeleteAllDownLoadThread()
{
	std::map<CDownloadAndSetupThread*, bool>::const_iterator begin;

	::EnterCriticalSection (m_lpCS);
	while (true)
	{
		begin = m_pDLThreadList.begin ();
		if (begin == m_pDLThreadList.end ())
			break;

		m_pDLThreadList.erase(begin);	
	}
	//m_pDLThreadList.clear ();
	::LeaveCriticalSection (m_lpCS);
}

CDownloadAndSetupThread* CDownloadManagerThread::SetPercentAndGetAFinishDownloadThread(LPSTR lpBkID)
{
	//ATLASSERT(NULL != lpBkID);
	
	std::map<CDownloadAndSetupThread*, bool>::iterator begin;
	::EnterCriticalSection (m_lpCS);
	for (begin = m_pDLThreadList.begin (); begin != m_pDLThreadList.end (); begin ++)
	{
		CDownloadAndSetupThread* pTemp = (*begin).first;
		ATLASSERT (NULL != pTemp);
		if (NULL == pTemp)
			continue;

		if ((*begin).second)// ����Ѿ���ȡ����
			continue;

		int nPercent = pTemp->GetDownloadPercent();
		if (nPercent < 100)
		{
			if (emUsbKey == pTemp->GetDownloadKind ())
			{
				CBankData::GetInstance ()->UpdateUSB (pTemp->m_dlUSBParam.nVid, pTemp->m_dlUSBParam.nPid, pTemp->m_dlUSBParam.nMid, nPercent);
			}
			else if (emBkActiveX & pTemp->GetDownloadKind())
			{
				if (NULL != lpBkID)
					CBankData::GetInstance ()->SaveFav (lpBkID, nPercent);
			}
		}
		

		if (pTemp->IsFinished ())
		{
			// ������ص���USBKEY�������ȼ�����пؼ��Ƿ��Ѿ����غ���
			bool bBkCtrlSetup = true;
			if (emUsbKey == pTemp->GetDownloadKind ())
			{

				std::map<CDownloadAndSetupThread*, bool>::iterator subbegin;
				for (subbegin = m_pDLThreadList.begin (); subbegin != m_pDLThreadList.end (); subbegin ++)
				{
					CDownloadAndSetupThread* pSubTemp = (*subbegin).first;
					ATLASSERT (NULL != pSubTemp);
					if (NULL == pSubTemp)
						continue;

					if ((emBkActiveX & pSubTemp->GetDownloadKind ()) && !pSubTemp->IsFinished ()) 
					{
						bBkCtrlSetup = false;// ���ص����пؼ�����δ���
					}
				}
			}

			if (bBkCtrlSetup)
			{
				(*begin).second = true; // ����Ѿ���ȡ��
				::LeaveCriticalSection (m_lpCS);
				return pTemp;
			}
		}
	}

	::LeaveCriticalSection (m_lpCS);
	return NULL;
}

bool CDownloadManagerThread::AllDownloadThreadExit(void) // �������е��߳��Ƿ��Ѿ��˳�
{
	if (m_pDLThreadList.size () <= 0)
		return false;

	std::map<CDownloadAndSetupThread*, bool>::const_iterator begin;
	::EnterCriticalSection (m_lpCS);
	for (begin = m_pDLThreadList.begin (); begin != m_pDLThreadList.end (); begin ++)
	{
		CDownloadAndSetupThread* pTemp = (*begin).first;
		ATLASSERT (NULL != pTemp);
		if (NULL == pTemp)
			continue;

		if (!pTemp->IsFinished () && !pTemp->IsCancled ()) // ��������
		{
			::LeaveCriticalSection (m_lpCS);
			return false;
		}
	}
	::LeaveCriticalSection (m_lpCS);

	return true;
}

// ��ȡ���е������ļ���·��
void CDownloadManagerThread::ReadAllDownLoadPath(std::list<std::wstring>& listStor)
{
	listStor.clear ();
	std::map<CDownloadAndSetupThread*, bool>::const_iterator itPointer;
	::EnterCriticalSection (m_lpCS);
	for (itPointer = m_pDLThreadList.begin (); itPointer != m_pDLThreadList.end (); itPointer ++)
	{
		CDownloadAndSetupThread* pT= (*itPointer).first;
		if (NULL == pT)
			continue;

		std::wstring str = pT->GetDownloadStorePath ();
		listStor.push_back (str);
	}
	::LeaveCriticalSection (m_lpCS);
}


bool CDownloadManagerThread::IsBankCtrlSetup(void)
{
	if (m_pDLThreadList.size () <= 0)
		return true;

	std::map<CDownloadAndSetupThread*, bool>::const_iterator begin;
	::EnterCriticalSection (m_lpCS);
	for (begin = m_pDLThreadList.begin (); begin != m_pDLThreadList.end (); begin ++)
	{
		CDownloadAndSetupThread* pTemp = (*begin).first;
		ATLASSERT (NULL != pTemp);
		if (NULL == pTemp)
			continue;

		// �µ������пؼ����������
		if (!pTemp->IsFinished () && (emBkActiveX & pTemp->GetDownloadKind ()))
		{
			::LeaveCriticalSection (m_lpCS);

			return false;
		}
	}
	::LeaveCriticalSection (m_lpCS);

	return true;
}

bool CDownloadManagerThread::IsBankCtrlCancled(void)// ���пؼ�ȡ���ղ�
{
	if (m_pDLThreadList.size () <= 0)
		return false;

	std::map<CDownloadAndSetupThread*, bool>::const_iterator begin;
	::EnterCriticalSection (m_lpCS);
	for (begin = m_pDLThreadList.begin (); begin != m_pDLThreadList.end (); begin ++)
	{
		CDownloadAndSetupThread* pTemp = (*begin).first;
		ATLASSERT (NULL != pTemp);
		if (NULL == pTemp)
			continue;

		// �µ������пؼ����������
		if (pTemp->IsCancled () && (emBkActiveX & pTemp->GetDownloadKind ()))
		{
			::LeaveCriticalSection (m_lpCS);

			return true;
		}
	}
	::LeaveCriticalSection (m_lpCS);

	return false;
}


bool CDownloadManagerThread::AddBankCtrlAlready(void)
{
	std::map<CDownloadAndSetupThread*, bool>::const_iterator begin;
	::EnterCriticalSection (m_lpCS);
	for (begin = m_pDLThreadList.begin (); begin != m_pDLThreadList.end (); begin ++)
	{
		CDownloadAndSetupThread* pTemp = (*begin).first;
		ATLASSERT (NULL != pTemp);
		if (NULL == pTemp)
			continue;

		// �µ������пؼ�
		if (emBkActiveX & pTemp->GetDownloadKind ())
		{
			::LeaveCriticalSection (m_lpCS);
			return true;
		}
	}
	::LeaveCriticalSection (m_lpCS);

	return false;
}


bool CDownloadManagerThread::HasDownloadTask(void)
{
	::EnterCriticalSection (m_lpCS);
	CString strTemp;
	strTemp.Format (L"task size = %d");
	int nSize = m_pDLThreadList.size ();
	::LeaveCriticalSection (m_lpCS);

	if (nSize > 0)
		return true;

	return false;
}