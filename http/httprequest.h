#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>

#include "../buffer/buffer.h"
#include "../sqlpool/sql_connection_pool.h"
#include "../log/log.h"
#include "../session/session.h"

class HttpRequest {
public:
    // 主状态机状态
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,        
    };
    // 服务器处理HTTP请求的可能结果，报文解析的结果
    enum HTTP_CODE { NO_REQUEST = 0, GET_REQUEST, POST_REQUEST,BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    
    HttpRequest() {
        Init();
    }
    ~HttpRequest() = default;

    void Init();
    bool parse(Buffer &buff);

    std::string path() const;
    std::string& path();
    std::string method() const;  // 请求方法
    std::string version() const;  // HTTP版本
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;  // 是否保持长连接


private:
    bool ParseRequestLine_(const std::string& line);  // 解析请求行
    void ParseHeader_(const std::string& line);  // 解析请求头
    void ParseBody_(const std::string& line);  // 解析请求体

    void ParsePath_(); 
    void ParsePost_(); // 解析POST请求
    void ParseFromUrlencoded_();  // 解析POST请求的表单

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin); // 判断账户密码是否正确

    PARSE_STATE state_;  // 当前正在解析的位置
    std::string method_, path_, version_, body_; // 请求方法、文件、版本、请求体
    std::unordered_map<std::string, std::string> header_; // 请求头内容
    std::unordered_map<std::string, std::string> post_;  // post表单内容

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);

    // static Session session;  //session保存所有连接登录状态
    // LOGIN_STATUS login_stat;    // 当前连接登录状态
    // std::string m_sessionid;     // sessionid 保存状态
};

#endif