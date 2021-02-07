#include "cmoondb.h"
#include "cparsexml.hpp"

namespace MoonDb {

CMoonDb::CMoonDb()
{
	BytesPerRead = 8192;

	Started = false;
	Stopped = false;
	Restart = false;
	AsyncThreadNum = 0;
	SynchThreadNum = 0;
	GroupConnectionNum = 0;

	GroupConnectionNumPerThread = nullptr;

	LoadAllSchemasOnLoading = false;

#if defined(_WIN32)
	WSADATA ws;
	if (::WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
		TriggerError("Init Windows Socket Failed:" + MoonLastError());
	}
#endif
}

CMoonDb::~CMoonDb()
{
	Clear();
#if defined(_WIN32)
	::WSACleanup();
#endif
	DeletedDbMutexes.clear();
	for(auto it = DatabaseMutexes.begin(); it != DatabaseMutexes.end(); it++) {
		delete *it;
	}
	DatabaseMutexes.clear();
}

uint32_t CMoonDb::IPToLong(const string& ip) const noexcept
{
#if defined(_WIN32)
	return ::inet_addr(ip.c_str());
#else
	in_addr addr;
	::inet_aton(ip.c_str(), &addr);
	return addr.s_addr;
#endif
}

void CMoonDb::LoadConfiguration(const string& programdir, bool showinfo, const string& configfile)
{
	ProgramDirectory = programdir;
	ShowInfo = showinfo;
	if(configfile.substr(0, 2) == "./" || configfile.substr(0, 2) == ".\\") {
		ConfigFile = ProgramDirectory + DIRECTORY_SEPARATOR + configfile.substr(2);
	}
	else if(configfile[0] != '/' && configfile.find(":") == string::npos) {
		ConfigFile = ProgramDirectory + DIRECTORY_SEPARATOR + configfile;
	}
	else {
		ConfigFile = configfile;
	}
	CParseXML::CNode node;
	CParseXML::Read(ConfigFile.c_str(), node);
	auto& params = node["MoonDb"].children;

	if(params.find("InternetFamily") != params.end()) {
		string infa = to_lower_copy(params["InternetFamily"].content);
		if("ipv4" == infa) {
			InternetFamily = IF_IPv4;
		}
		else if("ipv6" == infa) {
			InternetFamily = IF_IPv6;
		}
		else {
			TriggerError("Invalid InternetFamily:" + params["InternetFamily"].content);
		}
	}
	else {
		InternetFamily = IF_IPv4;
	}

	if(params.find("DataServerIP") != params.end()) {
		string serverip = params["DataServerIP"].content;
		if(serverip.empty()) {
			TriggerError("Invalid DataServerIP:" + serverip);
		}

		if(IF_IPv4 == InternetFamily) {
			DataServerIPv4 = IPToLong(serverip);
		}
		else {
			inet_pton(AF_INET6, serverip.c_str(), &DataServerIPv6);
		}
	}
	else {
		if(IF_IPv4 == InternetFamily) {
			DataServerIPv4 = INADDR_ANY;
		}
		else {
			DataServerIPv6 = in6addr_any;
		}
	}

	if(params.find("DataServerPort") != params.end()) {
		string content = params["DataServerPort"].content;
		if(!is_digit(content)) {
			TriggerError("Wrong DataServerPort:" + content);
		}
		DataServerPort = static_cast<uint16_t>(::stoul(content));
	}
	else {
		DataServerPort = 9999;
	}

	if(params.find("DataClientIP") != params.end()) {
		string clientip = params["DataClientIP"].content;
		if(clientip.empty()) {
			TriggerError("Invalid DataClientIP:" + clientip);
		}
		if(IF_IPv4 == InternetFamily) {
			DataClientIPv4 = IPToLong(clientip);
		}
		else {
			inet_pton(AF_INET6, clientip.c_str(), &DataClientIPv6);
		}
	}
	else {
		if(IF_IPv4 == InternetFamily) {
			DataClientIPv4 = INADDR_ANY;
		}
		else {
			DataClientIPv6 = in6addr_any;
		}
	}

	if(params.find("DataDirectory") != params.end()) {
		DataDirectory = params["DataDirectory"].content;
		to_current_os_path(DataDirectory);
		// 如果是程序所在目录转换成绝对地址
		if(DataDirectory.substr(0, 2) == string(".") + DIRECTORY_SEPARATOR) {
			DataDirectory = ProgramDirectory + DIRECTORY_SEPARATOR + DataDirectory.substr(2);
		}
		if(DataDirectory[DataDirectory.size() - 1] == DIRECTORY_SEPARATOR) {
			string trimchar(1, DIRECTORY_SEPARATOR);
			rtrim(DataDirectory, trimchar);
		}
		if(DataDirectory.empty() || !CFileSystem::IsDirectory(DataDirectory)) {
			TriggerError("Invalid DataDirectory:" + DataDirectory);
		}
	}
	else {
		DataDirectory = ProgramDirectory + DIRECTORY_SEPARATOR + "data";
	}

	if(params.find("SQLiteDirectory") != params.end()) {
		SQLiteDirectory = params["SQLiteDirectory"].content;
		to_current_os_path(SQLiteDirectory);
		// 如果是程序所在目录转换成绝对地址
		if(SQLiteDirectory.substr(0, 2) == string(".") + DIRECTORY_SEPARATOR) {
			SQLiteDirectory = ProgramDirectory + DIRECTORY_SEPARATOR + SQLiteDirectory.substr(2);
		}
		if(SQLiteDirectory[SQLiteDirectory.size() - 1] == DIRECTORY_SEPARATOR) {
			string trimchar(1, DIRECTORY_SEPARATOR);
			rtrim(SQLiteDirectory, trimchar);
		}
		if(SQLiteDirectory.empty() || !CFileSystem::IsDirectory(SQLiteDirectory)) {
			TriggerError("Invalid SQLiteDirectory:" + SQLiteDirectory);
		}
	}
	else {
		SQLiteDirectory = ProgramDirectory + DIRECTORY_SEPARATOR + "sqlite";
	}

	if(params.find("MaxThreads") != params.end()) {
		string content = params["MaxThreads"].content;
		if(!is_digit(content)) {
			TriggerError("Wrong MaxThreads:" + content);
		}
		MaxThreads = ::stoul(content);
		if(0 == MaxThreads) {
			TriggerError("Wrong MaxThreads:" + content);
		}
	}
	else {
		MaxThreads = max(static_cast<uint32_t>(4), thread::hardware_concurrency()) - 1;
	}

	if(params.find("BackLog") != params.end()) {
		string content = params["BackLog"].content;
		if(!is_digit(content)) {
			TriggerError("Wrong BackLog:" + content);
		}
		BackLog = stoul(content);
		if(BackLog <= 0) {
			TriggerError("Wrong BackLog:" + content);
		}
	}
	else {
		BackLog = MaxThreads * 25;
	}
	BackLog = min(BackLog, static_cast<uint32_t>(SOMAXCONN));

	if(params.find("MaxConnections") != params.end()) {
		string content = params["MaxConnections"].content;
		if(!is_digit(content)) {
			TriggerError("Wrong MaxConnections:" + content);
		}
		MaxConnections = stoul(content);
		if(0 == MaxConnections) {
			TriggerError("Wrong MaxConnections:" + content);
		}
	}
	else {
		MaxConnections = MaxThreads * 50;
	}

	if(params.find("MaxAllowedPacket") != params.end()) {
		string content = params["MaxAllowedPacket"].content;
//		if(content.empty() || !regex_match(content, regex("^[1-9]+\\d*(K|M)?$", regex_constants::icase))) {
		if(!is_capacity(content)) {
			TriggerError("Wrong MaxAllowedPacket:" + content);
		}
		char unit = content[content.size() - 1];
		if(::isdigit(unit)) {
			MaxAllowedPacket = stoll(content);
		}
		else {
			MaxAllowedPacket = stoll(content.substr(0, content.size() - 1));
			unit = static_cast<char>(::toupper(unit));
			switch(unit) {
			case 'K':
				MaxAllowedPacket *= 1024;
				break;
			case 'M':
				MaxAllowedPacket *= 1048576;
				break;
			}
			if(MaxAllowedPacket <= 0) {
				TriggerError("Wrong MaxAllowedPacket:" + content);
			}
		}
	}
	else {
		MaxAllowedPacket = 16777216;//16M
	}

	if(params.find("SendBufSize") != params.end()) {
		string content = params["SendBufSize"].content;
//		if(content.empty() || !regex_match(content, regex("^[1-9]+\\d*(K|M)?$", regex_constants::icase))) {
		if(!is_capacity(content)) {
			TriggerError("Wrong SendBufSize:" + content);
		}
		char unit = content[content.size() - 1];
		if(::isdigit(unit)) {
			SendBufSize = stol(content);
		}
		else {
			SendBufSize = stol(content.substr(0, content.size() - 1));
			unit = static_cast<char>(::toupper(unit));
			switch(unit) {
			case 'K':
				SendBufSize *= 1024;
				break;
			case 'M':
				SendBufSize *= 1048576;
				break;
			}
			if(SendBufSize <= 0) {
				TriggerError("Wrong SendBufSize:" + content);
			}
		}
	}
	else {
		SendBufSize = 0;
	}

	if(params.find("ReceiveBufSize") != params.end()) {
		string content = params["ReceiveBufSize"].content;
//		if(content.empty() || !regex_match(content, regex("^[1-9]+\\d*(K|M)?$", regex_constants::icase))) {
		if(!is_capacity(content)) {
			TriggerError("Wrong ReceiveBufSize:" + content);
		}
		char unit = content[content.size() - 1];
		if(::isdigit(unit)) {
			ReceiveBufSize = stol(content);
		}
		else {
			ReceiveBufSize = stol(content.substr(0, content.size() - 1));
			unit = static_cast<char>(::toupper(unit));
			switch(unit) {
			case 'K':
				ReceiveBufSize *= 1024;
				break;
			case 'M':
				ReceiveBufSize *= 1048576;
				break;
			}
			if(ReceiveBufSize <= 0) {
				TriggerError("Wrong ReceiveBufSize:" + content);
			}
		}
	}
	else {
		ReceiveBufSize = 0;
	}

	if(params.find("WaitTimeout") != params.end()) {
		string content = params["WaitTimeout"].content;
		if(!is_digit(content)) {
			TriggerError("Wrong WaitTimeout:" + content);
		}
		WaitTimeout = stoull(content);
		if(0 == WaitTimeout || WaitTimeout >= 2147483647000) {
			TriggerError("Wrong WaitTimeout (1-2147483647000):" + content);
		}
	}
	else {
		WaitTimeout = 28800000000;//28800秒
	}
	NanoWaitTimeout = static_cast<int64_t>(WaitTimeout * 1000);

	if(params.find("RecvTimeout") != params.end()) {
		string content = params["RecvTimeout"].content;
		if(!is_digit(content)) {
			TriggerError("Wrong RecvTimeout:" + content);
		}
		RecvTimeout = stoull(content);
		if(0 == RecvTimeout || RecvTimeout >= 2147483647000) {
			TriggerError("Wrong RecvTimeout (1-2147483647000):" + content);
		}
	}
	else {
		RecvTimeout = 5000000;//5秒
	}
	AsyncRecvTimeout = static_cast<int64_t>(RecvTimeout * 1000);

	if(params.find("SendTimeout") != params.end()) {
		string content = params["SendTimeout"].content;
		if(!is_digit(content)) {
			TriggerError("Wrong SendTimeout:" + content);
		}
		SendTimeout = stoull(content);
		if(0 == SendTimeout || SendTimeout >= 2147483647000) {
			TriggerError("Wrong SendTimeout (1-2147483647000):" + content);
		}
	}
	else {
		SendTimeout = 5000000;//5秒
	}
	AsyncSendTimeout = static_cast<int64_t>(SendTimeout * 1000);

	if(params.find("SelectTimeout") != params.end()) {
		string content = params["SelectTimeout"].content;
		if(!is_digit(content)) {
			TriggerError("Wrong SelectTimeout:" + content);
		}
		SelectTimeout = stoull(content);
		if(0 == SelectTimeout || SelectTimeout > 9223372034707292159LL) {
			TriggerError("Wrong SelectTimeout (1-9223372034707292159):" + content);
		}
	}
	else {
		SelectTimeout = 1000000;
	}

	if(params.find("Async") != params.end()) {
		string content = params["Async"].content;
		if(!is_digit(content)) {
			TriggerError("Wrong Async:" + content);
		}
		Async = stoul(content);
		if(Async > 2) {
			TriggerError("Wrong Async:" + content);
		}
	}
	else {
#if defined(_WIN32)
		Async = MaxThreads > 1 ? 2 : 1;
#else
		Async = 1;
#endif
	}

	if(params.find("LoadAllSchemasOnLoading") != params.end()) {
		string content = to_lower_copy(params["LoadAllSchemasOnLoading"].content);
		if("1" == content || "true" == content) {
			LoadAllSchemasOnLoading = true;
		}
		else if("0" == content || "false" == content){
			LoadAllSchemasOnLoading = false;
		}
		else {
			TriggerError("Wrong LoadAllSchemasOnLoading:" + content);
		}
	}

	//cout << DataDirectory << "," << Port << "," << MaxThreads << "," << BackLog << "," << MaxConnections << "," << MaxAllowedPacket << endl;
}

uint32_t CMoonDb::SynchGetNewThreadNum()
{
	if(MaxThreads > 1) {
		ThreadMutex.lock();
	}
	SynchThreadNum ++;
	uint32_t num = SynchIdleThreads.front();
	SynchIdleThreads.pop();
	if(MaxThreads > 1) {
		ThreadMutex.unlock();
	}
	return num;
}

void CMoonDb::SynchDeleteThread(uint32_t threadid)
{
	if(MaxThreads > 1) {
		ThreadMutex.lock();
	}
	SynchIdleThreads.push(threadid);
	if(SynchThreadNum > 0) {
		SynchThreadNum --;
	}
	if(MaxThreads > 1) {
		ThreadMutex.unlock();
	}
}

SOCKET CMoonDb::_CreateSocket(bool asyn)
{
#if defined(_WIN32)
	SOCKET socksvr = ::socket((IF_IPv4 == InternetFamily ? AF_INET : AF_INET6), SOCK_STREAM, IPPROTO_TCP);
	if (socksvr == INVALID_SOCKET) {
		Clear();
		TriggerError("Create Socket Failed:" + MoonLastError());
	}
	if(asyn) {
		u_long ul = 1;
		if(SOCKET_ERROR == ::ioctlsocket(socksvr, static_cast<long>(FIONBIO), &ul)) {
			Clear();
			TriggerError("Nonblock setting Error:" + MoonLastError());
		}
	}
#else
	SOCKET socksvr = ::socket((IF_IPv4 == InternetFamily ? AF_INET : AF_INET6), (asyn ? SOCK_STREAM | SOCK_NONBLOCK : SOCK_STREAM), IPPROTO_TCP);
	if (socksvr < 0) {
		Clear();
		TriggerError("Create Socket Failed:" + MoonLastError());
	}
#endif

	int on = 1;
	if(setsockopt(socksvr, SOL_SOCKET, SO_REUSEADDR, static_cast<char*>(static_cast<void*>(&on)), sizeof(on))) {
		CLog::Instance()->Put(CLog::L_WARNING, "setsockopt() failed (SO_REUSEADDR): " + MoonLastError());
	}

	return socksvr;
}

SOCKET CMoonDb::CreateSocketv4(uint32_t ip, uint16_t port, bool asyn)
{
	SOCKET socksvr = _CreateSocket(asyn);

	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = ip;
	serveraddr.sin_port = htons(port);
	memset(serveraddr.sin_zero, 0x00, 8);

	int ires;
	auto starttime = CTime::Now();
	do {
		ires = ::bind(socksvr, static_cast<sockaddr*>(static_cast<void*>(&serveraddr)), sizeof(serveraddr));
		if(SOCKET_ERROR != ires) {
			break;
		}
	} while(starttime + 5000000000 < CTime::Now());
	if(SOCKET_ERROR == ires) {
		Clear();
		TriggerError("Bind Error:" + MoonLastError());
	}

	ListenAndSetting(socksvr);

	return socksvr;
}

SOCKET CMoonDb::CreateSocketv6(in6_addr ip, uint16_t port, bool asyn)
{
	SOCKET socksvr = _CreateSocket(asyn);

//	char buf[1024];
//	::inet_ntop(AF_INET6, &ip, buf, 1024);
//	cout << buf << endl;

	sockaddr_in6 serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin6_family = AF_INET6;
	serveraddr.sin6_addr = ip;
	serveraddr.sin6_port = htons(port);

	int ires;
	auto starttime = CTime::Now();
	do {
		ires = ::bind(socksvr, static_cast<sockaddr*>(static_cast<void*>(&serveraddr)), sizeof(sockaddr_in6));
		if(SOCKET_ERROR != ires) {
			break;
		}
	} while(starttime + 5000000000 < CTime::Now());
	if(SOCKET_ERROR == ires) {
		Clear();
		TriggerError("Bind Error:" + MoonLastError());
	}

	ListenAndSetting(socksvr);

	return socksvr;
}

void CMoonDb::ListenAndSetting(SOCKET socksvr)
{
	int on = 1;
	if(setsockopt(socksvr, IPPROTO_TCP, TCP_NODELAY, static_cast<char*>(static_cast<void*>(&on)), sizeof(on))) {
		CLog::Instance()->Put(CLog::L_WARNING, "setsockopt() failed (TCP_NODELAY): " + MoonLastError());
	}

	// 对于同步情况设置超时时间，异步情况不好使
	if(!Async) {
#if defined(_WIN32)
		//设置发送超时
		int nNetTimeout = static_cast<int>(::ceil(SendTimeout / 1000.0));
		if(::setsockopt(socksvr, SOL_SOCKET, SO_SNDTIMEO, static_cast<const char *>(static_cast<const void*>(&nNetTimeout)), sizeof(int)) && ShowInfo) {
			cout << "setsockopt() failed (SO_SNDTIMEO): " << MoonLastError() << endl;
		}
		//设置接收超时
		nNetTimeout = static_cast<int>(::ceil(RecvTimeout / 1000.0));
		if(::setsockopt(socksvr, SOL_SOCKET, SO_RCVTIMEO, static_cast<const char *>(static_cast<const void*>(&nNetTimeout)), sizeof(int)) && ShowInfo) {
			cout << "setsockopt() failed (SO_RCVTIMEO): " << MoonLastError() << endl;
		}
#else
		//设置发送超时
		struct timeval timeout = {0, 0};
		timeout.tv_sec = SendTimeout / 1000000;
		timeout.tv_usec = SendTimeout % 1000000;
		//struct timeval timeout = {SendTimeout / 1000000, SendTimeout % 1000000};
		if(::setsockopt(socksvr, SOL_SOCKET, SO_SNDTIMEO, static_cast<const char *>(static_cast<const void*>(&timeout)), sizeof(struct timeval)) && ShowInfo) {
			cout << "setsockopt() failed (SO_SNDTIMEO): " << MoonLastError() << endl;
		}
		//设置接收超时
		timeout.tv_sec = RecvTimeout / 1000000;
		timeout.tv_usec = RecvTimeout % 1000000;
		//struct timeval timeout = {RecvTimeout / 1000000, RecvTimeout % 1000000};
		if(::setsockopt(socksvr, SOL_SOCKET, SO_RCVTIMEO, static_cast<const char *>(static_cast<const void*>(&timeout)), sizeof(struct timeval)) && ShowInfo) {
			cout << "setsockopt() failed (SO_RCVTIMEO): " << MoonLastError() << endl;
		}
#endif
	}

#if defined(_WIN32)
	int32_t optLen = sizeof(int32_t);
#else
	socklen_t optLen = sizeof(int32_t);
#endif
	int sendbufsize = 0;
	int redvbufsize = 0;
	::getsockopt(socksvr, SOL_SOCKET, SO_SNDBUF, static_cast<char*>(static_cast<void*>(&sendbufsize)), &optLen);
	if(sendbufsize < SendBufSize) {
		if(0 != ::setsockopt(socksvr, SOL_SOCKET, SO_SNDBUF, static_cast<const char*>(static_cast<const void*>(&SendBufSize)), optLen)) {
			SendBufSize = sendbufsize;
		}
	}
	else {
		SendBufSize = sendbufsize;
	}
	::getsockopt(socksvr, SOL_SOCKET, SO_RCVBUF, static_cast<char*>(static_cast<void*>(&redvbufsize)), &optLen);
	if(redvbufsize < ReceiveBufSize) {
		if(0 != ::setsockopt(socksvr, SOL_SOCKET, SO_RCVBUF, static_cast<const char*>(static_cast<const void*>(&ReceiveBufSize)), optLen)) {
			ReceiveBufSize = redvbufsize;
		}
	}
	else {
		ReceiveBufSize = redvbufsize;
	}
	if(ShowInfo) {
		printf("Server socket, send buf is %d(old is %d) and recv buf is %d(old is %d)\n", SendBufSize, sendbufsize, ReceiveBufSize, redvbufsize);
	}

	//int32_t optLen = 4;
	//SendBufSize = 128 * 1024;
	//::setsockopt(socksvr, SOL_SOCKET, SO_SNDBUF, static_cast<const char*>(static_cast<const void*>(&SendBufSize)), optLen);

	int ires;
	auto starttime = CTime::Now();
	do {
		ires = ::listen(socksvr, static_cast<int32_t>(BackLog));
		if(SOCKET_ERROR != ires) {
			break;
		}
	} while(starttime + 5000000000 < CTime::Now());
	if(SOCKET_ERROR == ires) {
		Clear();
		TriggerError("Listen Error:" + MoonLastError());
	}
}

void CMoonDb::Start()
{
	if(IF_IPv4 == InternetFamily) {
		DataSeverSocket = CreateSocketv4(DataServerIPv4, DataServerPort, Async);
	}
	else {
		DataSeverSocket = CreateSocketv6(DataServerIPv6, DataServerPort, Async);
	}

	if(Async) {
		if(1 == Async) {
			AsyncConnections.initialize(MaxConnections, MaxConnections);
		}
		else if(2 == Async) {
			GroupNewConnections.resize(MaxThreads);
			GroupConnectionNumPerThread = new atomic<uint32_t>[MaxThreads];
			for(uint32_t i = 0; i < MaxThreads; i++) {
				GroupConnectionNumPerThread[i] = 0;
			}
		}
	}
	else {
		SynchBuffers.resize(MaxThreads);
		for(uint32_t i = 0; i < MaxThreads; i++) {
			SynchBuffers[i].Reallocate(static_cast<size_t>(max(ReceiveBufSize, SendBufSize)));
			SynchIdleThreads.push(i);
		}
	}

	if(LoadAllSchemasOnLoading) {
		LoadSchemas();
	}

	Started = true;
	Stopped = false;
}

void CMoonDb::Clear() noexcept
{
	MoonSockClose(DataSeverSocket);
//	MoonSockClose(ManagementSeverSocket);
	if(Databases.size() > 0) {
		for(auto it = Databases.begin(); it != Databases.end(); it++)	{
			delete it->second->GetMutex();
			delete it->second;
		}
		Databases.clear();
	}
	if(Async) {
		AsyncThreadNum = 0;
		if(1 == Async) {
			AsyncConnections.clear();
			while(!AsyncNewConnections.empty()) {
				AsyncNewConnections.pop();
			}
		}
		else if(2 == Async) {
			GroupConnectionNum = 0;
			delete [] GroupConnectionNumPerThread;
			GroupNewConnections.clear();
//			while(!GroupNewConnections.empty()) {
//				GroupNewConnections.pop();
//			}
		}
	}
	else {
		SynchThreadNum = 0;
		SynchBuffers.clear();
		while(!SynchIdleThreads.empty()) {
			SynchIdleThreads.pop();
		}
	}
	DeletedDbMutexes.clear();
	for(auto it = DatabaseMutexes.begin(); it != DatabaseMutexes.end(); it++) {
		DeletedDbMutexes.push_back(*it);
	}
}

void CMoonDb::Stop() noexcept
{
	Started = false;
}

bool CMoonDb::IsStopped() const noexcept
{
	return Stopped;
}

void CMoonDb::Run()
{
	switch(Async) {
	case 0:
		SynchRun();
		break;
	case 1:
		AsyncRun();
		break;
	case 2:
		GroupRun();
		break;
	default:
		exit(1);
	}
	Stopped = true;
	if(Restart) {
		Restart = false;
		LoadConfiguration(ProgramDirectory, ShowInfo, ConfigFile);
		Start();
		Run();
	}
}

void CMoonDb::AsyncRun()
{
	if(MaxThreads > 1) {
		for(uint32_t i = 0; i < MaxThreads; i++) {
			thread tn([this]() {
				this->AsyncQuery();
			});
			tn.detach();
		}
		msleep(10);
	}

	unique_lock<mutex> lck(ThreadMutex, defer_lock);
	while (true) {
		if(!Started) {
			break;
		}
		AsyncAccept();
		// 循环遍历
		for(auto it = AsyncConnections.begin(); it != AsyncConnections.end();) {
			if(!Started) {
				break;
			}
			CConnection* conn = AsyncConnections.at(it);
			if(SESS_DISCONNECTED == conn->Status) {
				auto del_it = it;
				it++;
				AsyncConnections.erase(del_it);
				continue;
			}
			if((SESS_CONNECTED == conn->Status || SESS_SENT == conn->Status) && conn->Time + NanoWaitTimeout < CTime::Now()) {
				MoonSockClose(conn->Socket);
				auto del_it = it;
				it++;
				AsyncConnections.erase(del_it);
				continue;
			}
			if(SESS_CONNECTED == conn->Status || SESS_SENT == conn->Status || SESS_RECEIVING == conn->Status) {
				AsyncReceive(conn);
			}
			else if(SESS_PROCESSED == conn->Status || SESS_SENDING == conn->Status) {
				AsyncSend(conn);
			}
			if(AsyncThreadNum < MaxThreads && SESS_RECEIVED == conn->Status) {
				conn->Status = SESS_PROCESSING;
				if(1 == MaxThreads) {
					_AsyncQuery(conn);
				}
				else {
					AsyncThreadNum++;
					lck.lock();
					AsyncNewConnections.push(conn);
					AsyncCondVar.notify_one();
					lck.unlock();
				}
			}
			it++;
		}
	}

	if(!Started) {
		while(AsyncThreadNum > 0) {
			msleep(1);
		}
		// 停止前将数据发送出去
		while(AsyncConnections.size() > 0) {
			for(auto it = AsyncConnections.begin(); it != AsyncConnections.end();) {
				CConnection* conn = AsyncConnections.at(it);
				if(SESS_PROCESSED == conn->Status || SESS_SENDING == conn->Status) {
					AsyncSend(conn);
					it++;
				}
				else {
					auto del_it = it;
					it++;
					AsyncConnections.erase(del_it);
				}
			}
		}
		Clear();
	}
}

void CMoonDb::GroupDistributeConnection(const vector<SOCKET>& clientsocks, vector<pair<uint32_t, uint32_t>>& connnumperthread)
{
	uint32_t isock = 0;
	while(true) {
		for(uint32_t i = 1; i < MaxThreads; i++) {
			uint32_t newconnnum = connnumperthread[i].first - connnumperthread[i - 1].first + 1;
			for(uint32_t j = 0; j < newconnnum; j++) {
				GroupNewConnections[connnumperthread[i - 1].second].push_back(clientsocks[isock]);
				GroupConnectionNumPerThread[connnumperthread[i - 1].second]++;
				isock++;
				if(isock >= clientsocks.size()) {
					return;
				}
			}
		}
		GroupNewConnections[MaxThreads - 1].push_back(clientsocks[isock]);
		GroupConnectionNumPerThread[MaxThreads - 1]++;
		isock++;
		if(isock >= clientsocks.size()) {
			return;
		}
	}
}

void CMoonDb::GroupRun()
{
	for(uint32_t i = 0; i < MaxThreads; i++) {
		thread tn([this, i]() {
			this->GroupQuery(i);
		});
		tn.detach();
	}
	msleep(10);

	fd_set fdread;
	timeval tv;
	tv.tv_sec = static_cast<int32_t>(SelectTimeout / 1000000);
	tv.tv_usec = static_cast<int32_t>(SelectTimeout % 1000000);
	int maxfd = static_cast<int>(DataSeverSocket + 1);
	bool wrongip;
	vector<SOCKET> clientsocks;
	clientsocks.reserve(BackLog);
	unique_lock<mutex> lck(ThreadMutex, defer_lock);
	// pair第一项为连接数，第二项为线程编号
	vector<pair<uint32_t, uint32_t>> connnumperthread;
	connnumperthread.reserve(MaxThreads);
	while(true) {
		if(!Started) {
			break;
		}
		FD_ZERO(&fdread);//初始化fd_set
		FD_SET(DataSeverSocket, &fdread);//分配套接字句柄到相应的fd_set
		if(::select(maxfd, &fdread, nullptr, nullptr, &tv) > 0) {
			if(FD_ISSET(DataSeverSocket, &fdread)) {
				for(uint32_t i = 0; i < BackLog && GroupConnectionNum < MaxConnections; i++) {
					SOCKET sock_client = Accept(wrongip);
					if(sock_client == INVALID_SOCKET) {
						if(wrongip) {
							continue;
						}
						else {
							break;
						}
					}
					GroupConnectionNum++;
					clientsocks.push_back(sock_client);
				}
				if(!clientsocks.empty()) {
					for(uint32_t i = 0; i < MaxThreads; i++) {
						uint32_t iconn = GroupConnectionNumPerThread[i];
						connnumperthread.push_back(make_pair(iconn, i));
					}
					sort(connnumperthread.begin(), connnumperthread.end(), [](pair<uint32_t, uint32_t> i, pair<uint32_t, uint32_t> j) {
						return i.first < j.first;
					});
					lck.lock();
					GroupDistributeConnection(clientsocks, connnumperthread);
					AsyncCondVar.notify_all();
					lck.unlock();
					clientsocks.clear();
					connnumperthread.clear();
				}
			}
		}
	}

	if(!Started) {
		lck.lock();
		AsyncCondVar.notify_all();
		lck.unlock();
		while(AsyncThreadNum > 0) {
			msleep(1);
		}
		Clear();
	}
}

void CMoonDb::GroupQuery(uint32_t threadid)
{
	AsyncThreadNum++;
	unique_lock<mutex> lck(ThreadMutex, defer_lock);
	CQueue<CConnection> connections(MaxConnections, static_cast<uint32_t>(ceil(static_cast<double>(MaxConnections) / MaxThreads) * 1.2));
	vector<SOCKET> newconns;
	atomic<uint32_t>& curconnum = GroupConnectionNumPerThread[threadid];
	while(true) {
		if(!Started) {
			break;
		}
		lck.lock();
		while(GroupNewConnections[threadid].empty()) {
			if(!Started) {
				break;
			}
			AsyncCondVar.wait(lck);
		}
		if(!Started) {
			lck.unlock();
			break;
		}
		newconns = GroupNewConnections[threadid];
		GroupNewConnections[threadid].clear();
		lck.unlock();

		for(uint32_t i = 0; i < newconns.size(); i++) {
			CConnection* conn = connections.push();
			if(conn->Buffer.GetSize() == 0) {
				conn->Buffer.Allocate(static_cast<size_t>(max(ReceiveBufSize, SendBufSize)));
			}
			conn->Initialize(newconns[i]);
		}
		newconns.clear();

		while(!connections.empty()) {
			for(auto it = connections.begin(); it != connections.end();) {
				if(!Started) {
					break;
				}
				CConnection* conn = connections.at(it);
				if(SESS_DISCONNECTED == conn->Status) {
					auto del_it = it;
					it++;
					connections.erase(del_it);
					GroupConnectionNum--;
					curconnum--;
					continue;
				}
				if((SESS_CONNECTED == conn->Status || SESS_SENT == conn->Status) && conn->Time + NanoWaitTimeout < CTime::Now()) {
					MoonSockClose(conn->Socket);
					auto del_it = it;
					it++;
					connections.erase(del_it);
					GroupConnectionNum--;
					curconnum--;
					continue;
				}
				if(SESS_CONNECTED == conn->Status || SESS_SENT == conn->Status || SESS_RECEIVING == conn->Status) {
					AsyncReceive(conn);
				}
				if(SESS_RECEIVED == conn->Status) {
					conn->Status = SESS_PROCESSING;
					_AsyncQuery(conn);
				}
				if(SESS_PROCESSED == conn->Status || SESS_SENDING == conn->Status) {
					AsyncSend(conn);
				}
				it++;
			}

			if(!Started) {
				break;
			}
			if(lck.try_lock()) {
				//cout << threadid << "," << connections.size() << endl;
				if(GroupNewConnections[threadid].empty()) {
					lck.unlock();
				}
				else {
					newconns = GroupNewConnections[threadid];
					GroupNewConnections[threadid].clear();
					lck.unlock();

					for(uint32_t i = 0; i < newconns.size(); i++) {
						CConnection* conn = connections.push();
						if(conn->Buffer.GetSize() == 0) {
							conn->Buffer.Allocate(static_cast<size_t>(max(ReceiveBufSize, SendBufSize)));
						}
						conn->Initialize(newconns[i]);
					}
					newconns.clear();
				}
			}
		}
	}

	while(connections.size() > 0) {
		for(auto it = connections.begin(); it != connections.end();) {
			CConnection* conn = connections.at(it);
			if(SESS_PROCESSED == conn->Status || SESS_SENDING == conn->Status) {
				AsyncSend(conn);
				it++;
			}
			else {
				auto del_it = it;
				it++;
				connections.erase(del_it);
				GroupConnectionNum--;
				curconnum--;
				continue;
			}
		}
	}

	AsyncThreadNum--;
}

void CMoonDb::SynchRun()
{
	//循环接收数据
	fd_set fdread;
//	int maxfd = static_cast<int>(max(DataSeverSocket, ManagementSeverSocket) + 1);
	int maxfd = static_cast<int>(DataSeverSocket + 1);
	timeval tv;
	tv.tv_sec = static_cast<int32_t>(SelectTimeout / 1000000);
	tv.tv_usec = static_cast<int32_t>(SelectTimeout % 1000000);
	while (true) {
		if(!Started) {
			break;
		}
		if(SynchThreadNum >= MaxThreads) {
			continue;
		}
		FD_ZERO(&fdread);//初始化fd_set
		FD_SET(DataSeverSocket, &fdread);//分配套接字句柄到相应的fd_set
//		FD_SET(ManagementSeverSocket, &fdread);
		if (::select(maxfd, &fdread, nullptr, nullptr, &tv) > 0) {
			//cout << ThreadNum << ",";
			if(FD_ISSET(DataSeverSocket, &fdread)) {
				uint32_t tn = SynchGetNewThreadNum();
				if(1 == MaxThreads) {
					SynchAcceptAndQuery(tn);
				}
				else {
					try {
						thread th([this, tn]() {
							this->SynchAcceptAndQuery(tn);
						});
						th.detach();
					}
					catch(exception& e) {
						string error = string("The query thread error: ") + e.what();
						if(ShowInfo) {
							CLog::Instance()->Put(CLog::L_WARNING, error);
						}
						else {
							cout << error << endl;
						}
						SynchDeleteThread(tn);
					}
				}
			}
		}
	}

	if(!Started) {
		while(SynchThreadNum > 0) {
			msleep(1);
		}
		Clear();
	}
}

void CMoonDb::SynchAcceptAndQuery(uint32_t threadid)
{
	bool wrongip;
	SOCKET sock_client = Accept(wrongip);
	if(INVALID_SOCKET == sock_client) {
		SynchDeleteThread(threadid);
		return;
	}

	/*__int128_t msg_len = 0;
	cout << MoonSockRecv(sock_client, static_cast<char*>(static_cast<void*>(&msg_len)), 16) << endl;
	cout << msg_len << endl;
	MoonSockSend(sock_client, static_cast<const char*>(static_cast<const void*>(&msg_len)), 16);*/

	CPack* buf = &SynchBuffers[threadid];
	chrono::high_resolution_clock::rep time = CTime::Now();
	try {
		while(true) {
			int64_t bytes = SynchReceive(sock_client, *buf);
//			if(ShowInfo) {
//				cout << "Received bytes: " << bytes << endl;
//			}
			if(bytes <= 0) {
				if(SOCKET_TIMEOUT == MoonLastErrno() && time + NanoWaitTimeout >= CTime::Now()) {
					continue;
				}
				break;
			}

			uint8_t apitype = 0;
			buf->Get(apitype);
			if(1 == apitype) {
				NoSQLQuery(*buf);
				SynchSend(sock_client, *buf);
			}
			else if(2 == apitype) {
				SQLQuery(*buf);
				SynchSend(sock_client, *buf);
			}
			else {
				ThrowError(ERR_WRONG_API_TYPE, "Wrong API type: " + num_to_string(apitype));
				break;
			}

			time = CTime::Now();
		}
	}
	catch(runtime_error& e) {
		SynchSendError(sock_client, *buf, e.what());
	}
	MoonSockClose(sock_client);
	SynchDeleteThread(threadid);
}

void CMoonDb::SynchSend(SOCKET sock_client, CPack& pack)
{
	size_t size = pack.GetSize();
	size_t pos = 0;
	while(size > pos) {
		int32_t bytes = static_cast<int32_t>(min(static_cast<size_t>(SendBufSize), size - pos));
		bytes = MoonSockSend(sock_client, static_cast<char*>(pack.GetPointer()) + pos, bytes);
		if(SOCKET_ERROR == bytes) {
			if(ShowInfo) {
				cout << "Send Error: " + MoonLastError() << endl;
				break;
			}
		}
		pos += static_cast<size_t>(bytes);
	}
}

int64_t CMoonDb::SynchReceive(SOCKET sock_client, CPack& pack)
{
	// 获取本次数据数量
	int64_t msg_len = 0;
	int32_t recv_len = MoonSockRecv(sock_client, static_cast<char*>(static_cast<void*>(&msg_len)), 8);
	if(8 != recv_len || msg_len <= 0) {
		if(SOCKET_ERROR == recv_len && ShowInfo) {
			cout << "Receive Error: " + MoonLastError() << endl;
		}
		return 0 == recv_len ? 0 : -1;
	}
	if(msg_len > MaxAllowedPacket) {
		// 这里接收剩余全部数据，是因为不这样话的无法正常将错误信息发送至客户端，客户端在接收8个字节长度后不能再继续接收剩余数据，
		// 不过如果一次接收比较多的数据倒是可以成功，目前这里暂时这样处理。
//		pack.Reallocate(8192);
//		while(true) {
//			if(MoonSockRecv(sock_client, static_cast<char*>(pack.GetPointer()), 8192) <= 0) {
//				break;
//			}
//		}
		ThrowError(ERR_SOCKET, "Exceed maximum bytes of allowed packet (" + num_to_string(MaxAllowedPacket) + " < " + num_to_string(msg_len) + ")");
		return -1;
	}
	// 读取数据
	pack.Reallocate(static_cast<size_t>(msg_len));
	int64_t read_len = 0;
	while(true) {
		int32_t cur_len = static_cast<int32_t>(min(static_cast<int64_t>(BytesPerRead), msg_len - read_len));
		int32_t recv_len = MoonSockRecv(sock_client, static_cast<char*>(pack.GetPointer()) + read_len, cur_len);
		if(recv_len > 0)
		{
			read_len += recv_len;
			if(msg_len == read_len) {
				break;
			}
		}
		else {
			if(SOCKET_ERROR == recv_len && ShowInfo) {
				cout << "Receive Error: " + MoonLastError() << endl;
			}
			return -1;
		}
	}
	pack.SetSize(static_cast<size_t>(msg_len));
	pack.Seek(0);
	return msg_len;
}

void CMoonDb::SynchSendError(SOCKET sock_client, CPack& pack, const string& text)
{
	size_t size = text.size();
	pack.Clear();
	pack.Reallocate(size + 10);
	pack.Put(static_cast<int64_t>(size + 2));
	pack.Put(static_cast<uint16_t>(RT_ERROR));
	pack.Write(text.c_str(), size);
	SynchSend(sock_client, pack);
}

SOCKET CMoonDb::Accept(bool& wrongip) const noexcept
{
	SOCKET sock_client;
	wrongip = false;
	if(IF_IPv4 == InternetFamily) {
		sockaddr_in remoteAddr;
		socklen_t nAddrlen = sizeof(sockaddr_in);
		uint32_t curclientip;
		sock_client = ::accept(DataSeverSocket, static_cast<sockaddr*>(static_cast<void*>(&remoteAddr)), &nAddrlen);
		if(sock_client == INVALID_SOCKET)
		{
//			if(MoonLastErrno() == SOCKET_WOULDBLOCK) {
//				continue;
//			}
			return INVALID_SOCKET;
		}
#if defined(_WIN32)
		curclientip = remoteAddr.sin_addr.S_un.S_addr;
#else
		curclientip = remoteAddr.sin_addr.s_addr;
#endif
		if(ShowInfo) {
			printf("Recieve a connection: %s:%u \r\n", ::inet_ntoa(remoteAddr.sin_addr), remoteAddr.sin_port);
		}
		if(DataClientIPv4 != INADDR_ANY && curclientip != DataClientIPv4) {
			MoonSockClose(sock_client);
			wrongip = true;
			return INVALID_SOCKET;
		}
	}
	else {
		sockaddr_in6 remoteAddr;
		socklen_t nAddrlen = sizeof(sockaddr_in6);
		in6_addr curclientip;
		sock_client = ::accept(DataSeverSocket, static_cast<sockaddr*>(static_cast<void*>(&remoteAddr)), &nAddrlen);
		if(sock_client == INVALID_SOCKET)
		{
//			if(MoonLastErrno() == SOCKET_WOULDBLOCK) {
//				continue;
//			}
			return INVALID_SOCKET;
		}
		curclientip = remoteAddr.sin6_addr;
		if(ShowInfo) {
			char buf[1024];
			::inet_ntop(AF_INET6, &curclientip, buf, 1024);
			printf("Recieved a connection: %s:%u \r\n", buf, remoteAddr.sin6_port);
		}
		if(::memcmp(&DataClientIPv6, &in6addr_any, sizeof(in6_addr)) != 0 && ::memcmp(&DataClientIPv6, &curclientip, sizeof(in6_addr)) != 0) {
			MoonSockClose(sock_client);
			wrongip = true;
			return INVALID_SOCKET;
		}
	}
	return sock_client;
}

void CMoonDb::AsyncAccept() noexcept
{
	if(AsyncConnections.size() >= MaxConnections) {
		return;
	}
	fd_set fdread;
	int maxfd = static_cast<int>(DataSeverSocket + 1);
	timeval tv{0, 0};
	bool wrongip;
	if(AsyncConnections.size() == 0) {
		tv.tv_sec = static_cast<int32_t>(SelectTimeout / 1000000);
		tv.tv_usec = static_cast<int32_t>(SelectTimeout % 1000000);
	}
	FD_ZERO(&fdread);//初始化fd_set
	FD_SET(DataSeverSocket, &fdread);//分配套接字句柄到相应的fd_set
	if(::select(maxfd, &fdread, nullptr, nullptr, &tv) > 0) {
		if(FD_ISSET(DataSeverSocket, &fdread)) {
			for(uint32_t i = 0; i < BackLog && AsyncConnections.size() < MaxConnections; i++) {
				SOCKET sock_client = Accept(wrongip);
				if(sock_client == INVALID_SOCKET) {
					if(wrongip) {
						continue;
					}
					else {
						break;
					}
				}
				CConnection* conn = AsyncConnections.push();
				if(conn->Buffer.GetSize() == 0) {
					conn->Buffer.Allocate(static_cast<size_t>(max(ReceiveBufSize, SendBufSize)));
				}
				conn->Initialize(sock_client);
				AsyncReceive(conn);
			}
		}
	}
}

void CMoonDb::AsyncCloseClient(CConnection* conn)
{
	conn->Status = SESS_DISCONNECTED;
	MoonSockClose(conn->Socket);
	conn->Socket = INVALID_SOCKET;
}

void CMoonDb::AsyncSend(CConnection* conn)
{
	if(SESS_PROCESSED == conn->Status) {
		conn->BufPos = 0;
		conn->Status = SESS_SENDING;
		conn->Time = CTime::Now();
	}
	// 发送超时检测
	else if(conn->Time + AsyncSendTimeout < CTime::Now()) {
		AsyncCloseClient(conn);
		return;
	}
	size_t size = conn->Buffer.GetSize();
	while(size > conn->BufPos) {
		int32_t bytes = static_cast<int32_t>(min(static_cast<size_t>(SendBufSize), size - conn->BufPos));
		bytes = MoonSockSend(conn->Socket, static_cast<char*>(conn->Buffer.GetPointer()) + conn->BufPos, bytes);
		if(SOCKET_ERROR == bytes) {
			if(MoonLastErrno() == SOCKET_AGAIN) {
				return;
			}
			if(ShowInfo) {
				cout << "Send Error: " + MoonLastError() << endl;
			}
			AsyncCloseClient(conn);
			return;
		}
		conn->BufPos += static_cast<size_t>(bytes);
		conn->Time = CTime::Now();
	}
	if(conn->Error || size != conn->BufPos) {
		AsyncCloseClient(conn);
	}
	else {
		conn->Status = SESS_SENT;
	}
}

void CMoonDb::AsyncReceive(CConnection* conn)
{
	// 接收数据长度
	if(SESS_CONNECTED == conn->Status || SESS_SENT == conn->Status) {
		int64_t msg_len = 0;
		int32_t recv_len = MoonSockRecv(conn->Socket, static_cast<char*>(static_cast<void*>(&msg_len)), 8);
		if(SOCKET_ERROR == recv_len) {
			if(MoonLastErrno() == SOCKET_WOULDBLOCK) {
				return;
			}
			AsyncCloseClient(conn);
			return;
		}

		if(8 != recv_len || msg_len <= 0) {
			if(SOCKET_ERROR == recv_len && ShowInfo) {
				cout << "Receive Error: " << MoonLastError() << endl;
			}
			AsyncCloseClient(conn);
			return;
		}
		if(msg_len > MaxAllowedPacket) {
			SynchGenerateError(conn, "Exceed maximum bytes of allowed packet (" + num_to_string(MaxAllowedPacket) + " < " + num_to_string(msg_len) + ")");
			return;
		}
		conn->BufPos = 0;
		conn->Buffer.Reallocate(static_cast<size_t>(msg_len));
		conn->Buffer.SetSize(static_cast<size_t>(msg_len));
		conn->Buffer.Seek(0);
		conn->Status = SESS_RECEIVING;
		conn->Time = CTime::Now();
	}
	// 如果已经开始接收那么进行接收超时检测
	else if(conn->Time + AsyncRecvTimeout < CTime::Now()) {
		AsyncCloseClient(conn);
		return;
	}
	// 接收数据
	size_t msg_len = conn->Buffer.GetSize();
	while(true) {
		int32_t cur_len = static_cast<int32_t>(min(static_cast<size_t>(ReceiveBufSize), msg_len - conn->BufPos));
		int32_t recv_len = MoonSockRecv(conn->Socket, static_cast<char*>(conn->Buffer.GetPointer()) + conn->BufPos, cur_len);
		if(recv_len > 0) {
			conn->Time = CTime::Now();
			conn->BufPos += static_cast<size_t>(recv_len);
			if(msg_len == conn->BufPos) {
				conn->Status = SESS_RECEIVED;
				return;
			}
		}
		else if(SOCKET_ERROR == recv_len) {
			if(MoonLastErrno() == SOCKET_WOULDBLOCK) {
				return;
			}
			else {
				if(ShowInfo) {
					cout << "Receive Error: " << MoonLastError() << endl;
				}
				AsyncCloseClient(conn);
				return;
			}
		}
	}
}

void CMoonDb::SynchGenerateError(CConnection* conn, const string& text)
{
	size_t size = text.size();
	conn->Error = true;
	conn->Buffer.Clear();
	conn->Buffer.Reallocate(size + 10);
	conn->Buffer.Put(static_cast<int64_t>(size + 2));
	conn->Buffer.Put(static_cast<uint16_t>(RT_ERROR));
	conn->Buffer.Write(text.c_str(), size);
}

void CMoonDb::_AsyncQuery(CConnection* conn)
{
	/*__int128_t msg_len = 0;
	cout << MoonSockRecv(sock_client, static_cast<char*>(static_cast<void*>(&msg_len)), 16) << endl;
	cout << msg_len << endl;
	MoonSockSend(sock_client, static_cast<const char*>(static_cast<const void*>(&msg_len)), 16);*/

	try {
		uint8_t apitype = 0;
		conn->Buffer.Get(apitype);
		if(1 == apitype) {
			NoSQLQuery(conn->Buffer);
		}
		else if(2 == apitype) {
			SQLQuery(conn->Buffer);
		}
		else {
			ThrowError(ERR_WRONG_API_TYPE, "Wrong API type: " + num_to_string(apitype));
			//return;
		}
	}
	catch(exception& e) {
		SynchGenerateError(conn, e.what());
	}
	conn->Status = SESS_PROCESSED;
}

void CMoonDb::AsyncQuery()
{
	unique_lock<mutex> lck(ThreadMutex, defer_lock);
	while(true) {
		if(!Started) {
			AsyncThreadNum--;
			break;
		}
		lck.lock();
		while(AsyncNewConnections.size() == 0) {
			if(!Started) {
				break;
			}
			AsyncCondVar.wait(lck);
		}
		if(!Started) {
			AsyncThreadNum--;
			break;
		}
		CConnection* conn = AsyncNewConnections.front();
		AsyncNewConnections.pop();
		lck.unlock();
		_AsyncQuery(conn);
		AsyncThreadNum--;
	}
}

CSQLite* CMoonDb::GetSQLite(const string& dbname)
{
	CSQLite* dbobj = nullptr;
	/*SchemaMutex.lock_shared();
	auto it = SQLites.find(dbname);
	if(it != SQLites.end()) {
		dbobj = it->second;
	}
	SchemaMutex.unlock_shared();
	if(!LoadAllSchemasOnLoading) {
		string dbpath = DataDirectory + DIRECTORY_SEPARATOR + dbname;
		if(CFileSystem::Exists(dbpath)) {
			SchemaMutex.lock();
			auto it = SQLites.find(dbname);
			if(it != SQLites.end()) {
				dbobj = it->second;
			}
			else {
				try {
					dbobj = new CSQLite(dbpath);
					SQLites.emplace(dbname, dbobj);
					shared_timed_mutex* mutex = new shared_timed_mutex;
					dbobj->SetMutex(mutex);
				}
				catch(runtime_error& e) {
					if(ShowInfo) {
						cout << e.what() << endl;
					}
				}
			}
			SchemaMutex.unlock();
		}
	}*/
	return dbobj;
}

void CMoonDb::SQLiteQuery(CPack& pack)
{
	string sql;
	pack.Get<uint32_t>(sql);
	unordered_map<string, CAny> data;
	ParseStringMap(pack, data);
	pack.Clear();
	if("CONNECT" == to_upper_copy(sql)) {
		auto it = data.find("version");
		if(it == data.end() || it->second.GetType() != FT_STRING) {
			ThrowError(ERR_DB_WRONG_NAME, "The database is not specified");
		}
	}
	else {
		auto it = data.find("database");
		if(it == data.end() || it->second.GetType() != FT_STRING) {
			ThrowError(ERR_DB_WRONG_NAME, "The database is not specified");
		}
		string dbname = it->second.ToString();
		if(!is_word(dbname)) {
			ThrowError(ERR_DB_WRONG_NAME, "The database name '" + dbname + "' is wrong");
		}
		CSQLite sqlite;
		sqlite.Connect(SQLiteDirectory + DIRECTORY_SEPARATOR + dbname);
		if("SELECT" == to_upper_copy(sql.substr(0, 6))) {

		}
		else {

		}
	}
}

void CMoonDb::SQLQuery(CPack& pack)
{
	string sql;
	pack.Get<uint32_t>(sql);
	unordered_map<string, CAny> data;
	ParseStringMap(pack, data);
	pack.Clear();
	vector<CSQLParser::CToken> tokens;
	SQLParser.Parse(sql, tokens);
	for(size_t i = 0; i < tokens.size(); i++) {
		cout << tokens[i].KType << "," << tokens[i].TType << "," << tokens[i].Content << endl;
	}
	for(auto it = data.begin(); it != data.end(); it++) {
		cout << it->first << "," << it->second << endl;
	}
}

void CMoonDb::NoSQLQuery(CPack& pack)
{
	uint16_t opertype = 0;
	pack.Get(opertype);
	if(opertype == 0 || opertype >= OPER_SIZE) {
		ThrowError(ERR_WRONG_OPER_TYPE, "Wrong operation type: " + num_to_string(opertype));
		return;
	}
	string dbname;
	pack.Get<uint16_t>(dbname);
	if(!is_word(dbname)) {
		ThrowError(ERR_WRONG_NAME, "Wrong database name: " + dbname);
		return;
	}
	string tablename;
	pack.Get<uint16_t>(tablename);
	if(!is_word(tablename)) {
		ThrowError(ERR_WRONG_NAME, "Wrong table name: " + tablename);
		return;
	}
	CDatabase* dbh = GetDatabase(dbname);
	if(nullptr == dbh) {
		ThrowError(ERR_DB_NOT_EXIST, "Database " + dbname + " doesn't exist.");
		return;
	}
	CTable* tableh = dbh->GetTable(tablename);
	if(nullptr == tableh) {
		ThrowError(ERR_TABLE_NOT_EXIST, "Table " + tablename + " doesn't exist.");
		return;
	}
	unordered_map<string, CAny> data;
	ParseStringMap(pack, data);
	pack.Clear();
	shared_timed_mutex* mutex = dbh->GetMutex();
	switch(static_cast<OperType>(opertype)) {
	case OPER_SELECT:
		mutex->lock_shared();
		try {
			tableh->GetData(data["rowid"], pack);
			mutex->unlock_shared();
		}
		catch(runtime_error& e) {
			mutex->unlock_shared();
			throw e;
		}
		break;
	case OPER_INSERT:
		mutex->lock();
		try {
			tableh->InsertData(data, pack);
			mutex->unlock();
		}
		catch(runtime_error& e) {
			mutex->unlock();
			throw e;
		}
		break;
	case OPER_UPDATE:
		mutex->lock();
		try {
			tableh->UpdateData(data["rowid"], data, pack);
			mutex->unlock();
		}
		catch(runtime_error& e) {
			mutex->unlock();
			throw e;
		}
		break;
	case OPER_DELETE:
		mutex->lock();
		try {
			tableh->DeleteData(data["rowid"], pack);
			mutex->unlock();
		}
		catch(runtime_error& e) {
			mutex->unlock();
			throw e;
		}
		break;
	case OPER_REPLACE:
		mutex->lock();
		try {
			tableh->ReplaceData(data["rowid"], data, pack);
			mutex->unlock();
		}
		catch(runtime_error& e) {
			mutex->unlock();
			throw e;
		}
		break;
	default:
		break;
	}
}

void CMoonDb::ParseStringMap(CPack& pack, unordered_map<string, CAny>& data)
{
	uint16_t count = 0;
	pack.Get(count);
	for(uint16_t i = 0; i < count; ++i) {
		string key;
		pack.Get<uint16_t>(key);
		CAny value;
		auto it = data.emplace(key, value);
		if(it.second) {
			it.first->second.Load(pack);
		}
		//data.insert(pair<string, CAny>(key, value));
	}
}

CDatabase* CMoonDb::GetDatabase(const string& dbname)
{
	CDatabase* dbobj = nullptr;
	SchemaMutex.lock_shared();
	auto it = Databases.find(dbname);
	if(it != Databases.end()) {
		dbobj = it->second;
	}
	SchemaMutex.unlock_shared();
	if(!LoadAllSchemasOnLoading) {
		string dbpath = DataDirectory + DIRECTORY_SEPARATOR + dbname;
		if(CFileSystem::Exists(dbpath)) {
			SchemaMutex.lock();
			auto it = Databases.find(dbname);
			if(it != Databases.end()) {
				dbobj = it->second;
			}
			else {
				try {
					dbobj = new CDatabase(dbpath);
					Databases.emplace(dbname, dbobj);
					dbobj->SetMutex(ApplyForMutex());
				}
				catch(runtime_error& e) {
					if(ShowInfo) {
						cout << e.what() << endl;
					}
				}
			}
			SchemaMutex.unlock();
		}
	}
	return dbobj;
}

shared_timed_mutex* CMoonDb::ApplyForMutex()
{
	shared_timed_mutex* mutex = nullptr;
	if(DeletedDbMutexes.empty()) {
		mutex = new shared_timed_mutex;
		DatabaseMutexes.insert(mutex);
	}
	else {
		mutex = DeletedDbMutexes.front();
		DeletedDbMutexes.pop_front();
	}
	return mutex;
}

void CMoonDb::ReleaseMutex(shared_timed_mutex* mutex)
{
	auto it = DatabaseMutexes.find(mutex);
	if(it != DatabaseMutexes.end()) {
		DeletedDbMutexes.push_back(*it);
		DatabaseMutexes.erase(it);
	}
}

void CMoonDb::LoadSchemas()
{
	vector<string> dbnames;
	CFileSystem::ReadDirectoryOnlyDirectory(DataDirectory, dbnames, true);
	for(size_t i = 0; i < dbnames.size(); i++) {
		try {
			CDatabase* db = new CDatabase(DataDirectory + DIRECTORY_SEPARATOR + dbnames[i]);
			Databases.emplace(dbnames[i], db);
			db->SetMutex(ApplyForMutex());
		}
		catch(runtime_error& e) {
			if(ShowInfo) {
				cout << e.what() << endl;
			}
		}
	}
}

void CMoonDb::CloseDatabase(const string& dbname)
{
	SchemaMutex.lock();
	auto it = Databases.find(dbname);
	if(it != Databases.end()) {
		ReleaseMutex(it->second->GetMutex());
		delete it->second;
		Databases.erase(it);
	}
	SchemaMutex.unlock();
}

}
