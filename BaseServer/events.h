#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <sys/types.h>

/*
#define EPOLLIN        0x001
#define EPOLLPRI       0x002
#define EPOLLOUT       0x004
#define EPOLLRDNORM    0x040
#define EPOLLRDBAND    0x080
#define EPOLLWRNORM    0x100
#define EPOLLWRBAND    0x200
#define EPOLLMSG       0x400
#define EPOLLERR       0x008
#define EPOLLHUP       0x010


#define EPOLLET        0x80000000
#define EPOLLONESHOT   0x40000000
*/


#define EVENT_READ		0x0000000000000001
#define EVENT_WRITE		0x0000000000000002
#define EVENT_CONTINUE	0x0000000100000000
#define EVENT_TIMEOUT	0x0000000200000000


#define EPOLL_SIZE 		1000000
#define MAX_EVENTS		64
// ms
#define WAIT_PERIOD		10


class Events
{
public:
	Events();
	~Events();
	
	int Init();
	int AddWatch(int fd, u_int32_t events, void* handler);
	int ModifyWatch(int fd, u_int32_t events, void* handler);
	int DeleteWatch(int fd);
	
	int	m_epoll_fd;
	//int m_fd_num;	// now, we don't care. maybe care about it later.
	
protected:

};


#endif
