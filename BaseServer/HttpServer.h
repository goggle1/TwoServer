#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include "HttpSession.h"
#include "TcpServer.h"

class HttpServer : public TcpServer
{
public:
	HttpServer();
	virtual ~HttpServer();
	int				DoRead(TaskThread* threadp = NULL);
	virtual int 	Run();
	HttpSession*   	GetSession(int fd, struct sockaddr_in* addr, TaskThread* threadp);
	virtual int		DoEvents(u_int32_t events, TaskThread* threadp);

protected:
	
};


extern HttpServer*			g_http_server;

#endif