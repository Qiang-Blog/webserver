#ifndef SQL_CONNECTION_POOL
#define SQL_CONNECTION_POOL

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <error.h>
#include <queue>
#include <mysql/mysql.h>
#include "../lock/locker.h"
class connection_pool {
public:
    MYSQL *GetConection();
    bool ReleaseConnection(MYSQL* con);

    void destory();


    static connection_pool* getInstance();
    void init(std::string  url,int  port, std::string  user, std::string  password, std::string  databasename,int max_conn, int close_log);

private:
    // 使用单例模式创建数据库连接池，将构造析构初始化私有
    connection_pool();
    ~connection_pool();

    // int m_maxConn;   // 最大连接数
    // int m_curConn;  //当前已使用的连接数
    // int m_freeConn;  //当前空闲的连接数
    std::queue<MYSQL *> connque;  //数据库连接池
    locker lock; 
    sem reverse;

public:
    int m_maxConn;   //最大连接数
    int m_curConn;  //当前已使用的连接数
    int m_freeConn;  //当前空闲的连接数
    std::string m_url;    // 主机地址
    std::string m_port;   // 数据库端口号
    std::string m_user;    //数据库登录名
    std::string m_password;   //数据库登录密码
    std::string m_databaseName;  //使用的数据库名称
    bool m_close_log;  // 日志的开启与关闭

};

class connectionRAII {
public:
    connectionRAII(MYSQL** conn, connection_pool* connPoll);
    ~connectionRAII();
private:
    MYSQL* connRAII;
    connection_pool* poolRAII;
};

#endif