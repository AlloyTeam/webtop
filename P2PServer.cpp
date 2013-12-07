/* P2P 程序服务端
 * 
 * 文件名：P2PServer.c
 * Download:http://www.codefans.net
 * 日期：2004-5-21
 *
 * 作者：shootingstars(zhouhuis22@sina.com)
 *
 */
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include "windows.h"
#include "proto.h"
#include "Exception.h"
#include <iostream>
using namespace std;

UserList ClientList;

void InitWinSock()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Windows sockets 2.2 startup");
		throw Exception("");
	}
	else{
		printf("Using %s (Status: %s)\n",
			wsaData.szDescription, wsaData.szSystemStatus);
		printf("with API versions %d.%d to %d.%d\n\n",
			LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion),
			LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
		
	}
}

SOCKET mksock(int type)
{
	SOCKET sock = socket(AF_INET, type, 0);
	if (sock < 0)
	{
        printf("create socket error");
		throw Exception("");
	}
	return sock;
}

stUserListNode GetUser(char *username)
{
	for(UserList::iterator UserIterator=ClientList.begin();
						UserIterator!=ClientList.end();
							++UserIterator)
	{
		if( strcmp( ((*UserIterator)->userName), username) == 0 )
			return *(*UserIterator);
	}
	throw Exception("not find this user");
}

int main(int argc, char* argv[])
{
	try{
		InitWinSock();
		
		SOCKET PrimaryUDP;
		PrimaryUDP = mksock(SOCK_DGRAM);

		sockaddr_in local;
		local.sin_family=AF_INET;
		local.sin_port= htons(SERVER_PORT); 
		local.sin_addr.s_addr = htonl(INADDR_ANY);
		int nResult=bind(PrimaryUDP,(sockaddr*)&local,sizeof(sockaddr));
		if(nResult==SOCKET_ERROR)
			throw Exception("bind error");

		sockaddr_in sender;
		stMessage recvbuf;
		memset(&recvbuf,0,sizeof(stMessage));

		// 开始主循环.
		// 主循环负责下面几件事情:
		// 一:读取客户端登陆和登出消息,记录客户列表
		// 二:转发客户p2p请求
		for(;;)
		{
			int dwSender = sizeof(sender);
			int ret = recvfrom(PrimaryUDP, (char *)&recvbuf, sizeof(stMessage), 0, (sockaddr *)&sender, &dwSender);
			if(ret <= 0)
			{
				printf("recv error");
				continue;
			}
			else
			{
				int messageType = recvbuf.iMessageType;
				switch(messageType){
				case LOGIN:
					{
						//  将这个用户的信息记录到用户列表中
						printf("has a user login : %s\n", recvbuf.message.loginmember.userName);
						stUserListNode *currentuser = new stUserListNode();
						strcpy(currentuser->userName, recvbuf.message.loginmember.userName);
						currentuser->ip = ntohl(sender.sin_addr.S_un.S_addr);
						currentuser->port = ntohs(sender.sin_port);
						
						ClientList.push_back(currentuser);

						// 发送已经登陆的客户信息
						int nodecount = (int)ClientList.size();
						sendto(PrimaryUDP, (const char*)&nodecount, sizeof(int), 0, (const sockaddr*)&sender, sizeof(sender));
						for(UserList::iterator UserIterator=ClientList.begin();
								UserIterator!=ClientList.end();
								++UserIterator)
						{
							sendto(PrimaryUDP, (const char*)(*UserIterator), sizeof(stUserListNode), 0, (const sockaddr*)&sender, sizeof(sender)); 
						}

						break;
					}
				case LOGOUT:
					{
						// 将此客户信息删除
						printf("has a user logout : %s\n", recvbuf.message.logoutmember.userName);
						UserList::iterator removeiterator;// = NULL;
						bool flag=false;
						for(UserList::iterator UserIterator=ClientList.begin();
							UserIterator!=ClientList.end();
							++UserIterator)
						{
							if( strcmp( ((*UserIterator)->userName), recvbuf.message.logoutmember.userName) == 0 )
							{
								removeiterator = UserIterator;
								flag=true;
								break;
							}
						}
						if(flag)
							ClientList.remove(*removeiterator);
						break;
					}
				case P2PTRANS:
					{
						// 某个客户希望服务端向另外一个客户发送一个打洞消息
						printf("%s wants to p2p %s\n",inet_ntoa(sender.sin_addr),recvbuf.message.translatemessage.userName);
						stUserListNode node = GetUser(recvbuf.message.translatemessage.userName);
						sockaddr_in remote;
						remote.sin_family=AF_INET;
						remote.sin_port= htons(node.port); 
						remote.sin_addr.s_addr = htonl(node.ip);

						in_addr tmp;
						tmp.S_un.S_addr = htonl(node.ip);
						printf("the address is %s,and port is %d\n",inet_ntoa(tmp), node.port);

						stP2PMessage transMessage;
						transMessage.iMessageType = P2PSOMEONEWANTTOCALLYOU;
						transMessage.iStringLen = ntohl(sender.sin_addr.S_un.S_addr);
						transMessage.Port = ntohs(sender.sin_port);
                        
						sendto(PrimaryUDP,(const char*)&transMessage, sizeof(transMessage), 0, (const sockaddr *)&remote, sizeof(remote));

						break;
					}
				
				case GETALLUSER:
					{
						int command = GETALLUSER;
						sendto(PrimaryUDP, (const char*)&command, sizeof(int), 0, (const sockaddr*)&sender, sizeof(sender));

						int nodecount = (int)ClientList.size();
						sendto(PrimaryUDP, (const char*)&nodecount, sizeof(int), 0, (const sockaddr*)&sender, sizeof(sender));

						for(UserList::iterator UserIterator=ClientList.begin();
								UserIterator!=ClientList.end();
								++UserIterator)
						{
							sendto(PrimaryUDP, (const char*)(*UserIterator), sizeof(stUserListNode), 0, (const sockaddr*)&sender, sizeof(sender)); 
						}
						break;
					}
				}
			}
		}

	}
	catch(Exception &e)
	{
		printf(e.GetMessage());
		return 1;
	}

	return 0;
}

