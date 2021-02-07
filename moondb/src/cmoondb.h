#pragma once

#if defined(_WIN32)
	#define MoonSockRecv(sock, buf, len)	::recv(sock, buf, len, 0)
	#define MoonSockSend(sock, buf, len)	::send(sock, buf, len, 0)
	#define MoonSockClose(sock)				::closesocket(sock)
	#define MoonLastError()                 std::to_string(::GetLastError())
	//#define socklen_t						int
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

#include <atomic>
#include <shared_mutex>
#include "cqueue.hpp"
#include "csqlparser.h"
#include "cdatabase.h"
#include "ctable.h"
#include "csqlite.h"

namespace MoonDb {

// 如果打算一次提交多个sql语句（比如多次插入数据），可以构造多个sql输入，比如：先写入sql语句个数，再将各条sql及其数据依次写入即可

//class __declspec(dllexport) CMoonDb
class CMoonDb
{

public:
	CMoonDb();
	~CMoonDb();

	void LoadConfiguration(const string& programdir, bool showinfo, const string& configfile);
	void Run();
	void Start();
	void Stop() noexcept;
	bool IsStopped() const noexcept;

protected:
	enum TokenType {
		TT_NONE,
		TT_STRING,
		TT_USE,
		TT_INSERT,
		TT_UPDATE,
		TT_INTO,
		TT_VALUES,
		TT_SET,
		TT_DELETE,
		TT_FROM,
	};

	enum OperType {
		OPER_SELECT = 1,
		OPER_INSERT,
		OPER_UPDATE,
		OPER_DELETE,
		OPER_REPLACE,
		OPER_SIZE,
	};

	enum InternetFamilyType {
		IF_IPv4 = 1,
		IF_IPv6
	};

	struct CToken
	{
		TokenType Type;
		uint32_t Line;
		string Content;
	};

	enum StatusType {
		SESS_UNCONNECTED,
		SESS_CONNECTED,
		SESS_DISCONNECTED,
		SESS_RECEIVING,
		SESS_RECEIVED,
		SESS_PROCESSING,
		SESS_PROCESSED,
		SESS_SENDING,
		SESS_SENT,
		SESS_SIZE
	};

	class CConnection
	{
	public:
		SOCKET Socket;
		string Database;
		string Username;
		CPack Buffer;
		uint64_t BufPos;
		StatusType Status;
		bool Error;
		chrono::high_resolution_clock::rep Time;
		CConnection() noexcept : Socket(INVALID_SOCKET), BufPos(0), Status(SESS_UNCONNECTED), Error(false), Time(0)
		{}
		inline void Initialize(SOCKET socket) noexcept
		{
			Socket = socket;
			Username.clear();
			Buffer.Clear();
			BufPos = 0;
			Database.clear();
			Status = SESS_CONNECTED;
			Error = false;
			Time = CTime::Now();
		}
	};

	void LoadSchemas();

	inline void Clear() noexcept;

	inline SOCKET CreateSocketv4(uint32_t ip, uint16_t port, bool asyn);
	inline SOCKET CreateSocketv6(in6_addr ip, uint16_t port, bool asyn);
	inline SOCKET _CreateSocket(bool asyn);
	inline void ListenAndSetting(SOCKET socksvr);

	inline SOCKET Accept(bool& wrongip) const noexcept;
	inline void AsyncAccept() noexcept;

	inline uint32_t IPToLong(const string& ip) const noexcept;

	void AsyncRun();
	void SynchRun();
	void GroupRun();
	inline void AsyncQuery();
	inline void _AsyncQuery(CConnection* conn);
	inline void SynchAcceptAndQuery(uint32_t threadid);
	inline void GroupQuery(uint32_t threadid);
	inline void SQLQuery(CPack& pack);
	inline void NoSQLQuery(CPack& pack);
	inline void SQLiteQuery(CPack& pack);
	inline void AsyncSend(CConnection* conn);
	inline void AsyncReceive(CConnection* conn);
	inline void AsyncCloseClient(CConnection* conn);
	inline void SynchGenerateError(CConnection* conn, const string& text);
	inline void ParseStringMap(CPack& pack, unordered_map<string, CAny>& data);
	inline void SynchSend(SOCKET sock_client, CPack& pack);
	inline int64_t SynchReceive(SOCKET sock_client, CPack& pack);
	inline void SynchSendError(SOCKET sock_client, CPack& pack, const string& text);
	inline void GroupDistributeConnection(const vector<SOCKET>& clientsocks, vector<pair<uint32_t, uint32_t>>& connnumperthread);

	unordered_set<shared_timed_mutex*> DatabaseMutexes;	/**< 每个数据库一个互斥锁，这里存储着所有的数据库互斥锁 */
	deque<shared_timed_mutex*> DeletedDbMutexes;		/**< 数据库被关闭后删除的互斥锁存在这里备用 */
	inline shared_timed_mutex* ApplyForMutex();
	inline void ReleaseMutex(shared_timed_mutex* mutex);
	/**
	 * @brief GetDatabase 返回数据库对象指针
	 * @param dbname 数据库名称
	 * @return 数据库对象指针
	 */
	inline CDatabase* GetDatabase(const string& dbname);
	/**
	 * @brief CloseDatabase 关闭数据库并从数据库列表中删除
	 * @param dbname 数据库名称
	 */
	inline void CloseDatabase(const string& dbname);
	inline CSQLite* GetSQLite(const string& dbname);

	inline uint32_t SynchGetNewThreadNum();
	inline void SynchDeleteThread(uint32_t threadid);

	string ProgramDirectory;			/**< 程序所在目录 */
	bool ShowInfo;						/**< 是否以服务方式运行 */
	string ConfigFile;					/**< 配置文件 */
	InternetFamilyType InternetFamily;	/**< ipv4或ipv6 */

	int32_t BytesPerRead;				/**< 每次读取数据大小，单位字节 */
	int32_t SendBufSize;				/**< socket发送缓冲区大小 */
	int32_t ReceiveBufSize;				/**< socket接收缓冲区大小 */
	uint32_t DataServerIPv4;			/**< 监听数据服务IP v4 */
	in6_addr DataServerIPv6;			/**< 监听数据服务IP v6 */
	uint16_t DataServerPort;			/**< 监听数据服务端口 */
	uint32_t DataClientIPv4;			/**< 连接到数据服务的客户端IP v4 */
	in6_addr DataClientIPv6;			/**< 连接到数据服务的客户端IP v6 */
	uint64_t WaitTimeout;				/**< 没有访问数据库不断开连接的最长时间，单位为微秒 */
	int64_t NanoWaitTimeout;			/**< WaitTimeout的纳秒值 */
	uint64_t RecvTimeout;				/**< 接收数据循环时间隔超时时间，单位为微秒 */
	uint64_t SendTimeout;				/**< 发送数据循环时间隔超时时间，单位为微秒 */
	int64_t AsyncRecvTimeout;			/**< 异步接收数据超时时间，单位为纳秒 */
	int64_t AsyncSendTimeout;			/**< 异步发送数据超时时间，单位为纳秒 */
	uint64_t SelectTimeout;				/**< select函数等待时间，单位为微秒 */
	uint32_t BackLog;					/**< 等待队列长度 */
	SOCKET DataSeverSocket;				/**< 监听处理数据 */
	string DataDirectory;				/**< 数据目录 */
	string SQLiteDirectory;				/**< sqlite目录 */
	uint32_t MaxThreads;				/**< 开启的最大线程数 */
	uint32_t MaxConnections;			/**< 最大连接数 */
	int64_t MaxAllowedPacket;			/**< 最大接收数据包 */
	uint32_t Async;						/**< 运行方式，0：同步，1：全局异步，2：分组异步 */
	bool LoadAllSchemasOnLoading;		/**< 是否在启动时一次性加载全部数据库 */

	// 如果接收指令停止运行Started置为false
	atomic<bool> Started;
	// 如果接收指令重新启动运行Started置为false，Restart置为true
	bool Restart;
	// 已经停止
	bool Stopped;

	unordered_map<string, CDatabase*> Databases;/**< 已打开的数据库 */
	unordered_map<string, CSQLite*> SQLites;	/**< 已打开的数据库 */
	CQueue<CConnection> AsyncConnections;		/**< 连接 */
	atomic<uint32_t> AsyncThreadNum;			/**< 当前线程数量 */
	atomic<uint32_t> GroupConnectionNum;		/**< 当前连接数 */
	uint32_t SynchThreadNum;					/**< 当前线程数量 */
	mutex ThreadMutex;							/**< 用于线程操作的互斥锁 */
	queue<uint32_t> SynchIdleThreads;			/**< 空闲线程编号组 */
	vector<CPack> SynchBuffers;					/**< 线程接收发送数据缓冲区 */
	shared_timed_mutex SchemaMutex;				/**< 数据库互斥锁 */

	queue<CConnection*> AsyncNewConnections;	/**< 新线程连接 */
	condition_variable AsyncCondVar;			/**< 阻塞执行条件 */

	vector<vector<SOCKET>> GroupNewConnections;	/**< 分组新连接 */
	atomic<uint32_t>* GroupConnectionNumPerThread;/**< 每个组的连接数 */

	CSQLParser SQLParser;
};

}
