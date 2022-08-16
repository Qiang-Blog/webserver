#include "sql_connection_pool.h"
#include "../lock/locker.h"

connection_pool::connection_pool() {

} 

connection_pool::~connection_pool() {
    destory();
}

connection_pool *connection_pool::getInstance() {
    static connection_pool instance;
    return &instance;
}

void connection_pool::init(std::string  url,int  port, std::string  user, std::string  password, std::string  databasename,int max_conn, int close_log) {
    m_url = url;
    m_port = port;
    m_user = user;
    m_password = password;
    m_databaseName = databasename;
    m_maxConn = max_conn;
    m_curConn = 0;
    m_freeConn = max_conn;
    for(int i=0;i<max_conn;i++) {
        MYSQL* con = NULL;
        con = mysql_init(con);
        con = mysql_real_connect(con,url.c_str(),user.c_str(),password.c_str(),databasename.c_str(),port,NULL,0);
        connque.push(con);
    }
    reverse = sem(m_freeConn);

}
MYSQL *connection_pool::GetConection() {
    MYSQL* con = NULL;
    reverse.wait();
    lock.lock();
    con = connque.front();
    connque.pop();

    m_freeConn--;
    m_curConn++;
    lock.unlock();

    return con;
}

bool connection_pool::ReleaseConnection(MYSQL* con) {
    if(con == NULL) return false;
    lock.lock();
    connque.push(con);
    m_freeConn++;
    m_curConn--;
    reverse.post();
    lock.unlock();
    return true;
}

void connection_pool::destory() {
    lock.lock();
    while(!connque.empty()) {
        MYSQL* con = connque.front();
        connque.pop();
        mysql_close(con);
    }
    lock.unlock();
}

connectionRAII::connectionRAII(MYSQL** conn, connection_pool* connPoll) {
    *conn = connPoll->GetConection();

    connRAII = *conn;
    poolRAII = connPoll;
}
connectionRAII::~connectionRAII() {
    poolRAII->ReleaseConnection(connRAII);
}