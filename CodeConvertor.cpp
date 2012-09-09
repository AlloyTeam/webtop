#include "CodeConvertor.h"

CodeConvertor::CodeConvertor(void)
{
}

CodeConvertor::~CodeConvertor(void)
{
}


LPCSTR CodeConvertor::Gb2Utf8(LPCSTR input)
{
	WCHAR *temp;
	int i = MultiByteToWideChar(CP_ACP, 0, input, -1, NULL, 0);
	temp = new WCHAR[i+1];
	MultiByteToWideChar(CP_ACP, 0, input, -1, temp, i);

	i = WideCharToMultiByte(CP_UTF8, 0, temp, -1, NULL, 0, NULL, NULL);
	CHAR* output = new CHAR[i+1];
	int j=WideCharToMultiByte(CP_UTF8, 0, temp, -1, output, i, NULL, NULL);
	return output;
}

LPCSTR CodeConvertor::Utf82Gb(LPCSTR input)
{
	WCHAR *temp;
	int i = MultiByteToWideChar(CP_UTF8, 0, input, -1, NULL, 0);
	temp = new WCHAR[i+1];
	MultiByteToWideChar(CP_UTF8, 0, input, -1, temp, i);

	i = WideCharToMultiByte(CP_ACP, 0, temp, -1, NULL, 0, NULL, NULL);
	CHAR* output = new CHAR[i+1];
	int j=WideCharToMultiByte(CP_ACP, 0, temp, -1, output, i, NULL, NULL);
	return output;
}
