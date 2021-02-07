#pragma once

#include <string>
#include "cpack.hpp"
#include "sqlite/sqlite3.h"

namespace MoonDb {

class CSQLite
{
public:
	CSQLite();
	~CSQLite();

	// path为数据库文件路径，建议使用绝对路径，如果为“:memory:”表示使用内存存储
	void Connect(const std::string& path);
	void Close();
	void Execute(const std::string& sql, CPack& ret);
	void Query(const std::string& sql, CPack& ret);

protected:
	sqlite3* DbObj;
};

}
