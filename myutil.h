#ifndef __MYUTIL_H__
#define __MYUTIL_H__
#include <xstring>
using namespace std;

wstring&   replace_allW(wstring&   str,const   wstring&   old_value,const   wstring&   new_value);
wstring&   replace_all_distinct(wstring&   str,const   wstring&   old_value,const   wstring&   new_value);
string&   replace_all(string&   str,const   string&   old_value,const   string&   new_value);

#endif
