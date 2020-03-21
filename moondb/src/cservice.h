#pragma once

#include "header.h"
#include "cmoondb.h"

#if defined(_WIN32)
#include <winbase.h>
#endif

namespace MoonDb {

class CService
{
public:
	static string ProgramDir;
	static string ConfigFile;
	static bool ShowInfo;
#if defined(_WIN32)
	static bool IsService;
	static string ServiceName;
	static SERVICE_STATUS ServiceStatus;
	static SERVICE_STATUS_HANDLE ServiceHandle;
	static const char* WinErrorInfo();
	static void FatalError(const char * sFmt, ...) __attribute__ ((format ( printf, 1, 2))) __attribute((noreturn));
	static void InfoError(const char * sFmt, ...) __attribute__ ((format ( printf, 1, 2))) __attribute((noreturn));
	static void Install(string execfile, string inifile, string servicename);
	static void Uninstall(string servicename);
	static void SetStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
	static void WINAPI Control(DWORD dwControlCode);
	static void Start();
#endif
	static void Main(unsigned long argc, char **argv);

protected:
	static CMoonDb MDbH;
};

}
