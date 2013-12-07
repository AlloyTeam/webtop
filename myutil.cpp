#include "myutil.h"
#include "windows.h"
#include <string>
using namespace std;

wstring&   replace_allW(wstring&   str,const   wstring&   old_value,const   wstring&   new_value)   
{   
	while(true)   {   
		wstring::size_type   pos(0);   
		if(   (pos=str.find(old_value))!=wstring::npos   )   
			str.replace(pos,old_value.length(),new_value);   
		else   break;   
	}   
	return   str;   
}

string&   replace_all(string&   str,const   string&   old_value,const   string&   new_value)   
{   
	while(true)   {   
		string::size_type   pos(0);   
		if(   (pos=str.find(old_value))!=string::npos   )   
			str.replace(pos,old_value.length(),new_value);   
		else   break;   
	}   
	return   str;   
}   
/*wstring&   replace_all_distinctW(wstring&   str,const   wstring&   old_value,const   wstring&   new_value)   
{   
for(wstring::size_type   pos(0);   pos!=wstring::npos;   pos+=new_value.length())   {   
if(   (pos=str.find(old_value,pos))!=wstring::npos   )   
str.replace(pos,old_value.length(),new_value);   
else   break;   
}   
return   str;   
}*/
string&   replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value)   
{   
	for(string::size_type   pos(0);   pos!=string::npos;   pos+=new_value.length())   {   
		if(   (pos=str.find(old_value,pos))!=string::npos   )   
			str.replace(pos,old_value.length(),new_value);   
		else   break;   
	}   
	return   str;   
}

bool isGB(const char*gb,int len){
	for(int i=0;i<len;++i){
		if(gb[i]<0){
			return true;
		}
	}
	return false;
}

int IsTextUTF8(const char* str,long length)
{
	int i;
	long nBytes=0;//UFT8可用1-6个字节编码,ASCII用一个字节
	unsigned char chr;
	bool bAllAscii=true; //如果全部都是ASCII, 说明不是UTF-8
	for(i=0;i<length;i++)
	{
		chr= *(str+i);
		if( (chr&0x80) != 0 ) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
			bAllAscii= false;
		if(nBytes==0) //如果不是ASCII码,应该是多字节符,计算字节数
		{
			if(chr>=0x80)
			{
				if(chr>=0xFC&&chr<=0xFD)
					nBytes=6;
				else if(chr>=0xF8)
					nBytes=5;
				else if(chr>=0xF0)
					nBytes=4;
				else if(chr>=0xE0)
					nBytes=3;
				else if(chr>=0xC0)
					nBytes=2;
				else
				{
					return false;
				}
				nBytes--;
			}
		}
		else //多字节符的非首字节,应为 10xxxxxx
		{
			if( (chr&0xC0) != 0x80 )
			{
				return false;
			}
			nBytes--;
		}
	}
	if( nBytes > 0 ) //违返规则
	{
		return false;
	}
	if( bAllAscii ) //如果全部都是ASCII, 说明不是UTF-8
	{
		return false;
	}
	return true;
}
char* U2G(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	return str;
}
//GB2312到UTF-8的转换
char* G2U(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	return str;
}