/* P2P 程序传输协议
 * 
 * 日期：2004-5-21
 *
 * 作者：shootingstars(zhouhuis22@sina.com)
 *
 */

#pragma once
#include <list>

// 定义iMessageType的值
#define LOGIN 1
#define LOGOUT 2
#define P2PTRANS 3
#define IPANDPORT 4
#define GETALLUSER  5

// 服务器端口
#define SERVER_PORT 2280

// Client登录时向服务器发送的消息
struct stLoginMessage
{
	char userName[10];
	char password[10];
};

// Client注销时发送的消息
struct stLogoutMessage
{
	char userName[10];
};

// Client向服务器请求另外一个Client(userName)向自己方向发送UDP打洞消息
struct stP2PTranslate
{
	char userName[10];
};

// Client向服务器发送的消息格式
struct stMessage
{
	int iMessageType;
	union _message
	{
		stLoginMessage loginmember;
		stLogoutMessage logoutmember;
		stP2PTranslate translatemessage;
	}message;
};

// 客户节点信息
struct stUserListNode
{
	char userName[10];
	unsigned int ip;
	unsigned short port;
};

// Server向Client发送的消息
struct stServerToClient
{
	int iMessageType;
	union _message
	{
		stUserListNode user;
	}message;

};

//======================================
// 下面的协议用于客户端之间的通信
//======================================
#define P2PMESSAGE 100               // 发送消息
#define P2PMESSAGEACK 101            // 收到消息的应答
#define P2PSOMEONEWANTTOCALLYOU 102  // 服务器向客户端发送的消息
                                     // 希望此客户端发送一个UDP打洞包
#define P2PTRASH        103          // 客户端发送的打洞包，接收端应该忽略此消息
#define TCPMESSAGE 105               // TCP消息

// 客户端之间发送消息格式
struct stP2PMessage
{
	int iMessageType;
	int iStringLen;         // or IP address
	unsigned short Port; 
};

using namespace std;
typedef list<stUserListNode *> UserList;

