#include <system.h>
#include <sstream>
#include <tchar.h>
#include <shellapi.h>
#include <ShlObj.h>
#include "base64.h"

wstring GetExtW(wstring path){
	int index=path.find_last_of('.');
	if(index==-1){
		return L"";
	}
	else{
		wstring type=path.substr(index+1);
		return replace_allW(type,L"jpg",L"jpeg");
	}
}

string GetExt(string path){
	int index=path.find_last_of('.');
	if(index==-1){
		return "";
	}
	else{
		string type=path.substr(index+1);
		return replace_all(type,"jpg","jpeg");
	}
}

BOOL EnableShutdownPrivilege()
{
    HANDLE hProcess = NULL;
    HANDLE hToken = NULL;
    LUID uID = {0};
    TOKEN_PRIVILEGES stToken_Privileges = {0};

    hProcess = ::GetCurrentProcess();  //获取当前应用程序进程句柄
    
    if(!::OpenProcessToken(hProcess,TOKEN_ADJUST_PRIVILEGES,&hToken))  //打开当前进程的访问令牌句柄(OpenProcessToken函数调用失败返回值为零)
        return FALSE;

    if(!::LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&uID))  //获取权限名称为"SeShutdownPrivilege"的LUID(LookupPrivilegeValue函数调用失败返回值为零)
        return FALSE;

    stToken_Privileges.PrivilegeCount = 1;  //欲调整的权限个数
    stToken_Privileges.Privileges[0].Luid = uID;  //权限的LUID
    stToken_Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  //权限的属性,SE_PRIVILEGE_ENABLED为使能该权限
    
    if(!::AdjustTokenPrivileges(hToken,FALSE,&stToken_Privileges,sizeof stToken_Privileges,NULL,NULL))  //调整访问令牌里的指定权限(AdjustTokenPrivileges函数调用失败返回值为零)
        return FALSE;

    if(::GetLastError() != ERROR_SUCCESS)  //查看权限是否调整成功
        return FALSE;

    ::CloseHandle(hToken);
    return TRUE;
}

//关机函数
BOOL Shutdown(BOOL bForce)
{
    EnableShutdownPrivilege();  //使能关机特权函数
    if(bForce)
        return ::ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,0);  //强制关机
    else
        return ::ExitWindowsEx(EWX_SHUTDOWN,0);
}

//注销函数
BOOL Logoff(BOOL bForce)
{
    if(bForce)
        return ::ExitWindowsEx(EWX_LOGOFF | EWX_FORCE,0);  //强制注销
    else
        return ::ExitWindowsEx(EWX_LOGOFF,0);
}

//重启函数
BOOL Reboot(BOOL bForce)
{
    EnableShutdownPrivilege();  //使能关机特权函数
    if(bForce)
        return ::ExitWindowsEx(EWX_REBOOT | EWX_FORCE,0);  //强制重启
    else
        return ::ExitWindowsEx(EWX_REBOOT,0);
}
int CDECL MessageBoxPrintf (TCHAR * szCaption, TCHAR * szFormat, ...)  
{      
  TCHAR  szBuffer [1024] ;  
  va_list pArgList ;  
  // The va_start macro (defined in STDARG.H) is usually equivalent to:  
  // pArgList = (char *) &szFormat + sizeof (szFormat) ;  
  va_start (pArgList, szFormat) ;  
  // The last argument to wvsprintf points to the arguments  
  _vsntprintf ( szBuffer, sizeof (szBuffer) / sizeof (TCHAR),  
    szFormat, pArgList) ;  
  // The va_end macro just zeroes out pArgList for no good reason  
  va_end (pArgList) ;  //Defoe.Tu  tyysoft@yahoo.com.cn Windows 程序设计 PDF 美化版 
  return MessageBox (NULL, szBuffer, szCaption, 0) ;  
}
BOOL  SaveFileDialog(HWND hWnd, const TCHAR* fileNameStr, TCHAR* szFile)
{
	//TCHAR szFile[2048];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
// must !
	ofn.lpstrFile = szFile;
	ofn.lpstrTitle = TEXT("保存文件");
	ofn.lpstrFileTitle=(LPWSTR)fileNameStr;
	ofn.nMaxFile = 2048;//sizeof(szFile);
//
	ofn.lpstrFile[0] = '\0';
	wcscpy(ofn.lpstrFile, (LPWSTR)fileNameStr);
	ofn.Flags = OFN_OVERWRITEPROMPT;
//no extention file!    ofn.lpstrFilter="Any file(*.*)\0*.*\0ddfs\0ddfs*\0";
	return(GetSaveFileName((LPOPENFILENAME)&ofn));
}
BOOL  OpenFileDialog(HWND hWnd, const TCHAR* fileNameStr, TCHAR* szFile)
{
	//TCHAR szFile[2048];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
// must !
	ofn.lpstrFile = szFile;
	ofn.lpstrTitle = TEXT("打开文件");
	ofn.lpstrFileTitle=(LPWSTR)fileNameStr;
	ofn.nMaxFile = 2048;//sizeof(szFile);
//
	ofn.lpstrFile[0] = '\0';
	wcscpy(ofn.lpstrFile, (LPWSTR)fileNameStr);
	ofn.Flags = OFN_FILEMUSTEXIST;
//no extention file!    ofn.lpstrFilter="Any file(*.*)\0*.*\0ddfs\0ddfs*\0";
	return(GetOpenFileName((LPOPENFILENAME)&ofn));
}

//可同时处理目录和文件:path可以是路径，也可以是文件名，或者文件通配符
wstring find(wstring path,bool cursive)
{ 
     //取路径名最后一个"//"之前的部分,包括"//"
	replace_allW(path,L"\\",L"/");
	UINT index=path.find_last_of('/');
	if(index+1==path.length()){
		path.append(L"*.*");
	}
    wstring prefix=path.substr(0,path.find_last_of('/')+1);

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind=::FindFirstFile(path.data(),&FindFileData);
	std::wstringstream ss;
    if(INVALID_HANDLE_VALUE == hFind)
    {
       ss<<L"[]";
	   FindClose(hFind);
	   return ss.str();
    }
	else{
		ss<<L"[";
	}
    while(TRUE)
    {
		bool flag=false;;
      //目录
        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //不是当前目录，也不是父目录
            if(cursive&&FindFileData.cFileName[0]!='.')
            {
				wstring temp=prefix+FindFileData.cFileName;
				ss<<L"{\"name\":\""<<FindFileData.cFileName<<L"\",\"list\":"<<find(temp+L"/*.*",cursive).data()<<L"}";
				flag=true;
            }
        }
        //文件
        else
        {   
             ss<<L"\""<<FindFileData.cFileName<<L"\"";
			flag=true;
        }
        if(!FindNextFile(hFind,&FindFileData))
              break;
		else if(flag){
			ss<<L",";
		}
    }
	ss<<L"]";
    FindClose(hFind);
	return ss.str();
}

void SetDefaultBrowser(const TCHAR *strAppName){
	HKEY key;
	RegOpenKey(HKEY_CLASSES_ROOT,_T("http\\shell\\open\\command"),&key);
	RegSetValue(key,NULL,REG_SZ,strAppName,sizeof(strAppName));
	RegCloseKey(key);

	RegOpenKey(HKEY_CLASSES_ROOT,_T("HKEY_CLASSES_ROOT\\htmlfile\\shell\\open\\command"),&key);
	RegSetValue(key,NULL,REG_SZ,strAppName,sizeof(strAppName));
	RegCloseKey(key);

	RegOpenKey(HKEY_CLASSES_ROOT,_T("HKEY_CLASSES_ROOT\\mhtmlfile\\shell\\open\\command"),&key);
	RegSetValue(key,NULL,REG_SZ,strAppName,sizeof(strAppName));
	RegCloseKey(key);

	RegOpenKey(HKEY_CLASSES_ROOT,_T("HKEY_CLASSES_ROOT\\InternetShortcut\\shell\\open\\command"),&key);
	RegSetValue(key,NULL,REG_SZ,strAppName,sizeof(strAppName));
	RegCloseKey(key);
}
//---------------------------------------------------------------------------
// 检测文件关联情况
// strExt: 要检测的扩展名(例如: ".txt")
// strAppKey: ExeName扩展名在注册表中的键值(例如: "txtfile")
// 返回TRUE: 表示已关联，FALSE: 表示未关联
BOOL CheckFileRelation(const TCHAR *strExt, const TCHAR *strAppKey, TCHAR *strAppName, TCHAR *strDefaultIcon, TCHAR *strDescribe)
{
    int nRet=TRUE;
    HKEY hExtKey;
    TCHAR szPath[_MAX_PATH]; 
    DWORD dwSize=sizeof(szPath); 
    TCHAR strTemp[_MAX_PATH];
    if(RegOpenKey(HKEY_CLASSES_ROOT,strExt,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strAppKey)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strAppKey,wcslen(strAppKey)+1);
			//return nRet;
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    if(RegOpenKey(HKEY_CLASSES_ROOT,strAppKey,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strDescribe)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strDescribe,wcslen(strDescribe)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
	wsprintf(strTemp,L"%s\\DefaultIcon",strAppKey);
    if(RegOpenKey(HKEY_CLASSES_ROOT,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strDefaultIcon)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strDefaultIcon,wcslen(strDefaultIcon)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"%s\\Shell",strAppKey);
    if(RegOpenKey(HKEY_CLASSES_ROOT,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,L"Open")!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,L"Open",wcslen(L"Open")+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"%s\\Shell\\Open\\Command",strAppKey);
    if(RegOpenKey(HKEY_CLASSES_ROOT,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
		wsprintf(strTemp,L"%s \"%%1\"",strAppName);
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strTemp)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    if(RegOpenKey(HKEY_CURRENT_USER,strExt,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strAppKey)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strAppKey,wcslen(strAppKey)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    if(RegOpenKey(HKEY_CURRENT_USER,strAppKey,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strDescribe)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strDescribe,wcslen(strDescribe)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
	wsprintf(strTemp,L"%s\\DefaultIcon",strAppKey);
    if(RegOpenKey(HKEY_CURRENT_USER,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strDefaultIcon)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strDefaultIcon,wcslen(strDefaultIcon)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"%s\\Shell",strAppKey);
    if(RegOpenKey(HKEY_CURRENT_USER,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,L"Open")!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,L"Open",wcslen(L"Open")+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"%s\\Shell\\Open\\Command",strAppKey);
    if(RegOpenKey(HKEY_CURRENT_USER,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
		wsprintf(strTemp,L"%s \"%%1\"",strAppName);
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strTemp)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"Software\\Classes\\%s_auto_file\\Shell\\Open\\Command",strExt+1);
    if(RegOpenKey(HKEY_CURRENT_USER,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
		wsprintf(strTemp,L"%s \"%%1\"",strAppName);
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strTemp)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
        }
        RegCloseKey(hExtKey);
    }
    return nRet;
}
//---------------------------------------------------------------------------
// 注册文件关联
// strExe: 要检测的扩展名(例如: ".txt")
// strAppName: 要关联的应用程序名(例如: "C:\MyApp\MyApp.exe")
// strAppKey: ExeName扩展名在注册表中的键值(例如: "txtfile")
// strDefaultIcon: 扩展名为strAppName的图标文件(例如: "C:\MyApp\MyApp.exe,0")
// strDescribe: 文件类型描述
void RegisterFileRelation(TCHAR *strExt, TCHAR *strAppName, TCHAR *strAppKey, TCHAR *strDefaultIcon, TCHAR *strDescribe)
{
    TCHAR strTemp[_MAX_PATH];
    HKEY hKey;
    
    RegCreateKey(HKEY_CLASSES_ROOT,strExt,&hKey);
    RegSetValue(hKey,L"",REG_SZ,strAppKey,wcslen(strAppKey)+1);
    RegCloseKey(hKey);
    
    RegCreateKey(HKEY_CLASSES_ROOT,strAppKey,&hKey);
    RegSetValue(hKey,L"",REG_SZ,strDescribe,wcslen(strDescribe)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\DefaultIcon",strAppKey);
    RegCreateKey(HKEY_CLASSES_ROOT,strTemp,&hKey);
    RegSetValue(hKey,L"",REG_SZ,strDefaultIcon,wcslen(strDefaultIcon)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\Shell",strAppKey);
    RegCreateKey(HKEY_CLASSES_ROOT,strTemp,&hKey);
    RegSetValue(hKey,L"",REG_SZ,L"Open",wcslen(L"Open")+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\Shell\\Open\\Command",strAppKey);
    RegCreateKey(HKEY_CLASSES_ROOT,strTemp,&hKey);
    wsprintf(strTemp,L"%s \"%%1\"",strAppName);
    RegSetValue(hKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\DefaultIcon",strAppKey);
    RegCreateKey(HKEY_CURRENT_USER,strTemp,&hKey);
    RegSetValue(hKey,L"",REG_SZ,strDefaultIcon,wcslen(strDefaultIcon)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\Shell",strAppKey);
    RegCreateKey(HKEY_CURRENT_USER,strTemp,&hKey);
    RegSetValue(hKey,L"",REG_SZ,L"Open",wcslen(L"Open")+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\Shell\\Open\\Command",strAppKey);
    RegCreateKey(HKEY_CURRENT_USER,strTemp,&hKey);
    wsprintf(strTemp,L"%s \"%%1\"",strAppName);
    RegSetValue(hKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"Software\\Classes\\%s_auto_file\\Shell\\Open\\Command",strExt+1);
    RegCreateKey(HKEY_CURRENT_USER,strTemp,&hKey);
    wsprintf(strTemp,L"%s \"%%1\"",strAppName);
    RegSetValue(hKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
    RegCloseKey(hKey);
	SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSHNOWAIT,0, 0); 
}

Bitmap* GetImageFromBase64(string encodedImage)
{
	int imageSize = int((encodedImage.length()/3)+1)*4;
	char* t=new char[imageSize];
	base64_decode(encodedImage.c_str(),encodedImage.length(), t, &imageSize); // using the base64 
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
	LPVOID pImage = ::GlobalLock(hMem);
	memcpy(pImage, t, imageSize);

	IStream* pStream = NULL;
	::CreateStreamOnHGlobal(hMem, FALSE, &pStream);

	Bitmap *img = new Bitmap(pStream);
	pStream->Release();
	GlobalUnlock(hMem);
	GlobalFree(hMem);
	delete []t;
    return img;
}

//TransparentWnd::connection.RegisterConnection(L"WebtopSever");
int   GetEncoderClsid(const   WCHAR*   format,   CLSID*   pClsid) 
{ 
      UINT     num   =   0;                     //   number   of   image   encoders 
      UINT     size   =   0;                   //   size   of   the   image   encoder   array   in   bytes 

      ImageCodecInfo*   pImageCodecInfo   =   NULL; 

      GetImageEncodersSize(&num,   &size); 
      if(size   ==   0) 
            return   -1;     //   Failure 

      pImageCodecInfo   =   (ImageCodecInfo*)(malloc(size)); 
      if(pImageCodecInfo   ==   NULL) 
            return   -1;     //   Failure 

      GetImageEncoders(num,   size,   pImageCodecInfo); 

      for(UINT   j   =   0;   j   <   num;   ++j) 
      { 
            if(   wcscmp(pImageCodecInfo[j].MimeType,   format)   ==   0   ) 
            { 
                  *pClsid   =   pImageCodecInfo[j].Clsid; 
                  free(pImageCodecInfo); 
                  return   j;     //   Success 
            }         
      } 

      free(pImageCodecInfo); 
      return   -1;     //   Failure 
} 
void SaveBitmap(Bitmap* pbm, wstring path){
	CLSID tiffClsid;
	GetEncoderClsid((L"image/"+ GetExtW(path)).data(), &tiffClsid);
	pbm->Save(path.data(), &tiffClsid);
}
