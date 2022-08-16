out: main.o buffer.o httpconn.o httprequest.o httpresponse.o Epoller.o server.o heaptimer.o log.o sql_connection_pool.o session.o
	g++ -o out main.o buffer.o httpconn.o httprequest.o httpresponse.o Epoller.o server.o heaptimer.o log.o sql_connection_pool.o session.o -pthread -lmysqlclient -lcrypto
main.o:main.cpp
	g++ -c -o main.o main.cpp
buffer.o:./buffer/buffer.cpp
	g++ -c -o buffer.o ./buffer/buffer.cpp
httpconn.o:./http/httpconn.cpp
	g++ -c -o httpconn.o ./http/httpconn.cpp
httprequest.o:./http/httprequest.cpp
	g++ -c -o httprequest.o ./http/httprequest.cpp
httpresponse.o:./http/httpresponse.cpp
	g++ -c -o httpresponse.o ./http/httpresponse.cpp
Epoller.o:./server/Epoller.cpp
	g++ -c -o Epoller.o ./server/Epoller.cpp
server.o:./server/server.cpp
	g++ -c -o server.o ./server/server.cpp
heaptimer.o:./timer/heaptimer.cpp
	g++ -c -o heaptimer.o ./timer/heaptimer.cpp
log.o:./log/log.cpp
	g++ -c -o log.o ./log/log.cpp
sql_connection_pool.o:./sqlpool/sql_connection_pool.cpp
	g++ -c -o sql_connection_pool.o ./sqlpool/sql_connection_pool.cpp -lmysqlclient
session.o:./session/session.cpp
	g++ -c -o session.o ./session/session.cpp -lcrypto
clean:
	rm *.o
