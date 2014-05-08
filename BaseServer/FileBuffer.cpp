
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "FileBuffer.h"

FileBuffer::FileBuffer()
{
	m_fd			= -1;
	m_FileName		= NULL;
	m_AccessCount	= 0;
	m_FileLength	= -1;
	dequeh_init(&m_PiecesDeque);
}

FileBuffer::~FileBuffer()
{
	if(m_fd != -1)
	{
		close(m_fd);
		m_fd = -1;
	}
	dequeh_release(&m_PiecesDeque, piece_release);
}

int FileBuffer::Open(char* file_name)
{	
	m_FileName = strdup(file_name);
	if(m_FileName == NULL)
	{
		return -1;
	}
	
	m_fd = open(m_FileName, O_RDONLY);
	if(m_fd < 0)
	{
		return -1;
	}

	m_FileLength = lseek(m_fd, 0, SEEK_END);
	lseek(m_fd, 0, SEEK_SET);
	return 0;
}

int64_t FileBuffer::GetFileLength()
{
	return m_FileLength;
}

PIECE_T* FileBuffer::FindPiece(int piece_index)
{
	DEQUEH_NODE* nodep = m_PiecesDeque.headp;
	while(nodep != NULL)
	{
		PIECE_T* piecep = (PIECE_T*)nodep->elementp;
		if(piecep->index == piece_index)
		{
			return piecep;
		}		

		if(nodep->nextp == m_PiecesDeque.headp)
		{
			break;
		}
		nodep = nodep->nextp;
	}

	return NULL;
}

PIECE_T* FileBuffer::ReadPiece(int piece_index)
{
	int64_t file_offset = piece_index * PIECE_SIZE;
	int64_t	want_length = PIECE_SIZE;
	if(file_offset + want_length > m_FileLength)
	{
		want_length = m_FileLength - file_offset;
	}

	PIECE_T* piecep = (PIECE_T*)malloc(sizeof(PIECE_T));
	if(piecep == NULL)
	{
		return NULL;
	}
	piecep->index 			= piece_index;
	piecep->access_count 	= 0;
	piecep->size 			= want_length;
	piecep->datap 			= (u_int8_t*)malloc(piecep->size);
	if(piecep->datap == NULL)
	{
		free(piecep);
		piecep = NULL;
		return NULL;
	}
	
	lseek(m_fd, file_offset, SEEK_SET);
	int64_t read_len = read(m_fd, piecep->datap, want_length);
	piecep->len = read_len;

	int ret = dequeh_append(&m_PiecesDeque, piecep);
	if(ret < 0)
	{
		if(piecep->datap != NULL)
		{
			free(piecep->datap);
			piecep->datap = NULL;
		}	
		free(piecep);
		piecep = NULL;
		return NULL;
	}
	
	return piecep;	
}

int64_t FileBuffer::Read(int64_t file_offset, u_int8_t* buffer, u_int64_t size)
{
	int64_t read_len = 0;
	
	int64_t file_position_begin = file_offset;
	int64_t	file_position_end 	= file_position_begin + size;
	if(file_position_end >= m_FileLength)
	{
		file_position_end = m_FileLength - 1;
	}
	
	int64_t piece_index_begin 	= file_position_begin / PIECE_SIZE;
	int64_t	piece_index_end		= file_position_end	  / PIECE_SIZE;

	int64_t piece_index = 0;
	for(piece_index=piece_index_begin; piece_index<=piece_index_end; piece_index++)
	{
		PIECE_T* piecep = this->FindPiece(piece_index);
		fprintf(stdout, "%s: FindPiece(%ld) return %p\n", __PRETTY_FUNCTION__, piece_index, piecep);		
		if(piecep == NULL) 
		{
			piecep = this->ReadPiece(piece_index);	
			fprintf(stdout, "%s: ReadPiece(%ld) return %p\n", __PRETTY_FUNCTION__, piece_index, piecep);		
			if(piecep == NULL)
			{
				return read_len;
			}
		}
		int64_t piece_position_begin 	= 0;
		int64_t piece_position_end		= PIECE_SIZE - 1;		
		if(piece_index == piece_index_begin)
		{
			piece_position_begin = file_position_begin % PIECE_SIZE;
		}
		if(piece_index == piece_index_end)
		{
			piece_position_end	= file_position_end % PIECE_SIZE;
		}
		int64_t	piece_data_len = piece_position_end - piece_position_begin + 1;
		memcpy(buffer+read_len, piecep->datap+piece_position_begin, piece_data_len);
		read_len += piece_data_len;		
	}
		
	return read_len;
}

CFile::CFile(FileBuffer* FileBufferp)
{
	m_FileBuffer = FileBufferp;
	m_FileOffset = 0;
}

CFile::~CFile()
{
	
}

int64_t CFile::GetFileLength()
{
	int64_t file_len = m_FileBuffer->GetFileLength();
	return file_len;
}

int64_t CFile::Read(u_int8_t* buffer, u_int64_t size)
{
	int64_t read_len = m_FileBuffer->Read(m_FileOffset, buffer, size);
	return read_len;
}

