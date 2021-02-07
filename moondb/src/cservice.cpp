#include "cservice.h"

namespace MoonDb {

string CService::ProgramDir = "";
string CService::ConfigFile = "";
bool CService::ShowInfo = false;
CMoonDb CService::MDbH;

#if defined(_WIN32)

bool CService::IsService = false;
string CService::ServiceName = "";
SERVICE_STATUS CService::ServiceStatus;
SERVICE_STATUS_HANDLE CService::ServiceHandle;

const char* CService::WinErrorInfo()
{
	static char sBuf[1024];
	DWORD uErr = ::GetLastError ();
	snprintf ( sBuf, sizeof(sBuf), "code=%ld, error=", uErr );
	size_t iLen = strlen(sBuf);
	if ( !FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, nullptr, uErr, 0, static_cast<STRPOINTER>(static_cast<void*>(sBuf+iLen)), static_cast<DWORD>(sizeof(sBuf)-iLen), nullptr ) )
		snprintf ( sBuf+iLen, sizeof(sBuf)-iLen, "(no message)" );
	trim(sBuf);
	return sBuf;
}

void CService::FatalError(const char * sFmt, ...)
{
	va_list ap;
	va_start(ap, sFmt);
	CLog::Instance()->Put(CLog::L_FATAL, sFmt, ap);
	va_end(ap);
	//Shutdown ();
	exit(1);
}

void CService::InfoError(const char * sFmt, ...)
{
	va_list ap;
	va_start(ap, sFmt);
	CLog::Instance()->Put(CLog::L_INFO, sFmt, ap);
	va_end(ap);
	//Shutdown ();
}

void CService::Install(string execfile, string inifile, string servicename)
{
	if(servicename.empty()) {
		servicename = "moondb";
	}
	string szpath = execfile + " --ntservice " + servicename + " -i " + inifile;

	SC_HANDLE hSCM = ::OpenSCManager (nullptr, nullptr, SC_MANAGER_ALL_ACCESS );
	if(nullptr == hSCM) {
		cout << "OpenSCManager() failed." << endl;
		cout << WinErrorInfo() << endl;
		return;
	}

	SC_HANDLE hService = ::CreateService (
		hSCM,
		static_cast<CONSTSTRPOINTER>(static_cast<const void*>(servicename.c_str())),
		static_cast<CONSTSTRPOINTER>(static_cast<const void*>(servicename.c_str())),
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		static_cast<CONSTSTRPOINTER>(static_cast<const void*>(szpath.c_str())),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	);

	if ( !hService ) {
		CloseServiceHandle ( hSCM );
		cout << "CreateService() failed." << endl;
		cout << WinErrorInfo() << endl;
		return;

	}
	else{
		cout << "Service '" + servicename + "' installed successfully." << endl;
	}

	string description = servicename + "-" + MOONDB_VERSION;

	SERVICE_DESCRIPTION tDesc;
	tDesc.lpDescription = static_cast<STRPOINTER>(static_cast<void*>(&description.front()));
	if(!ChangeServiceConfig2 ( hService, SERVICE_CONFIG_DESCRIPTION, &tDesc)) {
		cout << "failed to set service description" << endl;
		cout << WinErrorInfo() << endl;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
}

void CService::Uninstall(string servicename)
{
	SC_HANDLE hSCM = OpenSCManager (nullptr, nullptr, SC_MANAGER_ALL_ACCESS );
	if(nullptr == hSCM) {
		cout << "OpenSCManager() failed." << endl;
		cout << WinErrorInfo() << endl;
		return;
	}

	// open service
	SC_HANDLE hService = OpenService ( hSCM, static_cast<CONSTSTRPOINTER>(static_cast<const void*>(servicename.c_str())), DELETE );
	if ( !hService ) {
		CloseServiceHandle ( hSCM );
		cout << "OpenService() failed." << endl;;
		cout << WinErrorInfo() << endl;
		return;
	}

	// do delete
	bool bRes = !!DeleteService ( hService );
	CloseServiceHandle ( hService );
	CloseServiceHandle ( hSCM );

	if ( !bRes ) {
		cout << "DeleteService() failed." << endl;
		cout << WinErrorInfo() << endl;
	}
	else
		cout << "Service '" + servicename + "' deleted successfully." << endl;
}

void CService::SetStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	if(dwCurrentState == SERVICE_START_PENDING) {
		ServiceStatus.dwControlsAccepted = 0;
	}
	else {
		ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	}

	ServiceStatus.dwCurrentState = dwCurrentState;
	ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	ServiceStatus.dwWaitHint = dwWaitHint;

	if ( dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED ) {
		ServiceStatus.dwCheckPoint = 0;
	}
	else {
		ServiceStatus.dwCheckPoint = dwCheckPoint++;
	}

	::SetServiceStatus(ServiceHandle, &ServiceStatus);
}

void WINAPI CService::Control(DWORD dwControlCode)
{
	switch(dwControlCode)
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			MDbH.Stop();
			SetStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
			while(true) {
				msleep(1);
				if(MDbH.IsStopped()) {
					SetStatus(SERVICE_STOPPED, NO_ERROR, 0);
					break;
				}
			}
			break;

		default:
			SetStatus(ServiceStatus.dwCurrentState, NO_ERROR, 0);
			break;
	}
}

void CService::Start()
{
	SERVICE_TABLE_ENTRY dDispatcherTable[] =
	{
		{ static_cast<LPSTR>(&ServiceName[0]), static_cast<LPSERVICE_MAIN_FUNCTION>(Main) },
		{ nullptr, nullptr }
	};
	if ( !StartServiceCtrlDispatcher ( dDispatcherTable ) ) {
		FatalError("StartServiceCtrlDispatcher() failed: %s", CService::WinErrorInfo());
	}
}

#endif

void CService::Main(unsigned long argc, char **argv)
{
#if defined(_WIN32)
	if(IsService) {
		ServiceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS;
		ServiceStatus.dwServiceSpecificExitCode = 0;
		ServiceHandle = RegisterServiceCtrlHandler(ServiceName.c_str(), Control);
		if(!ServiceHandle) {
			FatalError("failed to start service: RegisterServiceCtrlHandler() failed: %s", WinErrorInfo());
		}
		/*char szPipeName[64];
		snprintf(szPipeName, sizeof(szPipeName), "\\\\.\\pipe\\moondb_%d", getpid());
		HandlePipe = CreateNamedPipe(szPipeName, PIPE_ACCESS_INBOUND,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
			PIPE_UNLIMITED_INSTANCES, 0, WIN32_PIPE_BUFSIZE, NMPWAIT_NOWAIT, nullptr);
		ConnectNamedPipe(HandlePipe, nullptr);*/
	}
#endif
	CLog::Instance()->Put(CLog::L_INFO, "Server is prepared.");
	MDbH.LoadConfiguration(ProgramDir, ShowInfo, ConfigFile);
	CLog::Instance()->Put(CLog::L_INFO, "Configuration is loaded.");
	MDbH.Start();
#if defined(_WIN32)
	if(IsService)
		SetStatus(SERVICE_RUNNING, NO_ERROR, 0);
#endif
	CLog::Instance()->Put(CLog::L_INFO, "Server Started.");
	MDbH.Run();
	CLog::Instance()->Put(CLog::L_INFO, "Server Stopped.");
}

}
