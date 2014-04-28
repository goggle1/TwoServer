#include <stdio.h>

#include "task.h"
#include "TaskThread.h"

Task::Task() 
{	
	m_task_thread = NULL;	
	//fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);
}

Task::~Task()
{	
	//fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);
}



