#ifndef __TCPSESSION_H__
#define __TCPSESSION_H__

#include <netinet/in.h>

#include "StrPtrLen.h"
#include "task.h"

#define REQUEST_BUFF_SIZE	(1024*16)
#define RESPONSE_BUFF_SIZE	(1024*256)
#define MAX_IPADDR_LEN		32

#define SEND_ERROR				-1
#define SEND_OK					0
#define SEND_TO_BE_CONTINUE		1

class TcpSession : public Task
{
public:
	TcpSession(int fd, struct sockaddr_in* addr);
	virtual ~TcpSession();
	int 			Init(TaskThread* threadp = NULL);	
	void			Close();
	virtual int		DoEvents(u_int32_t events, TaskThread* threadp);
	
protected:
	int				DoRead();
	int		    	RecvData();
	int		    	SendData();	
	bool			IsFullRequest();
	int				DoRequest();
	void			MoveOnRequest();
	
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