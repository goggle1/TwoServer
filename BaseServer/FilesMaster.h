#ifndef __FILESMASTER_H__
#define __FILESMASTER_H__

#include "FileBuffer.h"

class FilesMaster
{
public:
	FilesMaster();
	virtual ~FilesMaster();
	int			AccessFile(char* file_name);
	CFile*		OpenFile(char* file_name);
	
protected:
	int			AddFileBuffer(FileBuffer* FileBufferp);
	FileBuffer*	FindFile(char* file_name);
	DEQUEH_T	m_FilesDeque;
	// todo: hash replace deque.
};

extern FilesMaster*	g_files_master;

#endif