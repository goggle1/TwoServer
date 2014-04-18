#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>

#include "events.h"

Events::Events()
{
	m_epoll_fd = -1;	
}

Events::~Events()
{
}

int Events::Init()
{
	m_epoll_fd = epoll_create(EPOLL_SIZE);
	if(m_epoll_fd < 0)
	{
		return -1;
	}
	
	return 0;
}

int Events::AddWatch(int fd, u_int32_t events, void* handler)
{
	int ret = 0;
	
	struct epoll_event epoll_event = {0};	
	epoll_event.events |= EPOLLET;	
	if(events&EVENT_READ)
	{
		epoll_event.events |= EPOLLIN;
	}
	if(events&EVENT_WRITE)
	{
		epoll_event.events |= EPOLLOUT;
	}

	epoll_event.data.ptr = handler;
	
	ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event);
	if(ret < 0)
	{
		fprintf(stderr, "%s: epoll_ctl add %d return %d, errno=%d, %s\n", __PRETTY_FUNCTION__, fd, ret, errno, strerror(errno));
	}
	
	return ret;
}

int Events::DeleteWatch(int fd)
{
	int ret = 0;
	
	ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	if(ret < 0)
	{
		fprintf(stderr, "%s: epoll_ctl add %d return %d, errno=%d, %s\n", __PRETTY_FUNCTION__, fd, ret, errno, strerror(errno));
	}
	
	return ret;
}

int Events::ModifyWatch(int fd, u_int32_t events, void* handler)
{
	int ret = 0;

	struct epoll_event epoll_event = {0};	
	epoll_event.events |= EPOLLET;	
	if(events&EVENT_READ)
	{
		epoll_event.events |= EPOLLIN;
	}
	if(events&EVENT_WRITE)
	{
		epoll_event.events |= EPOLLOUT;
	}

	epoll_event.data.ptr = handler;
	
	ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &epoll_event);
	if(ret < 0)
	{
		fprintf(stderr, "%s: epoll_ctl modify %d return %d, errno=%d, %s\n", __PRETTY_FUNCTION__, fd, ret, errno, strerror(errno));
	}
	
	return ret;
}


