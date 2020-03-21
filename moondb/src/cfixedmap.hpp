#pragma once

#include <vector>
#include <map>
#include <queue>
#include "crunningerror.hpp"
#include "ctime.hpp"
#include "crandom.hpp"
#include "functions.hpp"

namespace MoonDb {

/**
 * CFixedMap类包含vector和map两个数组，可以同时用T_Key或编号访问元素，主要用于存储字段等，仅当添加或修改数据时才更新过期时间，读取时不修改
 */
template <typename T_Key>
class CFixedMap
{
public:
	CFixedMap() noexcept : Size(0), Capacity(0), MaxSize(0), RowLength(0), Contents(nullptr), IfCollectGarbage(false)
	{}

	CFixedMap(uint64_t maxsize, uint64_t rowlength, uint64_t capacity)
	{
		initialize(maxsize, rowlength, capacity);
	}

	~CFixedMap() noexcept
	{
		free(Contents);
	}

	inline void initialize(uint64_t maxsize, uint64_t rowlength, uint64_t capacity)
	{
		Size = 0;
		MaxSize = maxsize;
		RowLength = rowlength;
		Capacity = capacity;
		IfCollectGarbage = false;
		Contents = ::malloc(uint64_t(capacity) * uint64_t(rowlength));
		if(nullptr == Contents) {
			ThrowError(ERR_MEMORY_ALLOCATE, "CFixedMap failed to allocate " + num_to_string(uint64_t(capacity) * uint64_t(rowlength)) + " bytes.");
		}
	}

	inline void reserve(uint64_t capacity)
	{
		capacity = std::min(capacity, MaxSize);
		if(capacity == MaxSize && Capacity == MaxSize) {
			ThrowError(ERR_MEMORY_ALLOCATE, "Reach maximal rows(" + num_to_string(uint64_t(capacity)) + ") in the class CFixedMap.");
			return;
		}
		if(capacity > Capacity) {
			void* more_mem = ::realloc(Contents, uint64_t(capacity) * uint64_t(RowLength));
			if(nullptr == more_mem) {
				ThrowError(ERR_MEMORY_ALLOCATE, "Reallocate memory (function::realloc, " + num_to_string(uint64_t(capacity) * uint64_t(RowLength)) + ") in the class CFixedMap failed.");
				return;
			}
			Contents = more_mem;
			Capacity = capacity;
		}
	}

	inline void* at(const T_Key& key)
	{
		auto it = Keys.find(key);
		if(it != Keys.end()) {
			if(0 == it->second.ExpiredTime || it->second.ExpiredTime > CTime::Now()) {
				return GetRowPointer(it->second.Position);
			}
			else {
				Delete(it);
			}
		}
		return nullptr;
	}

	inline const void* at(const T_Key& key) const
	{
		auto it = Keys.find(key);
		if(it != Keys.end()) {
			if(0 == it->second.ExpiredTime || it->second.ExpiredTime > CTime::Now()) {
				return GetRowPointer(it->second.Position);
			}
		}
		return nullptr;
	}

	inline void* operator [](const T_Key& key)
	{
		return at(key);
	}

	inline const void* operator [](const T_Key& key) const
	{
		return at(key);
	}

	inline void* insert(const T_Key& key, uint32_t lifetime = 0)
	{
		if(lifetime > 0) {
			IfCollectGarbage = true;
		}
		auto it = Keys.find(key);
		if(it != Keys.end()) {
			return nullptr;
		}
		else {
			uint64_t pos = 0;
			if(IfCollectGarbage && (CRandom()(0, 1000) == 500 || (Deleted.empty() && Size == MaxSize))) {
				collect_garbage();
			}
			std::chrono::high_resolution_clock::rep expiredtime = lifetime > 0 ? CTime::Now() + lifetime * CTime::NanoTime : 0;
			if(Deleted.empty()) {
				if(Capacity == Size) {
					reserve(static_cast<uint64_t>(::ceil(Size + 1) * 1.5));
				}
				pos = Size;
				Keys.emplace(key, CValue(Size, expiredtime));
				Size++;
			}
			else {
				pos = Deleted.front();
				Deleted.pop();
				Keys.emplace(key, CValue(pos, expiredtime));
			}
			return GetRowPointer(pos);
		}
	}

	inline void* update(const T_Key& key, uint32_t lifetime = 0)
	{
		auto it = Keys.find(key);
		if(it != Keys.end()) {
			it->second.ExpiredTime = lifetime > 0 ? CTime::Now() + lifetime * CTime::NanoTime : 0;
			return GetRowPointer(it->second.Position);
		}
		else {
			return nullptr;
		}
	}

	inline void* replace(const T_Key& key, uint32_t lifetime = 0)
	{
		if(lifetime > 0) {
			IfCollectGarbage = true;
		}
		uint64_t pos;
		std::chrono::high_resolution_clock::rep expiredtime = lifetime > 0 ? CTime::Now() + lifetime * CTime::NanoTime : 0;
		auto it = Keys.find(key);
		if(it != Keys.end()) {
			pos = it->second.Position;
			it->second.ExpiredTime = expiredtime;
		}
		else {
			if(IfCollectGarbage && (CRandom()(0, 1000) == 500 || (Deleted.empty() && Size == MaxSize))) {
				collect_garbage();
			}
			if(Deleted.empty()) {
				if(Capacity == Size) {
					reserve(static_cast<uint64_t>(::ceil(Size + 1) * 1.5));
				}
				pos = Size;
				Keys.emplace(key, CValue(Size, expiredtime));
				Size++;
			}
			else {
				pos = Deleted.front();
				Deleted.pop();
				Keys.emplace(key, CValue(pos, expiredtime));
			}
		}

		return GetRowPointer(pos);
	}

	inline size_t size() const noexcept
	{
		return Keys.size();
	}

	inline bool empty() const noexcept
	{
		return Keys.size() == 0;
	}

	inline bool erase(const T_Key& key)
	{
		auto it = Keys.find(key);
		if(it != Keys.end()) {
			Delete(it);
			return true;
		}
		return false;
	}

	inline void collect_garbage()
	{
		auto timestamp = CTime::Now();
		for(auto it = Keys.begin(); it != Keys.end();) {
			if(it->second.ExpiredTime > 0 && it->second.ExpiredTime <= timestamp) {
				Deleted.emplace(it->second.Position);
				auto del_it = it;
				it++;
				Keys.erase(del_it);
			}
			else {
				++it;
			}
		}
	}

protected:
	struct CValue {
		uint64_t Position;
		std::chrono::high_resolution_clock::rep ExpiredTime;
		CValue() = default;
		CValue(uint64_t pos, std::chrono::high_resolution_clock::rep exptime)
			: Position(pos), ExpiredTime(exptime) {}
	};
	std::map<T_Key, CValue> Keys;
	std::queue<uint64_t> Deleted;
	uint64_t Size;
	uint64_t Capacity;
	uint64_t MaxSize;
	uint64_t RowLength;
	void* Contents;
	bool IfCollectGarbage;

	inline void* GetRowPointer(uint64_t pos) noexcept
	{
		return static_cast<char*>(Contents) + pos * RowLength;
	}

	inline void Delete(typename std::map<T_Key, CValue>::iterator& it)
	{
		Deleted.emplace(it->second.Position);
		Keys.erase(it);
		if(IfCollectGarbage && CRandom()(0, 1000) == 500) {
			collect_garbage();
		}
	}
};

}
