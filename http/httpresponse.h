#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff); // 响应包
    void UnmapFile(); // 文件解除映射
    char* File(); 
    size_t FileLen() const; // 文件长度
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return code_; } 

private:
    void AddStateLine_(Buffer &buff); // 添加响应行
    void AddHeader_(Buffer &buff);  //添加响应头
    void AddContent_(Buffer &buff);  //添加响应内容

    void ErrorHtml_();  // 回应错误页面
    std::string GetFileType_(); // 获取文件类型

    int code_;  // 响应包状态码
    bool isKeepAlive_;  // 是否保持长连接

    std::string path_;  // 请求文件
    std::string srcDir_;  //文件目录
    
    char* mmFile_;  // 目标文件
    struct stat mmFileStat_; // 目标文件的状态。通过它我们可以判断文件是否存在、是否为目录、是否可读，并获取文件大小等信息

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};


#endif