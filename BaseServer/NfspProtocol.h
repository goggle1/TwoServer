#ifndef __NFSPPROTOCOL_H__
#define __NFSPPROTOCOL_H__

#include <sys/types.h>

#pragma pack(1)

typedef struct nfsp_packet_header_t
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	//<reserved><length><type><version><session id>
	/*
 	u_int32_t		reserved_31_24	: 8;
 	u_int32_t		reserved_23_16	: 8;
 	u_int32_t		reserved_15_8	: 8;
 	u_int32_t		reserved_7_0		: 8;
 	*/
 	u_int32_t		random_key_15_8	: 8;
 	u_int32_t		random_key_7_0	: 8;
 	u_int32_t		const_key_group	: 4;
 	u_int32_t		const_key_index	: 4; 	
 	u_int32_t		reserved_7_0	: 8;
 	u_int32_t		length_31_24	: 8;
 	u_int32_t		length_23_16	: 8;
 	u_int32_t		length_15_8		: 8;
 	u_int32_t		length_7_0		: 8;
 	u_int32_t		type_15_8		: 8;
 	u_int32_t		type_7_0		: 8;
 	u_int32_t		version_15_8	: 8;
 	u_int32_t		version_7_0		: 8;
 	u_int32_t		session_id_15_8	: 8;
 	u_int32_t		session_id_7_0	: 8;
#elif __BYTE_ORDER == __BIG_ENDIAN
	//<reserved><length><type><version><session id>
	/*
 	u_int32_t		reserved_31_24	: 8;
 	u_int32_t		reserved_23_16	: 8;
 	u_int32_t		reserved_15_8	: 8;
 	u_int32_t		reserved_7_0		: 8;
 	*/
 	u_int32_t		random_key_16_8	: 8;
 	u_int32_t		random_key_7_0	: 8;
 	u_int32_t		const_key_index	: 4;
 	u_int32_t		const_key_group	: 4;
 	u_int32_t		reserved_7_0	: 8;
 	u_int32_t		length_31_24	: 8;
 	u_int32_t		length_23_16	: 8;
 	u_int32_t		length_15_8		: 8;
 	u_int32_t		length_7_0		: 8;
 	u_int32_t		type_15_8		: 8;
 	u_int32_t		type_7_0		: 8;
 	u_int32_t		version_15_8	: 8;
 	u_int32_t		version_7_0		: 8;
 	u_int32_t		session_id_15_8	: 8;
 	u_int32_t		session_id_7_0	: 8;
#endif
 	u_int8_t		data[0];
} NFSP_PACKET_HEADER_T;

#pragma pack();

typedef struct nfsp_command_header_t
{
	u_int16_t		random_key;
 	u_int8_t		const_key_index;
 	u_int8_t		const_key_group;
 	u_int8_t		reserved;
 	u_int32_t		length;
 	u_int16_t		type;
 	u_int16_t		version;
 	u_int16_t		session_id;
} NFSP_COMMAND_HEADER_T;

typedef struct nfsp_request_t
{
 	u_int32_t		piece_index;
 	u_int32_t		piece_begin;
 	u_int32_t		request_length;
} NFSP_REQUEST_T;

#define NFSP_TYPE_HANDSHAKE			0x0601
#define NFSP_TYPE_KEEPALIVE			0x0602
#define NFSP_TYPE_CHOKE				0x0603
#define NFSP_TYPE_UNCHOKE			0x0604
#define NFSP_TYPE_INTERESTED		0x0605
#define NFSP_TYPE_NOTINTERESTED		0x0606
#define NFSP_TYPE_DENY				0x0607
#define NFSP_TYPE_BITFIELD			0x0608
#define NFSP_TYPE_HAVE				0x0609
#define NFSP_TYPE_REQUEST			0x060A
#define NFSP_TYPE_PIECE				0x060B
#define NFSP_TYPE_BADPIECE			0x060C
#define NFSP_TYPE_QUERYMETA			0x060D
#define NFSP_TYPE_PUSHMETA			0x060E
#define NFSP_TYPE_QUERYMP4LIST		0x060F
#define NFSP_TYPE_PUSHMP4LIST		0x0610
#define NFSP_TYPE_QUERYMP4HEAD		0x0611
#define NFSP_TYPE_PUSHMP4HEAD		0x0612

#define INFO_HASH_LEN				20
#define PEER_ID_LEN					20
#define INFO_HASH_SHOW_LEN			(INFO_HASH_LEN*2+4)
#define INFO_HASH_FILE_NAME_LEN		(INFO_HASH_LEN*2+8)
#define PIECE_LEN					(1024*256)
#define SLICE_LEN					(1024*16)
#define MAX_META_LEN				(1024*512)


#endif

