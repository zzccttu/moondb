#pragma once

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <ctype.h>

namespace MoonDb {

class CString : public std::string
{
public:
	inline CString():std::string(){}
	inline CString(const CString& str):std::string(str){}
	inline CString(const CString& str, size_t pos, size_t len = npos):std::string(str, pos, len){}
	inline CString(const char* s):std::string(s){}
	inline CString(const char* s, size_t n):std::string(s, n){}
	inline CString(size_t n, char c):std::string(n, c){}
	inline CString(const std::string str):std::string(str){}

	/**
	 * @brief explode函数用字符c将字符串劈开成字符串数组
	 * @param c 用于劈开字符串的字符
	 * @return 劈开后的字符串数组
	 */
	inline std::vector<CString> explode(const char& c) const
	{
		CString buff{""};
		std::vector<CString> v;

		for(auto n:*this)
		{
			if(n != c) {
				buff += n;
			}
			else if(n == c && buff != "") {
				v.push_back(buff);
				buff = "";
			}
		}
		if(buff != "") {
			v.push_back(buff);
		}

		return v;
	}

	/**
	 * @brief explode_set函数用字符c将字符串劈开成字符串set数组
	 * @param c 用于劈开字符串的字符
	 * @return 劈开后的字符串set数组
	 */
	inline std::set<CString> explode_set(const char& c) const
	{
		CString buff{""};
		std::set<CString> v;

		for(auto n:*this)
		{
			if(n != c) {
				buff += n;
			}
			else if(n == c && buff != "") {
				v.insert(buff);
				buff = "";
			}
		}
		if(buff != "") {
			v.insert(buff);
		}

		return v;
	}

	/**
	 * @brief explode 用字符串seq将源字符串劈成字符串数组
	 * @param sep 间隔的字符列表
	 * @return 劈开后的字符串数组
	 */
	inline std::vector<CString> explode(const std::string& sep) const
	{
		CString buff{""};
		std::vector<CString> ret;
		CString tmp;
		std::string::size_type pos_begin = this->find_first_not_of(sep);
		std::string::size_type comma_pos = 0;
		while (pos_begin != std::string::npos)
		{
			comma_pos = this->find(sep, pos_begin);
			if (comma_pos != std::string::npos)
			{
				tmp = this->substr(pos_begin, comma_pos - pos_begin);
				pos_begin = comma_pos + sep.length();
			}
			else
			{
				tmp = this->substr(pos_begin);
				pos_begin = comma_pos;
			}

			if (!tmp.empty())
			{
				ret.push_back(tmp);
				tmp.clear();
			}
		}
		return ret;
	}

	/**
	 * @brief explode_set 用字符串seq将源字符串劈成字符串set数组
	 * @param sep 间隔的字符列表
	 * @return 劈开后的字符串set数组
	 */
	inline std::set<CString> explode_set(const std::string& sep) const
	{
		CString buff{""};
		std::set<CString> ret;
		CString tmp;
		std::string::size_type pos_begin = this->find_first_not_of(sep);
		std::string::size_type comma_pos = 0;
		while (pos_begin != std::string::npos)
		{
			comma_pos = this->find(sep, pos_begin);
			if (comma_pos != std::string::npos)
			{
				tmp = this->substr(pos_begin, comma_pos - pos_begin);
				pos_begin = comma_pos + sep.length();
			}
			else
			{
				tmp = this->substr(pos_begin);
				pos_begin = comma_pos;
			}

			if (!tmp.empty())
			{
				ret.insert(tmp);
				tmp.clear();
			}
		}
		return ret;
	}

	/**
	 * @brief ltrim函数将字符串左侧的空白字符去掉
	 * @param charmask 去掉的左侧字符列表
	 * @return 返回去掉左侧空白的字符串
	 */
	inline CString ltrim(const std::string& charmask = " \t\n\r\v\f") const
	{
		int64_t str_size = static_cast<int64_t>(this->size());
		int64_t mask_size = static_cast<int64_t>(charmask.size());
		const char* str_mem_pos = &this->front();
		const char* mask_mem_pos = &charmask.front();

		// 从左侧找非空白字符
		int64_t first_not_mask_pos = std::numeric_limits<int64_t>::max();
		for(int64_t i = 0; i < str_size; i ++) {
			bool found_mask = false;
			for(int64_t j = 0; j < mask_size; j ++) {
				if(*(str_mem_pos + i) == *(mask_mem_pos + j)) {
					found_mask = true;
					break;
				}
			}
			if(!found_mask) {
				first_not_mask_pos = i;
				break;
			}
		}
		if(first_not_mask_pos == std::numeric_limits<int64_t>::max()) {
			return "";
		}
		return this->substr(static_cast<size_t>(first_not_mask_pos));
	}

	/**
	 * @brief rtrim函数将字符串右侧的空白字符去掉
	 * @param charmask 去掉的右侧字符列表
	 * @return 返回去掉右侧空白的字符串
	 */
	inline CString rtrim(const std::string& charmask = " \t\n\r\v\f") const
	{
		int64_t str_size = static_cast<int64_t>(this->size());
		int64_t mask_size = static_cast<int64_t>(charmask.size());
		const char* str_mem_pos = &this->front();
		const char* mask_mem_pos = &charmask.front();

		// 从右侧找非空白字符
		int64_t last_not_mask_pos = std::numeric_limits<int64_t>::min();
		for(int64_t i = str_size - 1; i >= 0; i --) {
			bool found_mask = false;
			for(int64_t j = 0; j < mask_size; j ++) {
				if(*(str_mem_pos + i) == *(mask_mem_pos + j)) {
					found_mask = true;
					break;
				}
			}
			if(!found_mask) {
				last_not_mask_pos = i;
				break;
			}
		}
		if(last_not_mask_pos == std::numeric_limits<int64_t>::min()) {
			return "";
		}

		return this->substr(0, static_cast<size_t>(last_not_mask_pos + 1));
	}

	/**
	 * @brief trim函数将字符串两侧的空白字符去掉
	 * @param charmask 去掉的两侧字符列表
	 * @return 返回去掉两侧空白的字符串
	 */
	inline CString trim(const std::string& charmask = " \t\n\r\v\f") const
	{
		int64_t str_size = static_cast<int64_t>(this->size());
		int64_t mask_size = static_cast<int64_t>(charmask.size());
		const char* str_mem_pos = &this->front();
		const char* mask_mem_pos = &charmask.front();

		// 从左侧找非空白字符
		int64_t first_not_mask_pos = std::numeric_limits<int64_t>::max();
		for(int64_t i = 0; i < str_size; i ++) {
			bool found_mask = false;
			for(int64_t j = 0; j < mask_size; j ++) {
				if(*(str_mem_pos + i) == *(mask_mem_pos + j)) {
					found_mask = true;
					break;
				}
			}
			if(!found_mask) {
				first_not_mask_pos = i;
				break;
			}
		}
		if(first_not_mask_pos == std::numeric_limits<int64_t>::max()) {
			return "";
		}

		// 从右侧找非空白字符
		int64_t last_not_mask_pos = std::numeric_limits<int64_t>::min();
		for(int64_t i = str_size - 1; i >= 0; i --) {
			bool found_mask = false;
			for(int64_t j = 0; j < mask_size; j ++) {
				if(*(str_mem_pos + i) == *(mask_mem_pos + j)) {
					found_mask = true;
					break;
				}
			}
			if(!found_mask) {
				last_not_mask_pos = i;
				break;
			}
		}

		return this->substr(static_cast<size_t>(first_not_mask_pos), static_cast<size_t>(last_not_mask_pos + 1 - first_not_mask_pos));
	}

	/**
	 * @brief to_upper将本字符串转为大写
	 * @return 返回大写字符串
	 */
	inline CString to_upper() const
	{
		size_t size = this->size();
		CString ret(size, '\0');
		for(size_t i = 0; i < size; i ++) {
			ret[i] = static_cast<char>(::toupper((*this)[i]));
		}
		return ret;
	}

	/**
	 * @brief to_lower将本字符串转为小写
	 * @return 返回小写字符串
	 */
	inline CString to_lower() const
	{
		size_t size = this->size();
		CString ret(size, '\0');
		for(size_t i = 0; i < size; i ++) {
			ret[i] = static_cast<char>(::tolower((*this)[i]));
		}
		return ret;
	}

	inline CString& operator= (const char* s)
	{
		this->assign(s);
		return *this;
	}

	inline CString& operator= (const std::string& s)
	{
		this->assign(s);
		return *this;
	}

	inline CString& operator= (const CString& s)
	{
		this->assign(s);
		return *this;
	}

	inline CString& operator= (char c)
	{
		this->assign(1, c);
		return *this;
	}
};

}
