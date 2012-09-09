/* P2P 程序客户端
 * 
 * 文件名：P2PClient.c
 *
 * 日期：2004-5-21
 *
 * 作者：shootingstars(zhouhuis22@sina.com)
 *
 */

#ifndef _P2P_H
#define _P2P_H
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include "windows.h"
#include "proto.h"
#include "Exception.h"
#include <iostream>
using namespace std;


#define COMMANDMAXC 256
#define MAXRETRY    5

class P2P
{
private:
	SOCKET PrimaryUDP;
	char UserName[100];
	char ServerIP[20];
	bool RecvedACK;
public:
	char IP[20];
	unsigned short port;
	UserList ClientList;
	LPVOID winHandler;
	int tryCount;
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

	void BindSock(SOCKET sock)
	{
		sockaddr_in sin;
		sin.sin_addr.S_un.S_addr = INADDR_ANY;
		sin.sin_family = AF_INET;
		sin.sin_port = 0;
	
		if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
			throw Exception("bind error");
	}

	void ConnectToServer(SOCKET sock, const char *username, const char *serverip)
	{
		sockaddr_in remote;
		remote.sin_addr.S_un.S_addr = inet_addr(serverip);
		remote.sin_family = AF_INET;
		remote.sin_port = htons(SERVER_PORT);
	
		stMessage sendbuf;
		sendbuf.iMessageType = LOGIN;
		strncpy(sendbuf.message.loginmember.userName, username, 10);

		sendto(sock, (const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr*)&remote,sizeof(remote));
	}

	void OutputUsage()
	{
		cout<<"You can input you command:\n"
			<<"Command Type:\"send\",\"exit\",\"getu\"\n"
			<<"Example : send Username Message\n"
			<<"          exit\n"
			<<"          getu\n"
			<<endl;
	}

	void GetU(){
		int command = GETALLUSER;
		sockaddr_in server;
		server.sin_addr.S_un.S_addr = inet_addr(ServerIP);
		server.sin_family = AF_INET;
		server.sin_port = htons(SERVER_PORT);

		sendto(PrimaryUDP,(const char*)&command, sizeof(command), 0, (const sockaddr *)&server, sizeof(server));
	}

	/* 这是主要的函数：发送一个消息给某个用户(C)
	 *流程：直接向某个用户的外网IP发送消息，如果此前没有联系过
	 *      那么此消息将无法发送，发送端等待超时。
	 *      超时后，发送端将发送一个请求信息到服务端，
	 *      要求服务端发送给客户C一个请求，请求C给本机发送打洞消息
	 *      以上流程将重复MAXRETRY次
	 */
	bool SendMessageTo(const char *UserName, const char *Message)
	{
		char realmessage[256];
		unsigned int UserIP;
		unsigned short UserPort;
		bool FindUser = false;
		for(UserList::iterator UserIterator=ClientList.begin();
							UserIterator!=ClientList.end();
							++UserIterator)
		{
			if( strcmp( ((*UserIterator)->userName), UserName) == 0 )
			{
				UserIP = (*UserIterator)->ip;
				UserPort = (*UserIterator)->port;
				FindUser = true;
			}
		}

		if(!FindUser){
			GetU();
			Sleep(1000);
			++tryCount;
			if(tryCount<MAXRETRY){
				return SendMessageTo(UserName,Message);
			}
			else{
				tryCount=0;
				return false;
			}
		}
		tryCount=0;
		//return false;

		strcpy(realmessage, Message);
		for(int i=0;i<MAXRETRY;i++)
		{
			RecvedACK = false;

			sockaddr_in remote;
			remote.sin_addr.S_un.S_addr = htonl(UserIP);
			remote.sin_family = AF_INET;
			remote.sin_port = htons(UserPort);
			stP2PMessage MessageHead;
			MessageHead.iMessageType = P2PMESSAGE;
			MessageHead.iStringLen = (int)strlen(realmessage)+1;
			int isend = sendto(PrimaryUDP, (const char *)&MessageHead, sizeof(MessageHead), 0, (const sockaddr*)&remote, sizeof(remote));
			isend = sendto(PrimaryUDP, (const char *)&realmessage, MessageHead.iStringLen, 0, (const sockaddr*)&remote, sizeof(remote));
		
			// 等待接收线程将此标记修改
			for(int j=0;j<10;j++)
			{
				if(RecvedACK)
					return true;
				else
					Sleep(300);
			}

			// 没有接收到目标主机的回应，认为目标主机的端口映射没有
			// 打开，那么发送请求信息给服务器，要服务器告诉目标主机
			// 打开映射端口（UDP打洞）
			sockaddr_in server;
			server.sin_addr.S_un.S_addr = inet_addr(ServerIP);
			server.sin_family = AF_INET;
			server.sin_port = htons(SERVER_PORT);
	
			stMessage transMessage;
			transMessage.iMessageType = P2PTRANS;
			strcpy(transMessage.message.translatemessage.userName, UserName);

			sendto(PrimaryUDP, (const char*)&transMessage, sizeof(transMessage), 0, (const sockaddr*)&server, sizeof(server));
			Sleep(100);// 等待对方先发送信息。
		}
		return false;
	}

	/* 这是主要的函数：发送一个消息给某个用户(C)
	 *流程：直接向某个用户的外网IP发送消息，如果此前没有联系过
	 *      那么此消息将无法发送，发送端等待超时。
	 *      超时后，发送端将发送一个请求信息到服务端，
	 *      要求服务端发送给客户C一个请求，请求C给本机发送打洞消息
	 *      以上流程将重复MAXRETRY次
	 */
	bool SendMessageToEx(const char* UserIP, unsigned short UserPort, const char *Message)
	{
		unsigned int ip=inet_addr(UserIP);
		char realmessage[256];
		strcpy(realmessage, Message);
		for(int i=0;i<MAXRETRY;i++)
		{
			RecvedACK = false;

			sockaddr_in remote;
			remote.sin_addr.S_un.S_addr = ip;
			remote.sin_family = AF_INET;
			remote.sin_port = htons(UserPort);
			stP2PMessage MessageHead;
			MessageHead.iMessageType = P2PMESSAGE;
			MessageHead.iStringLen = (int)strlen(realmessage)+1;
			int isend = sendto(PrimaryUDP, (const char *)&MessageHead, sizeof(MessageHead), 0, (const sockaddr*)&remote, sizeof(remote));
			isend = sendto(PrimaryUDP, (const char *)&realmessage, MessageHead.iStringLen, 0, (const sockaddr*)&remote, sizeof(remote));
		
			// 等待接收线程将此标记修改
			for(int j=0;j<10;j++)
			{
				if(RecvedACK)
					return true;
				else
					Sleep(300);
			}

		}
		return false;
	}

	bool SendMessageToServer(const char *Message)
	{
		char realmessage[256];
		strcpy(realmessage, Message);
		for(int i=0;i<MAXRETRY;i++)
		{
			RecvedACK = false;

			sockaddr_in remote;
			remote.sin_addr.S_un.S_addr = inet_addr(ServerIP);
			remote.sin_family = AF_INET;
			remote.sin_port = htons(SERVER_PORT);
			stP2PMessage MessageHead;
			MessageHead.iMessageType = P2PMESSAGE;
			MessageHead.iStringLen = (int)strlen(realmessage)+1;
			int isend = sendto(PrimaryUDP, (const char *)&MessageHead, sizeof(MessageHead), 0, (const sockaddr*)&remote, sizeof(remote));
			isend = sendto(PrimaryUDP, (const char *)&realmessage, MessageHead.iStringLen, 0, (const sockaddr*)&remote, sizeof(remote));
		
			// 等待接收线程将此标记修改
			for(int j=0;j<10;j++)
			{
				if(RecvedACK)
					return true;
				else
					Sleep(300);
			}

		}
		return false;
	}


	// 解析命令，暂时只有exit和send命令
	// 新增getu命令，获取当前服务器的所有用户
	void ParseCommand(char * CommandLine)
	{
		if(strlen(CommandLine)<4)
			return;
		char Command[5];
		strncpy(Command, CommandLine, 4);
		Command[4]='\0';

		if(strcmp(Command,"exit")==0)
		{
			stMessage sendbuf;
			sendbuf.iMessageType = LOGOUT;
			strncpy(sendbuf.message.logoutmember.userName, UserName, 10);
			sockaddr_in server;
			server.sin_addr.S_un.S_addr = inet_addr(ServerIP);
			server.sin_family = AF_INET;
			server.sin_port = htons(SERVER_PORT);

			sendto(PrimaryUDP,(const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr *)&server, sizeof(server));
			shutdown(PrimaryUDP, 2);
			closesocket(PrimaryUDP);
			exit(0);
		}
		else if(strcmp(Command,"send")==0)
		{
			char sendname[20];
			char message[COMMANDMAXC];
			int i;
			for(i=5;;i++)
			{
				if(CommandLine[i]!=' ')
					sendname[i-5]=CommandLine[i];
				else
				{
					sendname[i-5]='\0';
					break;
				}
			}
			strcpy(message, &(CommandLine[i+1]));
			tryCount=0;
			if(SendMessageTo(sendname, message))
				printf("Send OK!\n");
			else 
				printf("Send Failure!\n");
		}
		else if(strcmp(Command,"tose")==0){
			char message[COMMANDMAXC];
			strcpy(message, &(CommandLine[5]));
			if(SendMessageToServer(message))
				printf("Send OK!\n");
			else 
				printf("Send Failure!\n");
		}
		else if(strcmp(Command,"toip")==0)
		{
			char sendip[20];
			char port[10];
			char message[COMMANDMAXC];
			int i;
			for(i=5;;i++)
			{
				if(CommandLine[i]!=' ')
					sendip[i-5]=CommandLine[i];
				else
				{
					sendip[i-5]='\0';
					break;
				}
			}
			++i;
			int index=i;
			for(;;i++)
			{
				if(CommandLine[i]!=' ')
					port[i-index]=CommandLine[i];
				else
				{
					port[i-index]='\0';
					break;
				}
			}
			unsigned short UserPort=atoi(port);
			strcpy(message, &(CommandLine[i+1]));
			cout<<sendip<<"\n"<<port<<"\n"<<message<<"\n";
			tryCount=0;
			if(SendMessageToEx(sendip, UserPort, message))
				printf("Send OK!\n");
			else 
				printf("Send Failure!\n");
		}
		else if(strcmp(Command,"getu")==0)
		{
			int command = GETALLUSER;
			sockaddr_in server;
			server.sin_addr.S_un.S_addr = inet_addr(ServerIP);
			server.sin_family = AF_INET;
			server.sin_port = htons(SERVER_PORT);

			sendto(PrimaryUDP,(const char*)&command, sizeof(command), 0, (const sockaddr *)&server, sizeof(server));
		}
	}
	// 接受消息线程
	static DWORD WINAPI RecvThreadProc(LPVOID lpParameter);

	void GetIP(char *ip, const char* hostName){
		BYTE   *p;   
		struct   hostent   *hp; 
		//char   ip[16]; 

		if((hp   =gethostbyname(hostName))!=0) 
		{ 
			p   =(BYTE   *)hp-> h_addr;   
			sprintf(ip,   "%d.%d.%d.%d ",   p[0],   p[1],   p[2],   p[3]); 
			cout<<ip<<"\n";
		} 
	}
	void GetLocalIP(char *ip){
		char   temp[100];
		if(gethostname(temp, sizeof(temp))==0){
			GetIP(ip, temp);
		}
	}
	void GetPort(SOCKET sock){
		struct sockaddr addr;
		struct sockaddr_in* addr_v4;
		int addr_len = sizeof(addr);
		//获取local ip and port
		ZeroMemory(&addr, sizeof(addr));
		if (0 == getsockname(sock, &addr, &addr_len))
		{
			if (addr.sa_family == AF_INET)
			{
				addr_v4 = (sockaddr_in*)&addr;
				port = ntohs(addr_v4->sin_port);
			}
		}

	}
	void Init(){
		InitWinSock();
		PrimaryUDP = mksock(SOCK_DGRAM);
		BindSock(PrimaryUDP);
		tryCount=0;
	}
	void Connect(const char* IP, const char* uid){
		Init();
		strcpy(ServerIP, IP);
		strcpy(UserName, uid);
		ConnectToServer(PrimaryUDP, uid, IP);
		HANDLE threadhandle = CreateThread(NULL, 0, RecvThreadProc, (LPVOID)this, NULL, NULL);
		CloseHandle(threadhandle);
	}

	void ConnectByHost(const char* hostName, const char* uid){
		Init();
		char ip[100];
		GetIP(ip, hostName);
		strcpy(ServerIP, ip);
		strcpy(UserName, uid);
		ConnectToServer(PrimaryUDP, uid, ip);
		HANDLE threadhandle = CreateThread(NULL, 0, RecvThreadProc, (LPVOID)this, NULL, NULL);
		CloseHandle(threadhandle);
	}
	~P2P(){
		stMessage sendbuf;
		sendbuf.iMessageType = LOGOUT;
		strncpy(sendbuf.message.logoutmember.userName, UserName, 10);
		sockaddr_in server;
		server.sin_addr.S_un.S_addr = inet_addr(ServerIP);
		server.sin_family = AF_INET;
		server.sin_port = htons(SERVER_PORT);

		sendto(PrimaryUDP,(const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr *)&server, sizeof(server));
		shutdown(PrimaryUDP, 2);
		closesocket(PrimaryUDP);
	}
};
#endif