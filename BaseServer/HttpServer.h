#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include "HttpSession.h"
#include "TcpServer.h"

class HttpServer : public TcpServer
{
public:
	HttpServer();
	virtual ~HttpServer();
	virtual TcpSession*   	GetSession(int fd, struct sockaddr_in* addr, TaskThread* threadp);

protected:
	
};


extern HttpServer*			g_http_server;

#endif