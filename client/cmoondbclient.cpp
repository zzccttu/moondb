#include "cmoondbclient.h"

namespace MoonDb {

CMoonDbClient::CMoonDbClient(const string& host, uint16_t port, const string& dbname, uint64_t timeout, bool autoreconn) :
	Host(host), Port(port), DatabaseName(dbname), Timeout(timeout ? timeout : 5000000), AutoReconnect(autoreconn)
{
	IPv6 = Host.find(":") != string::npos;
	SendBufSize = 65536;
	ReceiveBufSize = 65536;
	BytesPerRead = 8192;
	Socket = INVALID_SOCKET;
	Content.Allocate(1048576);//1M
	Connect();
}

CMoonDbClient::~CMoonDbClient()
{
	Clear();
}

void CMoonDbClient::Clear()
{
	if(INVALID_SOCKET != Socket) {
		MoonSockClose(Socket);
	}
}

void CMoonDbClient::Connect()
{
	// 创建socket
	Socket = ::socket(IPv6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP);
#if defined(_WIN32)
	if (Socket == INVALID_SOCKET)
#else
	if (Socket < 0)
#endif
	{
		Clear();
		ThrowError(ERR_SOCKET, "Create Socket Failed: " + MoonLastError());
	}

	// 设置接收发送数据超时时间
#if defined(_WIN32)
	int nNetTimeout = static_cast<int>(::ceil(Timeout / 1000.0));
	if(::setsockopt(Socket, SOL_SOCKET, SO_SNDTIMEO, static_cast<const char *>(static_cast<const void*>(&nNetTimeout)), sizeof(int))) {
		Clear();
		ThrowError(ERR_SOCKET, "setsockopt() failed (SO_SNDTIMEO): " + MoonLastError());
	}
	if(::setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, static_cast<const char *>(static_cast<const void*>(&nNetTimeout)), sizeof(int))) {
		Clear();
		ThrowError(ERR_SOCKET, "setsockopt() failed (SO_RCVTIMEO): " + MoonLastError());
	}
#else
	//设置发送超时
	timeval timeout;
	timeout.tv_sec = Timeout / 1000000;
	timeout.tv_usec = Timeout % 1000000;
	if(::setsockopt(Socket, SOL_SOCKET, SO_SNDTIMEO, static_cast<const char *>(static_cast<const void*>(&timeout)), sizeof(timeval))) {
		Clear();
		ThrowError(ERR_SOCKET, "setsockopt() failed (SO_SNDTIMEO): " + MoonLastError());
	}
	//设置接收超时
	if(::setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, static_cast<const char *>(static_cast<const void*>(&timeout)), sizeof(timeval))) {
		Clear();
		ThrowError(ERR_SOCKET, "setsockopt() failed (SO_RCVTIMEO): " + MoonLastError());
	}
#endif

	// 获取发送和接收缓冲区大小
#if defined(_WIN32)
	int32_t optLen = sizeof(int32_t);
#else
	socklen_t optLen = sizeof(int32_t);
#endif
	::getsockopt(Socket, SOL_SOCKET, SO_SNDBUF, static_cast<char*>(static_cast<void*>(&SendBufSize)), &optLen);
	::getsockopt(Socket, SOL_SOCKET, SO_RCVBUF, static_cast<char*>(static_cast<void*>(&ReceiveBufSize)), &optLen);

	int optval = 1;
	// turn off SIGPIPE signal
#ifdef __linux__
	::setsockopt(Socket, SOL_SOCKET, MSG_NOSIGNAL, static_cast<char*>(static_cast<void*>(&optval)), sizeof(optval));
#elif defined(SO_NOSIGPIPE)
	::setsockopt(Socket, SOL_SOCKET, SO_NOSIGPIPE, static_cast<char*>(static_cast<void*>(&optval)), sizeof(optval));
#endif
	// set SO_LINGER so socket closes gracefully
	linger ling;
	ling.l_onoff = 1;
	ling.l_linger = 10;
	::setsockopt(Socket, SOL_SOCKET, SO_LINGER, static_cast<char*>(static_cast<void*>(&ling)), sizeof(ling));
	// disable the Nagle algorithm so that small packets get sent immediately
	::setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY,  static_cast<char*>(static_cast<void*>(&optval)), sizeof(int));

	//SynchConnect();
	AsyncConnect();
}

void CMoonDbClient::SynchConnect()
{
	if(IPv6) {
		sockaddr_in6 dest;
		bzero(&dest, sizeof(dest));
		dest.sin6_family = AF_INET6;
		dest.sin6_port = htons(Port);
		if(inet_pton(AF_INET6, Host.c_str(), &dest.sin6_addr) < 0) {
			Clear();
			ThrowError(ERR_CONNECT, "Wrong ip address: " + Host);
		}
		int ires;
		auto starttime = CTime::Now();
		do {
			ires = connect(Socket, static_cast<sockaddr*>(static_cast<void*>(&dest)), sizeof(sockaddr_in6));
			if(SOCKET_ERROR != ires) {
				break;
			}
		} while(starttime + 10000000000 < CTime::Now());
		if(SOCKET_ERROR == ires) {
			Clear();
			ThrowError(ERR_CONNECT, "Can't connect " + Host + ":" + to_string(Port));
		}
	}
	else {
		sockaddr_in dest;
		dest.sin_family = AF_INET;
		dest.sin_port = htons(Port);
		if(0 == (dest.sin_addr.s_addr = IPToLong(Host))) {
			Clear();
			ThrowError(ERR_CONNECT, "Wrong ip address: " + Host);
		}
		int ires;
		auto starttime = CTime::Now();
		do {
			ires = connect(Socket, static_cast<sockaddr*>(static_cast<void*>(&dest)), sizeof(sockaddr_in));
			if(SOCKET_ERROR != ires) {
				break;
			}
		} while(starttime + 10000000000 < CTime::Now());
		if(SOCKET_ERROR == ires) {
			Clear();
			ThrowError(ERR_CONNECT, "Can't connect " + Host + ":" + to_string(Port));
		}
	}
}

void CMoonDbClient::AsyncConnect()
{
#if defined(_WIN32)
	int32_t optLen = sizeof(int32_t);
#else
	socklen_t optLen = sizeof(int32_t);
#endif
	u_long ul = 1;
#if defined(_WIN32)
	::ioctlsocket(Socket, static_cast<long>(FIONBIO), &ul);
#else
	::ioctl(Socket, FIONBIO, &ul);
#endif

	// 连接数据库
	int error=-1;
	timeval tm{10, 0};
	fd_set set;
	bool ret = false;
	if(IPv6) {
		sockaddr_in6 dest;
		bzero(&dest, sizeof(dest));
		dest.sin6_family = AF_INET6;
		dest.sin6_port = htons(Port);
		if(inet_pton(AF_INET6, Host.c_str(), &dest.sin6_addr) < 0) {
			Clear();
			ThrowError(ERR_CONNECT, "Wrong ip address: " + Host);
		}

		if(SOCKET_ERROR == connect(Socket, static_cast<sockaddr*>(static_cast<void*>(&dest)), sizeof(sockaddr_in6))) {
			FD_ZERO(&set);
			FD_SET(Socket, &set);
			if(select(static_cast<int>(Socket + 1), nullptr, &set, nullptr, &tm) > 0){
				::getsockopt(Socket, SOL_SOCKET, SO_ERROR, static_cast<char*>(static_cast<void*>(&error)), &optLen);
				if(error == 0) {
					ret = true;
				}
				else {
					ret = false;
				}
			}
			else {
				ret = false;
			}
		}
		else {
			ret = true;
		}
	}
	else {
		sockaddr_in dest;
		dest.sin_family = AF_INET;
		dest.sin_port = htons(Port);
		if(0 == (dest.sin_addr.s_addr = IPToLong(Host))) {
			Clear();
			ThrowError(ERR_CONNECT, "Wrong ip address: " + Host);
		}

		if(SOCKET_ERROR == connect(Socket, static_cast<sockaddr*>(static_cast<void*>(&dest)), sizeof(sockaddr_in))) {
			FD_ZERO(&set);
			FD_SET(Socket, &set);
			if(select(static_cast<int>(Socket + 1), nullptr, &set, nullptr, &tm) > 0){
				::getsockopt(Socket, SOL_SOCKET, SO_ERROR, static_cast<char*>(static_cast<void*>(&error)), &optLen);
				if(error == 0) {
					ret = true;
				}
				else {
					ret = false;
				}
			}
			else {
				ret = false;
			}
		}
		else {
			ret = true;
		}
	}
	if(!ret) {
		Clear();
		ThrowError(ERR_CONNECT, "Can't connect " + Host + ":" + to_string(Port));
	}

	ul = 0;
#if defined(_WIN32)
	::ioctlsocket(Socket, static_cast<long>(FIONBIO), &ul);
#else
	::ioctl(Socket, FIONBIO, &ul);
#endif
}

uint32_t CMoonDbClient::IPToLong(const string& ip)
{
#if defined(_WIN32)
	return ::inet_addr(ip.c_str());
#else
	in_addr addr;
	::inet_aton(ip.c_str(), &addr);
	return addr.s_addr;
#endif
}

void CMoonDbClient::Send(const CPack& pack)
{
	size_t size = pack.GetSize();
	size_t pos = 0;
	while(size > pos) {
		int32_t bytes = static_cast<int32_t>(min(static_cast<size_t>(SendBufSize), size - pos));
		bytes = MoonSockSend(Socket, static_cast<const char*>(pack.GetPointer()) + pos, bytes);
		if(SOCKET_ERROR == bytes) {
			ThrowError(ERR_SEND, "Send Error: " + MoonLastError());
		}
		pos += static_cast<size_t>(bytes);
	}
}

ResponseType CMoonDbClient::Receive(CPack& pack)
{
	// 为了避免发送过大的数据被数据库服务拒绝，停止接收剩余数据，以致只能读取一次数据，所以这里多读些数据包含错误信息。
	// 注：如果服务器端接收完数据再返回错误信息则无此问题
	pack.Clear();
	pack.Reallocate(static_cast<size_t>(BytesPerRead));
	int32_t recv_len = MoonSockRecv(Socket, static_cast<char*>(pack.GetPointer()), BytesPerRead);
	if(SOCKET_ERROR == recv_len) {
		ThrowError(ERR_RECEIVE, "Receive Error: " + MoonLastError());
	}
	else if(recv_len < 11) {
		ThrowError(ERR_RECEIVE, "An error occor when recieving data (recv_len " + to_string(recv_len) + ").");
	}
	pack.SetSize(static_cast<size_t>(recv_len));
	int64_t msg_len = 0;
	pack.Get(msg_len);
	if(msg_len < 3) {
		ThrowError(ERR_RECEIVE, "An error occor when recieving data (msg_len).");
	}
	uint16_t rettype = 0;
	pack.Get(rettype);
	if(rettype <= RT_ERROR) {
		ThrowError(ERR_FROM_SERVER, "\"" + string(static_cast<char*>(pack.GetPointer()) + 10) + "\"");
	}
	pack.Reallocate(static_cast<size_t>(msg_len) + 8);
	pack.SetSize(static_cast<size_t>(msg_len) + 8);
	// 读取剩余数据
	int64_t read_len = recv_len - 8;
	int64_t left_len = msg_len - read_len;
	if(left_len < 0) {
		ThrowError(ERR_RECEIVE, "Extra data has been read.");
	}
	else if(left_len > 0) {
		while(true) {
			int32_t cur_len = static_cast<int32_t>(min(static_cast<int64_t>(BytesPerRead), msg_len - read_len));
			int32_t recv_len = MoonSockRecv(Socket, static_cast<char*>(pack.GetPointer()) + read_len + 8, cur_len);
			if(recv_len > 0) {
				read_len += recv_len;
				if(msg_len == read_len) {
					break;
				}
			}
			else if(SOCKET_ERROR == recv_len) {
				ThrowError(ERR_RECEIVE, "An error occor when recieving data (remain).");
			}
		}
	}
	return static_cast<ResponseType>(rettype);
}

string CMoonDbClient::Quote(const string& src_str)
{
	string des_str;
	size_t size = src_str.size();
	des_str.reserve(size + 16);
	const char* p = &src_str.front();
	size_t i = 0;
	while(i < size) {
		switch(*(p + i)) {
		case '\0':
			des_str.push_back('\\');
			des_str.push_back('0');
			break;
		case '\'':
			des_str.push_back('\\');
			des_str.push_back('\'');
			break;
		case '"':
			des_str.push_back('\\');
			des_str.push_back('"');
			break;
		case '\b':
			des_str.push_back('\\');
			des_str.push_back('b');
			break;
		case '\n':
			des_str.push_back('\\');
			des_str.push_back('n');
			break;
		case '\r':
			des_str.push_back('\\');
			des_str.push_back('r');
			break;
		case '\t':
			des_str.push_back('\\');
			des_str.push_back('t');
			break;
		case '\\':
			des_str.push_back('\\');
			des_str.push_back('\\');
			break;
		default:
			des_str.push_back(*(p + i));
		}
		i++;
	}
	return des_str;
}

void CMoonDbClient::PrepareData(CPack& pack, OperationType oper, const string& table, const map<string, CAny>& data)
{
	pack.Clear();
	pack.Put(static_cast<int64_t>(0));
	pack.Put(static_cast<uint8_t>(1));
	pack.Put(static_cast<uint16_t>(oper));
	pack.Put<uint16_t>(DatabaseName);
	pack.Put<uint16_t>(table);
	pack.Put(static_cast<uint16_t>(data.size()));
	for(auto it = data.begin(); it != data.end(); it++) {
		pack.Put<uint16_t>(it->first);
		pack.Put(it->second.GetType());
		it->second.Store(pack);
	}
	int64_t length = static_cast<int64_t>(pack.GetSize()) - 8;
	pack.Seek(0);
	pack.Put(length);
}

__uint128_t CMoonDbClient::IdNumResult(CPack& pack)
{
	uint16_t type;
	pack.Get(type);
	switch(type) {
	case FT_INT8:
	{
		int8_t val;
		pack.Get(val);
		return static_cast<__uint128_t>(val);
	}
	case FT_UINT8:
	{
		uint8_t val;
		pack.Get(val);
		return static_cast<__uint128_t>(val);
	}
	case FT_INT16:
	{
		int16_t val;
		pack.Get(val);
		return static_cast<__uint128_t>(val);
	}
	case FT_UINT16:
	{
		uint16_t val;
		pack.Get(val);
		return static_cast<__uint128_t>(val);
	}
	case FT_INT32:
	{
		int32_t val;
		pack.Get(val);
		return static_cast<__uint128_t>(val);
	}
	case FT_UINT32:
	{
		uint32_t val;
		pack.Get(val);
		return static_cast<__uint128_t>(val);
	}
	case FT_INT64:
	{
		int64_t val;
		pack.Get(val);
		return static_cast<__uint128_t>(val);
	}
	case FT_UINT64:
	{
		uint64_t val;
		pack.Get(val);
		return static_cast<__uint128_t>(val);
	}
	case FT_INT128:
	{
		__int128_t val;
		pack.Get(val);
		return static_cast<__uint128_t>(val);
	}
	case FT_UINT128:
	{
		__uint128_t val;
		pack.Get(val);
		return val;
	}
	default:
		return 0;
	}
}

__uint128_t CMoonDbClient::InsertData(const string& table, map<string, CAny>& data)
{
	PrepareData(Content, OPER_INSERT, table, data);
	Send(Content);
	ResponseType rettype = Receive(Content);
	if(RT_LAST_INSERT_ID == rettype) {
		return IdNumResult(Content);
	}
	return 0;
}

__uint128_t CMoonDbClient::UpdateData(const string& table, __uint128_t id, map<string, CAny>& data)
{
	data["rowid"] = id;
	PrepareData(Content, OPER_UPDATE, table, data);
	Send(Content);
	ResponseType rettype = Receive(Content);
	if(RT_AFFECTED_ROWS == rettype) {
		return IdNumResult(Content);
	}
	return 0;
}

__uint128_t CMoonDbClient::DeleteData(const string& table, __uint128_t id)
{
	map<string, CAny> data;
	data["rowid"] = id;
	PrepareData(Content, OPER_DELETE, table, data);
	Send(Content);
	ResponseType rettype = Receive(Content);
	if(RT_AFFECTED_ROWS == rettype) {
		return IdNumResult(Content);
	}
	return 0;
}

__uint128_t CMoonDbClient::ReplaceData(const string& table, __uint128_t id, map<string, CAny>& data)
{
	data["rowid"] = id;
	PrepareData(Content, OPER_REPLACE, table, data);
	Send(Content);
	ResponseType rettype = Receive(Content);
	if(RT_AFFECTED_ROWS == rettype) {
		return IdNumResult(Content);
	}
	return 0;
}

__uint128_t CMoonDbClient::GetData(const string& table, __uint128_t id, map<string, CAny>& data)
{
	data.clear();
	data["rowid"] = id;
	PrepareData(Content, OPER_SELECT, table, data);
	Send(Content);
	data.clear();
	ResponseType rettype = Receive(Content);
	if(RT_QUERY == rettype) {
		uint16_t count = 0;
		Content.Get(count);
		if(count > 0) {
			__uint128_t rid = IdNumResult(Content);
			if(rid != id) {
				ThrowError(ERR_DATA_INVALID, "Invaid data are retrived.");
			}
			uint16_t fieldnum = 0;
			Content.Get(fieldnum);
			for(uint16_t i = 0; i < fieldnum; i++) {
				string fieldname;
				Content.Get<uint16_t>(fieldname);
				CAny val;
				val.Load(Content);
				data[fieldname] = std::move(val);
			}
		}
		return count;
	}
	return 0;
}

std::ostream & operator << (std::ostream & os, const map<string, CAny>& data)
{
	for(auto it = data.begin(); it != data.end(); it++) {
		os << it->first << ": " << it->second << endl;
	}
	return os;
}

}
