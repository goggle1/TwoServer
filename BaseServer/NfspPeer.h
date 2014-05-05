#ifndef __NFSPPEER_H__
#define __NFSPPEER_H__

#include "NfspProtocol.h"
#include "TaskThread.h"
#include "TcpSession.h"

class NfspPeer : public TcpSession
{
public:
	NfspPeer(int fd, struct sockaddr_in* addr);
	virtual ~NfspPeer();	
	virtual	int		DoEvents(u_int32_t events, TaskThread* threadp);

protected:
	int				DoRead();
	bool 			IsFullPacketHeader();
	int 			DecodePacketHeader();
	int 			EncodePacketHeader(NFSP_PACKET_HEADER_T * packetp);
	bool 			IsFullCommand();
	int 			DoCommand();
	int 			DoHandShake();
	int 			MakeHandShake();

protected:
	enum 
	{
		COMMAND_STATE_RECV_HEADER 	= 0,
		COMMAND_STATE_RECV_CONTENT 	= 1,
	};
	int 					m_CommandState;
	NFSP_PACKET_HEADER_T*	m_RecvPacketp;
	NFSP_COMMAND_HEADER_T	m_RecvCommand;
	NFSP_PACKET_HEADER_T*	m_SendPacketp;
	NFSP_COMMAND_HEADER_T	m_SendCommand;
	u_int8_t 				m_PeerId[PEER_ID_LEN];
	u_int8_t 				m_InfoHash[INFO_HASH_LEN];	
	char 					m_InfoHashShow[INFO_HASH_SHOW_LEN];
	char					m_InfoHashFileName[INFO_HASH_FILE_NAME_LEN];
	char					m_InfoHashFullPath[PATH_MAX];

};

#endif