#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "HttpServer.h"

HttpServer::HttpServer()
{
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);
}

HttpServer::~HttpServer()
{
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);
}

TcpSession* HttpServer::GetSession(int fd, struct sockaddr_in * addr, TaskThread* threadp)
{
	HttpSession* sessionp = new HttpSession(fd, addr);
	sessionp->Init(threadp);
	return sessionp;
}


