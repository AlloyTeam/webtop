#pragma once
#include <Windows.h>
#include <tchar.h>

class CodeConvertor
{
public:
	CodeConvertor(void);
	~CodeConvertor(void);

public:
	LPCSTR Gb2Utf8(LPCSTR input);
	LPCSTR Utf82Gb(LPCSTR input);
};
