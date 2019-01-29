#ifndef _CLIENTOBJECT_H
#define _CLIENTOBJECT_H
#ifdef _WIN32
	#define FD_SETSIZE  1024 //在WinSocok2.h前面定义，突破Windows下只能64的限制
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h> //uni std
	#include<arpa/inet.h>
	#include<string.h>
	#include <signal.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#define RECV_BUFF_SIZE 10240
#define BUFFER_SIZE 1024
class ClientObject
{
public:
	ClientObject(SOCKET sockfd = INVALID_SOCKET);
	virtual ~ClientObject();
public:
	SOCKET getClientSocket();

	char* getMsgBuf();
	size_t getCurPos();
	void setCurPos(size_t pos);
	//消息缓冲区的数据位置
	size_t _curPos;
private:
	SOCKET m_clientsocket;

	//第二缓冲区，消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	
};
#endif
