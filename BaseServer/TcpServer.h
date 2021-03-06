#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include "task.h"
#include "TcpSession.h"

class TcpServer : public Task
{
public:
	TcpServer();
	virtual ~TcpServer();
	int 					Init(u_int32_t ip, u_int16_t port);
	virtual TcpSession*   	GetSession(int fd, struct sockaddr_in* addr, TaskThread* threadp);
	virtual int				DoRead(TaskThread* threadp);
	virtual int				DoEvents(u_int32_t events, TaskThread* threadp);

protected:
	//int		m_SockFd;
};

extern TcpServer*			g_tcp_server;

#endif
