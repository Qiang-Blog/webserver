#include "server.h"

server::server( int port, int timeoutMS, bool optLinger,int trigMode,
                int sqlport, const char* sqlUser, const char* sqlPwd, const char* dbName,int connPoolNum, 
                int threadNum, bool openLog, int logLevel):
                m_port(port), m_openLinger(optLinger),m_timeoutMS(timeoutMS),m_isclose(false),
                timer(new HeapTimer), thread_pool(new ThreadPool(threadNum)), epoller(new Epoller())        
{
    
    // 初始化数据库连接池
    connection_pool::getInstance()->init("localhost",sqlport,sqlUser,sqlPwd,dbName,connPoolNum,0);
    // 初始化事件模型
    initEventMode(trigMode);

    if(openLog) {
        Log::get_instance()->init("ServerLog/", 0, 20000, 800000, 10);
        Log::get_instance()->m_close_log = 0;
        if(m_isclose) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", m_port, optLinger? "true":"false");
            // LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
            //                 (listenEvent_ & EPOLLET ? "ET": "LT"),
            //                 (connEvent_ & EPOLLET ? "ET": "LT"));
            // LOG_INFO("LogSys level: %d", logLevel);
            // LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
    if(!initSocket()) {
        m_isclose = true;
    }

}

server::~server() {
    close(m_listenfd);
    m_isclose = true;

}

void server::initEventMode(int trigMode) {
    m_listenEvent = EPOLLRDHUP;
    m_connEvent = EPOLLONESHOT | EPOLLRDHUP;
    // 默认是LT + LT;

    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        m_connEvent |= EPOLLET;
        break;
    case 2:
        m_listenEvent |= EPOLLET;
        break;
    case 3:
        m_listenEvent |= EPOLLET;
        m_connEvent |= EPOLLET;
        break;
    default:
        m_listenEvent |= EPOLLET;
        m_connEvent |= EPOLLET;
        break;
    }
    HttpConn::isET = (m_connEvent & EPOLLET);
}

void server::start() {
    int timeMS = -1;  /* epoll wait timeout == -1 无事件将阻塞 */
    if(!m_isclose) { LOG_INFO("========== Server start =========="); }
    while(!m_isclose) {
        if(m_timeoutMS > 0) {
            timeMS = timer->GetNextTick(); // 触发tick
        }
        int eventCnt = epoller->Wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            // 处理事件
            int fd = epoller->getEventFd(i);
            uint32_t events = epoller->getEvents(i);
            if(fd == m_listenfd) {
                dealListen();
            }
            // 客户端关闭
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                CloseConn(&users[fd]);
            }
            // 读事件
            else if(events & EPOLLIN) {
                dealRead(&users[fd]);
            }
            // 写事件
            else if(events & EPOLLOUT) {
                dealWrite(&users[fd]);
            } 
            else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}



// 发送错误信息
void server::SendError(int fd, const char*info) {
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

// 重置客户端超时时间
void server::ExtentTime(HttpConn* client) {
    if(m_timeoutMS > 0) {
        timer->adjust(client->GetFd(), m_timeoutMS);
    }
}

// 添加连接
void server::addClient(int fd, sockaddr_in addr) {
    users[fd].init(fd, addr);
    if(m_timeoutMS > 0) {
        timer->add(fd, m_timeoutMS, std::bind(&server::CloseConn, this, &users[fd]));
    }
    epoller->addfd(fd, EPOLLIN | m_connEvent);
    setnonblocking(fd);
    LOG_INFO("Client[%d] in!", users[fd].GetFd());
}


// 处理监听事件
void server::dealListen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(m_listenfd, (struct sockaddr *)&addr, &len);
        if(fd <= 0) { return;}
        else if(HttpConn::userCount >= MAX_FD) {
            SendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        addClient(fd, addr);
    } while(m_listenEvent & EPOLLET);

}


// 处理读请求
void server::dealRead(HttpConn* client) {
    ExtentTime(client);
    //Reactor模式   把事件放入线程池中处理读事件
    // thread_pool->addtask(std::bind(&server::OnRead, this, client));
    // thread_pool->addtask(std::bind(&server::OnProcess, this, client));
    //Proactor模式  主线程处理读事件
    OnRead(client);
    thread_pool->addtask(std::bind(&server::OnProcess, this, client));

}

//处理写请求
void server::dealWrite(HttpConn* client) {
    ExtentTime(client);
    //Reactor模式   把事件放入线程池中处理写事件
    // thread_pool->addtask(std::bind(&server::OnWrite, this, client));
    //Proactor模式  主线程处理写事件
    OnWrite(client);

}

void server::OnRead(HttpConn* client) {
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn(client);
        return;
    }
    // OnProcess(client);
}

void server::OnProcess(HttpConn* client) {
    if(client->process()) {
        epoller->modfd(client->GetFd(), m_connEvent | EPOLLOUT);
    } else {
        epoller->modfd(client->GetFd(), m_connEvent | EPOLLIN);
    }
}

void server::OnWrite(HttpConn* client) {
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        /* 传输完成 */
        if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            /* 继续传输 */
            epoller->modfd(client->GetFd(), m_connEvent | EPOLLOUT);
            return;
        }
    }
    CloseConn(client);
}


// 关闭连接
void server::CloseConn(HttpConn* client) {
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller->delfd(client->GetFd());
    client->Close();
}

// 初始化socket
bool server::initSocket() {
    int ret;

    struct sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(m_port);

    //创建流式socket
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(m_listenfd < 0) {
        LOG_ERROR("Create socket error!");
        return false;
    }
    struct linger optLinger = { 0 };
    if(m_openLinger) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }
    ret = setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(m_listenfd);
        LOG_ERROR("Init linger error!", m_port);
        return false;
    }
    // 端口复用   只有最后一个套接字会正常接收数据
    int reuse = 1;
    ret = setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof( reuse ) );
    if(ret < 0) {
        close(m_listenfd);
        LOG_ERROR("set socket setsockopt error !");
        return false;
    }

    // 指定用于通信的ip和端口
    ret = bind(m_listenfd, (struct sockaddr*)&address, sizeof(address));
    if(ret < 0) {
        close(m_listenfd);
        LOG_ERROR("Bind Port:%d error!", m_port);
        return false;
    }
    
    // 将socket设为监听模式
    ret = listen(m_listenfd, 5);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", m_port);
        close(m_listenfd);
        return false;
    }
    
    ret = epoller->addfd(m_listenfd, m_listenEvent | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(m_listenfd);
        return false;
    }
    
    setnonblocking(m_listenfd);
    LOG_INFO("Server port:%d", m_port);
    return true;

}

// 设置为非阻塞
int server::setnonblocking(int fd) {
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}