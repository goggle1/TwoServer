#ifndef __FILESMASTER_H__
#define __FILESMASTER_H__

#define USE_FILE_BUFFER 1

#include "FileBuffer.h"

class FilesMaster
{
public:
	FilesMaster();
	virtual ~FilesMaster();
	int			AccessFile(char* file_name);
	FileBuffer*	OpenFileBuffer(char* file_name);
	CFile*		OpenFile(char* file_name);
	
protected:
	int			AddFileBuffer(FileBuffer* FileBufferp);
	FileBuffer*	FindFileBuffer(char* file_name);
	DEQUEH_T	m_FilesDeque;
	// todo: hash replace deque.
};

extern FilesMaster*	g_files_master;

#endif