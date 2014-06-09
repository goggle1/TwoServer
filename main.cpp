#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "BaseServer/FilesMaster.h"
#include "BaseServer/TcpServer.h"
#include "BaseServer/HttpServer.h"
#include "BaseServer/NfspServer.h"
#include "BaseServer/TaskThreadPool.h"

FilesMaster*		g_files_master		= NULL;
TcpServer*			g_tcp_server 		= NULL;
HttpServer*			g_http_server 		= NULL;
NfspServer*			g_nfsp_server		= NULL;
TaskThreadPool* 	g_task_thread_pool 	= NULL;

int start_files_master()
{
	int ret = 0;

	g_files_master = new FilesMaster();	
	
	return ret;
}


int start_server()
{
	int ret = 0;

	signal(SIGPIPE, SIG_IGN);
	
	u_int32_t ip = 0;
	u_int16_t port = 9595;
	fprintf(stdout, "%s: ip=0x%08X, port=%u\n", __FUNCTION__, ip, port);
	g_tcp_server = new TcpServer();
	ret = g_tcp_server->Init(ip, port);
	if(ret < 0)
	{
		fprintf(stderr, "%s: server init error, return %d\n", __FUNCTION__, ret);
		return ret;
	}
	

	u_int32_t ip2 = 0;
	u_int16_t port2 = 9696;
	fprintf(stdout, "%s: ip=0x%08X, port=%u\n", __FUNCTION__, ip2, port2);
	g_http_server = new HttpServer();
	ret = g_http_server->Init(ip2, port2);
	if(ret < 0)
	{
		fprintf(stderr, "%s: server init error, return %d\n", __FUNCTION__, ret);
		return ret;
	}

	u_int32_t ip3 = 0;
	u_int16_t port3 = 6601;
	fprintf(stdout, "%s: ip=0x%08X, port=%u\n", __FUNCTION__, ip3, port3);
	g_nfsp_server = new NfspServer();
	ret = g_nfsp_server->Init(ip3, port3);
	if(ret < 0)
	{
		fprintf(stderr, "%s: server init error, return %d\n", __FUNCTION__, ret);
		return ret;
	}
	
	return ret;
}

int start_thread_workers()
{
	int ret = 0;
	
	g_task_thread_pool = new TaskThreadPool();
	ret = g_task_thread_pool->Init();

	return ret;
}


int main(int argc, char* argv[])
{
	// 1. start server.
	// 2. start N worker thread.	
	
	int ret = 0;
	ret = start_files_master();
	if(ret < 0)
	{
		return ret;
	}
	ret = start_server();	
	if(ret < 0)
	{
		return ret;
	}
	ret = start_thread_workers();
	if(ret < 0)
	{
		return ret;
	}

	while(1)
	{
		sleep(10);
	}

	return ret;
}

