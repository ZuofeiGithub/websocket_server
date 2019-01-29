#define _CRT_SECURE_NO_WARNINGS
#include "BaseServer.h"
#include "decode.h"


BaseServer::BaseServer()
{
	m_ServerSocket = INVALID_SOCKET;
	memset(_szRecv, 0, RECV_BUFF_SIZE);
    m_Clients.clear();
}


BaseServer::~BaseServer()
{
	Close();
}

int BaseServer::InitSocket()
{
	//启动Win Sock 2.x环境
#ifdef _WIN32
	//socket的版本号
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#else
	signal(SIGPIPE,SIG_IGN);
#endif

	if (m_ServerSocket != INVALID_SOCKET)
	{
		//关闭之前的连接
		Close();
	}
	//1.建立一个新socket
	m_ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_ServerSocket) {
		return -1;
	}
	return 0;
}
//绑定ip和端口号
int BaseServer::Bind(const char * ip, unsigned short port)
{
	if (INVALID_SOCKET == m_ServerSocket)
	{
		InitSocket();
	}
	// 2 bind 绑定用于接受客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);//host to net unsigned short
#ifdef _WIN32
	if (ip)
	{
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
	}
	else {
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	}
#else
	if (ip)
	{
		_sin.sin_addr.s_addr = inet_addr(ip);
	}
	else {
		_sin.sin_addr.s_addr = INADDR_ANY;
	}

#endif
	int ret = bind(m_ServerSocket, (sockaddr*)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == ret)
	{
		printf("错误,绑定网络端口<%d>失败...\n", port);
	}
	else {
		printf("绑定网络端口<%d>成功...\n", port);
	}
	return ret;
}

int BaseServer::Listen(int nlink)
{
	// 3 listen 监听网络端口
	int ret = listen(m_ServerSocket, nlink);
	if (SOCKET_ERROR == ret)
	{
		printf("错误,监听网络端口失败...\n");
	}
	else {
		printf("监听网络端口成功...\n");
	}
	return ret;
}

SOCKET BaseServer::Accept()
{
	// 4 accept 等待接受客户端连接
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
	cSock = accept(m_ServerSocket, (sockaddr*)&clientAddr, &nAddrLen);
#else
	cSock = accept(m_ServerSocket, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
	if (INVALID_SOCKET == cSock)
	{
		printf("错误,接受到无效客户端SOCKET...\n");
	}
	else
	{
		shakehands(new ClientObject(cSock));
		m_Clients.push_back(new ClientObject(cSock));
		printf("新客户端加入：socket = %d,IP = %s \n", (int)cSock, inet_ntoa(clientAddr.sin_addr));
	}
	return cSock;
}

size_t BaseServer::base64_encode(char * in_str, int in_len, char * out_str)
{
	BIO *b64, *bio;
	BUF_MEM *bptr = NULL;
	size_t size = 0;

	if (in_str == NULL || out_str == NULL)
		return -1;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_write(bio, in_str, in_len);
	BIO_flush(bio);

	BIO_get_mem_ptr(bio, &bptr);
	memcpy(out_str, bptr->data, bptr->length);
	out_str[bptr->length - 1] = '\0';
	size = (size_t)bptr->length;

	BIO_free_all(bio);
	return size;
}

int BaseServer::readline(char * allbuf, size_t level, char * linebuf)
{
	size_t len = strlen(allbuf);
	for (; level < len; ++level)
	{
		if (allbuf[level] == '\r'&&allbuf[level + 1] == '\n')
		{
			return level + 2;
		}
		else {
			*(linebuf++) = allbuf[level];
		}
	}
	return -1;
}

int BaseServer::shakehands(ClientObject* client)
{
	//下一行的指针位置
	int level = 0;
	//所有请求数据
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	//一行数据
	char linebuf[256];
	memset(linebuf, 0, 256);
	//Sec-WebSocket-Accept
	char sec_accept[32];
	memset(sec_accept, 0, 32);
	//哈希数据
	unsigned char sha1_data[SHA_DIGEST_LENGTH + 1];
	memset(sha1_data, 0, SHA_DIGEST_LENGTH + 1);
	//响应头数据
	char head[BUFFER_SIZE];
	memset(head, 0, BUFFER_SIZE);
	if (recv(client->getClientSocket(), buffer, BUFFER_SIZE, 0) <= 0)
	{
		perror("取数据出错...\n");
		return -1;
	}
	/*printf("请求是:%s", buffer);*/

	do {
		memset(linebuf, 0, sizeof(linebuf));
		level = readline(buffer, level, linebuf);
		//找子串
		if (strstr(linebuf, "Sec-WebSocket-Key") != NULL)
		{
			strcat(linebuf, GUID);//将guid拼接在linebuf后面
			SHA1((unsigned char*)&linebuf + 19, strlen(linebuf + 19), (unsigned char*)&sha1_data);
			base64_encode((char*)sha1_data, 20, sec_accept);
			sprintf(head, "HTTP/1.1 101 Switching Protocols\r\n" \
				"Upgrade: websocket\r\n" \
				"Connection: Upgrade\r\n" \
				"Sec-WebSocket-Accept: %s\r\n" \
				"\r\n", sec_accept);
			/*printf("响应头:%s\n", head);*/
			if (SendData(client->getClientSocket(), head) < 0)
			{
				perror("写数据出错...\n");
				return -1;
			}
			break;
		}
	} while (_szRecv[level] != '\r' || (_szRecv[level + 1] != '\n'&& level != -1));
	return 0;
}

int BaseServer::shakehands(int cli_fd)
{
	//next line's point num
	int level = 0;
	//all request data
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	//a line data
	char linebuf[256];
	memset(linebuf, 0, 256);
	//Sec-WebSocket-Accept
	char sec_accept[32];
	memset(sec_accept, 0, 32);
	//sha1 data
	char sha1_data[SHA_DIGEST_LENGTH + 1];
	memset(sha1_data, 0, SHA_DIGEST_LENGTH + 1);
	//reponse head buffer
	char head[BUFFER_SIZE];
	memset(head, 0, BUFFER_SIZE);

	if (recv(cli_fd, buffer, sizeof(buffer), 0) <= 0)
	{
		perror("read");
		return -1;
	}

	printf("request\n");
	printf("%s\n", buffer);

	do {
		memset(linebuf, 0, sizeof(linebuf));
		level = readline(buffer, level, linebuf);
		//printf("line:%s\n",linebuf);

		if (strstr(linebuf, "Sec-WebSocket-Key") != NULL)
		{
			strcat(linebuf, GUID);
			SHA1((unsigned char*)&linebuf + 19, strlen(linebuf + 19), (unsigned char*)&sha1_data);
			base64_encode(sha1_data, 20, sec_accept);
			sprintf(head, "HTTP/1.1 101 Switching Protocols\r\n" \
				"Upgrade: websocket\r\n" \
				"Connection: Upgrade\r\n" \
				"Sec-WebSocket-Accept: %s\r\n" \
				"\r\n", sec_accept);
			/*printf("response\n");
			printf("%s", head);*/
			if (send(cli_fd, head, strlen(head), 0) < 0)
			{
				perror("write");
				return -1;
			}

			break;
		}
	} while ((buffer[level] != '\r' || buffer[level + 1] != '\n') && level != -1);
	return 0;
}

void BaseServer::inverted_string(char * str, int len)
{
	int i = 0;
	char temp = 0;
	for (i = 0; i < len / 2; ++i)
	{
		temp = *(str + i);
		*(str + i) = *(str + len - i - 1);
		*(str + len - i - 1) = temp;
	}
}

int BaseServer::recv_frame_head(ClientObject * client, frame_head * head)
{
	char one_char;
	if (recv(client->getClientSocket(), &one_char, 1, 0) <= 0)
	{
		return -1;
	}
	head->fin = (one_char & 0x80) == 0x80;
	head->opcode = one_char & 0x0F;
	if (recv(client->getClientSocket(), &one_char, 1, 0) <= 0)
	{
		perror("读取mask出错...\n");
		return -1;
	}
	head->mask = (one_char & 0x80) == 0x80;
	//获取有用数据
	head->payload_length = one_char & 0x7F;
	if (head->payload_length == 126)
	{
		char extern_len[2] = { 0 };
		if (recv(client->getClientSocket(), extern_len, 2, 0) <= 0)
		{
			perror("读取额外长度出错...\n");
			return -1;
		}
		head->payload_length = (extern_len[0] & 0xFF) << 8 | (extern_len[1] & 0xFF);
	}
	else if (head->payload_length == 127)
	{
		char extern_len[8];
		memset(extern_len, 0, 8);
		if (recv(client->getClientSocket(), extern_len, 8, 0) <= 0)
		{
			perror("read extern_len");
			return -1;
		}
		inverted_string(extern_len, 8);
		memcpy(&(head->payload_length), extern_len, 8);
	}

	if (recv(client->getClientSocket(), head->masking_key, 4, 0) <= 0)
	{
		perror("read masking-key");
		return -1;
	}

	return 0;
}

int BaseServer::recv_frame_head(int fd, frame_head * head)
{
	char one_char;
	/*read fin and op code*/
	if (recv(fd, &one_char, 1, 0) <= 0)
	{
		//perror("read fin");
		//printf("客户端退出!!!");
		return -1;
	}
	head->fin = (one_char & 0x80) == 0x80;
	head->opcode = one_char & 0x0F;
	if (recv(fd, &one_char, 1, 0) <= 0)
	{
		perror("read mask");
		return -1;
	}
	head->mask = (one_char & 0x80) == 0X80;

	/*get payload length*/
	head->payload_length = one_char & 0x7F;

	if (head->payload_length == 126)
	{
		char extern_len[2];
		memset(extern_len, 0, 2);
		if (recv(fd, extern_len, 2, 0) <= 0)
		{
			perror("read extern_len");
			return -1;
		}
		head->payload_length = (extern_len[0] & 0xFF) << 8 | (extern_len[1] & 0xFF);
	}
	else if (head->payload_length == 127)
	{
		char extern_len[8] = { 0 };
		if (recv(fd, extern_len, 8, 0) <= 0)
		{
			perror("read extern_len");
			return -1;
		}
		inverted_string(extern_len, 8);
		memcpy(&(head->payload_length), extern_len, 8);
	}

	/*read masking-key*/
	if (recv(fd, head->masking_key, 4, 0) <= 0)
	{
		perror("read masking-key");
		return -1;
	}

	return 0;
}

int BaseServer::send_frame_head(ClientObject * client, frame_head * head)
{

	char *response_head;
	int head_length = 0;
	if (head->payload_length < 126)
	{
		response_head = (char*)malloc(2);
		memset(response_head, 0, 2);
		response_head[0] = 0x82; //0x81不能发中文,0x82可以发中文
		response_head[1] = (char)head->payload_length;
		head_length = 2;
	}
	else if (head->payload_length < 0xFFFF)
	{
		response_head = (char*)malloc(4);
		memset(response_head, 0, 4);
		memset(response_head, 0, 4);
		response_head[0] = 0x82;
		response_head[1] = 126;
		response_head[2] = (head->payload_length >> 8 & 0xFF);
		response_head[3] = (head->payload_length & 0xFF);
		head_length = 4;
	}
	else
	{
		response_head = (char*)malloc(12);
		memset(response_head, 0, 12);
		memset(response_head, 0, 12);
		response_head[0] = 0x82;
		response_head[1] = 127;
		memcpy(response_head + 2, (const void*)head->payload_length, sizeof(unsigned long long));
		inverted_string(response_head + 2, sizeof(unsigned long long));
		head_length = 12;
	}
	if (send(client->getClientSocket(), response_head, head_length, 0) <= 0)
	{
		return -1;
	}
	free(response_head);
	return 0;
}

void BaseServer::umask(char * data, int len, char * mask)
{
	int i;
	for (i = 0; i < len; ++i)
	{
		*(data + i) ^= *(mask + (i % 4));
	}

}


void BaseServer::Close()
{
	if (m_ServerSocket != INVALID_SOCKET)
	{
#ifdef _WIN32
		for (int n = (int)m_Clients.size() - 1; n >= 0; n--)
		{
			closesocket(m_Clients[n]->getClientSocket());
			delete m_Clients[n];
		}
		// 8 关闭套节字closesocket
		closesocket(m_ServerSocket);
		//------------
		//清除Windows socket环境
		WSACleanup();
#else
		for (int n = (int)m_Clients.size() - 1; n >= 0; n--)
		{
			close(m_Clients[n]->getClientSocket());
			delete m_Clients[n];
		}
		// 8 关闭套节字closesocket
		close(m_ServerSocket);
#endif
		//服务端关闭，清理所有客户端残留
		m_Clients.clear();
	}
}

bool BaseServer::OnRun()
{
	if (isRun())
	{
		//伯克利套接字 BSD socket
		fd_set fdRead;//描述符（socket） 集合
		fd_set fdWrite;
		fd_set fdExp;
		//清理集合
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		//将描述符（socket）加入集合
		FD_SET(m_ServerSocket, &fdRead);
		FD_SET(m_ServerSocket, &fdWrite);
		FD_SET(m_ServerSocket, &fdExp);
		SOCKET maxSock = m_ServerSocket;
		for (int n = (int)m_Clients.size() - 1; n >= 0; n--)
		{
			FD_SET(m_Clients[n]->getClientSocket(), &fdRead);
			if (maxSock < m_Clients[n]->getClientSocket())
			{
				maxSock = m_Clients[n]->getClientSocket();
			}
		}
		///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
		///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
		timeval t = { 1,0 };
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0)
		{
			printf("select任务结束。\n");
			Close();
			return false;
		}
		//判断描述符（socket）是否在集合中
		if (FD_ISSET(m_ServerSocket, &fdRead))
		{
			FD_CLR(m_ServerSocket, &fdRead);
			Accept();
		}
		/*for (int n = (int)m_Clients.size() - 1; n >= 0; n--)*/
		for(int n = 0; n < (int)m_Clients.size();n++)
		{
			if (FD_ISSET(m_Clients[n]->getClientSocket(), &fdRead))
			{
				if (-1 == ReadTextMsg(m_Clients[n]))
				{
					auto iter = m_Clients.begin() + n;//std::vector<SOCKET>::iterator
					if (iter != m_Clients.end())
					{
						delete m_Clients[n];
                        m_Clients[n] = NULL;
						m_Clients.erase(iter);
					}
				}
			}
		}
		return true;
	}
	return false;
}

bool BaseServer::isRun()
{
	return m_ServerSocket != INVALID_SOCKET;
}


int BaseServer::ReadData(SOCKET cSock)
{
	int ret = recv_frame_head(cSock, &header);
	if (ret < 0)
		return -1;
	printf("fin=%d\nopcode=0x%X\nmask=%d\npayload_len=%llu\n", header.fin, header.opcode, header.mask, header.payload_length);
	//缓冲区
	char szRecv[4096] = { 0 };
	// 5 接收客户端数据
	int nLen = (int)recv(cSock, szRecv, 4096, 0);
	if (nLen <= 0)
	{
		printf("客户端<Socket=%d>已退出，任务结束。\n", cSock);
		return -1;
	}
	umask(szRecv, nLen, header.masking_key);
	printf("接收到的数据是:%s\n", szRecv);
	//处理网络消息
	//OnNetMsg(cSock, szRecv);
	return 0;
}


//服务端粘包处理
int BaseServer::ReadData(ClientObject *pClient)
{
	// 5 接收客户端数据
	if (pClient)
	{
		int nLen = (int)recv(pClient->getClientSocket(), _szRecv, RECV_BUFF_SIZE, 0);

		if (nLen <= 0)
		{
			printf("客户端<Socket=%d>已退出，任务结束。\n", pClient->getClientSocket());
			return -1;
		}

		//将收到的客户端发过来的数据拷贝到消息缓冲区
		memcpy(pClient->getMsgBuf() + pClient->getCurPos(), _szRecv, nLen);
		//消息缓冲区的位置移动
		pClient->setCurPos(pClient->getCurPos() + nLen);
		while (pClient->getCurPos() >= sizeof(DataHeader))//如果当前的位置大于消息头的长度了，那么就要取出消息头
		{
			DataHeader* header = (DataHeader*)pClient->getMsgBuf();
			if (pClient->getCurPos() >= header->dataLength)
			{
				//剩余的未处理消息缓冲区的长度
				long nSize = pClient->getCurPos() - header->dataLength;
				//处理消息
				OnNetMsg(pClient->getClientSocket(), header);
				//将剩余消息前移
				memcpy(pClient->getMsgBuf(), pClient->getMsgBuf() + header->dataLength, nSize);
				//当前位置就是剩余消息的长度
				pClient->setCurPos(nSize);
			}
			else {//剩余消息不足一个完整的消息
				break;
			}
		}
	}
	return 0;
}

int BaseServer::ReadWebData(ClientObject * pClient)
{
	int rul = recv_frame_head(pClient, &header);
	if (rul < 0)
	{
		return -1;
	}


	// 5 接收客户端数据
	int nLen = (int)recv(pClient->getClientSocket(), _szRecv, RECV_BUFF_SIZE, 0);

	if (nLen <= 0)
	{
		printf("客户端<Socket=%d>已退出，任务结束。\n", pClient->getClientSocket());
		return -1;
	}

	//将收到的客户端发过来的数据拷贝到消息缓冲区
	memcpy(pClient->getMsgBuf() + pClient->getCurPos(), _szRecv, nLen);
	//消息缓冲区的位置移动
	pClient->setCurPos(pClient->getCurPos() + nLen);
	while (pClient->getCurPos() >= sizeof(DataHeader))//如果当前的位置大于消息头的长度了，那么就要取出消息头
	{
		DataHeader* header = (DataHeader*)pClient->getMsgBuf();
		if (pClient->getCurPos() >= header->dataLength)
		{
			//剩余的未处理消息缓冲区的长度
			long nSize = pClient->getCurPos() - header->dataLength;
			//处理消息
			OnNetMsg(pClient->getClientSocket(), header);
			//将剩余消息前移
			memcpy(pClient->getMsgBuf(), pClient->getMsgBuf() + header->dataLength, nSize);
			//当前位置就是剩余消息的长度
			pClient->setCurPos(nSize);
		}
		else {//剩余消息不足一个完整的消息
			break;
		}
	}

	return 0;
}

int BaseServer::ReadTextMsg(ClientObject * pClient)
{
	if (pClient)
	{
		int rul = recv_frame_head(pClient, &header);
		printf("fin=%d\nopcode=0x%X\nmask=%d\npayload_len=%llu\n", header.fin, header.opcode, header.mask, header.payload_length);
		if (rul < 0)
		{
			return -1;
		}


		// 5 接收客户端数据
		int nLen = (int)recv(pClient->getClientSocket(), _szRecv, RECV_BUFF_SIZE, 0);

		if (nLen <= 0)
		{
			printf("客户端<Socket=%d>已退出，任务结束。\n", pClient->getClientSocket());
			return -1;
		}

		//将收到的客户端发过来的数据拷贝到消息缓冲区
		memcpy(pClient->getMsgBuf() + pClient->getCurPos(), _szRecv, (size_t)header.payload_length);
		//消息缓冲区的位置移动
		pClient->setCurPos(pClient->getCurPos() + header.payload_length);
		if (pClient->getCurPos() > 0)//如果当前的位置大于消息头的长度了，那么就要取出消息头
		{

			//剩余的未处理消息缓冲区的长度
			size_t nSize = pClient->getCurPos() - header.payload_length;
			//处理消息
			OnNetMsg(pClient, pClient->getMsgBuf());
			//当前位置就是剩余消息的长度
			//pClient->setCurPos(nSize);
			pClient->_curPos = nSize;
		}
	}	
	return 0;
}

void BaseServer::OnNetMsg(SOCKET cSock, DataHeader * header)
{
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{

		//Login* login = (Login*)header;
		//printf("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
		//忽略判断用户密码是否正确的过程
		LoginResult ret;
		SendData(cSock, &ret);
	}
	break;
	case CMD_LOGINOUT:
	{
		//Loginout* logout = (Loginout*)header;
		//printf("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
		//忽略判断用户密码是否正确的过程
		LoginoutResult ret;
		SendData(cSock, &ret);
	}
	break;
	default:
	{
		printf("<Socket=%d>收到未定义的消息...\n", cSock);
		DataHeader header;
		SendData(cSock, &header);
	}
	break;
	}
}

void BaseServer::OnNetMsg(ClientObject* pClient, char * data)
{
	umask(data, header.payload_length, header.masking_key);
	if (!strcmp(data, "exit"))
	{
		for (size_t n = 0; n < m_Clients.size();n++)
		{
			auto iter = m_Clients.begin()+n;//std::vector<SOCKET>::iterator
			if(*iter == pClient&&iter != m_Clients.end())
			{
				printf("客户端%d退出\n", m_Clients[n]->getClientSocket());
				m_Clients.erase(iter);
			}
		}
	}
	else {
		SendWebDataToAll(data);
	}
}

long BaseServer::SendData(SOCKET cSock, DataHeader * header)
{
	if (isRun() && header)
	{
		return send(cSock, (const char*)header, header->dataLength, 0);
	}
	return SOCKET_ERROR;
}

long BaseServer::SendData(SOCKET cSock, char * data)
{
	if (isRun() && data)
	{
		return send(cSock, (const char*)data, strlen(data), 0);
	}
	return SOCKET_ERROR;
}

long BaseServer::SendWebData(SOCKET cSock, char * data)
{
	if (isRun() && data)
	{
		send_frame_head(new ClientObject(cSock), &header);
		return send(cSock, (const char*)data, header.payload_length, 0);
	}
	return SOCKET_ERROR;
}

void BaseServer::SendDataToAll(DataHeader * header)
{
	for (int n = (int)m_Clients.size() - 1; n >= 0; n--)
	{
		SendData(m_Clients[n]->getClientSocket(), header);
	}
}

void BaseServer::SendDataToAll(char * header)
{
	for (int n = (int)m_Clients.size() - 1; n >= 0; n--)
	{
		SendData(m_Clients[n]->getClientSocket(), header);
	}
}

void BaseServer::SendWebDataToAll(char * header)
{
	for (int n = (int)m_Clients.size() - 1; n >= 0; n--)
	{
		SendWebData(m_Clients[n]->getClientSocket(), header);
	}
}

const char * BaseServer::gb2312Toutf8(const char * gb2312)
{
#ifdef _WIN32
	int len = MultiByteToWideChar(0, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(0, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(65001, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(65001, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
#endif
}

