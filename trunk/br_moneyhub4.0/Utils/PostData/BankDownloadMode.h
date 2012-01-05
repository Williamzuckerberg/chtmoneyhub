enum eDownloadMode
{
	eHttpNormal,
	eHttpBreak//�ϵ�����
};

class CDownloadMode
{
private:
	eDownloadMode m_DownloadMode;
	CDownloadMode(){}
	~CDownloadMode(){}
public:

	void SetMode(eDownloadMode eMode)
	{
		m_DownloadMode = eMode;
		DWORD dMode = (eMode == eHttpBreak) ? 0 : 1;//�ϵ�������0��http��1
		::SHSetValueW(HKEY_CURRENT_USER, _T("Software\\Bank\\Setting"),
			_T("DownloadMode"), REG_DWORD, &dMode, sizeof(DWORD));
	}

	eDownloadMode GetMode()
	{
		DWORD dwType = 0;
		DWORD dwValue = 0;
		DWORD dwReturnBytes = sizeof(DWORD);

		::SHGetValue(HKEY_CURRENT_USER, _T("Software\\Bank\\Setting"),
			_T("DownloadMode"), &dwType, &dwValue, &dwReturnBytes);

		m_DownloadMode = (dwValue == 0) ? eHttpBreak : eHttpNormal;
		
		return m_DownloadMode;
	}
private:
	static CDownloadMode* m_Instance;
public:
	static CDownloadMode* GetInstance()
	{
		if(m_Instance == NULL)
			m_Instance = new CDownloadMode();
		return m_Instance;
	}
};

CDownloadMode* CDownloadMode::m_Instance = NULL;