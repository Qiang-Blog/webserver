#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>

class Epoller {
public:
    Epoller(int maxEvent = 1024);

    ~Epoller();

    bool addfd(int fd, uint32_t events);

    bool modfd(int fd, uint32_t events);

    bool delfd(int fd);

    int Wait(int timeoutMs = -1);

    int getEventFd(size_t i) const;

    uint32_t getEvents(size_t i) const;
    int epollfd;
private:
    

    std::vector<struct epoll_event> events;
};

#endif