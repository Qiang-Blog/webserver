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
    void MakeResponse(Buffer& buff); // ��Ӧ��
    void UnmapFile(); // �ļ����ӳ��
    char* File(); 
    size_t FileLen() const; // �ļ�����
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return code_; } 

private:
    void AddStateLine_(Buffer &buff); // �����Ӧ��
    void AddHeader_(Buffer &buff);  //�����Ӧͷ
    void AddContent_(Buffer &buff);  //�����Ӧ����

    void ErrorHtml_();  // ��Ӧ����ҳ��
    std::string GetFileType_(); // ��ȡ�ļ�����

    int code_;  // ��Ӧ��״̬��
    bool isKeepAlive_;  // �Ƿ񱣳ֳ�����

    std::string path_;  // �����ļ�
    std::string srcDir_;  //�ļ�Ŀ¼
    
    char* mmFile_;  // Ŀ���ļ�
    struct stat mmFileStat_; // Ŀ���ļ���״̬��ͨ�������ǿ����ж��ļ��Ƿ���ڡ��Ƿ�ΪĿ¼���Ƿ�ɶ�������ȡ�ļ���С����Ϣ

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};


#endif