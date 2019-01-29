#ifndef _BASESERVER_H
#define _BASESERVER_H
	#include<stdio.h>
	#include<thread>
	#include<vector>
	#include "ClientObject.h"
	#include "DataPackage.h"
	//openssl库
	#include <openssl/sha.h>
	#include <openssl/pem.h>
	#include <openssl/bio.h>
	#include <openssl/evp.h>

	#define GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

typedef struct _frame_head {
	char fin;
	char opcode;
	char mask;
	unsigned long long payload_length;
	char masking_key[4];
} frame_head;

class BaseServer
{
private:
	SOCKET m_ServerSocket;
	//这里使用指针防止爆栈
	std::vector<ClientObject*> m_Clients; 

public:
	BaseServer();
	virtual ~BaseServer();
	//初始化socket
	int InitSocket();
	//绑定端口号
	int Bind(const char* ip,unsigned short port);
	//监听端口号
	int Listen(int nlink);
	//关闭socket
	void Close();
	//处理网络消息
	bool OnRun();
	//是否在工作中
	bool isRun();
	//接收数据
	int ReadData(SOCKET cSock);
	//接受数据粘包拆包处理
	int ReadData(ClientObject *pClient);

	int ReadWebData(ClientObject* pClient);

	int ReadTextMsg(ClientObject* pClient);
	//处理网络消息
	virtual void OnNetMsg(SOCKET cSock, DataHeader * header);

	virtual void OnNetMsg(ClientObject* pClient,char* data);
	//发送给指定客户端
	long SendData(SOCKET cSock,DataHeader* header);

	long SendData(SOCKET cSock, char* data);

	long SendWebData(SOCKET cSock,char*data);
	//广播给所有客户端
	void SendDataToAll(DataHeader* header);

	void SendDataToAll(char* header);

	void SendWebDataToAll(char* header);

	const char *gb2312Toutf8(const char * gb2312);

private:
	//接受客户端
	SOCKET Accept();
	//缓冲区
	char _szRecv[RECV_BUFF_SIZE];
	//base64编码
	size_t base64_encode(char *in_str,int in_len,char *out_str);
	//逐行读取,每次读取一行字符串，返回下一行的开始位置
	int readline(char* allbuf, size_t level, char* linebuf);
	//握手函数
	int shakehands(ClientObject* client);
	int shakehands(int cli_fd);
	//大小端转换
	void inverted_string(char *str,int len);
	//接受及存储数据帧头
	int recv_frame_head(ClientObject *client,frame_head* head);
	int recv_frame_head(int fd,frame_head* head);
	//发送数据帧头
	int send_frame_head(ClientObject *client, frame_head* head);
	//去掩码，客户端发来的数据是异或加密的
	void umask(char *data,int len,char *mask);
	//定义一个帧头
	frame_head header;
};

#endif

