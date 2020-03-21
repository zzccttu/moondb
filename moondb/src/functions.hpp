#pragma once

#include <vector>
#include <set>
#include <string>
#include <cmath>
#include <sstream>

#include <boost/algorithm/string/replace.hpp>

#include "setting.h"
#include "ciconv.hpp"
#include "../library/md5.h"
#include "../library/sha1.h"
#include "../library/base64.h"

namespace MoonDb {

/**
 * @brief explode函数用字符c将字符串劈开成字符串数组
 * @param str 待劈开的字符串
 * @param c 用于劈开字符串的字符
 * @return 劈开后的字符串数组
 */
inline std::vector<std::string> explode(const std::string& str, const char& c) noexcept
{
	std::string buff{""};
	std::vector<std::string> v;

	for(auto n:str)
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
 * @brief explode 用字符串seq将源字符串劈成字符串数组
 * @param str 待劈开的字符串
 * @param sep 间隔的字符列表
 * @return 劈开后的字符串数组
 */
inline std::vector<std::string> explode(const std::string& str, const std::string& sep) noexcept
{
	std::string buff{""};
	std::vector<std::string> ret;
	std::string tmp;
	std::string::size_type pos_begin = str.find_first_not_of(sep);
	std::string::size_type comma_pos = 0;
	while (pos_begin != std::string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != std::string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
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
 * @brief explode_set函数用字符c将字符串劈开成字符串set数组
 * @param str 待劈开的字符串
 * @param c 用于劈开字符串的字符
 * @return 劈开后的字符串set数组
 */
inline std::set<std::string> explode_set(const std::string& str, const char& c) noexcept
{
	std::string buff{""};
	std::set<std::string> v;

	for(auto n:str)
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
 * @brief explode_set 用字符串seq将源字符串劈成字符串set数组
 * @param str 待劈开的字符串
 * @param sep 间隔的字符列表
 * @return 劈开后的字符串set数组
 */
inline std::set<std::string> explode_set(const std::string& str, const std::string& sep) noexcept
{
	std::string buff{""};
	std::set<std::string> ret;
	std::string tmp;
	std::string::size_type pos_begin = str.find_first_not_of(sep);
	std::string::size_type comma_pos = 0;
	while (pos_begin != std::string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != std::string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
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

inline int64_t _ltrim_find(const std::string& str, const std::string& charmask) noexcept
{
	int64_t str_size = static_cast<int64_t>(str.size());
	int64_t mask_size = static_cast<int64_t>(charmask.size());
	const char* str_mem_pos = &str.front();
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
	return first_not_mask_pos;
}

/**
 * @brief ltrim函数将字符串str左侧的空白字符去掉
 * @param str 待处理的字符串
 * @param charmask 去掉的左侧字符列表
 */
inline void ltrim(std::string& str, const std::string& charmask = " \t\n\r\v\f") noexcept
{
	int64_t first_not_mask_pos = _ltrim_find(str, charmask);
	if(first_not_mask_pos == std::numeric_limits<int64_t>::max()) {
		str.clear();
	}
	else {
		str.erase(0, static_cast<size_t>(first_not_mask_pos));
	}
}

/**
 * @brief ltrim_copy函数返回将字符串str左侧的空白字符去掉的字符串
 * @param str 待处理的字符串
 * @param charmask 去掉的左侧字符列表
 * @return 返回去除左侧空白字符的字符串
 */
inline std::string ltrim_copy(const std::string& str, const std::string& charmask = " \t\n\r\v\f") noexcept
{
	int64_t first_not_mask_pos = _ltrim_find(str, charmask);
	if(first_not_mask_pos == std::numeric_limits<int64_t>::max()) {
		return "";
	}
	return str.substr(static_cast<size_t>(first_not_mask_pos));
}

inline int64_t _rtrim_find(const std::string& str, const std::string& charmask) noexcept
{
	int64_t str_size = static_cast<int64_t>(str.size());
	int64_t mask_size = static_cast<int64_t>(charmask.size());
	const char* str_mem_pos = &str.front();
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
	return last_not_mask_pos;
}

/**
 * @brief rtrim函数将字符串str右侧的空白字符去掉
 * @param str 待处理的字符串
 * @param charmask 去掉的右侧字符列表
 * @return 返回去掉右侧空白的字符串
 */
inline void rtrim(std::string& str, const std::string& charmask = " \t\n\r\v\f") noexcept
{
	int64_t last_not_mask_pos = _rtrim_find(str, charmask);
	if(last_not_mask_pos == std::numeric_limits<int64_t>::min()) {
		str.clear();
	}
	else {
		str.resize(static_cast<size_t>(last_not_mask_pos + 1));
	}
}

/**
 * @brief rtrim_copy函数返回将字符串str右侧的空白字符去掉的字符串
 * @param str 待处理的字符串
 * @param charmask 去掉的右侧字符列表
 * @return 返回去除右侧空白字符的字符串
 */
inline std::string rtrim_copy(const std::string& str, const std::string& charmask = " \t\n\r\v\f") noexcept
{
	int64_t last_not_mask_pos = _rtrim_find(str, charmask);
	if(last_not_mask_pos == std::numeric_limits<int64_t>::min()) {
		return "";
	}
	return str.substr(0, static_cast<size_t>(last_not_mask_pos + 1));
}

inline void _trim_find(const std::string& str, const std::string& charmask, int64_t& first_not_mask_pos, int64_t& last_not_mask_pos) noexcept
{
	int64_t str_size = static_cast<int64_t>(str.size());
	int64_t mask_size = static_cast<int64_t>(charmask.size());
	const char* str_mem_pos = &str.front();
	const char* mask_mem_pos = &charmask.front();

	// 从左侧找非空白字符
	first_not_mask_pos = std::numeric_limits<int64_t>::max();
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
		return;
	}

	// 从右侧找非空白字符
	last_not_mask_pos = std::numeric_limits<int64_t>::min();
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
}

/**
 * @brief trim函数将字符串str两侧的空白字符去掉
 * @param str 待处理的字符串
 * @param charmask 去掉的两侧字符列表
 * @return 返回去掉两侧空白的字符串
 */
inline void trim(std::string& str, const std::string& charmask = " \t\n\r\v\f") noexcept
{
	int64_t first_not_mask_pos, last_not_mask_pos;
	_trim_find(str, charmask, first_not_mask_pos, last_not_mask_pos);
	if(first_not_mask_pos == std::numeric_limits<int64_t>::max()) {
		str.clear();
	}
	else {
		str.assign(str, static_cast<size_t>(first_not_mask_pos), static_cast<size_t>(last_not_mask_pos + 1 - first_not_mask_pos));
	}
}

/**
 * @brief trim函数（C语言版本）将字符串str两侧的空白字符去掉
 * @param str 待处理的字符串
 * @param charmask 去掉的两侧字符列表
 * @return 返回去掉两侧空白的字符串
 */
inline void trim(char* src, const char charmask[] = " \t\n\r\v\f") noexcept
{
	char* begin = src;
	char* end = src;

	while(*end++);

	if(begin == end) return;

	while(strchr(charmask, *begin) != nullptr) {
		++begin;
	}
	while(*end == '\0' || strchr(charmask, *end) != nullptr) {
		--end;
	}

	if(begin > end) {
		*src = '\0';
		return;
	}
	//printf("begin=%1.1s; end=%1.1s\n", begin, end);
	while(begin != end) {
		*src++ = *begin++;
	}

	*src++ = *end;
	*src = '\0';
}

/**
 * @brief trim_copy函数返回将字符串str两侧的空白字符去掉的字符串
 * @param str 待处理的字符串
 * @param charmask 去掉的两侧字符列表
 * @return 返回去除两侧空白字符的字符串
 */
inline std::string trim_copy(const std::string& str, const std::string& charmask = " \t\n\r\v\f") noexcept
{
	int64_t first_not_mask_pos, last_not_mask_pos;
	_trim_find(str, charmask, first_not_mask_pos, last_not_mask_pos);
	if(first_not_mask_pos == std::numeric_limits<int64_t>::max()) {
		return "";
	}
	return str.substr(static_cast<size_t>(first_not_mask_pos), static_cast<size_t>(last_not_mask_pos + 1 - first_not_mask_pos));
}

/**
 * @brief to_upper_copy将字符串转为大写
 * @param str 原字符串
 * @return 返回大写字符串
 */
inline std::string to_upper_copy(const std::string& str) noexcept
{
	std::string ret(str.size(), '\0');
	const char* src = &str.front();
	char* des = &ret.front();
	while(*src) {
		if(*src >= 'a' && *src <= 'z') {
			*des = *src - 32;
		}
		else {
			*des = *src;
		}
		src++;
		des++;
	}
	return ret;
}

/**
 * @brief to_upper将字符串转为大写
 * @param str 待处理字符串
 */
inline void to_upper(std::string& str) noexcept
{
	char* p = &str.front();
	while(*p) {
		if(*p >= 'a' && *p <= 'z') {
			*p -= 32;
		}
		p++;
	}
}

/**
 * @brief to_lower_copy将字符串转为小写
 * @param str 原字符串
 * @return 返回小写字符串
 */
inline std::string to_lower_copy(const std::string& str) noexcept
{
	std::string ret(str.size(), '\0');
	const char* src = &str.front();
	char* des = &ret.front();
	while(*src) {
		if(*src >= 'A' && *src <= 'Z') {
			*des = *src + 32;
		}
		else {
			*des = *src;
		}
		src++;
		des++;
	}
	return ret;
}

/**
 * @brief to_lower将字符串转为小写
 * @param str 待处理字符串
 */
inline void to_lower(std::string& str) noexcept
{
	char* p = &str.front();
	while(*p) {
		if(*p >= 'A' && *p <= 'Z') {
			*p += 32;
		}
		p++;
	}
}

/**
 * @brief is_digit 是否为数字，如果为空返回false
 * @param str 待检测字符串
 * @return 如果为数字返回true，反之返回false
 */
inline bool is_digit(const std::string& str) noexcept
{
	if(str.empty()) {
		return false;
	}
	const char* p = &str.front();
	while(*p) {
		if(*p < '0' || *p > '9') {
			return false;
		}
		p++;
	}
	return true;
}

/**
 * @brief is_word 判断一个字符串的第一个字符是否为字母或下划线、后面的字符是否为数字字母下划线。
 * @param str dai待判断的字符串
 * @return 是否是个有效名称
 */
inline bool is_word(const std::string& str) noexcept
{
	if(str.empty()) {
		return false;
	}
	const char* p = &str.front();
	if(!(*p >= 'A' && *p <= 'Z') && !(*p >= 'a' && *p <= 'z') && *p != '_') {
		return false;
	}
	p++;
	while(*p) {
		if(!(*p >= '0' && *p <= '9') && !(*p >= 'A' && *p <= 'Z') && !(*p >= 'a' && *p <= 'z') && *p != '_') {
			return false;
		}
		p++;
	}
	return true;
}

/**
 * @brief is_capacity 是否为容量，例如1024K、2M，如果只为数字单位则为字节
 * @param str 待检测字符串
 * @return 返回是否符合要求
 */
inline bool is_capacity(const std::string& str) noexcept
{
	size_t size = str.size();
	if(size == 0) {
		return false;
	}
	if(str[0] < '1' || str[0] > '9') {
		return false;
	}
	size_t lastcharpos = size - 1;
	if(str[lastcharpos] == 'K' || str[lastcharpos] == 'k' || str[lastcharpos] == 'M' || str[lastcharpos] == 'm') {
		lastcharpos--;
	}
	for(size_t i = 1; i <= lastcharpos; i++) {
		if(str[i] < '0' || str[i] > '9') {
			return false;
		}
	}
	return true;
}

/**
 * @brief to_current_os_path 转换为操作系统使用的路径
 * @param path 路径
 */
inline void to_current_os_path(std::string& path) noexcept
{
	size_t size = path.size();
	for(size_t i = 0; i < size; i++) {
#if defined(_WIN32)
		if('/' == path[i]) {
			path[i] = '\\';
		}
#else
		if('\\' == path[i]) {
			path[i] = '/';
		}
#endif
	}
}

/**
 * @brief pad_left_copy函数给str字符串添加前导字符串pad_string直至长度达到pad_length
 * @param str 待处理字符串
 * @param pad_length 添加前导字符串后的长度
 * @param pad_string 前导字符串
 * @return 处理后的字符串
 */
inline std::string pad_left_copy(const std::string& str, size_t pad_length, const std::string& pad_string) noexcept
{
	if(pad_length > str.size() && pad_string.size() > 0) {
		size_t ps_len = pad_length - str.size();
		std::string ret;
		ret.reserve(pad_length);
		size_t pad_string_len = pad_string.size();
		size_t i;
		for(i = pad_string_len; i < ps_len; i += pad_string_len) {
			ret += pad_string;
		}
		ret += pad_string.substr(0, ps_len + pad_string_len - i) + str;
		return ret;
	}
	else {
		return str;
	}
}

/**
 * @brief pad_left_copy函数给str字符串添加前导字符pad_char直至长度达到pad_length
 * @param str 待处理字符串
 * @param pad_length 添加前导字符串后的长度
 * @param pad_char 前导字符
 * @return 处理后的字符串
 */
inline std::string pad_left_copy(const std::string& str, size_t pad_length, char pad_char) noexcept
{
	if(pad_length > str.size()) {
		return std::string(pad_length - str.size(), pad_char) + str;
	}
	else {
		return str;
	}
}

/**
 * @brief pad_left函数给str字符串添加前导字符串pad_string直至长度达到pad_length
 * @param str 待处理字符串
 * @param pad_length 添加前导字符串后的长度
 * @param pad_string 前导字符串
 */
inline void pad_left(std::string& str, size_t pad_length, const std::string& pad_string) noexcept
{
	size_t orisize = str.size();
	size_t padstrsize = pad_string.size();
	if(pad_length > orisize && padstrsize > 0) {
		str.resize(pad_length);
		char* p = &str.front();
		size_t ps_len = pad_length - orisize;
		for(long long i = static_cast<long long>(orisize - 1); i >= 0; i--) {
			*(p + i + ps_len) = *(p + i);
		}
		p = &str.front();
		size_t pos = 0;
		while(true) {
			for(size_t i = 0; i < padstrsize; i++) {
				*p = pad_string[i];
				p++;
				pos++;
				if(pos >= ps_len) {
					return;
				}
			}
		}
	}
}

/**
 * @brief pad_left函数给str字符串添加前导字符pad_char直至长度达到pad_length
 * @param str 待处理字符串
 * @param pad_length 添加前导字符串后的长度
 * @param pad_char 前导字符
 */
inline void pad_left(std::string& str, size_t pad_length, char pad_char) noexcept
{
	size_t orisize = str.size();
	if(pad_length > orisize) {
		str.resize(pad_length);
		char* p = &str.front();
		size_t ps_len = pad_length - orisize;
		for(long long i = static_cast<long long>(orisize - 1); i >= 0; i--) {
			*(p + i + ps_len) = *(p + i);
		}
		p = &str.front();
		for(size_t i = 0; i < ps_len; i++) {
			*(p + i) = pad_char;
		}
	}
}

/**
 * @brief pad_right_copy函数给str字符串添加后缀字符串pad_string直至长度达到pad_length
 * @param str 待处理字符串
 * @param pad_length 添加后缀字符串后的长度
 * @param pad_string 后缀字符串
 * @return 处理后的字符串
 */
inline std::string pad_right_copy(const std::string& str, size_t pad_length, const std::string& pad_string) noexcept
{
	if(pad_length > str.size() && pad_string.size() > 0) {
		size_t ps_len = pad_length - str.size();
		std::string ret = str;
		ret.reserve(pad_length);
		size_t pad_string_len = pad_string.size();
		size_t i;
		for(i = pad_string_len; i < ps_len; i += pad_string_len) {
			ret += pad_string;
		}
		ret += pad_string.substr(0, ps_len + pad_string_len - i);
		return ret;
	}
	else {
		return str;
	}
}

/**
 * @brief pad_right_copy函数给str字符串添加后缀字符pad_char直至长度达到pad_length
 * @param str 待处理字符串
 * @param pad_length 添加后缀字符串后的长度
 * @param pad_char 后缀字符
 * @return 处理后的字符串
 */
inline std::string pad_right_copy(const std::string& str, size_t pad_length, char pad_char) noexcept
{
	if(pad_length > str.size()) {
		return str + std::string(pad_length - str.size(), pad_char);
	}
	else {
		return str;
	}
}

/**
 * @brief pad_right函数给str字符串添加后缀字符串pad_string直至长度达到pad_length
 * @param str 待处理字符串
 * @param pad_length 添加后缀字符串后的长度
 * @param pad_string 后缀字符串
 */
inline void pad_right(std::string& str, size_t pad_length, const std::string& pad_string) noexcept
{
	size_t orisize = str.size();
	size_t padstrsize = pad_string.size();
	if(pad_length > orisize && padstrsize > 0) {
		str.resize(pad_length);
		char* p = &str.front() + orisize;
		size_t pos = orisize;
		while(true) {
			for(size_t i = 0; i < padstrsize; i++) {
				*p = pad_string[i];
				p++;
				pos++;
				if(pos >= pad_length) {
					return;
				}
			}
		}
	}
}

/**
 * @brief pad_right函数给str字符串添加后缀字符pad_char直至长度达到pad_length
 * @param str 待处理字符串
 * @param pad_length 添加后缀字符串后的长度
 * @param pad_char 后缀字符
 */
inline void pad_right(std::string& str, size_t pad_length, char pad_char) noexcept
{
	str.resize(pad_length, pad_char);
}

/**
 * @brief strlen 按照字符集计算字符串长度
 * @param str 字符串
 * @param charset 字符集
 * @return 长度
 */
inline size_t strlen(const std::string& str, CIconv::CharsetType charset = CIconv::CHARSET_ASCII)
{
	return CIconv::strlen(str, charset);
}

/**
 * @brief substr 截取字符串
 * @param str 原字符串
 * @param start 起始位置，可为负值
 * @param length 长度，可为负值
 * @param charset 字符集
 * @return 返回截取后的字符串
 */
inline std::string substr(const std::string& str, const int64_t& start, const int64_t& length = std::numeric_limits<int64_t>::max(), CIconv::CharsetType charset = CIconv::CHARSET_ASCII)
{
	return CIconv::substr(str, start, length, charset);
}

/**
 * @brief truncate 将字符串截断
 * @param str 字符串
 * @param length 截断长度
 * @param charnum 字符数
 * @param charset 字符集
 */
inline void truncate(std::string& str, uint32_t length, uint32_t& charnum, CIconv::CharsetType charset = CIconv::CHARSET_ASCII)
{
	CIconv::truncate(str, length, charnum, charset);
}

/**
 * @brief iconv 将字符串str从from字符集转为to字符集
 *		example: iconv("汉字", "GBK", "UTF-8")
 * @param str 待处理字符串
 * @param from 源字符串字符集
 * @param to 目标字符串字符集
 * @return 转换后的字符串
 */
inline std::string iconv(const std::string& str, CIconv::CharsetType from, CIconv::CharsetType to)
{
	return CIconv::iconv(str, from, to);
}

/**
 * @brief str_replace 在str字符串中用search替换replace
 * @param search 被替换字符
 * @param replace 替换字符
 * @param str 待处理字符串
 */
inline void str_replace(char search, char replace, std::string& str) noexcept
{
	size_t size = str.size();
	for(size_t i = 0; i < size; i ++) {
		if(search == str[i]) {
			str[i] = replace;
		}
	}
}

/**
 * @brief str_replace 在str字符串中用search替换replace
 * @param search 被替换字符串
 * @param replace 替换字符串
 * @param str 待处理字符串
 */
inline void str_replace(const std::string& search, const std::string& replace, std::string&str) noexcept
{
	boost::replace_all(str, search, replace);
}

/**
 * @brief standardize_path 规范化文件或目录的路径分隔符
 * @param path 路径
 */
inline void standardize_path(std::string& path) noexcept
{
	size_t size = path.size();
	if(0 == size) {
		return;
	}
#if DIRECTORY_SEPARATOR == '/'
#define DIRECTORY_SEPARATOR_SEARCH '\\'
#else
#define DIRECTORY_SEPARATOR_SEARCH '/'
#endif
	str_replace(DIRECTORY_SEPARATOR_SEARCH, DIRECTORY_SEPARATOR, path);
//	if(path[size - 1] != DIRECTORY_SEPARATOR) {
//		path += DIRECTORY_SEPARATOR;
//	}
}

/**
 * @brief sha1 返回str字符串的sha1值
 * @param str 待处理字符串
 * @param raw 若为true返回20字节二进制字符串（big endian），反之返回40字节16进制字符串
 * @return
 */
inline std::string sha1(const std::string& str, bool raw = false) noexcept
{
	SHA1 sha1;
	if(raw) {
		std::string ret(20, '\0');
		sha1.SHA_GO(str.data(), &ret.front(), true);
		return ret;
	}
	else {
		std::string ret(40, '\0');
		sha1.SHA_GO(str.data(), &ret.front(), false);
		return ret;
	}
}

/**
 * @brief sha1 返回str字符串的md5值
 * @param str 待处理字符串
 * @param raw 若为true返回16字节二进制字符串，反之返回32字节16进制字符串
 * @return
 */
inline std::string md5(const std::string& str, bool raw = false) noexcept
{
	MD5 md5(str);
	return md5.md5(raw);
}

/**
 * @brief base64_encode 基于base64转码
 * @param str 待处理字符串
 * @return 转码后的字符串
 */
inline std::string base64_encode(const std::string& str) noexcept
{
	return Base64::Encode(static_cast<const unsigned char *>(static_cast<const void *>(str.data())), static_cast<int>(str.size()));
}

/**
 * @brief base64_decode 基于base64解码
 * @param str 待处理的字符串
 * @return 解码后的字符串
 */
inline std::string base64_decode(const std::string& str) noexcept
{
	return Base64::Decode(str.data(), static_cast<int>(str.size()));
}

template<typename T>
struct num_limits
{
	inline static T min() noexcept { return T(); }
	inline static T max() noexcept { return T(); }
};

template<>
struct num_limits<int8_t>
{
	inline static int8_t min() noexcept { return -128; }
	inline static int8_t max() noexcept { return 127; }
};

template<>
struct num_limits<uint8_t>
{
	inline static uint8_t min() noexcept { return 0; }
	inline static uint8_t max() noexcept { return 255; }
};

template<>
struct num_limits<int16_t>
{
	inline static int16_t min() noexcept { return -32768; }
	inline static int16_t max() noexcept { return 32767; }
};

template<>
struct num_limits<uint16_t>
{
	inline static uint16_t min() noexcept { return 0; }
	inline static uint16_t max() noexcept { return 65535; }
};

template<>
struct num_limits<int32_t>
{
	inline static int32_t min() noexcept { return -2147483647L - 1; }
	inline static int32_t max() noexcept { return 2147483647L; }
};

template<>
struct num_limits<uint32_t>
{
	inline static uint32_t min() noexcept { return 0; }
	inline static uint32_t max() noexcept { return 4294967295UL; }
};

template<>
struct num_limits<int64_t>
{
	inline static int64_t min() noexcept { return -9223372036854775807LL - 1; }
	inline static int64_t max() noexcept { return 9223372036854775807LL; }
};

template<>
struct num_limits<uint64_t>
{
	inline static uint64_t min() noexcept { return 0; }
	inline static uint64_t max() noexcept { return 18446744073709551615ULL; }
};

template<>
struct num_limits<__int128_t>
{
	inline static __int128_t min() noexcept
	{
		return - (__int128_t(num_limits<int64_t>::max()) << 64) - __int128_t(num_limits<uint64_t>::max()) - 1;
	}
	inline static __int128_t max() noexcept
	{
		return (__int128_t(num_limits<int64_t>::max()) << 64) + __int128_t(num_limits<uint64_t>::max());
	}
};

template<>
struct num_limits<__uint128_t>
{
	inline static __uint128_t min() noexcept { return 0; }
	inline static __uint128_t max() noexcept
	{
		return (__uint128_t(num_limits<uint64_t>::max()) << 64) + __uint128_t(num_limits<uint64_t>::max());
	}
};

/**
 * @brief to_string函数将v转换为十进制字符串
 * @param v 128位无符号整数
 * @return 十进制字符串
 */
inline std::string num_to_string(__uint128_t v) noexcept
{
	std::string s(40, '\0');
	char* s_pointer = &s.front();
	uint8_t i = 0;
	do {
		s_pointer[i++] = v % 10 + '0';
		v /= 10;
	} while(v != 0);
	s.resize(i);
	uint8_t j = 0;
	i--;
	while(j < i) {
		char tmp = s_pointer[j];
		s_pointer[j] = s_pointer[i];
		s[i] = tmp;
		j ++;
		i --;
	}

	return s;
}

/**
 * @brief operator << 输出128位无符号整数
 * @param os 输出流
 * @param v 整数
 * @return
 */
inline std::ostream & operator << (std::ostream & os, const __uint128_t& v) noexcept
{
	os << pad_left_copy(num_to_string(v), static_cast<size_t>(os.width()), ' ');
	return os;
}

/**
 * @brief to_string函数将v转换为十进制字符串
 * @param v 128位有符号整数
 * @return 十进制字符串
 */
inline std::string num_to_string(__int128_t v) noexcept
{
	std::string s(41, '\0');
	char* s_pointer = &s.front();
	uint8_t i = 0;
	bool isnegative = v < 0;
	if(isnegative) {
		do {
			s_pointer[i++] = -(v % 10) + '0';
			v /= 10;
		} while(v != 0);
	}
	else {
		do {
			s_pointer[i++] = v % 10 + '0';
			v /= 10;
		} while(v != 0);
	}
	if(isnegative) {
		s_pointer[i++] = '-';
	}
	s.resize(i);
	uint8_t j = 0;
	i--;
	while(j < i) {
		char tmp = s_pointer[j];
		s_pointer[j] = s_pointer[i];
		s[i] = tmp;
		j ++;
		i --;
	}

	return s;
}

inline std::string num_to_string(int32_t v) noexcept
{
	return std::to_string(v);
}

inline std::string num_to_string(uint32_t v) noexcept
{
	return std::to_string(v);
}

inline std::string num_to_string(int64_t v) noexcept
{
	return std::to_string(v);
}

inline std::string num_to_string(uint64_t v) noexcept
{
	return std::to_string(v);
}

inline std::string num_to_string(float v) noexcept
{
	std::stringstream ss;
	ss.precision(std::numeric_limits<float>::digits10);
	ss << v;
	return ss.str();
}

inline std::string num_to_string(double v) noexcept
{
	std::stringstream ss;
	ss.precision(std::numeric_limits<double>::digits10);
	ss << v;
	return ss.str();
}

inline std::string num_to_string(long double v) noexcept
{
	std::stringstream ss;
	ss.precision(std::numeric_limits<long double>::digits10);
	ss << v;
	return ss.str();
}


/**
 * @brief operator << 输出128位有符号整数
 * @param os 输出流
 * @param v 整数
 * @return
 */
inline std::ostream & operator << (std::ostream & os, const __int128_t& v) noexcept
{
	os << pad_left_copy(num_to_string(v), static_cast<size_t>(os.width()), ' ');
	return os;
}

/**
 * @brief stolll 将字符串转为128位整数
 * @param str 待处理字符串
 * @return 返回128位整数
 */
inline __int128_t stolll(const std::string& str) noexcept
{
	__int128_t v = 0;
	size_t size = str.size();
	if(size > 0) {
		if(str[0] == '-') {
			for(size_t i = 1; i < size; i ++) {
				__int128_t tmp = str[i] - '0';
				for(size_t j = size - i; j > 1; j--) {
					tmp *= 10;
				}
				v -= tmp;
			}
		}
		else {
			for(size_t i = 0; i < size; i ++) {
				__int128_t tmp = str[i] - '0';
				for(size_t j = size - i; j > 1; j--) {
					tmp *= 10;
				}
				v += tmp;
			}
		}
	}
	return v;
}

/**
 * @brief stoulll 将字符串转为128位无符号整数
 * @param str 待处理字符串
 * @return 返回128位无符号整数
 */
inline __uint128_t stoulll(const std::string& str) noexcept
{
	__uint128_t v = 0;
	size_t size = str.size();
	if(size > 0) {
		for(size_t i = 0; i < size; i ++) {
			__uint128_t tmp = static_cast<__uint128_t>(str[i] - '0');
			for(size_t j = size - i; j > 1; j--) {
				tmp *= 10;
			}
			v += tmp;
		}
	}
	return v;
}

}
