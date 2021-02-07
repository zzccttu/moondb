#pragma once

#include <string>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <map>
using namespace std;

#include "ctime.hpp"
#include "cpack.hpp"
#include "cany.hpp"

#if defined(_WIN32)
// 注意将_mingw.h文件中的_WIN32_WINNT值置为0x0603
	#include <ws2tcpip.h>
	#define MoonSockRecv(sock, buf, len)	::recv(sock, buf, len, 0)
	#define MoonSockSend(sock, buf, len)	::send(sock, buf, len, 0)
	#define MoonSockClose(sock)				::closesocket(sock)
	#define MoonLastError()                 std::to_string(::GetLastError())
	#define socklen_t						int
	#define MoonLastErrno()					::WSAGetLastError()
	#define SOCKET_WOULDBLOCK				WSAEWOULDBLOCK
	#define SOCKET_AGAIN					WSAEWOULDBLOCK
	#define SOCKET_TIMEOUT					WSAETIMEDOUT
	#define bzero(dst, size)				::memset(dst, 0, size)
#else
	#include <netinet/tcp.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#define SOCKET							int
	#define SOCKET_ERROR					-1
	#define MoonSockRecv(sock, buf, len)	::recv(sock, buf, len, MSG_NOSIGNAL)
	#define MoonSockSend(sock, buf, len)	::send(sock, buf, len, MSG_NOSIGNAL)
	#define MoonSockClose(sock)				::close(sock)
	#define INVALID_SOCKET					-1
	#define MoonLastError()                 string(::strerror(errno))
	#define MoonLastErrno()					errno
	#define SOCKET_WOULDBLOCK				EWOULDBLOCK
	#define SOCKET_AGAIN					EAGAIN
	#define SOCKET_TIMEOUT					EAGAIN
#endif

namespace MoonDb {

class CMoonDbClient
{
public:
	CMoonDbClient(const string& host, uint16_t port, const string& dbname = "", uint64_t timeout = 5000000, bool autoreconn = false);
	~CMoonDbClient();
	void UseDatabase(const string& dbname)
	{
		DatabaseName = dbname;
	}

	__uint128_t InsertData(const string& table, map<string, CAny>& data);
	__uint128_t UpdateData(const string& table, __uint128_t id, map<string, CAny>& data);
	__uint128_t DeleteData(const string& table, __uint128_t id);
	__uint128_t ReplaceData(const string& table, __uint128_t id, map<string, CAny>& data);
	__uint128_t GetData(const string& table, __uint128_t id, map<string, CAny>& data);

	static string Quote(const string& str);

protected:
	void Connect();
	void SynchConnect();
	void AsyncConnect();
	void Clear();

	inline uint32_t IPToLong(const string& ip);
	void Send(const CPack& pack);
	ResponseType Receive(CPack& pack);
	void PrepareData(CPack& pack, OperationType oper, const string& table, const map<string, CAny>& data);
	__uint128_t IdNumResult(CPack& pack);

	string Host;
	uint16_t Port;
	string DatabaseName;
	uint64_t Timeout;
	bool AutoReconnect;

	bool IPv6;
	SOCKET Socket;
	int32_t SendBufSize;
	int32_t ReceiveBufSize;
	int32_t BytesPerRead;
	CPack Content;
};

std::ostream & operator << (std::ostream & os, const map<string, CAny>& data);

}
