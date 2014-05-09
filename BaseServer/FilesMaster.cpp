
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "FilesMaster.h"

FilesMaster::FilesMaster()
{
	dequeh_init(&m_FilesDeque);
}

FilesMaster::~FilesMaster()
{
	dequeh_release(&m_FilesDeque, FileBuffer_release);
}

int FilesMaster::AccessFile(char* file_name)
{
	FileBuffer* FileBufferp = FindFileBuffer(file_name);
	if(FileBufferp != NULL)
	{
		return 0;
	}
	
	int ret = 0;	
	ret = access(file_name, R_OK);
	return ret;
}

FileBuffer* FilesMaster::OpenFileBuffer(char * file_name)
{
	FileBuffer* FileBufferp = FindFileBuffer(file_name);
	fprintf(stdout, "%s: FindFileBuffer(%s) return %p\n", __PRETTY_FUNCTION__, file_name, FileBufferp);
	if(FileBufferp == NULL)
	{
		FileBufferp = new FileBuffer();	
		int ret = 0;
		ret = FileBufferp->Open(file_name);
		if(ret < 0)
		{
			delete FileBufferp;
			FileBufferp = NULL;
			return NULL;
		}
		ret = this->AddFileBuffer(FileBufferp);
		if(ret < 0)
		{
			delete FileBufferp;
			FileBufferp = NULL;
			return NULL;
		}
		return FileBufferp;
	}	

	return FileBufferp;
}

CFile*	FilesMaster::OpenFile(char* file_name)
{
	FileBuffer* FileBufferp = OpenFileBuffer(file_name);
	fprintf(stdout, "%s: OpenFileBuffer(%s) return %p\n", __PRETTY_FUNCTION__, file_name, FileBufferp);
	if(FileBufferp == NULL)
	{
		return NULL;
	}	
	
	CFile* filep = new CFile(FileBufferp);
	return filep;
}


FileBuffer*	FilesMaster::FindFileBuffer(char* file_name)
{
	DEQUEH_NODE* nodep = m_FilesDeque.headp;
	while(nodep != NULL)
	{
		FileBuffer* FileBufferp = (FileBuffer*)nodep->elementp;
		if(strcmp(file_name, FileBufferp->GetFileName()) == 0)
		{
			return FileBufferp;
		}		

		if(nodep->nextp == m_FilesDeque.headp)
		{
			break;
		}
		nodep = nodep->nextp;
	}
	
	return NULL;
}

int	FilesMaster::AddFileBuffer(FileBuffer* FileBufferp)
{
	int ret = 0;
	ret = dequeh_append(&m_FilesDeque, FileBufferp);
	return ret;
}
