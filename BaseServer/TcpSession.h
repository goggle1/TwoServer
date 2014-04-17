#ifndef __TCPSESSION_H__
#define __TCPSESSION_H__

#include <netinet/in.h>

#include "StrPtrLen.h"
#include "task.h"

#define REQUEST_BUFF_SIZE	(1024*16)
#define RESPONSE_BUFF_SIZE	(1024*16)
#define MAX_IPADDR_LEN		32
class TcpSession : public Task
{
public:
	TcpSession(int fd, struct sockaddr_in* addr);
	virtual ~TcpSession();
	int 			Init(TaskThread* threadp = NULL);
	int				DoRead();
	int 			DoTimeout();
	virtual int 	Run();
	virtual	void	Release();
	virtual int		DoEvents(u_int32_t events, TaskThread* threadp);
	
protected:
	//int 				m_SockFd;
	struct sockaddr_in 	m_SockAddr;
	char				m_IpAddr[MAX_IPADDR_LEN];
	
	char				m_RequestBuffer[REQUEST_BUFF_SIZE];
	StrPtrLen			m_StrReceived;
	StrPtrLen			m_StrRequest;

	char				m_ResponseBuffer[RESPONSE_BUFF_SIZE];			
	StrPtrLen			m_StrResponse; 
	StrPtrLen			m_StrRemained;
	
};

#endif