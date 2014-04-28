#ifndef __HTTPSESSION_H__
#define __HTTPSESSION_H__

#include "StringFormatter.h"
#include "HttpRequest.h"
#include "TcpSession.h"
#include "FilesMaster.h"

#define USE_FILE_BUFFER 1

//ms
#define SEND_INTERVAL	10

class HttpSession : public TcpSession
{
public:
	HttpSession(int fd, struct sockaddr_in* addr);
	virtual ~HttpSession();

	virtual int 	DoRead();
	virtual	int		DoContinue();
	virtual bool 	IsFullRequest();
	virtual int		DoRequest();	
	virtual	int 	DoEvents(u_int32_t events, TaskThread* threadp);
	
protected:
	int			DoGet();
	int 		ResponseFile(char* abs_path);
	int 		ResponseError(HTTPStatusCode status_code);
	int		    RecvData();
	int		    SendData();
	bool		SendDone();
	void        MoveOnRequest();
	
	HttpRequest 	m_Request;
	HTTPStatusCode 	fHttpStatus;

	//CONSTANTS:
	enum
	{
		kResponseBufferSizeInBytes = 1024*256,		 //UInt32	
		kReadBufferSize = 1024*16,
	};
	char		fResponseBuffer[kResponseBufferSizeInBytes];
	//one full http response.
	StrPtrLen	fStrResponse; 
	//http response left, which will be sended again.
	StrPtrLen	fStrRemained;
	// 
	StringFormatter 	fResponse;
	u_int64_t			fContentLen;
	// file
#if USE_FILE_BUFFER
	CFile*				m_CFile;
#else
	int					fFd;
#endif
	u_int64_t			m_ReadCount;
	char		fBuffer[kReadBufferSize];
	bool		fHaveRange;
	int64_t		fRangeStart;
	int64_t		fRangeStop;
	

};

#endif