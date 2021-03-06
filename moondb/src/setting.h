#pragma once

//#define UNICODE 1
#undef UNICODE
#if defined(_WIN32)
	// 注意将_mingw.h文件中的_WIN32_WINNT值置为0x0603
	#include <ws2tcpip.h>
	#define DIRECTORY_SEPARATOR '\\'
	#if defined(UNICODE)
		#define STRPOINTER LPWSTR
		#define CONSTSTRPOINTER LPCWSTR
	#else
		#define STRPOINTER LPSTR
		#define CONSTSTRPOINTER LPCSTR
	#endif
	#include <windows.h>
	#define msleep(n) Sleep(n)
#else
	#define DIRECTORY_SEPARATOR '/'
	#include <unistd.h>
	#define msleep(n) usleep(n * 1000)
#endif

//#if defined(__GNUC__) || defined(__GNUG__)
//#define GCC 1
//#endif

#define MOONDB_VERSION "3.0"
