#pragma once

#include <string>
#include <vector>
#include <set>
#include <algorithm>

#include <boost/algorithm/string/case_conv.hpp>
#define BOOST_LOCALE_ENABLE_CHAR16_T 1
#define BOOST_LOCALE_ENABLE_CHAR32_T 1
#include <boost/locale.hpp>

#include "crunningerror.hpp"

namespace MoonDb {

class CIconv
{
public:
	/**
	 * @brief 字符集类型
	 */
	enum CharsetType {
		CHARSET_NONE,
		CHARSET_ASCII,
		CHARSET_UTF8,
		CHARSET_GBK
	};

	/**
	 * @brief get_charset_length 获取字符集字符的最大长度
	 * @param ct 字符集
	 * @return 字符最大字节数
	 */
	inline static uint8_t get_charset_length(CharsetType ct) noexcept
	{
		switch(ct) {
		case CHARSET_ASCII:
			return 1;
		case CHARSET_UTF8:
			return 4;
		case CHARSET_GBK:
			return 2;
		default:
			return 0;
		}
	}

	const static int64_t npos = std::numeric_limits<int64_t>::max();

	/**
	 * @brief substr 截取字符串
	 * @param str 原字符串
	 * @param start 起始位置，可为负值
	 * @param length 长度，可为负值
	 * @return 返回截取后的字符串
	 */
	inline static std::string substr_ascii(const std::string& str, int64_t start, int64_t length = npos) noexcept
	{
		if(length == 0) {
			return "";
		}
		int64_t size = static_cast<int64_t>(str.size());
		if(_substr_pre(start, length, size)) {
			return "";
		}
		return str.substr(static_cast<size_t>(start), static_cast<size_t>(length));
	}

	/**
	 * @brief truncate_ascii 将ascii字符串截断到length长度
	 * @param str 待处理字符串
	 * @param length 截断长度
	 * @param charnum 字符数
	 */
	inline static void truncate_ascii(std::string& str, uint32_t length, uint32_t& charnum) noexcept
	{
		if(str.size() > length) {
			str.erase(length);
		}
		charnum = static_cast<uint32_t>(str.size());
	}

	/**
	 * @brief iconv 将字符串str从from字符集转为to字符集
	 *		example: iconv("汉字", CHARSET_GBK, CHARSET_UTF8)
	 * @param str 待处理字符串
	 * @param from 源字符串字符集
	 * @param to 目标字符串字符集
	 * @return 转换后的字符串
	 */
	inline static std::string iconv(const std::string& str, CharsetType from, CharsetType to)
	{
		return boost::locale::conv::between(str, _get_charset_str(to), _get_charset_str(from));
	}

	/**
	 * @brief to_char 转换为string类型的utf字符串
	 * @param str 源字符串
	 * @param from 原字符集
	 * @return 转换后的字符串
	 */
	inline static std::string to_char(const std::string& str, CharsetType from)
	{
		return boost::locale::conv::to_utf<char>(str, _get_charset_str(from));
	}

	/**
	 * @brief to_wchar 将字符串str从from字符集转为unicode wchar字符串
	 *		example: to_wchar("汉字", CHARSET_GBK)
	 * @param str 源字符串
	 * @param from 原字符集
	 * @return 转换后的字符串
	 */
	inline static std::wstring to_wchar(const std::string& str, CharsetType from)
	{
		return boost::locale::conv::to_utf<wchar_t>(str, _get_charset_str(from));
	}

	/**
	 * @brief to_char16 将字符串str从from字符集转为unicode char16字符串
	 * @param str 源字符串
	 * @param from 原字符集
	 * @return 转换后的字符串
	 */
	inline static std::u16string to_char16(const std::string& str, CharsetType from)
	{
		return boost::locale::conv::to_utf<char16_t>(str, _get_charset_str(from));
	}

	/**
	 * @brief to_char32 将字符串str从from字符集转为unicode char32字符串
	 * @param str 源字符串
	 * @param from 原字符集
	 * @return 转换后的字符串
	 */
	inline static std::u32string to_char32(const std::string& str, CharsetType from)
	{
		return boost::locale::conv::to_utf<char32_t>(str, _get_charset_str(from));
	}

	/**
	 * @brief from_char 将字符串str从utf字符串转为to字符集
	 * @param str 源字符串
	 * @param to 目标字符集
	 * @return 转换后的字符串
	 */
	inline static std::string from_char(const std::string& str, CharsetType to)
	{
		return boost::locale::conv::from_utf(str, _get_charset_str(to));
	}

	/**
	 * @brief from_wchar 将字符串wstr从unicode wchar字符串转为to字符集
	 * @param wstr 源字符串
	 * @param to 目标字符集
	 * @return 转换后的字符串
	 */
	inline static std::string from_wchar(const std::wstring& wstr, CharsetType to)
	{
		return boost::locale::conv::from_utf(wstr, _get_charset_str(to));
	}

	/**
	 * @brief from_char16 将字符串wstr从unicode u16string字符串转为to字符集
	 * @param wstr 源字符串
	 * @param to 目标字符集
	 * @return 转换后的字符串
	 */
	inline static std::string from_char16(const std::u16string& wstr, CharsetType to)
	{
		return boost::locale::conv::from_utf(wstr, _get_charset_str(to));
	}

	/**
	 * @brief from_char32 将字符串wstr从unicode u32string字符串转为to字符集
	 * @param wstr 源字符串
	 * @param to 目标字符集
	 * @return 转换后的字符串
	 */
	inline static std::string from_char32(const std::u32string& wstr, CharsetType to)
	{
		return boost::locale::conv::from_utf(wstr, _get_charset_str(to));
	}

	inline static std::wstring utf8_to_wchar(const std::string& str)
	{
		return boost::locale::conv::utf_to_utf<wchar_t, char>(str);
	}

	inline static std::u16string utf8_to_char16(const std::string& str)
	{
		return boost::locale::conv::utf_to_utf<char16_t, char>(str);
	}

	inline static std::u32string utf8_to_char32(const std::string& str)
	{
		return boost::locale::conv::utf_to_utf<char32_t, char>(str);
	}

	inline static std::string wchar_to_utf8(const std::wstring& wstr)
	{
		return boost::locale::conv::from_utf(wstr, "UTF-8");
	}

	inline static std::string char16_to_utf8(const std::u16string& wstr)
	{
		return boost::locale::conv::from_utf(wstr, "UTF-8");
	}

	inline static std::string char32_to_utf8(const std::u32string& wstr)
	{
		return boost::locale::conv::from_utf(wstr, "UTF-8");
	}

	/**
	 * @brief get_length_utf8 utf8字符长度1-4，根据每个字符第一个字节计算字符长度
			0xxxxxxx
			110xxxxx 10xxxxxx
			1110xxxx 10xxxxxx 10xxxxxx
			11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	 * @param str
	 * @return 字符长度
	 */
	inline static size_t strlen_utf8(const std::string& str)
	{
		size_t length = 0;
		const char* p = str.data();
		while(*p) {
			p += _charlen_utf8_check(p, length);
			length ++;
		}

		return length;
	}

	/**
	 * @brief strlen_gbk gbk字符长度1-2
	 * @param str
	 * @return 字符长度
	 */
	inline static size_t strlen_gbk(const std::string& str) noexcept
	{
		size_t length = 0;
		const char* p = str.data();
		while(*p) {
			p += _charlen_gbk(p);
			length ++;
		}
		return length;
	}

	/**
	 * @brief substr_utf8 截取utf8字符串
	 * @param str 原字符串
	 * @param start 起始位置，可为负值
	 * @param length 长度，可为负值
	 * @param size utf8字符数量，如果为-1则根据str计算长度
	 * @return 返回截取后的字符串
	 */
	inline static std::string substr_utf8(const std::string& str, int64_t start, int64_t length = npos, int64_t size = -1)
	{
		if(length == 0) {
			return "";
		}
		if(-1 == size) {
			size = static_cast<int64_t>(strlen_utf8(str));
		}
		if(_substr_pre(start, length, size)) {
			return "";
		}

		int64_t i = 0;
		const char* p = str.data();
		// 计算start_byte起始位置
		size_t start_byte = 0;
		while(*p) {
			if(i >= start) {
				break;
			}
			uint8_t charlen = _charlen_utf8(p);
			if(0 == charlen) {
				break;
			}
			i ++;
			start_byte += charlen;
			p += charlen;
		}
		// 计算length_byte长度
		size_t length_byte = 0;
		i = 0;
		while(*p) {
			if(i >= length) {
				break;
			}
			uint8_t charlen = _charlen_utf8(p);
			if(0 == charlen) {
				break;
			}
			i ++;
			length_byte += charlen;
			p += charlen;
		}

		return str.substr(static_cast<size_t>(start_byte), static_cast<size_t>(length_byte));
	}

	/**
	 * @brief substr_gbk 截取gbk字符串
	 * @param str 原字符串
	 * @param start 起始位置，可为负值
	 * @param length 长度，可为负值
	 * @param size gbk字符数量，如果为-1则根据str计算长度
	 * @return 返回截取后的字符串
	 */
	inline static std::string substr_gbk(const std::string& str, int64_t start, int64_t length = npos, int64_t size = -1) noexcept
	{
		if(length == 0) {
			return "";
		}
		if(-1 == size) {
			size = static_cast<int64_t>(strlen_gbk(str));
		}
		if(_substr_pre(start, length, size)) {
			return "";
		}

		int64_t i = 0;
		const char* p = str.data();
		// 计算start_byte起始位置
		size_t start_byte = 0;
		while(*p) {
			if(i >= start) {
				break;
			}
			uint8_t charlen = _charlen_gbk(p);
			if(0 == charlen) {
				break;
			}
			i ++;
			start_byte += charlen;
			p += charlen;
		}
		// 计算length_byte长度
		size_t length_byte = 0;
		i = 0;
		while(*p) {
			if(i >= length) {
				break;
			}
			uint8_t charlen = _charlen_gbk(p);
			if(0 == charlen) {
				break;
			}
			i ++;
			length_byte += charlen;
			p += charlen;
		}

		return str.substr(static_cast<size_t>(start_byte), static_cast<size_t>(length_byte));
	}

	/**
	 * @brief truncate_utf8 将utf8字符串截断到length(字节)长度
	 * @param str 待处理字符串
	 * @param length 截断长度
	 * @param charnum 字符数
	 */
	inline static void truncate_utf8(std::string& str, uint32_t length, uint32_t& charnum)
	{
		if(str.size() > length) {
			charnum = 0;
			size_t trunlength = 0;
			const char* p = str.data();
			while(*p) {
				uint8_t charlen = _charlen_utf8_check(p, charnum);
				if(trunlength + charlen > length) {
					break;
				}
				p += charlen;
				trunlength += charlen;
				charnum++;
			}
			str.erase(trunlength);
		}
		else {
			charnum = static_cast<uint32_t>(strlen_utf8(str));
		}
	}

	/**
	 * @brief truncate_gbk 将gbk字符串截断到length(字节)长度
	 * @param str 待处理字符串
	 * @param length 截断长度
	 * @param charnum 字符数
	 */
	inline static void truncate_gbk(std::string& str, uint32_t length, uint32_t& charnum) noexcept
	{
		if(str.size() > length) {
			charnum = 0;
			size_t trunlength = 0;
			const char* p = str.data();
			while(*p) {
				uint8_t charlen = _charlen_gbk(p);
				if(trunlength + charlen > length) {
					break;
				}
				p += charlen;
				trunlength += charlen;
				charnum++;
			}
			str.erase(trunlength);
		}
		else {
			charnum = static_cast<uint32_t>(strlen_gbk(str));
		}
	}

	/**
	 * @brief substr 截取字符串
	 * @param str 原字符串
	 * @param start 起始位置，可为负值
	 * @param length 长度，可为负值
	 * @param charset 字符集
	 * @return 返回截取后的字符串
	 */
	inline static std::string substr(const std::string& str, const int64_t& start, const int64_t& length = npos, CharsetType charset = CHARSET_ASCII)
	{
		switch(charset) {
		case CHARSET_ASCII:
			return substr_ascii(str, start, length);
		case CHARSET_UTF8:
			return substr_utf8(str, start, length);
		case CHARSET_GBK:
			return substr_gbk(str, start, length);
		default:
			return substr_ascii(str, start, length);
		}
	}

	/**
	 * @brief strlen 按照字符集计算字符串长度
	 * @param str 字符串
	 * @param charset 字符集
	 * @return 长度
	 */
	inline static size_t strlen(const std::string& str, CharsetType charset = CHARSET_ASCII)
	{
		switch(charset) {
		case CHARSET_ASCII:
			return str.size();
		case CHARSET_UTF8:
			return strlen_utf8(str);
		case CHARSET_GBK:
			return strlen_gbk(str);
		default:
			return str.size();
		}
	}

	/**
	 * @brief truncate 将字符串截断
	 * @param str 字符串
	 * @param length 截断长度
	 * @param charnum 字符数
	 * @param charset 字符集
	 */
	inline static void truncate(std::string& str, uint32_t length, uint32_t& charnum, CharsetType charset = CHARSET_ASCII) noexcept
	{
		switch(charset) {
		case CHARSET_ASCII:
			truncate_ascii(str, length, charnum);
			break;
		case CHARSET_UTF8:
			truncate_utf8(str, length, charnum);
			break;
		case CHARSET_GBK:
			truncate_gbk(str, length, charnum);
			break;
		default:
			truncate_ascii(str, length, charnum);
		}
	}

protected:
	inline static bool _substr_pre(int64_t& start, int64_t& length, const int64_t& size) noexcept
	{
		//if(length == 0) {
		//	return true;
		//}
		if(start < 0) {
			start = size + start;
			if(start < 0) {
				return true;
			}
		}
		if(start >= size) {
			return true;
		}
		if(length < 0) {
			length = size + length - start;
			if(length <= 0) {
				return true;
			}
		}
		else if(start + length > size) {
			length = size - start;
		}
		return false;
	}

	inline static uint8_t _charlen_gbk(const char* p) noexcept
	{
		if(*p < 0 && (*(p+1) < 0 || *(p+1) > 63)) {//中文汉字情况
			return 2;
		}
		else {
			return 1;
		}
	}

	inline static uint8_t _charlen_utf8(const char* p) noexcept
	{
		//1*******
		if(*p & 128) {
			//11*******
			if(*p & 64) {
				//111*****
				if(*p & 32) {
					//1111****
					if(*p & 16) {
						//11111***
						if(*p & 8) {
							// 出现错误
						}
						//11110***
						else {
							return 4;
						}
					}
					//1110****
					else {
						return 3;
					}
				}
				//110*****
				else {
					return 2;
				}
			}
			//10******
			else {
			}
		}
		//0*******
		else {
			return 1;
		}
		return 0;
	}

	// 如果出现错误抛出异常，信息中心含有错误字符的位置（从1开始）
	inline static uint8_t _charlen_utf8_check(const char* p, const size_t& length)
	{
		//1*******
		if(*p & 128) {
			//11*******
			if(*p & 64) {
				//111*****
				if(*p & 32) {
					//1111****
					if(*p & 16) {
						//11111***
						if(*p & 8) {
							// 出现错误
							ThrowError(ERR_STRING_VALID, "The UTF-8 string is invalid(5,pos:" + std::to_string(length + 1) + ").");
						}
						//11110***
						else {
							if(0 == *(p+1) || !((*(p+1) & 128) && !(*(p+1) & 64))) {
								ThrowError(ERR_STRING_VALID, "The UTF-8 string is invalid(4,2,pos:" + std::to_string(length + 1) + ").");
							}
							if(0 == *(p+2) || !((*(p+2) & 128) && !(*(p+2) & 64))) {
								ThrowError(ERR_STRING_VALID, "The UTF-8 string is invalid(4,3,pos:" + std::to_string(length + 1) + ").");
							}
							if(0 == *(p+3) || !((*(p+3) & 128) && !(*(p+3) & 64))) {
								ThrowError(ERR_STRING_VALID, "The UTF-8 string is invalid(4,4,pos:" + std::to_string(length + 1) + ").");
							}
							return 4;
						}
					}
					//1110****
					else {
						if(0 == *(p+1) || !((*(p+1) & 128) && !(*(p+1) & 64))) {
							ThrowError(ERR_STRING_VALID, "The UTF-8 string is invalid(3,2,pos:" + std::to_string(length + 1) + ").");
						}
						if(0 == *(p+2) || !((*(p+2) & 128) && !(*(p+2) & 64))) {
							ThrowError(ERR_STRING_VALID, "The UTF-8 string is invalid(3,3,pos:" + std::to_string(length + 1) + ").");
						}
						return 3;
					}
				}
				//110*****
				else {
					if(0 == *(p+1) || !((*(p+1) & 128) && !(*(p+1) & 64))) {
						ThrowError(ERR_STRING_VALID, "The UTF-8 string is invalid(2,2,pos:" + std::to_string(length + 1) + ").");
					}
					return 2;
				}
			}
			//10******
			else {
				ThrowError(ERR_STRING_VALID, "The UTF-8 string is invalid(1,1,pos:" + std::to_string(length + 1) + ").");
			}
		}
		//0*******
		else {
			return 1;
		}
		return 0;
	}

	inline static const char* _get_charset_str(CharsetType charset) noexcept
	{
		static const char allcharsets[][6] {"", "ASCII", "UTF-8", "GBK"};
		return allcharsets[charset];
	}
};

}
