#ifndef __TCPSESSION_H__
#define __TCPSESSION_H__

#include <netinet/in.h>

#include "StrPtrLen.h"
#include "task.h"

#define REQUEST_BUFF_SIZE	(1024*16)
#define RESPONSE_BUFF_SIZE	(1024*256)
#define MAX_IPADDR_LEN		32
//ms
#define SEND_INTERVAL	10

class TcpSession : public Task
{
public:
	TcpSession(int fd, struct sockaddr_in* addr);
	virtual ~TcpSession();
	int 			Init(TaskThread* threadp = NULL);
	int				DoRead();	
	virtual int		DoEvents(u_int32_t events, TaskThread* threadp);
	int		    	RecvData();
	int		    	SendData();	
protected:
	//int 				m_SockFd;
	struct sockaddr_in 	m_SockAddr;
	char				m_IpAddr[MAX_IPADDR_LEN];
	
	char				m_RequestBuffer[REQUEST_BUFF_SIZE];
	StrPtrLen			m_StrReceived;
	StrPtrLen			m_StrRequest;

	char				fResponseBuffer[RESPONSE_BUFF_SIZE];
	StrPtrLen			fStrResponse; 
	StrPtrLen			fStrRemained;

	
};

#endif