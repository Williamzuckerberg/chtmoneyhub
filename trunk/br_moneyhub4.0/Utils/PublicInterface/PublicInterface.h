#pragma once
using namespace std;

namespace PublicInterface
{
	// ���ַ���strSour���ָ��ַ�����ȡ�������浽setStor�����У� strSep��strSour�ķָ��ַ���
	bool SeparateStringBystr(std::vector<std::string>& setStor, std::string strSour, const std::string& strSep);
}