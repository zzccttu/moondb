#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>

#include "crunningerror.hpp"
#include "cpack.hpp"

namespace MoonDb {

/**
 * @brief   CFile文件类。
 * @details
 * @version 1.0
 * @author  mzt
 * @date    2016-7-14
 * @todo    开发为跨平台的类
 */
class CFile
{
public:
    /**
     * @brief 文件打开方式
     */
    enum OpenMode {
		ONLY_READ,		/**< 只读 */
		ONLY_WRITE,		/**< 只写 */
		ONLY_APPEND,	/**< 只追加 */
		READ_WRITE,		/**< 读写 */
		READ_APPEND,	/**< 读并追加写 */
		TRUNCATE_WRITE	/**< 写之前清空 */
    };

    /**
     * @brief 文件位置
     */
    enum Position {
        POS_BEGINNING = std::ios_base::beg,   /**< 开始位置 */
        POS_CURRENT = std::ios_base::cur,     /**< 当前位置 */
        POS_END = std::ios_base::end          /**< 结束为止 */
    };

    /**
	 * @brief 构造函数
     */
    inline CFile()
    {
    }

    /**
	 * @brief 析构函数
     */
    inline ~CFile()
    {
        this->Close();
    }

    /**
	 * @brief 打开文件
     * @param filename 文件名
     * @param mode 打开方式
     * @return 成功返回true，失败返回false
     */
    inline bool Open(const char* filename, OpenMode mode)
    {
		static const char openModeString[][15] {"ONLY_READ", "ONLY_WRITE" , "ONLY_APPEND", "READ_WRITE", "READ_APPEND", "TRUNCATE_WRITE"};
		static const uint32_t openModeMap[] {
            std::ios_base::in | std::ios_base::binary,
            std::ios_base::out | std::ios_base::binary,
            std::ios_base::app | std::ios_base::binary,
            std::ios_base::in | std::ios_base::out | std::ios_base::binary,
            std::ios_base::in | std::ios_base::app | std::ios_base::binary,
            std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary
        };

		this->filename = filename;
		this->fs.open(filename, static_cast<std::ios_base::openmode>(openModeMap[mode]));
        if(this->fs.fail()) {
			ThrowError(ERR_FILE_OPEN, this->getErrorMessage(ERR_OPEN, 2, this->filename.c_str(), openModeString[mode]));
            return false;
        }
        return true;
    }

    /**
	 * @brief 打开文件
     * @param filename 文件名
     * @param mode 打开方式
     * @return 成功返回true，失败返回false
     */
    inline bool Open(const std::string& filename, OpenMode mode)
    {
        return this->Open(filename.data(), mode);
    }

	/**
	 * @brief 检查文件是否被打开
	 * @return 文件是否被打开
	 */
	inline bool IsOpen()
	{
		return this->fs.is_open();
	}

    /**
	 * @brief 关闭文件
     * @return 成功返回true，失败返回false
     */
    inline bool Close()
    {
        this->fs.clear();
		if(this->fs.is_open()) {
            this->fs.close();
            if(this->fs.fail()) {
				ThrowError(ERR_FILE_CLOSE, this->getErrorMessage(ERR_CLOSE, 1, this->filename.c_str()));
                return false;
            }
        }
        return true;
    }

    /**
	 * @brief 移动文件指针（读取）
     * @param offset 文件指针位置，可正可负
     * @param way 移动的起点
     * @return 成功返回true，失败触发异常
     */
    inline bool SeekR(int64_t offset, Position way = POS_BEGINNING)
    {
		this->fs.seekg(offset,static_cast<std::ios_base::seekdir>(way));
        if(this->fs.fail()) {
		   ThrowError(ERR_FILE_SEEK, this->getErrorMessage(ERR_SEEK, 1, this->filename.c_str()));
           return false;
        }
        return true;
    }

	/**
	* @brief 移动文件指针（写入）
	* @param offset 文件指针位置，可正可负
	* @param way 移动的起点
	* @return 成功返回true，失败触发异常
	*/
	inline bool SeekW(int64_t offset, Position way = POS_BEGINNING)
	{
		this->fs.seekp(offset, static_cast<std::ios_base::seekdir>(way));
		if (this->fs.fail()) {
			ThrowError(ERR_FILE_SEEK, this->getErrorMessage(ERR_SEEK, 1, this->filename.c_str()));
			return false;
		}
		return true;
	}

    /**
	 * @brief 获取当前文件指针位置（读取）
     * @param offset 文件指针位置
     * @return 成功返回true，失败触发异常
     */
	inline bool TellR(int64_t& offset)
    {
       offset = this->fs.tellg();
       if(-1L == offset || this->fs.fail()) {
		   ThrowError(ERR_FILE_TELL, this->getErrorMessage(ERR_TELL, 1, this->filename.c_str()));
           return false;
       }
       return true;
    }

	/**
	* @brief 获取当前文件指针位置（写入）
	* @param offset 文件指针位置
	* @return 成功返回true，失败触发异常
	*/
	inline bool TellW(int64_t &offset)
	{
		offset = this->fs.tellp();
		if (-1L == offset || this->fs.fail()) {
			ThrowError(ERR_FILE_TELL, this->getErrorMessage(ERR_TELL, 1, this->filename.c_str()));
			return false;
		}
		return true;
	}

	/**
	* @brief 读取文件
	* @param data 读取的数据存储到该变量
	* @param count 读取字节长度
	* @return 成功返回true，失败触发异常
	*/
	inline bool Read(std::string& data, size_t count)
	{
		// 设置字符串的长度
		data.resize(count);
		// 读取数据到指针指向的地址
		fs.read(&data[0], static_cast<std::streamsize>(count));
		if (fs.fail()) { // 遇到错误返回
			ThrowError(ERR_FILE_READ, this->getErrorMessage(ERR_READ, 1, this->filename.c_str()));
			return false;
		}
		// 设置实际读取的数量为长度
		std::streamsize realcount = fs.gcount();
		if (static_cast<size_t>(realcount) < count) {
			data.resize(static_cast<size_t>(realcount));
		}
		return true;
	}

	/**
	* @brief 按照行读取文件
	* @param data 读取的数据存储到该变量
	* @param maxnum 每行最大字节长度
	* @return 成功返回true，失败返回false
	*/
	inline bool GetLine(std::string& data, uint32_t maxnum)
	{
		// 字符串长度设为最大读取长度
		data.resize(maxnum);
		// 读取数据
		fs.getline(&data[0], maxnum);
		if (fs.fail()) { // 遇到错误返回
			ThrowError(ERR_FILE_READ, this->getErrorMessage(ERR_READ, 1, this->filename.c_str()));
			return false;
		}
		// 设置实际读取的数量为长度
		std::streamsize realcount = fs.gcount();
		if (realcount < maxnum) {
			data.resize(static_cast<size_t>(realcount));
		}
		return true;
	}

	/**
	* @brief 读取全部文件
	* @param data 读取到data
	* @return 成功返回true，失败触发异常
	*/
	inline bool ReadAll(std::string& data)
	{
		if (!this->SeekR(0, POS_END)) {
			return false;
		}
		int64_t size;
		if (!this->TellR(size)) {
			return false;
		}
		if (!this->SeekR(0, POS_BEGINNING)) {
			return false;
		}
		return this->Read(data, static_cast<size_t>(size));
		//data.assign(std::istreambuf_iterator<char>(this->fs), std::istreambuf_iterator<char>());
		//return true;
	}

	/**
	* @brief 读取全部文件
	* @param data 读取到data
	* @return 成功返回true，失败触发异常
	*/
	inline bool ReadAll(CPack& pack)
	{
		if (!this->SeekR(0, POS_END)) {
			return false;
		}
		int64_t size;
		if (!this->TellR(size)) {
			return false;
		};
		if (!this->SeekR(0, POS_BEGINNING)) {
			return false;
		}
		pack.Allocate(static_cast<size_t>(size));
		pack.SetSize(static_cast<size_t>(size));
		return this->Read(pack.GetPointer(), static_cast<size_t>(size));
	}

    /**
	 * @brief 写数据
     * @param data 字符串
     * @param count 数量
     * @return 成功返回true，失败触发异常
     */
	inline bool Write(const void* data, size_t count)
    {
		this->fs.write(static_cast<const char*>(data), static_cast<std::streamsize>(count));
        if(fs.fail()) { // 遇到错误返回
			ThrowError(ERR_FILE_WRITE, this->getErrorMessage(ERR_WRITE, 1, this->filename.c_str()));
            return false;
        }
        return true;
    }

	/**
	 * @brief 写数据
	 * @param data string类型数据
	 * @return 成功返回true，失败触发异常
	 */
	inline bool Write(const std::string& data)
	{
		return this->Write(data.data(), data.length());
	}

	/**
	 * @brief 读取文件
	 * @param data 读取的数据存储到该变量
	 * @param count 读取字节长度
	 * @return 如果读取成功并且打算读取长度与实际长度相同返回true，否则返回false
	 */
	inline bool Read(void* data, size_t count)
	{
		this->fs.read(static_cast<char*>(data), static_cast<std::streamsize>(count));
		if (fs.fail()) { // 遇到错误返回
			ThrowError(ERR_FILE_READ, this->getErrorMessage(ERR_READ, 1, this->filename.c_str()));
			return false;
		}
		// 设置实际读取的数量为长度
		std::streamsize realcount = fs.gcount();
		if (static_cast<size_t>(realcount) < count) {
			ThrowError(ERR_FILE_READ, this->getErrorMessage(ERR_READ, 1, this->filename.c_str()));
			return false;
		}
		return true;
	}

	/**
	 * @brief 将缓冲区数据全部写出
	 */
	inline void Flush()
	{
		this->fs.flush();
	}

	inline bool Put(const bool & val)
	{
		return this->Write(&val, sizeof(bool));
	}

	inline bool Get(bool & val)
	{
		return this->Read(&val, sizeof(bool));
	}

	inline bool Put(const int8_t & val)
	{
		return this->Write(&val, sizeof(int8_t));
	}

	inline bool Get(int8_t & val)
	{
		return this->Read(&val, sizeof(int8_t));
	}

	inline bool Put(const uint8_t & val)
	{
		return this->Write(&val, sizeof(uint8_t));
	}

	inline bool Get(uint8_t & val)
	{
		return this->Read(&val, sizeof(uint8_t));
	}

	inline bool Put(const int16_t & val)
	{
		return this->Write(&val, sizeof(int16_t));
	}

	inline bool Get(int16_t & val)
	{
		return this->Read(&val, sizeof(int16_t));
	}

	inline bool Put(const uint16_t & val)
	{
		return this->Write(&val, sizeof(uint16_t));
	}

	inline bool Get(uint16_t & val)
	{
		return this->Read(&val, sizeof(uint16_t));
	}

	inline bool Put(const int32_t & val)
	{
		return this->Write(&val, sizeof(int32_t));
	}

	inline bool Get(int32_t & val)
	{
		return this->Read(&val, sizeof(int32_t));
	}

	inline bool Put(const uint32_t & val)
	{
		return this->Write(&val, sizeof(uint32_t));
	}

	inline bool Get(uint32_t & val)
	{
		return this->Read(&val, sizeof(uint32_t));
	}

	inline bool Put(const int64_t & val)
	{
		return this->Write(&val, sizeof(int64_t));
	}

	inline bool Get(int64_t & val)
	{
		return this->Read(&val, sizeof(int64_t));
	}

	inline bool Put(const uint64_t & val)
	{
		return this->Write(&val, sizeof(uint64_t));
	}

	inline bool Get(uint64_t & val)
	{
		return this->Read(&val, sizeof(uint64_t));
	}

	inline bool Put(const __int128_t & val)
	{
		return this->Write(&val, sizeof(__int128_t));
	}

	inline bool Get(__int128_t & val)
	{
		return this->Read(&val, sizeof(__int128_t));
	}

	inline bool Put(const __uint128_t & val)
	{
		return this->Write(&val, sizeof(__uint128_t));
	}

	inline bool Get(__uint128_t & val)
	{
		return this->Read(&val, sizeof(__uint128_t));
	}

	inline bool Put(const float & val)
	{
		return this->Write(&val, sizeof(float));
	}

	inline bool Get(float & val)
	{
		return this->Read(&val, sizeof(float));
	}

	inline bool Put(const double & val)
	{
		return this->Write(&val, sizeof(double));
	}

	inline bool Get(double & val)
	{
		return this->Read(&val, sizeof(double));
	}

	inline bool Put(const __float128 & val)
	{
		return this->Write(&val, sizeof(__float128));
	}

	inline bool Get(__float128 & val)
	{
		return this->Read(&val, sizeof(__float128));
	}

	inline bool Put(const std::string & val)
	{
		if(!this->Put(val.length())) {
			return false;
		}
		return this->Write(val.data(), val.length());
	}

	inline bool Get(std::string & val)
	{
		size_t length;
		if (!this->Get(length)) {
			return false;
		}
		val.resize(length);
		if(0 == length) {
			return true;
		}
		return this->Read(&val.front(), length);
		//this->Read(&val[0], length);一样的效果
	}

	inline bool Put(const std::stringstream & val)
	{
		if(!this->Put(val.str().size())) {
			return false;
		}
		return this->Write(val.str().c_str(), val.str().size());
	}

	inline bool Get(std::stringstream & val)
	{
		size_t length;
		if (!this->Get(length)) {
			return false;
		}
		val.str("");
		if(0 == length) {
			return true;
		}
		char* buf = new char[length];
		bool ret = this->Read(buf, length);
		if(ret) {
			val.write(buf, static_cast<std::streamsize>(length));
		}
		delete[] buf;
		return ret;
	}

protected:
    std::fstream fs;
	std::string filename;

    /**
     * @brief 错误类型
     */
    enum ErrorType {
        ERR_OPEN,
        ERR_CLOSE,
        ERR_SEEK,
        ERR_TELL,
        ERR_READ,
        ERR_WRITE
    };

    /**
     * 获得错误信息
     * @param et 错误类型
     * @param n 变量数量，后面的...为char*类型变量
     * @return 错误信息
     */
    inline static const char * getErrorMessage(ErrorType et, int n, ...)
    {
        const static char errors[][44] {
            "Failed to open the file (%s) as '%s' mode. ",
            "Failed to close the file (%s). ",
            "Failed to seek the file (%s). ",
            "Failed to tell the file (%s). ",
            "Failed to read the file (%s). ",
            "Failed to write the file (%s). ",
        };
        static char error[1024];
        char *str1{}, *str2{};
		va_list vl;
        va_start(vl, n);
        switch(n) {
        case 1:
            str1 = va_arg(vl, char*);
            sprintf(error, errors[et], str1);
            va_end(vl);
            return error;
        case 2:
            str1 = va_arg(vl, char*);
            str2 = va_arg(vl, char*);
            sprintf(error, errors[et], str1, str2);
            va_end(vl);
            return error;
        default:
            return errors[et];
        }
	}
};

}
