#include "Epoller.h"

Epoller::Epoller(int maxEvent):epollfd(epoll_create(512)), events(maxEvent) {

}

Epoller::~Epoller() {
    close(epollfd);
}

bool Epoller::addfd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}


bool Epoller::modfd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::delfd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::Wait(int timeoutMs) {
    return epoll_wait(epollfd, &events[0], static_cast<int>(events.size()), timeoutMs);
}

int Epoller::getEventFd(size_t i) const {
    return events[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const {
    return events[i].events;
}