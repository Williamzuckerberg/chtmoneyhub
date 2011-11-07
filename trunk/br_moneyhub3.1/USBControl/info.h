#pragma once
using namespace std;



enum SystemType
{
	sAll = 0,
	sUnSupported = 1,
	sWinNT = 2,
	sWin2000 = 3,
	sWinXP = 4,
	sWin2003 = 5,
	sWinVista = 6,
	sWin2008 = 7,
	sWin7 = 8
};
// �ڳ������õ���usb�豸������ֵ
struct USBEigenvalue
{
	USBEigenvalue()
	{
		hasMid = FALSE;
	}
	~USBEigenvalue()
	{
	}
	USHORT  VendorID;
	USHORT  ProductID;
	USHORT  VersionNumber;//�������vid��pid
	string  SerierlNumber; //�����¼sn������

	// MIDֵͨ��ָ��Ŀ¼�µ�dll�ĺ����ӿڻ�ã������Ҫ����ֱ���ṩ�����ǽ������ṩ������ͳһ����Ϊdll�ӿ�
	// ֵ����
	bool	hasMid;//��¼�Ƿ���mid
	// ֵ���ݴ洢λ��
	DWORD m_midDword;

	wstring midDLLName;
	wstring midFucName;	
};

struct USBHardwareInfo
{
	USBHardwareInfo()
	{
		bNeedInstall = false;
		bNeedForbidRun = false;
	}

	~USBHardwareInfo()
	{
	}
	USBEigenvalue eig;	

	wstring		version;//Ӳ���汾��Ϣ
	wstring		manufacturer;//����������
	wstring		goodsname;//��Ʒ����
	string		volumnname;//���

	wstring		inmodel;//�ڲ����
	wstring		model;//�ͺ�
	wstring		finaninstitution;//���ڻ�������
	string		financeid;//���ڻ�����Ŵ�Ϊstring������Ϊ�������ݿ���һ�£����ٽ���ת��
	bool		bNeedInstall;//�Ƿ���Ҫ�����Լ������������Ϣ���а�װ�����USBKey�Ѿ��������԰�װ�����ǾͲ��ù���
	bool		bNeedForbidRun;//�Ƿ���Ҫ����usb�豸���Զ����й���ȥ��
};
//��¼��װ�ؼ�������Ϣ
enum ProgramType
{
	pService = 0,	//�������
	pNormal = 1,	//һ�����
	pDriver = 2,	//��Ҫ������
};
struct  RegInfo
{
	HKEY rootkey;
	wstring subkey;
};
struct USBSoftwareInfo
{
	SystemType		ostype;								//	��¼����ϵͳ����
	int				osarchite;							//	��¼����ϵͳ�ܹ���32,64
	bool			bNeedUpdate;						//	��¼�Ƿ����Լ������Ĺ���
	bool			bNeedReboot;						//  ��¼�Ƿ���Ҫ����ϵͳ
	wstring			url;								//	���ذ�װ����url
	wstring			backurl;							//	�������ص�ַ
	DWORD			version;							//	����汾
	list<RegInfo>	reginfo;							//	��Ҫ��ע�����Ϣ,ֻ���ע���ؼ����Ĵ�����񣬷���̫����
	std::list<std::wstring>  files;						//	��Ҫ�ĳ����ļ�,��װ֮����е��ļ�
	std::map<std::wstring,ProgramType> programinfo;		//	��Ҫ����Ϣ��filename��¼ȫ·��
};

struct USBKeyInfo
{
	USBHardwareInfo hardware;	//Ӳ����Ϣ��Ψһ��
	USBSoftwareInfo software;	//�������ϵͳ�ǲ�ͬ��
	bool bSupport;				//��¼��ǰϵͳ�Ƿ�֧�ָ�usb�豸
};