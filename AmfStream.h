#pragma once

#define  AMF_STRING 0x2
#define  AMF_UNDEFINED 0x06

class AmfStream
{
public:
	AmfStream(PBYTE buffer);
	~AmfStream(void);

public:
	BOOL ReadBytes(PBYTE bytes, DWORD len);
	BOOL WriteBytes(const PBYTE bytes, DWORD len);

	ULONG ReadULong();
	BOOL WriteULong(ULONG len);
	BYTE ReadByte();
	BOOL WriteByte(const BYTE i);

	BOOL WriteString(LPCSTR str);
	BOOL ReadString(LPCSTR str, SHORT* readed, int max=0);

	BOOL WriteWString(LPCWSTR str);
	BOOL ReadWString(LPCWSTR str, SHORT* readed, int max=0);

	BOOL WriteStringSimple(LPCSTR str);
	BOOL ReadStringSimple(LPCSTR str, int len);

	BOOL WriteWStringSimple(LPCWSTR str);
	BOOL ReadWStringSimple(LPCWSTR str, int len);

	BOOL WriteUndefined(LPCSTR str, DWORD len);
	BOOL ReadUndefined(LPCSTR str, DWORD* readed);

	SHORT GetUndefinedLengthBytesNum(LONG len);

	LONG GetPosition(){return position;}
	void SetPosition(LONG pos){position=pos;}
	PBYTE GetStream(){return stream;}

public:
	void operator  += (LONG dwOffset){position += dwOffset;};
	void operator  -= (LONG dwOffset){position -= dwOffset;};
	void operator  =  (LONG pos){position = pos;};
	BYTE operator  [] (LONG pos){return stream[pos];};
	operator PBYTE()  {return stream;};

protected:
	LONG position;
	PBYTE stream;
};
