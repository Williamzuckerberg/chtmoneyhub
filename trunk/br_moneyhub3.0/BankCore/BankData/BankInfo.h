#include <map>
#include <list>
using namespace std;

// ��¼��������
enum TaskType
{
	ttActiveX,
	ttUSB
};
class TaskInfo
{
	TaskType m_type;	//��������ͣ��ؼ�����usb

	string m_address;	//���ص�ַ
	
	int m_process;		//��������Ľ���

	int vid;
	int pid;
	DWORD mid;
	string 
};
class BankControl
{
	// ע��ͬ��
	int process;// �ܵ����ؽ���
	// ����Ҫ����һ���㷨��������������ؽ��ȼ������
	// �ȴ����е��������Ȼ������ʾusbkey����ʹ��

	list<TaskInfo> banktask; //���е���������

	void AddTask();//����һ������

	void DelTask();// ɾ��һ������
};
class CBankTask
{
private:
	map<string appId, BankControl bankctl>  m_banktask;// �������е�id��¼���е���Ϣ

	HWND m_homepage;
public:
	void AddTask(string appId, TaskInfo task);// ִ�в���һ��������Ĵ���

	void DelTask(string appId);	//ɾ������ ��ɾ���������е�����

	void PauseTask(string appId);	//��ͣ����

	list<TaskInfo>& GetTask(string appId);//����appId �������appid�µ�����������Ϣ

	void SetDisplay(HWND home)
	{
		m_homepage = home;
	}

private:

	void SetDisplayInfo(string appId, )

};

// ����������洢�������ڸ��µ����ؿ�����
class CBankInfo
{
private:
	CBankTask m_task;
public:
	void AddUSB(int vid, int pid, DWORD mid);//����һ��usbkey����������
	void AddBankActiveX(string appId);//����һ�����пؼ�����������

	void SetDisplay(HWND home)
	{
		m_task.SetDisplay(home);
	}
};

extern CBankInfo g_bankinfo;