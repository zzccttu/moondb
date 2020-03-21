#pragma once

#include <stdio.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <stdexcept>

/**
 * 触发错误的宏函数
 */
#define ThrowError(code, message)		CRunningError::Throw(__FILE__, __LINE__, code, message)

namespace MoonDb {

enum ErrCodeType {
	ERR_STRING_VALID = 1,
	ERR_SOCKET,
	ERR_CONNECT,
	ERR_SEND,
	ERR_RECEIVE,
	ERR_MEMORY_ALLOCATE,
	ERR_EXCEED_MAXSIZE,
	ERR_WRONG_DATA_TYPE,
	ERR_FROM_SERVER,
	ERR_DATA_INVALID,
};

class CRunningError
{
public:
	static void Throw(const char* filename, std::uint32_t line, ErrCodeType code, const std::string& message)
	{
		std::string Message = "Error " + std::to_string(code) + ": '" + message + "' occur in the file " + filename + " at line " + std::to_string(line);
		throw std::runtime_error(Message);
	}

	static void Throw(const char* filename, std::uint32_t line, ErrCodeType code, const char* message)
	{
		Throw(filename, line, code, std::string(message));
	}
};

}
