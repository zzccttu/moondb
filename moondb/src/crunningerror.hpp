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

namespace MoonDb {

enum ErrCodeType {
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
};

class CRunningError
{
public:
	static void Throw(const char* filename, std::uint32_t line, ErrCodeType code, const std::string& message) __attribute((noreturn))
	{
		std::string Message = "Error " + std::to_string(code) + ": '" + message + "' occur in the file " + filename + " at line " + std::to_string(line);
		CLog::Instance()->Put(CLog::L_WARNING, Message.c_str());
		throw std::runtime_error(Message);
	}

	static void Throw(const char* filename, std::uint32_t line, ErrCodeType code, const char* message) __attribute((noreturn))
	{
		Throw(filename, line, code, std::string(message));
	}
};

}
