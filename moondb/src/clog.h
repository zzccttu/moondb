#pragma once

#include <string.h>
#include <fstream>
#include "ctime.hpp"
#include "setting.h"

namespace MoonDb {

class CLog
{
public:
	enum Level {
		L_FATAL,
		L_WARNING,
		L_INFO,
		L_DEBUG,
		L_SIZE,
	};

	static CLog* Instance(const std::string& programdir = "");

	~CLog();

	void Put(Level level, const char * sFmt, va_list ap);
	void Put(Level level, const char * str);
	void Put(Level level, const std::string& str);

protected:
	std::fstream LogFile;
	char Buffer[1048576];//1M
	const size_t BufSize = 1048576;
	static const char* LevelString[8];

	CLog(){}
	void Open(const std::string& filename);
	bool IsOpen();
};

}
