#pragma once

#include <stdio.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <stdexcept>
#include "clog.h"

/**
 * 触发错误的宏函数
 */
#define ThrowError(code, message)		CRunningError::Throw(__FILE__, __LINE__, code, message)
#define TraceError(e, message)		CRunningError::Throw(__FILE__, __LINE__, e, message)

namespace MoonDb {

enum ErrCodeType {
	ERR_TRACE = 0,// 仅用于多次捕获异常然后再抛出好追踪错误
	ERR_STRING_VALID = 1,
	ERR_SOCKET,
	ERR_WRONG_API_TYPE,
	ERR_WRONG_OPER_TYPE,
	ERR_WRONG_DATA_TYPE,
	ERR_WRONG_NAME,
	ERR_FILE_OPEN,
	ERR_FILE_CLOSE,
	ERR_FILE_SEEK,
	ERR_FILE_TELL,
	ERR_FILE_READ,
	ERR_FILE_WRITE,
	ERR_MEMORY_ALLOCATE,
	ERR_EXCEED_MAXSIZE,
	ERR_CREATE_DIRECTORY,
	ERR_DB_WRONG_NAME,
	ERR_DB_EXIST,
	ERR_DB_NOT_EXIST,
	ERR_DB_USER_FILE_VALID,
	ERR_DB_USER_EXIST,
	ERR_TABLE_EXIST,
	ERR_TABLE_NOT_EXIST,
	ERR_MISSING_DATA,
	ERR_WRONG_ROWID,
	ERR_WRONG_ENGINE,
	ERR_WRONG_FIELD_TYPE,
	ERR_DUPLICATE_ID,
	ERR_INTERPRETE_DATA,
	ERR_INVALID_SQL,
	ERR_OUT_OF_RANGE,
	ERR_DENOMINATOR_ZERO,
	ERR_NOT_INITIALIZE,
	ERR_CONNECT_DATABASE,
	ERR_WRONG_SQL,
	ERR_LOCK_TIME_EXCEED,
};

class CRunningError
{
public:
	static void Throw(const char* filename, std::uint32_t line, ErrCodeType code, const std::string& message) __attribute((noreturn))
	{
		std::string Message = std::to_string(code) + ": " + message + " in " + filename + " on line " + std::to_string(line) + std::string(1, 1);
		CLog* logobj = CLog::Instance();
		if(nullptr != logobj) {
			logobj->Put(CLog::L_WARNING, Message.c_str());
		}
		throw std::runtime_error(Message);
	}

	static void Throw(const char* filename, std::uint32_t line, ErrCodeType code, const char* message) __attribute((noreturn))
	{
		Throw(filename, line, code, std::string(message));
	}

	static void Throw(const char* filename, std::uint32_t line, const std::exception& e, const std::string& message) __attribute((noreturn))
	{
		std::string preverrmsg = e.what();
		std::string msg = preverrmsg.substr(0, preverrmsg.size() - 1) + "\n";
		// 追踪层级最多255
		char tracenum = preverrmsg[preverrmsg.size() - 1];
		if(1 == tracenum) {
			msg += "Stack trace:\n";
		}
		msg += "#" + std::to_string(tracenum) + " " + filename + "(" + std::to_string(line) + "): " + message + std::string(1, tracenum + 1);
		throw std::runtime_error(msg);
	}
};

}
