#include "common.h"

int my_printf(FILE *stream, char* buffer, int len)
{
	int index = 0;
	for(index=0; index<len; index++)
	{
		fprintf(stream, "%c", buffer[index]);
	}	
	return 0;
}

