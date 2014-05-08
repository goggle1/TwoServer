#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

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
	m_MetaDatap = NULL;
}

NfspPeer::~NfspPeer()
{
	if(m_MetaDatap != NULL)
	{
		free(m_MetaDatap);
		m_MetaDatap = NULL;
	}
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

	if(RESPONSE_BUFF_SIZE - m_StrRemained.Len < (int)m_SendCommand.length)
	{
		fprintf(stderr, "%s[%p]: insufficient buffer [%d-%d]<[%d]\n", 
			__PRETTY_FUNCTION__, this, 
			RESPONSE_BUFF_SIZE, m_StrRemained.Len, m_SendCommand.length);
		return -1;
	}
	
	m_SendCommand.version = 0x0001;
	m_SendCommand.type = NFSP_TYPE_HANDSHAKE;	
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


int NfspPeer::MakeBitField()
{
	int ret = 0;

	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);
	
	ret = GetBitField();
	if(ret < 0)
	{
		return ret;
	}

	m_SendCommand.length = m_BitField.byte_num + sizeof(NFSP_PACKET_HEADER_T);
	if(RESPONSE_BUFF_SIZE - m_StrRemained.Len < (int)m_SendCommand.length)
	{
		fprintf(stderr, "%s[%p]: insufficient buffer [%d-%d]<[%d]\n", 
			__PRETTY_FUNCTION__, this, 
			RESPONSE_BUFF_SIZE, m_StrRemained.Len, m_SendCommand.length);
		return -1;
	}

	m_SendCommand.type = NFSP_TYPE_BITFIELD;
	m_StrResponse.Set(m_StrRemained.Ptr+m_StrRemained.Len, RESPONSE_BUFF_SIZE-m_StrRemained.Len);
	NFSP_PACKET_HEADER_T* packetp = (NFSP_PACKET_HEADER_T*)m_StrResponse.Ptr;
	packetp->random_key_15_8	= m_SendCommand.random_key >> 8 & 0x000000FF;
	packetp->random_key_7_0 	= m_SendCommand.random_key & 0x000000FF;
	packetp->const_key_index	= m_SendCommand.const_key_index & 0x0000000F;
	packetp->const_key_group	= m_SendCommand.const_key_group & 0x0000000F;
	packetp->reserved_7_0		= m_SendCommand.reserved;
	packetp->length_31_24		= m_SendCommand.length >> 24 & 0x000000FF;
	packetp->length_23_16		= m_SendCommand.length >> 16 & 0x000000FF;
	packetp->length_15_8		= m_SendCommand.length >>  8 & 0x000000FF;
	packetp->length_7_0 		= m_SendCommand.length >>  0 & 0x000000FF;
	packetp->type_15_8			= m_SendCommand.type >> 8 & 0x000000FF;
	packetp->type_7_0			= m_SendCommand.type >> 0 & 0x000000FF;
	packetp->version_15_8		= m_SendCommand.version >> 8 & 0x000000FF;
	packetp->version_7_0		= m_SendCommand.version >> 0 & 0x000000FF;
	packetp->session_id_15_8	= m_SendCommand.session_id >> 8 & 0x000000FF;
	packetp->session_id_7_0 	= m_SendCommand.session_id >> 0 & 0x000000FF;

	EncodePacketHeader(packetp);

	memcpy(packetp->data, m_BitField.datap, m_BitField.byte_num);

	m_StrRemained.Len += m_SendCommand.length;	

	return 0;
}

int NfspPeer::MakePiece()
{
	fprintf(stdout, "%s[%p]: InfoHash=%s, PieceIndex=%u, PieceLength=%d, PieceBegin=%u, RequestLength=%u\n", 
		__PRETTY_FUNCTION__, this,
		m_InfoHashShow, m_Request.piece_index, m_PieceLength, m_Request.piece_begin, m_Request.request_length);

	int slice_length = m_Request.request_length;
	if(m_Request.piece_begin >= m_PieceLength)
	{
		fprintf(stderr, "%s[%p]: m_Request.piece_begin[%u] >= m_PieceLength[%d]\n", 
			__PRETTY_FUNCTION__, this,
			m_Request.piece_begin, m_PieceLength);
		return -1;
	}
	else if(m_Request.piece_begin + m_Request.request_length > m_PieceLength)
	{
		slice_length = m_PieceLength - m_Request.piece_begin;
		fprintf(stderr, "%s[%p]: m_Request.piece_begin[%u] + m_Request.request_length[%u] > m_PieceLength[%d], so slice_length=%d\n", 
			__PRETTY_FUNCTION__, this,
			m_Request.piece_begin, m_Request.request_length, m_PieceLength, slice_length);
	}
	
	m_SendCommand.length = slice_length + sizeof(m_Request.piece_index) +
			sizeof(m_Request.piece_begin) + sizeof(NFSP_PACKET_HEADER_T);
	if(RESPONSE_BUFF_SIZE - m_StrRemained.Len < (int)m_SendCommand.length)
	{
		fprintf(stderr, "%s[%p]: insufficient buffer [%d-%d]<[%d]\n", 
			__PRETTY_FUNCTION__, this, 
			RESPONSE_BUFF_SIZE, m_StrRemained.Len, m_SendCommand.length);
		return -1;
	}

	m_SendCommand.type = NFSP_TYPE_PIECE;
	m_StrResponse.Set(m_StrRemained.Ptr+m_StrRemained.Len, RESPONSE_BUFF_SIZE-m_StrRemained.Len);
	NFSP_PACKET_HEADER_T* packetp = (NFSP_PACKET_HEADER_T*)m_StrResponse.Ptr;
	packetp->random_key_15_8	= m_SendCommand.random_key >> 8 & 0x000000FF;
	packetp->random_key_7_0 	= m_SendCommand.random_key & 0x000000FF;
	packetp->const_key_index	= m_SendCommand.const_key_index & 0x0000000F;
	packetp->const_key_group	= m_SendCommand.const_key_group & 0x0000000F;
	packetp->reserved_7_0		= m_SendCommand.reserved;
	packetp->length_31_24		= m_SendCommand.length >> 24 & 0x000000FF;
	packetp->length_23_16		= m_SendCommand.length >> 16 & 0x000000FF;
	packetp->length_15_8		= m_SendCommand.length >>  8 & 0x000000FF;
	packetp->length_7_0 		= m_SendCommand.length >>  0 & 0x000000FF;
	packetp->type_15_8			= m_SendCommand.type >> 8 & 0x000000FF;
	packetp->type_7_0			= m_SendCommand.type >> 0 & 0x000000FF;
	packetp->version_15_8		= m_SendCommand.version >> 8 & 0x000000FF;
	packetp->version_7_0		= m_SendCommand.version >> 0 & 0x000000FF;
	packetp->session_id_15_8	= m_SendCommand.session_id >> 8 & 0x000000FF;
	packetp->session_id_7_0 	= m_SendCommand.session_id >> 0 & 0x000000FF;

	EncodePacketHeader(packetp);

	u_int32_t temp = 0;
	int offset = 0;

	temp = htonl(m_Request.piece_index);
	memcpy(packetp->data+offset, &temp, sizeof(temp));
	offset += sizeof(temp);

	temp = htonl(m_Request.piece_begin);
	memcpy(packetp->data+offset, &temp, sizeof(temp));
	offset += sizeof(temp);

	memcpy(packetp->data+offset, m_PieceData+m_Request.piece_begin, slice_length);
	offset += slice_length;

	m_StrRemained.Len += m_SendCommand.length;	

	return 0;
}

int NfspPeer::MakeMeta()
{
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);

	u_int16_t	check_sum = 0x0102;
	u_int32_t 	code = 0;	
	m_SendCommand.length = m_FileHeader.torrent_size + sizeof(check_sum) + sizeof(code) + sizeof(NFSP_PACKET_HEADER_T);
	if(RESPONSE_BUFF_SIZE - m_StrRemained.Len < (int)m_SendCommand.length)
	{
		fprintf(stderr, "%s[%p]: insufficient buffer [%d-%d]<[%d]\n", 
			__PRETTY_FUNCTION__, this, 
			RESPONSE_BUFF_SIZE, m_StrRemained.Len, m_SendCommand.length);
		return -1;
	}
	
	m_SendCommand.type = NFSP_TYPE_PUSHMETA;
	m_StrResponse.Set(m_StrRemained.Ptr+m_StrRemained.Len, RESPONSE_BUFF_SIZE-m_StrRemained.Len);
	NFSP_PACKET_HEADER_T* packetp = (NFSP_PACKET_HEADER_T*)m_StrResponse.Ptr;
	packetp->random_key_15_8	= m_SendCommand.random_key >> 8 & 0x000000FF;
	packetp->random_key_7_0 	= m_SendCommand.random_key & 0x000000FF;
	packetp->const_key_index	= m_SendCommand.const_key_index & 0x0000000F;
	packetp->const_key_group	= m_SendCommand.const_key_group & 0x0000000F;
	packetp->reserved_7_0		= m_SendCommand.reserved;
	packetp->length_31_24		= m_SendCommand.length >> 24 & 0x000000FF;
	packetp->length_23_16		= m_SendCommand.length >> 16 & 0x000000FF;
	packetp->length_15_8		= m_SendCommand.length >>  8 & 0x000000FF;
	packetp->length_7_0 		= m_SendCommand.length >>  0 & 0x000000FF;
	packetp->type_15_8			= m_SendCommand.type >> 8 & 0x000000FF;
	packetp->type_7_0			= m_SendCommand.type >> 0 & 0x000000FF;
	packetp->version_15_8		= m_SendCommand.version >> 8 & 0x000000FF;
	packetp->version_7_0		= m_SendCommand.version >> 0 & 0x000000FF;
	packetp->session_id_15_8	= m_SendCommand.session_id >> 8 & 0x000000FF;
	packetp->session_id_7_0 	= m_SendCommand.session_id >> 0 & 0x000000FF;

	EncodePacketHeader(packetp);

	u_int16_t temp 	= 0;
	u_int32_t temp2 = 0;
	int offset = 0;

	temp = htons(check_sum);
	memcpy(packetp->data+offset, &temp, sizeof(temp));
	offset += sizeof(temp);
	
	temp2 = htonl(code);
	memcpy(packetp->data+offset, &temp2, sizeof(temp2));
	offset += sizeof(temp2);

	memcpy(packetp->data+offset, m_MetaDatap, m_FileHeader.torrent_size);
	offset += m_FileHeader.torrent_size;

	m_StrRemained.Len += m_SendCommand.length;	

	return 0;
}


int NfspPeer::MakeUnChoke()
{
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);

	u_int32_t 	reserved = 0;
	m_SendCommand.length = sizeof(reserved) + sizeof(NFSP_PACKET_HEADER_T);
	if(RESPONSE_BUFF_SIZE - m_StrRemained.Len < (int)m_SendCommand.length)
	{
		fprintf(stderr, "%s[%p]: insufficient buffer [%d-%d]<[%d]\n", 
			__PRETTY_FUNCTION__, this, 
			RESPONSE_BUFF_SIZE, m_StrRemained.Len, m_SendCommand.length);
		return -1;
	}
	
	m_SendCommand.type = NFSP_TYPE_UNCHOKE;
	m_StrResponse.Set(m_StrRemained.Ptr+m_StrRemained.Len, RESPONSE_BUFF_SIZE-m_StrRemained.Len);
	NFSP_PACKET_HEADER_T* packetp = (NFSP_PACKET_HEADER_T*)m_StrResponse.Ptr;
	packetp->random_key_15_8	= m_SendCommand.random_key >> 8 & 0x000000FF;
	packetp->random_key_7_0 	= m_SendCommand.random_key & 0x000000FF;
	packetp->const_key_index	= m_SendCommand.const_key_index & 0x0000000F;
	packetp->const_key_group	= m_SendCommand.const_key_group & 0x0000000F;
	packetp->reserved_7_0		= m_SendCommand.reserved;
	packetp->length_31_24		= m_SendCommand.length >> 24 & 0x000000FF;
	packetp->length_23_16		= m_SendCommand.length >> 16 & 0x000000FF;
	packetp->length_15_8		= m_SendCommand.length >>  8 & 0x000000FF;
	packetp->length_7_0 		= m_SendCommand.length >>  0 & 0x000000FF;
	packetp->type_15_8			= m_SendCommand.type >> 8 & 0x000000FF;
	packetp->type_7_0			= m_SendCommand.type >> 0 & 0x000000FF;
	packetp->version_15_8		= m_SendCommand.version >> 8 & 0x000000FF;
	packetp->version_7_0		= m_SendCommand.version >> 0 & 0x000000FF;
	packetp->session_id_15_8	= m_SendCommand.session_id >> 8 & 0x000000FF;
	packetp->session_id_7_0 	= m_SendCommand.session_id >> 0 & 0x000000FF;

	EncodePacketHeader(packetp);

	u_int32_t temp = 0;
	int offset = 0;

	temp = htons(reserved);
	memcpy(packetp->data+offset, &temp, sizeof(temp));
	offset += sizeof(temp);

	m_StrRemained.Len += m_SendCommand.length;	

	return 0;
}


int NfspPeer::GetBitField()
{
	int ret = 0;

	int fd = ::open(m_InfoHashFullPath, O_RDONLY);
	if(fd == -1)
	{
		return -1;
	}

	DAT_FILE_HEADER_CONTROL_PART_T file_header = {0};
	ret = ::read(fd, &file_header, sizeof(DAT_FILE_HEADER_CONTROL_PART_T));
	if(ret < (int)sizeof(DAT_FILE_HEADER_CONTROL_PART_T))
	{
		::close(fd);
		fd = -1;
		return -1;
	}

	m_FileHeader.version 		= file_header.version;
	m_FileHeader.torrent_size 	= file_header.torrent_size_31_24 << 24 |
								  file_header.torrent_size_23_16 << 16 |
								  file_header.torrent_size_15_8  <<  8 |
								  file_header.torrent_size_7_0;
	m_FileHeader.infohash_size	= file_header.infohash_size_31_24 << 24 |
								  file_header.infohash_size_23_16 << 16 |
								  file_header.infohash_size_15_8  <<  8 |
								  file_header.infohash_size_7_0;
	m_FileHeader.bitfield_size	= file_header.bitfield_size_31_24 << 24 |
								  file_header.bitfield_size_23_16 << 16 |
								  file_header.bitfield_size_15_8  <<  8 |
								  file_header.bitfield_size_7_0;
	off_t bitfield_begin = CONTROL_PART_LENGTH + m_FileHeader.torrent_size + m_FileHeader.infohash_size;
	off_t seek_ret = ::lseek(fd, bitfield_begin, SEEK_SET);
	if(seek_ret == -1)
	{
		::close(fd);
		fd = -1;
		return -1;
	}

	m_BitField.datap = (u_int8_t*)malloc(m_FileHeader.bitfield_size);
	if(m_BitField.datap == NULL)
	{
		::close(fd);
		fd = -1;
		return -1;
	}

	ret = ::read(fd, m_BitField.datap, m_FileHeader.bitfield_size);
	if(ret < (int)m_FileHeader.bitfield_size)
	{
		return -1;
	}
	
	m_BitField.byte_num = m_FileHeader.bitfield_size;
	m_BitField.bits_num = m_BitField.byte_num*8; //todo:

	return 0;
}

int NfspPeer::GetPieceData()
{
	int ret = 0;

	m_PieceLength = 0;

	int fd = ::open(m_InfoHashFullPath, O_RDONLY);
	if(fd == -1)
	{
		return -1;
	}
	
	off_t piece_position = FILE_HEADER_LENGTH + m_Request.piece_index*PIECE_LEN;
	off_t seek_ret = ::lseek(fd, piece_position, SEEK_SET);
	if(seek_ret == -1)
	{
		::close(fd);
		fd = -1;
		return -1;
	}
	
	ret = ::read(fd, m_PieceData, PIECE_LEN);
	if(ret < 0)
	{
		close(fd);
		fd = -1;
		return -1;
	}

	m_PieceLength = ret;

	return 0;
}

int NfspPeer::GetMetaData()
{
	int ret = 0;

	m_PieceLength = 0;

	int fd = ::open(m_InfoHashFullPath, O_RDONLY);
	if(fd == -1)
	{
		return -1;
	}
	
	off_t meta_position = CONTROL_PART_LENGTH;
	off_t seek_ret = ::lseek(fd, meta_position, SEEK_SET);
	if(seek_ret == -1)
	{
		::close(fd);
		fd = -1;
		return -1;
	}

	m_MetaDatap = (u_int8_t*)malloc(m_FileHeader.torrent_size);
	if(m_MetaDatap == NULL)
	{
		::close(fd);
		fd = -1;
		return -1;
	}
	
	ret = ::read(fd, m_MetaDatap, m_FileHeader.torrent_size);
	if(ret != (int)m_FileHeader.torrent_size)
	{
		::close(fd);
		fd = -1;
		free(m_MetaDatap);
		m_MetaDatap = NULL;
		return -1;
	}

	return 0;
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
		fprintf(stderr, "%s[%p]: m_InfoHashFileName [%s] can not accessed! ret=%d\n", __PRETTY_FUNCTION__, this, m_InfoHashFileName, ret);
		return ret;
	}

	ret = MakeHandShake();
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: can not MakeHandShake()! ret=%d\n", __PRETTY_FUNCTION__, this, ret);
	}
	
	ret = SendData();
	if(ret < 0)
	{
		return ret;
	}

	ret = MakeBitField();
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: can not MakeBitField()! ret=%d\n", __PRETTY_FUNCTION__, this, ret);
	}

	ret = SendData();
	if(ret < 0)
	{
		return ret;
	}	
	
	return ret;
}

int NfspPeer::DoBitField()
{
	int ret = 0;

	// do nothing.

	return ret;
}

int NfspPeer::DoRequest()
{
	int ret = 0;

	u_int32_t temp = 0;
	int offset = 0;
	
	memcpy(&temp, m_RecvPacketp->data+offset, sizeof(temp));
	m_Request.piece_index = ntohl(temp);
	offset += sizeof(temp);

	memcpy(&temp, m_RecvPacketp->data+offset, sizeof(temp));
	m_Request.piece_begin = ntohl(temp);
	offset += sizeof(temp);

	memcpy(&temp, m_RecvPacketp->data+offset, sizeof(temp));
	m_Request.request_length = ntohl(temp);
	offset += sizeof(temp);	

	fprintf(stdout, "%s[%p]: InfoHash=%s, PieceIndex=%u, PieceBegin=%u, RequestLength=%u\n", __PRETTY_FUNCTION__, this,
		m_InfoHashShow, m_Request.piece_index, m_Request.piece_begin, m_Request.request_length);


	ret = GetPieceData();
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: GetPieceData() return %d\n", __PRETTY_FUNCTION__, this, ret);
		return ret;
	}

	ret = MakePiece();
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: MakePiece() return %d\n", __PRETTY_FUNCTION__, this, ret);
		return ret;
	}

	ret = SendData();
	if(ret < 0)
	{
		return ret;
	}	

	return ret;
}

int NfspPeer::DoPiece()
{
	int ret = 0;

	// do nothing.

	return ret;
}

int NfspPeer::DoQueryMeta()
{
	int ret = 0;

	memcpy(m_MetaTaskId, m_RecvPacketp->data, INFO_HASH_LEN);	

	fprintf(stdout, "%s[%p]: InfoHash=%s\n", __PRETTY_FUNCTION__, this, m_InfoHashShow);

	ret = GetMetaData();
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: GetMetaData() return %d\n", __PRETTY_FUNCTION__, this, ret);
		return ret;
	}

	ret = MakeMeta();
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: MakeMeta() return %d\n", __PRETTY_FUNCTION__, this, ret);
		return ret;
	}

	ret = SendData();
	if(ret < 0)
	{
		return ret;
	}	

	return ret;
}

int NfspPeer::DoPushMeta()
{
	int ret = 0;

	// do nothing.

	return ret;
}

int NfspPeer::DoInterested()
{
	int ret = 0;

	ret = MakeUnChoke();
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: MakeUnChoke() return %d\n", __PRETTY_FUNCTION__, this, ret);
		return ret;
	}

	ret = SendData();
	if(ret < 0)
	{
		return ret;
	}	

	return ret;
}

int NfspPeer::DoCommand()
{
	int ret = 0;

    if(m_RecvCommand.type == NFSP_TYPE_HANDSHAKE)
    {
    	ret = DoHandShake();
    }
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
    else if(m_RecvCommand.type == NFSP_TYPE_INTERESTED)
    {
    	ret = DoInterested();
    }
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

/*
funshion client test:
[participation]
just_media_server=1
enable_add_ms_info=1
saved_ms_ip=192.168.160.202
saved_ms_tcp_port=6601
saved_ms_udp_port=6601
*/

