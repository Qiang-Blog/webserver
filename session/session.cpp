#include "session.h"

LOGIN_STATUS Session::get_status(string sessionid) {
    m_lock.lock();
    LOGIN_STATUS status = UNLOGIN;
    if(m_cache.count(sessionid)) {
        movetotop(sessionid);
        status = m_cache[sessionid]->status;
    }

    m_lock.unlock();
    return status;
}


string Session::get_username(string sessionid) {
    m_lock.lock();
    string username = "";
    if(m_cache.count(sessionid)) {
        movetotop(sessionid);
        username = m_cache[sessionid]->username;
    }
    m_lock.unlock();
    return username;
}

string Session::put(string username) {
    m_lock.lock();
    string sessionid = MD5(username);
    if(m_cache.count(sessionid) == 0) {
        printf("sessionid not exist!\n");
        if(m_users.size() >= m_capacity) {
            m_cache.erase(m_users.back().sessionid);
            m_users.pop_back();
        }
        User user(username,sessionid,LOGIN,time(NULL));
        m_users.push_front(user);
        m_cache[sessionid] = m_users.begin();
    }
    m_cache[sessionid]->status = LOGIN;
    dealtimeout();
    m_lock.unlock();
    return sessionid;
}

void Session::movetotop(string key) {
    auto p = *m_cache[key];
    m_users.erase(m_cache[key]);
    m_users.push_front(p);
    m_cache[key]=m_users.begin();
    m_users.front().last_time = time(NULL);
    dealtimeout();
}

void Session::dealtimeout() {
    time_t curtime = time(NULL);
    while(!m_users.empty()&&m_users.back().last_time + m_timeout*60 < curtime) {
        m_cache.erase(m_users.back().sessionid);
        m_users.pop_back();
    }
}

string Session::MD5(const string &src) {
    MD5_CTX ctx;

    string md5_string;
    unsigned char md[16] = {0};
    char tmp[40] = {0};
    string code = src + to_string(time(NULL));
    MD5_Init(&ctx);
    MD5_Update(&ctx, code.c_str(), code.size());
    MD5_Final(md, &ctx);

    for (int i = 0; i < 16; ++i) {
        memset(tmp, 0x00, sizeof(tmp));
        sprintf(tmp, "%02X", md[i]);
        md5_string += tmp;
    }
    return md5_string;
}