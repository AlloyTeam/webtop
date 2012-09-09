/* 异常类
 *
 * 文件名：Exception.h
 *
 * 日期：2004.5.5
 *
 * 作者：shootingstars(zhouhuis22@sina.com)
 */

#ifndef __HZH_Exception__
#define __HZH_Exception__

#define EXCEPTION_MESSAGE_MAXLEN 256
#include "string.h"

class Exception
{
private:
	char m_ExceptionMessage[EXCEPTION_MESSAGE_MAXLEN];
public:
	Exception(char *msg)
	{
		strncpy(m_ExceptionMessage, msg, EXCEPTION_MESSAGE_MAXLEN);
	}

	char *GetMessage()
	{
		return m_ExceptionMessage;
	}
};



#endif