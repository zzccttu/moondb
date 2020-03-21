#include "clog.h"

namespace MoonDb {

const char* CLog::LevelString[8]= {"FATAL", "WARNING" , "INFO", "DEBUG"};

CLog* CLog::Instance(const std::string& programdir)
{
	static CLog logobj;
	if(!programdir.empty() && !logobj.IsOpen()) {
		logobj.Open(programdir + DIRECTORY_SEPARATOR + "moondb.log");
	}
	if(logobj.IsOpen()) {
		return &logobj;
	}
	else {
		return nullptr;
	}
}

void CLog::Open(const std::string& filename)
{
	LogFile.open(filename, std::ios_base::app | std::ios_base::binary);
}

CLog::~CLog()
{
	LogFile.close();
}

bool CLog::IsOpen()
{
	return LogFile.is_open();
}

void CLog::Put(Level level, const char * sFmt, va_list ap)
{
	if(level < L_FATAL || level >= L_SIZE) {
		return;
	}
	size_t size = 0;
	::strcpy(Buffer, LevelString[level]);
	size += strlen(LevelString[level]);
	Buffer[size++] = ' ';
	uint32_t timelen = 0;
	CTime::ToString(CTime::SolarCalendar(static_cast<int64_t>(CTime::CurrentTime()) + CTime::GetLocalTimeDifference()), Buffer + size, &timelen);
	size += timelen;
	Buffer[size++] = ':';
	if(sFmt) {
		vsnprintf(Buffer + size, BufSize - size - 1, sFmt, ap);
	}
	size = strlen(Buffer);
	Buffer[size++] = '\n';
	Buffer[size] = '\0';
	LogFile.write(Buffer, static_cast<std::streamsize>(size));
	LogFile.flush();
}

void CLog::Put(Level level, const char * str)
{
	if(level < L_FATAL || level >= L_SIZE) {
		return;
	}
	size_t size = 0;
	::strcpy(Buffer, LevelString[level]);
	size += strlen(LevelString[level]);
	Buffer[size++] = ' ';
	uint32_t timelen = 0;
	CTime::ToString(CTime::SolarCalendar(static_cast<int64_t>(CTime::CurrentTime()) + CTime::GetLocalTimeDifference()), Buffer + size, &timelen);
	size += timelen;
	Buffer[size++] = ':';
	::strncpy(Buffer + size, str, BufSize - size - 1);
	size = strlen(Buffer);
	Buffer[size++] = '\n';
	Buffer[size] = '\0';
	LogFile.write(Buffer, static_cast<std::streamsize>(size));
	LogFile.flush();
}

void CLog::Put(Level level, const std::string& str)
{
	Put(level, str.c_str());
}

}
