#include <Windows.h>
#include "AmfStream.h"
#include <tchar.h>

AmfStream::AmfStream(PBYTE buffer)
{
	stream = buffer;
	position = 0;
}

AmfStream::~AmfStream(void)
{
	stream = NULL;
}


BOOL AmfStream::ReadBytes(PBYTE bytes, DWORD len)
{
	if(!IsBadReadPtr(&stream[position], len))
	{
		CopyMemory(bytes, &stream[position], len);
		position += len;
		return TRUE;
	}
	return FALSE;
}

BOOL AmfStream::WriteBytes(const PBYTE bytes, DWORD len)
{
	if (!IsBadWritePtr(&stream[position], len))
	{
		CopyMemory(&stream[position], bytes, len);
		position += len;
		return TRUE;
	}
	return FALSE;
}

ULONG AmfStream::ReadULong()
{
	if(!IsBadReadPtr(&stream[position], 4))
	{
		LONG i = position;	
		ULONG l =	(stream[i+3] << 24) |  
					(stream[i+2] << 16) | 
					(stream[i+1] << 8) | 
					stream[i];
		position += 4;
		return l;
	}
	return 0;
}
BOOL AmfStream::WriteULong(ULONG len)
{
	if (!IsBadWritePtr(&stream[position], 4))
	{
		stream[position]     = (BYTE)(len & 0xff);
		stream[position+1]   = (BYTE)((len & 0xff00) >> 8);
		stream[position+2]   = (BYTE)((len & 0xff0000) >> 16);
		stream[position + 3] = (BYTE)((len & 0xff000000) >> 24);
		position += 4;
		return TRUE;
	}
	return FALSE;
}

BYTE AmfStream::ReadByte()
{
	if(!IsBadReadPtr(&stream[position], 1))
	{ 
		return stream[position++];
	}
	return 0;
}

BOOL AmfStream::WriteByte(const BYTE i)
{
	if (!IsBadWritePtr(&stream[position], 1))
	{
		stream[position++]     = i;
		return TRUE;
	}
	return FALSE;
}

BOOL AmfStream::WriteString(LPCSTR str)
{
	SHORT slen = strlen(str);
	if (!IsBadWritePtr(&stream[position], slen + 3))
	{
		stream[position] = AMF_STRING;
		stream[position + 1] = (slen >> 8) & 0xff;
		stream[position + 2] = slen & 0xff;
		CopyMemory(&stream[position+3], str, slen);
		position += 3 + slen;
		return TRUE;
	}
	return FALSE;
}
BOOL AmfStream::ReadString(LPCSTR str, SHORT* readed, int max)
{
	if(stream[position] == AMF_STRING)
	{
		SHORT len = (stream[position + 1] << 8) | stream[position + 2];
		if(max&&max<len){
			len=max;
		}
		if (!IsBadWritePtr((PVOID)str, len))
		{
			CopyMemory((PVOID)str, &stream[position + 3], len);
			PCHAR bytes = (PCHAR)str;
			bytes[len] = '\0';
			position += 3 + len;
		}
		if (readed != NULL)
		{
			*readed = len;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL AmfStream::WriteWString(LPCWSTR str)
{
	SHORT slen = wcslen(str)*2;
	if (!IsBadWritePtr(&stream[position], slen + 3))
	{
		stream[position] = AMF_STRING;
		stream[position + 1] = (slen >> 8) & 0xff;
		stream[position + 2] = slen & 0xff;
		CopyMemory(&stream[position+3], str, slen);
		position += 3 + slen;
		return TRUE;
	}
	return FALSE;
}
BOOL AmfStream::ReadWString(LPCWSTR str, SHORT* readed, int max)
{
	if(stream[position] == AMF_STRING)
	{
		SHORT len = (stream[position + 1] << 8) | stream[position + 2];
		if(max&&max<len){
			len=max;
		}
		if (!IsBadWritePtr((PVOID)str, len))
		{
			CopyMemory((PVOID)str, &stream[position + 3], len);
			PCHAR bytes = (PCHAR)str;
			bytes[len+1]=bytes[len] = '\0';
			position += 3 + len;
		}
		if (readed != NULL)
		{
			*readed = len;
		}
		return TRUE;
	}
	return FALSE;
}
BOOL AmfStream::WriteStringSimple(LPCSTR str)
{
	long slen = strlen(str);
	if (!IsBadWritePtr(&stream[position], slen))
	{
		CopyMemory(&stream[position], str, slen);
		position += slen;
		return TRUE;
	}
	return FALSE;
}
BOOL AmfStream::ReadStringSimple(LPCSTR str, int len)
{
	if (!IsBadWritePtr((PVOID)str, len))
	{
		CopyMemory((PVOID)str, &stream[position], len);
		PCHAR bytes = (PCHAR)str;
		bytes[len] = '\0';
		position += len;
	}
	return TRUE;
}

BOOL AmfStream::WriteWStringSimple(LPCWSTR str)
{
	SHORT slen = wcslen(str)*2;
	if (!IsBadWritePtr(&stream[position], slen))
	{
		CopyMemory(&stream[position], str, slen);
		position += slen;
		return TRUE;
	}
	return FALSE;
}
BOOL AmfStream::ReadWStringSimple(LPCWSTR str, int len)
{
	if (!IsBadWritePtr((PVOID)str, len))
	{
		CopyMemory((PVOID)str, &stream[position], len);
		PCHAR bytes = (PCHAR)str;
		bytes[len+1]=bytes[len] = '\0';
		position += + len;
	}
	return TRUE;
}
BOOL AmfStream::WriteUndefined(LPCSTR str, DWORD len)
{
	SHORT lenNum = GetUndefinedLengthBytesNum(len);
	if(!IsBadWritePtr(&stream[position], lenNum + len + 1 ))
	{
		DWORD nlen = len * 2 + 1;
		stream[position] = AMF_UNDEFINED;
		position ++;
		INT offset[4] = {0,7, 14, 21};
		for (INT i = lenNum -1; i > 0; i --)
		{
			stream[position] = (nlen >> offset[i]) % 0x80 + 0x80;
			position ++;
		}
		stream[position] = nlen % 0x80;
		position ++;
		CopyMemory(&stream[position], str, len);
		position += len;
		return TRUE;
	}
	return FALSE;
}
BOOL AmfStream::ReadUndefined(LPCSTR str, DWORD* readed)
{
	if (stream[position] == AMF_UNDEFINED)
	{
		position ++;
		DWORD len = 0;
		SHORT lenNum = 1;
		DWORD index = position;
		while(stream[index] >= 0x80)
		{
			lenNum ++;
			index ++;
		}
		SHORT offset[4] = {0,7, 14, 21};
		while(lenNum >= 0)
		{
			lenNum --;
			BYTE b = stream[position] % 0x80;
			SHORT of =  offset[lenNum];
			len += b << of;
			position ++;
		}
		position --;
		len = len > 1 ? (len - 1)/2 : len;
		if (len > 0 && !IsBadWritePtr((PVOID)str, len))
		{
			CopyMemory((PVOID)str, &stream[position], len);
			position += len;
		}
		if (readed != NULL)
		{
			*readed = len;
		}
		return TRUE;
	}
	return FALSE;
}

SHORT AmfStream::GetUndefinedLengthBytesNum(LONG len)
{
	SHORT i = 0;
	SHORT offset[4] = {0,7, 14, 21};
	while(((len >> offset[i])%80) != 0)
		i ++;
	return i == 0 ? 1 : i;
}