#pragma once
#include <map>
class CFavBankOperator
{
public:
	CFavBankOperator(void);
	~CFavBankOperator(void);
	
	// gao 2010-12-13
	// ��������strChk�Ƿ����������
	bool IsInFavBankSet(const std::string& strChk);

	void UpDateFavBankID(std::string& strBkID, BOOL bAdd = true);

	// ��ʱ�����������
	static std::string GetBankIDOrBankName(const std::string& strCondition, bool bGetBName = true);

	// �����Ƚ����ղص�BankID����ǰ�ղص�BankID�Ƿ���ͬ���������ֵ����0����ʾ�������ղأ����С��0����ʾɾ���ղ�, =0��ʾһ��
	// strDif�������ӻ�ɾ����BankID
	int FavBankCompare2Old(const std::set<std::string>& setStor, std::string& strDif);

	// ��ʱֻ������BankID��int֮���ת����sendmessage�ȽϺ���
//	static void MyTranslateBetweenBankIDAndInt(std::string& strBankID, int& nBankID, bool bToInt = true);

	bool ReadFavBankID(std::set<std::string>& setBankID);
private:
	// �������ֵ����0����ʾ�������ղأ����С��0����ʾɾ���ղ�, =0��ʾһ��
	int CompareTwoSet(const std::set<std::string>& setF, const std::set<std::string>& setSec, std::set<std::string>& setDiff);


	// ��һ���ṹ�ж�ȡbankID
	bool ReadBankIDFromBankItem(const std::string& strBankItem, std::string& bankID);

	// ��ȡ���ݿ��е�����ID����������
	static bool ReadBankConfigData();

private:
	std::set<std::string> m_setFavBankID; // ������¼�û��ղ��˵�����ID // gao 2010-12-17
	static std::map<std::string, std::string> m_mapBankNameAndID;
};
