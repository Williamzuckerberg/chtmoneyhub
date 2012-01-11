#include "stdafx.h"
#include "PublicInterface.h"

// ���ַ���strSour���ָ��ִ���ȡ�������浽setStor�����У� strSep��strSour�ķָ��ַ���
bool PublicInterface::SeparateStringBystr(std::vector<std::string>& setStor, std::string strSour, const std::string& strSep)
{
	if (strSour.length () <= 0 || strSep.length() < 0)
		return false;

	int nIndex;

	while((nIndex = strSour.find(strSep)) != string::npos)
	{
		string strT = strSour.substr(0, nIndex);
		strSour = strSour.substr(nIndex + strSep.length(), strSour.length());
		setStor.push_back(strT);
	}

	return true;
}