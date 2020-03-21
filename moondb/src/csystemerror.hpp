#pragma once

#include <stdio.h>
#include <cstdint>
#include <string>
#include <stdexcept>
#include "clog.h"

/**
 * 触发错误的宏函数
 */
#define TriggerError(message)		CSystemError::Throw(__FILE__, __LINE__, message)

namespace MoonDb {

class CSystemError
{
public:
	/**
	 * 构造函数
	 * @param filename  文件名
	 * @param line      行号
	 * @param message   提示信息
	 */
	static void	Throw(const char* filename, std::uint32_t line, const std::string& message) __attribute((noreturn))
	{
		std::string Message = "'Error: " + message + "' occur in the file '" + filename + "' at line " + std::to_string(line);
		CLog::Instance()->Put(CLog::L_FATAL, Message.c_str());
		throw std::runtime_error(Message);
	}

    /**
     * 构造函数
     * @param filename  文件名
     * @param line      行号
     * @param message   提示信息
     */
	static void	Throw(const char* filename, std::uint32_t line, const char* message) __attribute((noreturn))
    {
		Throw(filename, line, std::string(message));
    }
};

}
