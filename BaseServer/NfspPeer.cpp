#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "NfspPeer.h"

static const char HEX_STR[] = "0123456789ABCDEF";

//key group of version 1
/*
const unsigned short nfsp_key_group_v1[] = {
	0x8d4e, 0x9c74, 0xa591, 0xb96e, 
	0xca25, 0xd32d, 0xe6a6, 0xfc1d, 
	0xf19a, 0xec9f, 0xd8b0, 0xc36c,
	0xb2b3, 0xa91a, 0x927c, 0x872b
};
*/
const u_int8_t nfsp_key_group_v1[][2] = {
	{0x8d, 0x4e}, {0x9c, 0x74}, {0xa5, 0x91}, {0xb9, 0x6e}, 
	{0xca, 0x25}, {0xd3, 0x2d}, {0xe6, 0xa6}, {0xfc, 0x1d}, 
	{0xf1, 0x9a}, {0xec, 0x9f}, {0xd8, 0xb0}, {0xc3, 0x6c},
	{0xb2, 0xb3}, {0xa9, 0x1a}, {0x92, 0x7c}, {0x87, 0x2b}
};

int byte_hex(u_int8_t* bytes, int len, char* buffer, int buff_len)
{
	if(buff_len < len*2+1)
	{
		return -1;
	}	
	
	u_int8_t*	temp1 = bytes;
	char*		temp2 = buffer;
	for(int index=0; index<len; index++)
	{
		temp2[index*2] = HEX_STR[(*temp1 & 0xF0)>>4];
		temp2[index*2+1] = HEX_STR[*temp1 & 0x0F];
		temp1++;
	}
	temp2[len*2+1] = '\0';

	return 0;
}

NfspPeer::NfspPeer(int fd,struct sockaddr_in * addr) :
	TcpSession(fd, addr)
{
	m_CommandState = COMMAND_STATE_RECV_HEADER;
}

NfspPeer::~NfspPeer()
{
	
}

bool NfspPeer::IsFullPacketHeader()
{
	m_StrRequest.Ptr = m_StrReceived.Ptr;
    m_StrRequest.Len = 0;

    if(m_StrReceived.Len < (int)sizeof(NFSP_PACKET_HEADER_T))
    {
    	return false;
    }

	m_StrRequest.Len = sizeof(NFSP_PACKET_HEADER_T);	
	return true;
}

int NfspPeer::DecodePacketHeader()
{
	NFSP_PACKET_HEADER_T* packetp = (NFSP_PACKET_HEADER_T*)m_StrReceived.Ptr;
	if(packetp->const_key_group != 0x01)
	{
		return -1;
	}

	m_RecvPacketp = packetp;
	u_int8_t const_key[2] 	= { nfsp_key_group_v1[packetp->const_key_index][0], nfsp_key_group_v1[packetp->const_key_index][1]};
	u_int8_t random_key[2] 	= { packetp->random_key_15_8,  packetp->random_key_7_0 };
	u_int8_t encode_key[2] 	= { random_key[0]^const_key[0], random_key[1]^const_key[1]};

	u_int8_t* datap = (u_int8_t*)packetp;
	datap = datap + 4;
	int data_len = sizeof(NFSP_PACKET_HEADER_T);
	data_len = data_len - 4;

	int index = 0;
	for(index=0; index<data_len; index+=2)
	{
		datap[index+0] = datap[index+0] ^ encode_key[0];
		datap[index+1] = datap[index+1] ^ encode_key[1];
	}

	m_RecvCommand.random_key 		= packetp->random_key_15_8 << 8 | 
								  	  packetp->random_key_7_0;
	m_RecvCommand.const_key_index 	= packetp->const_key_index;
	m_RecvCommand.const_key_group 	= packetp->const_key_group;
	m_RecvCommand.reserved 			= packetp->reserved_7_0;
	m_RecvCommand.length 			= packetp->length_31_24 << 24 |
									  packetp->length_23_16 << 16 |
									  packetp->length_15_8	<<  8 |
									  packetp->length_7_0;
	m_RecvCommand.type				= packetp->type_15_8 << 8 |
									  packetp->type_7_0;
	m_RecvCommand.version			= packetp->version_15_8 << 8 |
									  packetp->version_7_0;
	m_RecvCommand.session_id		= packetp->session_id_15_8 << 8 |
									  packetp->session_id_7_0;
										  
	
	return 0;
}

int NfspPeer::EncodePacketHeader(NFSP_PACKET_HEADER_T * packetp)
{
	u_int8_t const_key[2] 	= { nfsp_key_group_v1[packetp->const_key_index][0], nfsp_key_group_v1[packetp->const_key_index][1]};
	u_int8_t random_key[2] 	= { packetp->random_key_15_8,  packetp->random_key_7_0 };
	u_int8_t encode_key[2] 	= { random_key[0]^const_key[0], random_key[1]^const_key[1]};

	u_int8_t* datap = (u_int8_t*)packetp;
	datap = datap + 4;
	int data_len = sizeof(NFSP_PACKET_HEADER_T);
	data_len = data_len - 4;

	int index = 0;
	for(index=0; index<data_len; index+=2)
	{
		datap[index+0] = datap[index+0] ^ encode_key[0];
		datap[index+1] = datap[index+1] ^ encode_key[1];
	}
	
	return 0;
}

bool NfspPeer::IsFullCommand()
{
	if(m_StrReceived.Len < (int)m_RecvCommand.length)
	{
		return false;
	}

	m_StrRequest.Len = m_RecvCommand.length;
	
	return true;
}

int NfspPeer::MakeHandShake()
{
	int ret = 0;

	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);

	if(RESPONSE_BUFF_SIZE - m_StrRemained.Len < m_SendCommand.length)
	{
		fprintf(stderr, "%s[%p]: insufficient buffer [%d-%d]<[%d]\n", __PRETTY_FUNCTION__, this, RESPONSE_BUFF_SIZE, m_StrRemained.Len, m_SendCommand.length);
		return -1;
	}

	m_StrResponse.Set(m_StrRemained.Ptr+m_StrRemained.Len, RESPONSE_BUFF_SIZE-m_StrRemained.Len);
	NFSP_PACKET_HEADER_T* packetp = (NFSP_PACKET_HEADER_T*)m_StrResponse.Ptr;
	packetp->random_key_15_8 	= m_SendCommand.random_key >> 8 & 0x000000FF;
	packetp->random_key_7_0		= m_SendCommand.random_key & 0x000000FF;
	packetp->const_key_index	= m_SendCommand.const_key_index & 0x0000000F;
	packetp->const_key_group	= m_SendCommand.const_key_group & 0x0000000F;
	packetp->reserved_7_0		= m_SendCommand.reserved;
	packetp->length_31_24		= m_SendCommand.length >> 24 & 0x000000FF;
	packetp->length_23_16		= m_SendCommand.length >> 16 & 0x000000FF;
	packetp->length_15_8		= m_SendCommand.length >>  8 & 0x000000FF;
	packetp->length_7_0			= m_SendCommand.length >>  0 & 0x000000FF;
	packetp->type_15_8			= m_SendCommand.type >> 8 & 0x000000FF;
	packetp->type_7_0			= m_SendCommand.type >> 0 & 0x000000FF;
	packetp->version_15_8		= m_SendCommand.version >> 8 & 0x000000FF;
	packetp->version_7_0		= m_SendCommand.version >> 0 & 0x000000FF;
	packetp->session_id_15_8	= m_SendCommand.session_id >> 8 & 0x000000FF;
	packetp->session_id_7_0		= m_SendCommand.session_id >> 0 & 0x000000FF;

	EncodePacketHeader(packetp);

	memcpy(packetp->data, m_InfoHash, sizeof(m_InfoHash));
	memcpy(packetp->data + sizeof(m_InfoHash), m_PeerId, sizeof(m_PeerId));

	m_StrRemained.Len += m_SendCommand.length;
	
	return ret;
}

int NfspPeer::DoHandShake()
{
	int ret = 0;

	memcpy(&m_SendCommand, &m_RecvCommand, sizeof(m_RecvCommand));
	memcpy(m_InfoHash, m_RecvPacketp->data, sizeof(m_InfoHash));
	memcpy(m_PeerId,   m_RecvPacketp->data+sizeof(m_InfoHash), sizeof(m_PeerId));
	// check version.
	#if 1
	if(m_SendCommand.version <= 0x0002)
	{
		// do nothing.
	}
	else
	{
		m_SendCommand.version = 0x0002;
	}
	#endif
	//m_SendCommand.version = 0x0001;
	// check length.
	if(m_SendCommand.length > 0x00000036)
	{
		fprintf(stderr, "%s[%p]: m_SendCommand.length=%u > %u",  __PRETTY_FUNCTION__, this, m_SendCommand.length, 0x00000036);		
		return -1;
	}

	byte_hex(m_InfoHash, sizeof(m_InfoHash), m_InfoHashShow, sizeof(m_InfoHashShow));
	fprintf(stdout, "%s[%p]: m_InfoHash=%s\n", __PRETTY_FUNCTION__, this, m_InfoHashShow);

	snprintf(m_InfoHashFileName, sizeof(m_InfoHashFileName), "%s.dat", m_InfoHashShow);
	m_InfoHashFileName[sizeof(m_InfoHashFileName)-1] = '\0';

	snprintf(m_InfoHashFullPath, PATH_MAX, "%s/%s", NFSP_HOME, m_InfoHashFileName);
	m_InfoHashFullPath[PATH_MAX-1] = '\0';
	
	ret = ::access(m_InfoHashFullPath, R_OK);
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: m_InfoHashFileName can not accessed! ret=%d\n", __PRETTY_FUNCTION__, this, m_InfoHashFileName, ret);
		return ret;
	}

	ret = MakeHandShake();
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: can not MakeHandShake()! ret=%d\n", __PRETTY_FUNCTION__, this, ret);
	}
	
	ret = SendData();
	
	return ret;
}

int NfspPeer::DoCommand()
{
	int ret = 0;


    if(m_RecvCommand.type == NFSP_TYPE_HANDSHAKE)
    {
    	ret = DoHandShake();
    }
#if 0
    else if(m_RecvCommand.type == NFSP_TYPE_BITFIELD)
    {
    	ret = DoBitField();
    }
    else if(m_RecvCommand.type == NFSP_TYPE_REQUEST)
    {
    	ret = DoRequest();
    }
    else if(m_RecvCommand.type == NFSP_TYPE_PIECE)
    {
    	ret = DoPiece();
    }
    else if(m_RecvCommand.type == NFSP_TYPE_QUERYMETA)
    {
    	ret = DoQueryMeta();
    }
    else if(m_RecvCommand.type == NFSP_TYPE_PUSHMETA)
    {
    	ret = DoPushMeta();
    }
#endif
    else
    {
    	fprintf(stderr, "%s[%p]: unknown command 0x%04X\n", __PRETTY_FUNCTION__, this, m_RecvCommand.type);    	
    	return -1;
    }
	
	return ret;
}

int NfspPeer::DoRead()
{	
	int ret = 0;
	ret = RecvData();
	if(ret < 0)
	{
		return ret;
	}
		
	while(1)
	{		
		if(m_CommandState == COMMAND_STATE_RECV_HEADER)
		{
			if(IsFullPacketHeader())
			{
				DecodePacketHeader();
				if(IsFullCommand())
				{
					ret = DoCommand();
					MoveOnRequest();
				}
				else
				{
					m_CommandState = COMMAND_STATE_RECV_CONTENT;
					break;
				}
			}
			else
			{
				break;
			}
		}
		else if(m_CommandState == COMMAND_STATE_RECV_CONTENT)
		{
			if(IsFullCommand())
			{
				ret = DoCommand();
				MoveOnRequest();
				m_CommandState = COMMAND_STATE_RECV_HEADER;
			}
			else
			{
				break;
			}
		}
	}
	
	return ret;
}

int NfspPeer::DoEvents(u_int32_t events,TaskThread * threadp)
{
	int ret = 0;

	if(events & EPOLLERR)
	{
		delete this;
		return 0;
	}
	if(events & EPOLLHUP)
	{
		delete this;
		return 0;
	}
	if(events & EVENT_READ)
	{
		ret = DoRead();	
		if(ret < 0)
		{
			delete this;
			return 0;
		}
		else if(ret == 0)
		{
			// do nothing.
		}
		else
		{
			m_task_thread->m_EventsMaster.ModifyWatch(m_SockFd, EVENT_READ|EVENT_WRITE, this);
		}
	}

	return ret;
}
