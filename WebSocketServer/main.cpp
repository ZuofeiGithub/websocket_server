#define _CRT_SECURE_NO_WARNINGS
#include "BaseServer.h"

bool g_bRun = true;
void cmdThread()
{
    char cmdBuf[256] = {0};
	while (true)
	{
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else {
			printf("不支持的命令.\n");
		}
	}
}
void StartCmdThread()
{
	//启动线程
	std::thread t1(cmdThread);
	t1.detach();
}
int main()
{
	BaseServer server;
	server.InitSocket();
	server.Bind(nullptr,4567);
	server.Listen(5);
	StartCmdThread();
	while (g_bRun)
	{
		server.OnRun();
	}
	server.Close();
	return 0;
}
