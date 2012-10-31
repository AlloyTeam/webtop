#include "myutil.h"
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
wstring&   replace_all_distinct(wstring&   str,const   wstring&   old_value,const   wstring&   new_value)   
{   
    for(wstring::size_type   pos(0);   pos!=wstring::npos;   pos+=new_value.length())   {   
        if(   (pos=str.find(old_value,pos))!=wstring::npos   )   
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

