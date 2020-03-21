#pragma once

#include <vector>
#include <list>
#include <queue>
#include "crunningerror.hpp"
#include "ctime.hpp"
#include "crandom.hpp"
#include "functions.hpp"

namespace MoonDb {

/**
 * CObjectMap类包含vector和map两个数组，可以同时用T_Key或编号访问元素，主要用于存储字段等
 */
template <typename T_Value>
class CQueue
{
public:
	CQueue() noexcept : Size(0), Capacity(0), MaxSize(0)
	{}

	CQueue(size_t maxsize, size_t capacity)
	{
		initialize(maxsize, capacity);
	}

	inline void clear() noexcept
	{
		Keys.clear();
		while (!Deleted.empty()) {
			Deleted.pop();
		}
		Size = 0;
		Capacity = 0;
		MaxSize = 0;
		Contents.clear();
	}

	inline void initialize(size_t maxsize, size_t capacity)
	{
		if(maxsize < capacity) {
			capacity = maxsize;
		}
		Size = 0;
		MaxSize = maxsize;
		Capacity = capacity;
		Contents.resize(capacity);
	}

	inline void reserve(size_t capacity)
	{
		capacity = std::min(capacity, static_cast<size_t>(MaxSize));
		if(capacity == MaxSize && Capacity == MaxSize) {
			ThrowError(ERR_MEMORY_ALLOCATE, "Reach maximal rows(" + num_to_string(size_t(capacity)) + ") in the class CQueue.");
			return;
		}
		if(capacity > Capacity) {
			Contents.resize(capacity);
			Capacity = capacity;
		}
	}

	inline size_t capicty() const noexcept
	{
		return Capacity;
	}

	inline typename std::list<size_t>::iterator begin() noexcept
	{
		return Keys.begin();
	}

	inline typename std::list<size_t>::iterator end() noexcept
	{
		return Keys.end();
	}

	inline T_Value* at(const typename std::list<size_t>::iterator& it)
	{
		return &Contents[*it];
	}

	inline const T_Value* at(const typename std::list<size_t>::iterator& it) const
	{
		return &Contents[*it];
	}

	inline T_Value* operator [](const typename std::list<size_t>::iterator& it)
	{
		return at(it);
	}

	inline const T_Value* operator [](const typename std::list<size_t>::iterator& it) const
	{
		return at(it);
	}

	inline T_Value& operator [](const size_t& iid)
	{
		return Contents[iid];
	}

	inline T_Value* push()
	{
		size_t iid = 0;
		if(Deleted.empty()) {
			if(Capacity == Size) {
				reserve(static_cast<size_t>(::ceil(Size + 1) * 1.5));
			}
			iid = Size;
			Keys.emplace_back(Size);
			Size++;
		}
		else {
			iid = Deleted.front();
			Deleted.pop();
			Keys.emplace_back(iid);
		}
		return &Contents[iid];
	}

	inline size_t size() const noexcept
	{
		return Keys.size();
	}

	inline bool empty() const noexcept
	{
		return Keys.size() == 0;
	}

	inline void erase(const typename std::list<size_t>::iterator& it)
	{
		Deleted.emplace(*it);
		Keys.erase(it);
	}

protected:
	std::list<size_t> Keys;
	std::queue<size_t> Deleted;
	size_t Size;
	size_t Capacity;
	size_t MaxSize;
	std::vector<T_Value> Contents;
};

}
