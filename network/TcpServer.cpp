#include "TcpServer.h"
#include "transparent_wnd.h"
#include <sstream>

DWORD WINAPI TcpServer::RecvThreadProc(LPVOID lpParameter)
{
	TcpServer* _this=(TcpServer*)lpParameter;
	TransparentWnd *win=(TransparentWnd *)_this->winHandler;
	int size=0;
	//std::stringstream ss;
	int count=0;
	char* s=NULL;
	bool startReceive=false;
	int retry=0;
	//开始接受数据，等待
	for(;;)
	{
		char recvBuf[40960] = {0};
		int l=recv(_this->curSocket,recvBuf,40960,0);
		if(l==0) {
			++retry;
			if(retry>3){
				if(!startReceive){
					size=0;
					if(s!=NULL){
						delete s;
						s=NULL;
					}
					count=0;
					waitForConnect(lpParameter);
					break;
					retry=0;
				}
			}
		}
		else{
			if(size==0&&l==4){
				int *b= (int*)recvBuf;
				size=(int)(*b);
				if(s!=NULL){
					delete s;
					s=NULL;
				}
				s=new char[size+1];
				s[size]=0;
			}
			else if(s!=NULL&&size>0){
				for(int i=count,j=0;i<count+l&&i<size;++i,++j){
					s[i]=recvBuf[j];
				}
				count+=l;
				if(count>=size){
					win->RecieveMessage(TCPMESSAGE,s,0,0);
					size=0;
					count=0;
				}
			}
		}

	}
	return 0;
}

void TcpServer::init(){
	try{
		InitWinSock();

		tcpSocket = mksock(SOCK_STREAM);

		sockaddr_in local;
		local.sin_family=AF_INET;
		local.sin_port= htons(SERVER_PORT); 
		local.sin_addr.s_addr = htonl(INADDR_ANY);
		int nResult=bind(tcpSocket,(sockaddr*)&local,sizeof(sockaddr));
		if(nResult==SOCKET_ERROR)
			throw Exception("bind error");
		listen(tcpSocket,10);

		//一下代码获得IP和计算机名  
		char name[255];  
		char* ip = NULL;  
		PHOSTENT hostInfo;  
		if (gethostname(name,sizeof(name)) == 0)  
		{  
			if (hostInfo = gethostbyname(name))  
			{  
				ip = inet_ntoa(*(in_addr*)*(hostInfo->h_addr_list));  
			}  
		}  
		printf("%s:%d--%s\n",ip,SERVER_PORT,name);
		HANDLE threadhandle = CreateThread(NULL, 0, waitForConnect, (LPVOID)this, NULL, NULL);
		CloseHandle(threadhandle);
	}
	catch(Exception &e)
	{
		printf(e.GetMessage());
	}
}
TcpServer::~TcpServer(){
	shutdown(tcpSocket, 2);
	closesocket(curSocket);
	curSocket=NULL;
}
