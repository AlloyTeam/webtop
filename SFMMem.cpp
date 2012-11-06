// SFMMem.cpp: implementation of the CSFMServer class.
//
//////////////////////////////////////////////////////////////////////

#include "SFMMem.h"
#include <stdlib.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSFMServer::CSFMServer()
{
	_Init();
	//
	Create(DEFAULT_FILENAME, DEFAULT_MAPNAME, DEFAULT_MAPSIZE);
}

CSFMServer::~CSFMServer()
{
	_Destory();
}

CSFMServer::CSFMServer(const TCHAR *szFileName, const TCHAR *szMapName, DWORD dwSize)
{
	_Init();
	//
	Create(szFileName, szMapName, dwSize);
}

void CSFMServer::_Init()
{
	m_hFile = NULL;
	m_hFileMap = NULL;
	m_lpFileMapBuffer = NULL;

	m_pFileName = NULL;
	m_pMapName = NULL;
	m_dwSize = 0;

	m_iCreateFlag = 0;
}

void CSFMServer::_Destory()
{
	if (m_lpFileMapBuffer)
	{
		UnmapViewOfFile(m_lpFileMapBuffer);
		m_lpFileMapBuffer = NULL;
	}
	
	if (m_hFileMap)
	{
		CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}
	
	if (m_hFile && m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}

	if (m_pFileName)
	{
		free(m_pFileName);
		m_pFileName = NULL;
	}

	if (m_pMapName)
	{
		free(m_pMapName);
		m_pMapName = NULL;
	}

	_Init();
}

void CSFMServer::Create(const TCHAR *szFileName, const TCHAR *szMapName, DWORD dwSize, DWORD dwSizeHigh)
{
	if (m_iCreateFlag)
		_Destory();

	if (szFileName)
		m_pFileName = _wcsdup(szFileName);
	//else m_pFileName = NULL;

	if (szMapName)
		m_pMapName = _wcsdup(szMapName);
	else m_pMapName = _wcsdup(DEFAULT_MAPNAME);

	if (dwSize > 0)
		m_dwSize = dwSize;
	else m_dwSize = DEFAULT_MAPSIZE;

	if (m_pFileName)
	{
		// file
		m_hFile = CreateFile(
			m_pFileName,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,//OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);
	}
	else
	{
		// system
		m_hFile = (HANDLE)0xFFFFFFFF;
	}

	if (m_hFile)
	{
		m_hFileMap = CreateFileMapping(
			m_hFile,
			NULL,
			PAGE_READWRITE,
			dwSizeHigh,
			m_dwSize,
			m_pMapName
			);

		//使只有一个CSFMServer对象能操作内存对象
		//if (m_hFileMap != NULL && ERROR_ALREADY_EXISTS == GetLastError())
		//{
		//	CloseHandle(m_hFileMap);
		//	m_hFileMap = NULL;
		//}
	}

	if (m_hFileMap)
	{
		m_lpFileMapBuffer = MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,//FILE_MAP_WRITE|FILE_MAP_READ,
			0,
			0,
			m_dwSize
			);
	}

	m_iCreateFlag = 1;
}

LPVOID CSFMServer::GetBuffer()
{
	return (m_lpFileMapBuffer)?(m_lpFileMapBuffer):(NULL);
}

DWORD CSFMServer::GetSize()
{
	return m_dwSize;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSFMClient::CSFMClient()
{
	_Init();
	//
	Open(FILE_MAP_READ, DEFAULT_MAPNAME);
}

CSFMClient::~CSFMClient()
{
	_Destory();
}

CSFMClient::CSFMClient(DWORD dwAccess, const TCHAR *szMapName)
{
	_Init();
	//
	Open(dwAccess, szMapName);
}

void CSFMClient::Open(DWORD dwAccess, const TCHAR *szMapName)
{
	if (m_iOpenFlag)
		_Destory();

	if (szMapName)
		m_pMapName = _wcsdup(szMapName);
	else m_pMapName = _wcsdup(DEFAULT_MAPNAME);

	m_hFileMap = OpenFileMapping(
		dwAccess,
		TRUE,
		m_pMapName
		);

	if (m_hFileMap)
	{
		m_lpFileMapBuffer = MapViewOfFile(
			m_hFileMap,
			dwAccess,
			0,
			0,
			0
			);
	}

	m_iOpenFlag = 1;
}

void CSFMClient::_Init()
{
	m_hFileMap = NULL;
	m_lpFileMapBuffer = NULL;

	m_pMapName = NULL;

	m_iOpenFlag = 0;
}

void CSFMClient::_Destory()
{
	if (m_lpFileMapBuffer)
	{
		UnmapViewOfFile(m_lpFileMapBuffer);
		m_lpFileMapBuffer = NULL;
	}
	
	if (m_hFileMap)
	{
		CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}

	if (m_pMapName)
	{
		free(m_pMapName);
		m_pMapName = NULL;
	}

	_Init();
}

LPVOID CSFMClient::GetBuffer()
{
	return (m_lpFileMapBuffer)?(m_lpFileMapBuffer):(NULL);
}

DWORD CSFMClient::GetSize()
{
	// unnable use
	return 0;
}
