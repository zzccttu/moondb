#include "csqlite.h"
#include "functions.hpp"
#include "definition.hpp"
#include "crunningerror.hpp"

namespace MoonDb {

CSQLite::CSQLite()
	:DbObj(nullptr)
{
	sqlite3_threadsafe();
}

CSQLite::~CSQLite()
{
	Close();
}

void CSQLite::Connect(const std::string& path)
{
	if(SQLITE_OK != sqlite3_open_v2(path.c_str(), &DbObj, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_SHAREDCACHE, NULL)) {
		ThrowError(ERR_CONNECT_DATABASE, "Can't open the file (" + path + ") with sqlite engine");
	}
}

void CSQLite::Execute(const std::string& sql, CPack& ret)
{
	char *errMsg = nullptr;
	if(SQLITE_OK != sqlite3_exec(DbObj, sql.c_str(), nullptr, nullptr, &errMsg)) {
		std::string err = std::move(errMsg);
		ThrowError(ERR_WRONG_SQL, err);
	}
	if(to_upper_copy(sql.substr(0, 6)) == "INSERT") {
		ret.Put(static_cast<int64_t>(12));
		ret.Put(static_cast<uint16_t>(RT_LAST_INSERT_ID));
		ret.Put(static_cast<uint16_t>(FT_UINT64));
		ret.Put(static_cast<uint64_t>(sqlite3_last_insert_rowid(DbObj)));
	}
	else {
		ret.Put(static_cast<int64_t>(12));
		ret.Put(static_cast<uint16_t>(RT_AFFECTED_ROWS));
		ret.Put(static_cast<uint16_t>(FT_UINT64));
		ret.Put(static_cast<uint64_t>(sqlite3_changes(DbObj)));
	}
}

void CSQLite::Query(const std::string& sql, CPack& ret)
{
	int nCols = 0;
	int nRows = 0;
	char **azResult;
	char *errMsg = nullptr;
	// 查询
	if(SQLITE_OK != sqlite3_get_table(DbObj, sql.c_str(), &azResult, &nRows, &nCols, &errMsg)) {
		std::string err = std::move(errMsg);
		ThrowError(ERR_WRONG_SQL, errMsg);
	}
	// 初始长度
	ret.Put(static_cast<int64_t>(4));
	// 返回类型
	ret.Put(static_cast<uint16_t>(RT_QUERY));
	// 数据条数
	ret.Put(static_cast<uint16_t>(nRows));
	// 数据列数
	ret.Put(static_cast<uint16_t>(nCols));
	// 输出列名
	for(int i = 0; i < nCols; i ++) {
		ret.Put<uint32_t>(azResult[i]);
	}
	// 输出数据
	size_t index = nCols;
	for(int i = 0; i < nRows; i ++) {
		for(int j = 0; j < nCols; j ++) {
			ret.Put<uint32_t>(azResult[index]);
			index++;
		}
	}
	sqlite3_free_table(azResult);
	ret.Seek(0);
	ret.Put(static_cast<int64_t>(ret.GetSize() - 8));
}

void CSQLite::Close()
{
	sqlite3_close_v2(DbObj);
	DbObj = nullptr;
}

}
