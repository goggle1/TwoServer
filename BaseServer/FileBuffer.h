#ifndef __FILEBUFFER_H__
#define __FILEBUFFER_H__

#include <sys/types.h>
#include <stdlib.h>

#include "dequeH.h"

#define PIECE_SIZE			(1024*256)

typedef struct piece_t
{
	int 	index;
	int 	size;
	int 	len;	
	int		access_count;
	void* 	data;	
} PIECE_T;

inline void piece_release(void* elementp)
{
	PIECE_T* piecep = (PIECE_T*)elementp;
	if(piecep->data != NULL)
	{
		free(piecep->data);
		piecep->data = NULL;
	}

	free(piecep);	
}

class FileBuffer
{
public:
	FileBuffer();
	virtual ~FileBuffer();
	int			Open(char* file_name);
	char*		GetFileName() { return m_FileName; };
	int64_t		GetFileLength();
	int64_t		Read(int64_t offset, void* buffer, u_int64_t size);
protected:
	PIECE_T*	FindPiece(int piece_index);
	PIECE_T*	ReadPiece(int piece_index);
	int 		m_fd;
	char*		m_FileName;
	int			m_AccessCount;
	int64_t		m_FileLength;
	DEQUEH_T	m_PiecesDeque;
};


inline void FileBuffer_release(void* elementp)
{
	FileBuffer* FileBufferp = (FileBuffer*)elementp;
	delete FileBufferp;
}

// CFile: ClassFile, which is similiar to FILE(man fopen).
class  CFile
{
public:
	CFile(FileBuffer* FileBufferp);
	virtual ~CFile();
	int64_t		GetFileLength();
	int64_t		Read(void* buffer, u_int64_t size);
	
protected:
	FileBuffer*		m_FileBuffer;
	int64_t			m_FileOffset;
};

#endif
