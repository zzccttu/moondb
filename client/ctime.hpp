#pragma once

#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>

namespace MoonDb {

class CTime
{
public:
	const static constexpr double TimeRatio = static_cast<double>(std::chrono::high_resolution_clock::period::num) / static_cast<double>(std::chrono::high_resolution_clock::period::den); /**< 时间比例，乘以该参数后，单位为秒 */
	const static constexpr int64_t NanoTime = static_cast<int64_t>(std::chrono::high_resolution_clock::period::den) / static_cast<int64_t>(std::chrono::high_resolution_clock::period::num); /**< 时间倍数，乘以该参数后，单位为纳秒 */

    /**
     * 计算函数运行时间，主要用于测试
     * @return 返回当前时间截，单位：纳秒
     */
    inline static std::chrono::high_resolution_clock::rep Now()
    {
        return std::chrono::high_resolution_clock::now().time_since_epoch().count();
    }

    /**
     * 计算函数运行时间，主要用于测试
     * @param func  函数
     * @return 返回运行时间，单位：纳秒
     */
    inline static auto Duration(void func())
    {
        auto t1 = Now();
        func();
        auto t2 = Now();
        return t2 - t1;
    }

	/**
	* 获得当前时间截
	* @return 返回时间截
	*/
	inline static time_t CurrentTime()
	{
		return ::time(nullptr);
	}

	/**
	* 获得“当前时区时间 - 格林尼治时间”的值
	* @return 返回相隔的秒数
	*/
	inline static int32_t GetLocalTimeDifference()
	{
		static int32_t difftime = std::numeric_limits<int32_t>::max();
		if (std::numeric_limits<int32_t>::max() == difftime) {
			// 因为可能存在缓存，所以分开获取时间
			time_t rawtime = 0;
			tm * ptm1 = ::gmtime(&rawtime);
			int gmhour = ptm1->tm_hour;
			int gmmin = ptm1->tm_min;
			int gmsec = ptm1->tm_sec;
			tm * ptm2 = ::localtime(&rawtime);
			int localhour = ptm2->tm_hour;
			int localmin = ptm2->tm_min;
			int localsec = ptm2->tm_sec;
			difftime = (localhour - gmhour) * 3600 + (localmin - gmmin) * 60 + localsec - gmsec;
		}
		return difftime;
	}
};

}
