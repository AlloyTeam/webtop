#ifndef __MYUTIL_H__
#define __MYUTIL_H__
#include <xstring>
using namespace std;

wstring&   replace_allW(wstring&   str,const   wstring&   old_value,const   wstring&   new_value);
string&   replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value);
string&   replace_all(string&   str,const   string&   old_value,const   string&   new_value);
bool isGB(const char*gb,int len);
int IsTextUTF8(const char* str,long length);
char* U2G(const char* utf8);
char* G2U(const char* gb2312);

#endif
