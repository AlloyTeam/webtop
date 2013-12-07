/* P2P 程序客户端
* 
* 文件名：P2PClient.c
*
* 日期：2004-5-21
*
* 作者：shootingstars(zhouhuis22@sina.com)
*
*/

#ifndef _TCPSERVER_H
#define _TCPSERVER_H
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include "windows.h"
#include "proto.h"
#include "Exception.h"
#include <iostream>
#include "cefclient/string_util.h"
using namespace std;

class TcpServer
{
private:
	SOCKET tcpSocket;
	SOCKET curSocket;
	char ServerIP[20];
public:
	char IP[20];
	unsigned short port;
	UserList ClientList;
	LPVOID winHandler;
	static DWORD WINAPI RecvThreadProc(LPVOID lpParameter);
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

	void BindSock(SOCKET sock)
	{
		sockaddr_in sin;
		sin.sin_addr.S_un.S_addr = INADDR_ANY;
		sin.sin_family = AF_INET;
		sin.sin_port = 0;

		if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
			throw Exception("bind error");
	}
	void TcpServer::init();

	void sendMsg(const char* msg){
		send(curSocket,msg,strlen(msg)+1, MSG_OOB);
	}

	static DWORD WINAPI waitForConnect(LPVOID lpParameter){
		TcpServer* _this=(TcpServer*)lpParameter;
		if(_this->curSocket!=NULL){
			closesocket(_this->curSocket);
			_this->curSocket=NULL;
		}
		int len=sizeof(SOCKADDR);  
		sockaddr_in sender;
		_this->curSocket = accept(_this->tcpSocket,(SOCKADDR*)&sender,&len);  

		HANDLE threadhandle = CreateThread(NULL, 0, RecvThreadProc, (LPVOID)lpParameter, NULL, NULL);
		CloseHandle(threadhandle);
		/*while(true){
			SOCKET temp=_this->curSocket;
			_this->curSocket = accept(_this->tcpSocket,(SOCKADDR*)&sender,&len);  
			closesocket(temp);
			//TerminateThread(threadhandle, 0);
			//threadhandle = CreateThread(NULL, 0, RecvThreadProc, (LPVOID)lpParameter, NULL, NULL);
		}*/
		return 0;
	}

	static void GetIP(char *ip, const char* hostName){
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

	static CefString GetLocalIP(){
		char ip[100];
		char   temp[100];
		if(gethostname(temp, sizeof(temp))==0){
			GetIP(ip, temp);
		}
		return ip;
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
	~TcpServer();
};
#endif