#pragma once

#include <vector>
#include <map>

namespace MoonDb {

/**
 * CMap类包含vector和map两个数组，可以同时用T_Key或编号访问元素，主要用于存储字段等
 */
template <typename T_Key, typename T_Value>
class CMap
{
public:
	CMap() = default;

	CMap(size_t capacity)
	{
		reserve(capacity);
	}

	inline void reserve(size_t capacity)
	{
		Values.reserve(capacity);
	}

	inline T_Value& operator [](const T_Key& key)
	{
		return Values[Keys[key]];
	}

	inline const T_Value& operator [](const T_Key& key) const
	{
		return Values[Keys[key]];
	}

	inline T_Value& operator [](const size_t& i)
	{
		return Values[i];
	}

	inline const T_Value& operator [](const size_t& i) const
	{
		return Values[i];
	}

	inline void insert(const T_Key& key, const T_Value& value)
	{
		Values.push_back(std::move(value));
		Keys.insert(std::pair<T_Key, size_t>(key, Values.size() - 1));
	}

	inline T_Value& insert(const T_Key& key)
	{
		Values.resize(Values.size() + 1);
		Keys.emplace(key, Values.size() - 1);
		return Values[Values.size() - 1];
	}

	inline size_t size() const noexcept
	{
		return Values.size();
	}

	inline bool empty() const noexcept
	{
		return Values.size() == 0;
	}

protected:
	std::vector<T_Value> Values;
	std::map<T_Key, size_t> Keys;
};

}
