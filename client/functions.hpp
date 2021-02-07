#pragma once

#include <vector>
#include <set>
#include <string>
#include <cmath>
#include <sstream>
#include <limits>
#include <quadmath.h>

namespace MoonDb {

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
 * @brief str_replace 在str字符串中用search替换replace
 * @param search 被替换字符
 * @param replace 替换字符
 * @param str 待处理字符串
 */
inline void str_replace(char search, char replace, std::string& str)
{
	size_t size = str.size();
	for(size_t i = 0; i < size; i ++) {
		if(search == str[i]) {
			str[i] = replace;
		}
	}
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

inline std::string num_to_string(__float128 v) noexcept
{
	char buf[64];
	quadmath_snprintf(buf, sizeof buf, "%.*Qe", FLT128_DIG, v);
	return std::string(buf);
}

/**
 * @brief operator << 输出128位浮点数
 * @param os 输出流
 * @param v 整数
 * @return
 */
inline std::ostream & operator << (std::ostream & os, const __float128& v) noexcept
{
	os << pad_left_copy(num_to_string(v), static_cast<size_t>(os.width()), ' ');
	return os;
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
