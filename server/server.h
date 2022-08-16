#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>

#include "Epoller.h"
#include "../lock/locker.h"
#include "../threadpool/threadpool.h"
#include "../http/httpconn.h"
#include "../timer/heaptimer.h"
#include "../log/log.h"
#include "../sqlpool/sql_connection_pool.h"

class server {
public:
    server(int port, int timeoutMS, bool optLinger,int trigMode,
            int sqlport, const char* sqlUser, const char* sqlPwd, const char* dbName,int connPoolNum, 
            int threadNum, bool openLog, int logLevel);
    ~server();

    void start();


private:

    bool initSocket();
    // 初始化事件模型
    void initEventMode(int trigMode);
    void addClient(int fd, sockaddr_in addr);

    void dealListen();
    void dealWrite(HttpConn* client);
    void dealRead(HttpConn* client);

    void SendError(int fd, const char*info);
    void ExtentTime(HttpConn* client);
    void CloseConn(HttpConn* client);

    void OnRead(HttpConn* client);
    void OnWrite(HttpConn* client);
    void OnProcess(HttpConn* client);

    static const int MAX_FD = 65536;
    static int setnonblocking(int fd);

    int m_port;
    int m_listenfd;
    bool m_openLinger;
    int m_timeoutMS;  // 定时器超时时间
    bool m_isclose;

    uint32_t m_listenEvent;
    uint32_t m_connEvent;

    std::unique_ptr<HeapTimer> timer;  //定时器
    std::unique_ptr<ThreadPool> thread_pool; //线程池
    std::unique_ptr<Epoller> epoller;  // epoll对象
    std::unordered_map<int, HttpConn> users;
};

#endif