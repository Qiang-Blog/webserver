#include <unistd.h>
#include "server/server.h"

int main() {
    // server(int port, int timeoutMS, bool optLinger,int trigMode,
    //         int sqlport, const char* sqlUser, const char* sqlPwd, const char* dbName,int connPoolNum, 
    //         int threadNum, bool openLog, int logLevel);
    server server(
        10000, 5000, false, 1, 
        3306, "root", "123456", "mydb", 4, 
        8, true, 1
    );
    server.start();

}
/*
~/Linux/webserver5/test_presure/WebBench$ 
./webbench -c 8000 -t 5 -2 -k http://192.168.145.128:10000/login.html
*