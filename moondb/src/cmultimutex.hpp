#pragma once

#include <thread>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <deque>
#include "crunningerror.hpp"

namespace MoonDb {

template <typename T_Key>
class CMultiMutex
{
public:
	/**
	 * @brief CMultiMutex 带索引的互斥量
	 * @param mutex 全局互斥量
	 * @param removeifunlock 如果加锁次数为1解锁后删除锁
	 * @param maxwaittime 最大等待加锁时间，单位为秒，程序内部使用的纳秒
	 */
	inline CMultiMutex(std::mutex* mutex, bool removeifunlock, uint64_t maxwaittime)
		: Mutex(mutex), RemoveIfUnlock(removeifunlock), WaitTimes(maxwaittime * 1000000000 / NanosecondSpan)
	{}

	void Lock(const T_Key& key, uint64_t waittimes = 0)
	{
		Mutex->lock();
		auto it = MultiMutexes.find(key);
		if(it != MultiMutexes.end()) {
			if(!it->second.Locked) {
				it->second.Locked = true;
				it->second.Shared = false;
			}
			else {
				Mutex->unlock();
				if(waittimes >= WaitTimes) {
					ThrowError(ERR_LOCK_TIME_EXCEED, "Can't lock " + key + " in " + std::to_string(WaitTimes * NanosecondSpan / 1000000000) + " seconds");
				}
				std::this_thread::sleep_for(std::chrono::nanoseconds(NanosecondSpan));
				Lock(key, waittimes + 1);
			}
		}
		else {
			MultiMutexes.emplace(key, CStatus(true, false));
		}
		Mutex->unlock();
	}

	void Unlock(const T_Key& key)
	{
		Mutex->lock();
		auto it = MultiMutexes.find(key);
		if(it != MultiMutexes.end()) {
			if(RemoveIfUnlock) {
				MultiMutexes.erase(it);
			}
			else {
				it->second.Locked = false;
				it->second.Shared = false;
			}
		}
		Mutex->unlock();
	}

	void LockShared(const T_Key& key, uint64_t waittimes = 0)
	{
		Mutex->lock();
		auto it = MultiMutexes.find(key);
		if(it != MultiMutexes.end()) {
			// 未被锁定
			if(!it->second.Locked) {
				it->second.Locked = true;
				it->second.Shared = true;
				it->second.Times = 1;
			}
			// 锁定且为共享
			else if(it->second.Shared) {
				it->second.Times++;
			}
			// 锁定且为非共享
			else {
				Mutex->unlock();
				if(waittimes >= WaitTimes) {
					ThrowError(ERR_LOCK_TIME_EXCEED, "Can't lock shared " + key + " in " + std::to_string(WaitTimes * NanosecondSpan / 1000000000) + " seconds");
				}
				std::this_thread::sleep_for(std::chrono::nanoseconds(NanosecondSpan));
				LockShared(key, waittimes + 1);
			}
		}
		else {
			MultiMutexes.emplace(key, CStatus(true, true));
		}
		Mutex->unlock();
	}

	void UnlockShared(const T_Key& key)
	{
		Mutex->lock();
		auto it = MultiMutexes.find(key);
		if(it != MultiMutexes.end()) {
			// 如果是多次共享锁定
			if(it->second.Times > 1) {
				it->second.Times--;
			}
			// 如果为1次锁定或未锁定
			else {
				if(RemoveIfUnlock) {
					MultiMutexes.erase(it);
				}
				else {
					it->second.Locked = false;
					it->second.Shared = false;
					it->second.Times = 0;
				}
			}
		}
		Mutex->unlock();
	}

	void Lock(std::deque<T_Key>& keys, uint64_t waittimes = 0, std::deque<T_Key>& locked_keys = {})
	{
		Mutex->lock();
		for(auto it_k = keys.begin(); it_k != keys.end();) {
			auto it = MultiMutexes.find(*it_k);
			if(it != MultiMutexes.end()) {
				if(!it->second.Locked) {
					it->second.Locked = true;
					it->second.Shared = false;
					locked_keys.push_back(*it_k);
					auto del_it_k = it_k;
					it_k++;
					keys.erase(del_it_k);
				}
				else {
					it_k++;
				}
			}
			else {
				MultiMutexes.emplace(*it_k, CStatus(true, false));
				locked_keys.push_back(*it_k);
				auto del_it_k = it_k;
				it_k++;
				keys.erase(del_it_k);
			}
		}
		Mutex->unlock();
		if(!keys.empty()) {
			if(waittimes >= WaitTimes) {
				Unlock(locked_keys);
				ThrowError(ERR_LOCK_TIME_EXCEED, "Can't lock keys in " + std::to_string(WaitTimes * NanosecondSpan / 1000000000) + " seconds");
			}
			//nanosleep(&WaitTime, nullptr);
			std::this_thread::sleep_for(std::chrono::nanoseconds(NanosecondSpan));
			Lock(keys, waittimes + 1);
		}
	}

	void Unlock(const std::deque<T_Key>& keys)
	{
		Mutex->lock();
		for(auto it_k = keys.begin(); it_k != keys.end(); it_k++) {
			auto it = MultiMutexes.find(*it_k);
			if(it != MultiMutexes.end()) {
				if(RemoveIfUnlock) {
					MultiMutexes.erase(it);
				}
				else {
					it->second.Locked = false;
					it->second.Shared = false;
				}
			}
		}
		Mutex->unlock();
	}

	void LockShared(std::deque<T_Key>& keys, uint64_t waittimes = 0, std::deque<T_Key>& locked_keys = {})
	{
		Mutex->lock();
		for(auto it_k = keys.begin(); it_k != keys.end();) {
			auto it = MultiMutexes.find(*it_k);
			if(it != MultiMutexes.end()) {
				// 未被锁定
				if(!it->second.Locked) {
					it->second.Locked = true;
					it->second.Shared = true;
					it->second.Times = 1;
					locked_keys.push_back(*it_k);
					auto del_it_k = it_k;
					it_k++;
					keys.erase(del_it_k);
				}
				// 锁定且为共享
				else if(it->second.Shared) {
					it->second.Times++;
					locked_keys.push_back(*it_k);
					auto del_it_k = it_k;
					it_k++;
					keys.erase(del_it_k);
				}
				// 锁定且为非共享
				else {
					it_k++;
				}
			}
			else {
				MultiMutexes.emplace(*it_k, CStatus(true, true));
				locked_keys.push_back(*it_k);
				auto del_it_k = it_k;
				it_k++;
				keys.erase(del_it_k);
			}
		}
		Mutex->unlock();
		if(!keys.empty()) {
			if(waittimes >= WaitTimes) {
				UnlockShared(locked_keys);
				ThrowError(ERR_LOCK_TIME_EXCEED, "Can't lock keys in " + std::to_string(WaitTimes * NanosecondSpan / 1000000000) + " seconds");
			}
			//nanosleep(&WaitTime, nullptr);
			std::this_thread::sleep_for(std::chrono::nanoseconds(NanosecondSpan));
			LockShared(keys, waittimes + 1);
		}
	}

	void UnlockShared(const std::deque<T_Key>& keys)
	{
		Mutex->lock();
		for(auto it_k = keys.begin(); it_k != keys.end(); it_k++) {
			auto it = MultiMutexes.find(*it_k);
			if(it != MultiMutexes.end()) {
				// 如果是多次共享锁定
				if(it->second.Times > 1) {
					it->second.Times--;
				}
				// 如果为1次锁定或未锁定
				else {
					if(RemoveIfUnlock) {
						MultiMutexes.erase(it);
					}
					else {
						it->second.Locked = false;
						it->second.Shared = false;
						it->second.Times = 0;
					}
				}
			}
		}
		Mutex->unlock();
	}

protected:
	struct CStatus
	{
		bool Locked : 1;
		bool Shared : 1;
		unsigned short Times : 14;
		CStatus(bool locked, bool shared)
			: Locked(locked), Shared(shared), Times(1)
		{}
	};

	std::mutex* Mutex;
	bool RemoveIfUnlock;
	std::unordered_map<T_Key, CStatus> MultiMutexes;
	uint64_t WaitTimes;/**< 等待次数 */
	const uint64_t NanosecondSpan = 100;/**< 每次锁定等待时间间隔，目前为100纳秒 */
};

}
