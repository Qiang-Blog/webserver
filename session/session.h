#ifndef SESSION_H
#define SESSION_H

#include <openssl/md5.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <string.h>
#include <map>
#include <list>

#include "../lock/locker.h"

using namespace std;

enum LOGIN_STATUS //用户状态
{
    LOGIN = 0,
    UNLOGIN,
    FIRST_LOGIN
};
//用户信息
struct User {
    string username;
    string sessionid;
    LOGIN_STATUS status;
    time_t last_time;   // 上次请求时间
    User(string _username,string _sessionid,LOGIN_STATUS _status,time_t _lasttime):username(_username),sessionid(_sessionid),status(_status),last_time(_lasttime) {}
    
};
class Session {
public:
    //初始化session最多能缓存多少sessionid   以及   超时时间
    Session(int capacity, int timeout):m_capacity(capacity),m_timeout(timeout) {}
    // 根据sessionid返回  当前用户状态
    LOGIN_STATUS get_status(string sessionid);
    // 根据sessionid 查询用户名
    string get_username(string sessionid);
    // 首次添加，返回sessionid
    string put(string username);


private:
    //将最新访问的用户移动到最前面
    void movetotop(string key);
    // 删除超时的sessionid
    void dealtimeout();
    // 将字符串转换成MD5编码
    string MD5(const string& src);

    typedef list<User> List;
    size_t m_capacity;  //最多sessions数量
    int m_timeout;   // 超时时间
    List m_users;    //访问时间排序
    map<string,List::iterator> m_cache;  //缓冲
    locker m_lock;


};


#endif