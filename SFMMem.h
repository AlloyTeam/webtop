// 2002/07/05
// awzzz

// SFMMem.h: interface for the CSFMServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SFMSERVER_H__2D76A439_6388_4B07_AE7A_C82F458642ED__INCLUDED_)
#define AFX_SFMSERVER_H__2D76A439_6388_4B07_AE7A_C82F458642ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <Windows.h>
#define	DEFAULT_FILENAME	NULL
#define	DEFAULT_MAPNAME		L"_SFM_OBJ_"
#define	DEFAULT_MAPSIZE		(0xFFFF + 1)

// Shared FileMap Server
// 使用说明
// 1.
// 创建CSFMServer对象
// CSFMServer(char *szFileName, char *szMapName, DWORD dwSize);
// Create(char *szFileName, char *szMapName, DWORD dwSize);
// 参数1:NULL或指定的文件(将创建或打开并读写/麻烦)
// 参数2:要创建的共享内存对象名
// 参数3:要创建的共享内存对象大小
// 2.
// 本地使用内存
// LPVOID GetBuffer()
// 返回共享内存地址
//
// 自动销毁
class CSFMServer  
{
public:
	CSFMServer();
	virtual ~CSFMServer();
	CSFMServer(const TCHAR *szFileName, const TCHAR *szMapName, DWORD dwSize);

protected:
	HANDLE	m_hFile;
	HANDLE	m_hFileMap;
	LPVOID	m_lpFileMapBuffer;

	TCHAR	*m_pFileName;
	TCHAR	*m_pMapName;
	DWORD	m_dwSize;

	int		m_iCreateFlag;

private:
	void _Init();
	void _Destory();

public:
	void Create(const TCHAR *szFileName, const TCHAR *szMapName, DWORD dwSize, DWORD dwSizeHigh=0);
	LPVOID GetBuffer();
	DWORD GetSize();
};

// Shared FileMap Client
// 使用说明
// 1.
// 创建CSFMClient对象
// CSFMClient(DWORD dwAccess, char *szMapName);
// Open(DWORD dwAccess, char *szMapName);
// 参数1:共享内存对象访问方式(FILE_MAP_READ|FILE_MAP_WRITE)
// 参数2:共享内存对象名
// 2.
// 本地使用内存
// LPVOID GetBuffer()
// 返回共享内存地址
//
// 自动销毁
class CSFMClient  
{
public:
	CSFMClient();
	virtual ~CSFMClient();
	CSFMClient(DWORD dwAccess, const TCHAR *szMapName);

protected:
	HANDLE	m_hFileMap;
	LPVOID	m_lpFileMapBuffer;

	TCHAR	*m_pMapName;

	int		m_iOpenFlag;

private:
	void _Init();
	void _Destory();

public:
	void Open(DWORD dwAccess, const TCHAR *szMapName);
	LPVOID GetBuffer();
	DWORD GetSize();
};

#endif // !defined(AFX_SFMSERVER_H__2D76A439_6388_4B07_AE7A_C82F458642ED__INCLUDED_)
