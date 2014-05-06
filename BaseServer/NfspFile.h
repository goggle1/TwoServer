#ifndef __NFSPFILE_H__
#define __NFSPFILE_H__

// dat file header, only control part.
#pragma pack(1)
typedef struct dat_file_header_control_part_t
{
	u_int32_t 	version 			: 8;
	u_int32_t 	torrent_size_31_24 	: 8;
	u_int32_t 	torrent_size_23_16 	: 8;
	u_int32_t 	torrent_size_15_8 	: 8;
	u_int32_t 	torrent_size_7_0 	: 8;
	u_int32_t 	infohash_size_31_24	: 8;
	u_int32_t 	infohash_size_23_16	: 8;
	u_int32_t 	infohash_size_15_8 	: 8;
	u_int32_t 	infohash_size_7_0 	: 8;
	u_int32_t 	bitfield_size_31_24	: 8;
	u_int32_t 	bitfield_size_23_16	: 8;
	u_int32_t 	bitfield_size_15_8 	: 8;
	u_int32_t 	bitfield_size_7_0 	: 8;
} DAT_FILE_HEADER_CONTROL_PART_T;
#pragma pack(0)

typedef struct dat_file_header_t
{
	u_int8_t	version;
	u_int32_t	torrent_size;
	u_int32_t	infohash_size;
	u_int32_t	bitfield_size;
	u_int8_t* 	torrentp;
	u_int8_t* 	infohashp;
	u_int8_t*	bitfieldp;
} DAT_FILE_HEADER_T;

#define NFSP_HOME					"/home/html"
#define CONTROL_PART_LENGTH			(1024*4)
#define FILE_HEADER_LENGTH			(1024*1024*4)

#endif
