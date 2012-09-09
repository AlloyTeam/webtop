#include "P2P.h"
#include "transparent_wnd.h"
DWORD WINAPI P2P::RecvThreadProc(LPVOID lpParameter)
{
	sockaddr_in remote;
	int sinlen = sizeof(remote);
	stP2PMessage recvbuf;
	P2P* _this=(P2P*)lpParameter;
	TransparentWnd *win=(TransparentWnd *)_this->winHandler;
	for(;;)
	{
		int iread = recvfrom(_this->PrimaryUDP, (char *)&recvbuf, sizeof(recvbuf), 0, (sockaddr *)&remote, &sinlen);
		if(iread<=0)
		{
			printf("recv error\n");
			continue;
		}
		switch(recvbuf.iMessageType)
		{
		case P2PMESSAGE:
			{
				// 接收到P2P的消息
				char *comemessage= new char[recvbuf.iStringLen];
				int iread1 = recvfrom(_this->PrimaryUDP, comemessage, 256, 0, (sockaddr *)&remote, &sinlen);
				comemessage[iread1-1] = '\0';
				if(iread1<=0)
					throw Exception("Recv Message Error\n");
				else
				{
					printf("Recv a Message:%s\n",comemessage);
					
					stP2PMessage sendbuf;
					sendbuf.iMessageType = P2PMESSAGEACK;
					sendto(_this->PrimaryUDP, (const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr*)&remote, sizeof(remote));
				}

				win->RecieveMessage(P2PMESSAGE,comemessage,inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
				delete []comemessage;
				break;

			}
		case P2PSOMEONEWANTTOCALLYOU:
			{
				// 接收到打洞命令，向指定的IP地址打洞
				printf("Recv p2someonewanttocallyou data\n");
				sockaddr_in remote;
				remote.sin_addr.S_un.S_addr = htonl(recvbuf.iStringLen);
				remote.sin_family = AF_INET;
				remote.sin_port = htons(recvbuf.Port);

				// UDP hole punching
				stP2PMessage message;
				message.iMessageType = P2PTRASH;
				sendto(_this->PrimaryUDP, (const char *)&message, sizeof(message), 0, (const sockaddr*)&remote, sizeof(remote));
                
				break;
			}
		case P2PMESSAGEACK:
			{
				// 发送消息的应答
				_this->RecvedACK = true;
				break;
			}
		case P2PTRASH:
			{
				// 对方发送的打洞消息，忽略掉。
				//do nothing ...
				printf("Recv p2ptrash data\n");
				break;
			}
		case IPANDPORT:
			{
				int fromlen = sizeof(remote);
				stUserListNode node;
				in_addr tmp;
				recvfrom(_this->PrimaryUDP, (char*)&node, sizeof(stUserListNode), 0, (sockaddr *)&remote, &fromlen);
				tmp.S_un.S_addr = htonl(node.ip);
				strcpy(_this->IP, inet_ntoa(tmp));
				_this->port=node.port;
				win->RecieveMessage(IPANDPORT,"",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
				break;
			}
		case GETALLUSER:
			{
				int usercount;
				int fromlen = sizeof(remote);
				int iread = recvfrom(_this->PrimaryUDP, (char *)&usercount, sizeof(int), 0, (sockaddr *)&remote, &fromlen);
				if(iread<=0)
				{
					throw Exception("Login error\n");
				}
				
				_this->ClientList.clear();

				cout<<"Have "<<usercount<<" users logined server:"<<endl;
				for(int i = 0;i<usercount;i++)
				{
					stUserListNode *node = new stUserListNode;
					recvfrom(_this->PrimaryUDP, (char*)node, sizeof(stUserListNode), 0, (sockaddr *)&remote, &fromlen);
					_this->ClientList.push_back(node);
					cout<<"Username:"<<node->userName<<endl;
					in_addr tmp;
					tmp.S_un.S_addr = htonl(node->ip);
					cout<<"UserIP:"<<inet_ntoa(tmp)<<endl;
					cout<<"UserPort:"<<node->port<<endl;
					cout<<""<<endl;
				}
				win->RecieveMessage(GETALLUSER,"",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
				break;
			}
		}
	}
};
