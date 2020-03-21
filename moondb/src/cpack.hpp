#pragma once

#include <string>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <boost/multiprecision/gmp.hpp>
#include "crunningerror.hpp"

namespace MoonDb {

class CPack
{
public:
	CPack() noexcept
		:Cache(nullptr), Capacity(0), Size(0), Position(0), Internal(true)
	{}

	CPack(size_t capacity)
	{
		Internal = true;
		Allocate(capacity);
	}

	CPack(std::string& data) noexcept
	{
		Cache = &data.front();
		Capacity = data.capacity();
		Size = data.size();
		Position = 0;
		Internal = false;
	}

	CPack(void* cache, size_t size) noexcept
	{
		Cache = cache;
		Capacity = size;
		Size = 0;
		Position = 0;
		Internal = false;
	}

	~CPack() noexcept
	{
		if(Internal) {
			free(Cache);
		}
	}

	enum Position {
		POS_BEGINNING = std::ios_base::beg,   /**< 开始位置 */
		POS_CURRENT = std::ios_base::cur,     /**< 当前位置 */
		POS_END = std::ios_base::end          /**< 结束为止 */
	};

	// 注意该函数只能在初始化时使用一次
	inline void Allocate(size_t capacity)
	{
		Cache = ::malloc(capacity);
		if(nullptr == Cache) {
			ThrowError(ERR_MEMORY_ALLOCATE, "CPack failed to allocate " + std::to_string(capacity) + " bytes.");
		}
		Capacity = capacity;
		Size = 0;
		Position = 0;
	}

	inline void Reallocate(size_t newcapacity)
	{
		if(!Internal) {
			ThrowError(ERR_MEMORY_ALLOCATE, "External memroy can't be reallocated in the class CPack.");
		}
		if(nullptr == Cache) {
			Allocate(newcapacity);
		}
		else if(newcapacity > Capacity) {
			void* more_mem = ::realloc(Cache, newcapacity);
			if(nullptr == more_mem) {
				free(Cache);
				Cache = nullptr;
				ThrowError(ERR_MEMORY_ALLOCATE, "Reallocate memory (function::realloc, " + std::to_string(newcapacity) + ") failed in the class CPack.");
			}
			Cache = more_mem;
			Capacity = newcapacity;
		}
	}

	inline bool Seek(int64_t offset, Position way = POS_BEGINNING) noexcept
	{
		switch(way) {
		case POS_BEGINNING:
			if(offset < 0 || static_cast<uint64_t>(offset) > Size) {
				return false;
			}
			Position = static_cast<uint64_t>(offset);
			break;
		case POS_CURRENT:
			if(0 == offset) {
			}
			else if(offset > 0) {
				if(Position + static_cast<uint64_t>(offset) > Size) {
					return false;
				}
				Position += static_cast<uint64_t>(offset);
			}
			else {
				if(Position < static_cast<uint64_t>(-offset)) {
					return false;
				}
				Position -= static_cast<uint64_t>(-offset);
			}
			break;
		case POS_END:
			if(offset > 0 || static_cast<uint64_t>(-offset) > Size) {
				return false;
			}
			Position = Size - static_cast<uint64_t>(-offset);
		}
		return true;
	}

	inline uint64_t Tell() const noexcept
	{
		return Position;
	}

	inline void MoveAhead(uint32_t step) noexcept
	{
		Position += step;
	}

	inline void Clear() noexcept
	{
		Size = 0;
		Position = 0;
	}

	inline void Write(const void* s, size_t count)
	{
		/*size_t newsize = Size + count;
		if(newsize > Capacity) {
			Reallocate(static_cast<size_t>(newsize * 1.2));
		}
		::memcpy(static_cast<char*>(Cache) + Size, s, count);
		Size = newsize;*/
		uint64_t newposition = Position + count;
		if(newposition > Capacity) {
			Reallocate(static_cast<uint64_t>(newposition * 1.2));
		}
		::memcpy(static_cast<char*>(Cache) + Position, s, count);
		Position = newposition;
		if(newposition > Size) {
			Size = newposition;
		}
	}

	inline void Read(void* s, size_t count)
	{
		size_t newposition = Position + count;
		if(newposition > Size) {
			ThrowError(ERR_EXCEED_MAXSIZE, "Exceed size(" + std::to_string(Size) + "," + std::to_string(count)  + "," +  std::to_string(newposition) + ").");
		}
		::memcpy(s, static_cast<char*>(Cache) + Position, count);
		Position = newposition;
	}

	inline void* GetPointer() noexcept
	{
		return Cache;
	}

	inline const void* GetPointer() const noexcept
	{
		return Cache;
	}

	inline void Put(const bool & val)
	{
		this->Write(&val, sizeof(bool));
	}

	inline void Get(bool & val)
	{
		this->Read(&val, sizeof(bool));
	}

	inline void Put(const int8_t & val)
	{
		this->Write(&val, sizeof(int8_t));
	}

	inline void Get(int8_t & val)
	{
		this->Read(&val, sizeof(int8_t));
	}

	inline void Put(const uint8_t & val)
	{
		this->Write(&val, sizeof(uint8_t));
	}

	inline void Get(uint8_t & val)
	{
		this->Read(&val, sizeof(uint8_t));
	}

	inline void Put(const int16_t & val)
	{
		this->Write(&val, sizeof(int16_t));
	}

	inline void Get(int16_t & val)
	{
		this->Read(&val, sizeof(int16_t));
	}

	inline void Put(const uint16_t & val)
	{
		this->Write(&val, sizeof(uint16_t));
	}

	inline void Get(uint16_t & val)
	{
		this->Read(&val, sizeof(uint16_t));
	}

	inline void Put(const int32_t & val)
	{
		this->Write(&val, sizeof(int32_t));
	}

	inline void Get(int32_t & val)
	{
		this->Read(&val, sizeof(int32_t));
	}

	inline void Put(const uint32_t & val)
	{
		this->Write(&val, sizeof(uint32_t));
	}

	inline void Get(uint32_t & val)
	{
		this->Read(&val, sizeof(uint32_t));
	}

	inline void Put(const int64_t & val)
	{
		this->Write(&val, sizeof(int64_t));
	}

	inline void Get(int64_t & val)
	{
		this->Read(&val, sizeof(int64_t));
	}

	inline void Put(const uint64_t & val)
	{
		this->Write(&val, sizeof(uint64_t));
	}

	inline void Get(uint64_t & val)
	{
		this->Read(&val, sizeof(uint64_t));
	}

	inline void Put(const __int128_t & val)
	{
		this->Write(&val, sizeof(__int128_t));
	}

	inline void Get(__int128_t & val)
	{
		this->Read(&val, sizeof(__int128_t));
	}

	inline void Put(const __uint128_t & val)
	{
		this->Write(&val, sizeof(__uint128_t));
	}

	inline void Get(__uint128_t & val)
	{
		this->Read(&val, sizeof(__uint128_t));
	}

	inline void Put(const float & val)
	{
		this->Write(&val, sizeof(float));
	}

	inline void Get(float & val)
	{
		this->Read(&val, sizeof(float));
	}

	inline void Put(const double & val)
	{
		this->Write(&val, sizeof(double));
	}

	inline void Get(double & val)
	{
		this->Read(&val, sizeof(double));
	}

	inline void Put(const long double & val)
	{
		this->Write(&val, sizeof(long double));
	}

	inline void Get(long double & val)
	{
		this->Read(&val, sizeof(long double));
	}

	inline void Put(const std::string & val)
	{
		this->Put(val.length());
		this->Write(val.data(), val.length());
	}

	inline void Get(std::string & val)
	{
		size_t length;
		this->Get(length);
		val.resize(length);
		if(0 == length) {
			return;
		}
		this->Read(&val.front(), length);
		//this->Read(&val[0], length);一样的效果
	}

	template<typename SizeType>
	inline void Put(const std::string& val, size_t pad_length, char pad_char = '\0')
	{
		SizeType size = val.size();
		this->Put(size);
		this->Write(&val.front(), size);
		if(pad_length > size) {
			std::string pad_str(pad_length - size, pad_char);
			this->Write(&pad_str.front(), pad_length - size);
		}
	}

	template<typename SizeType>
	inline void Get(std::string& val, size_t pad_length)
	{
		SizeType size;
		this->Get(size);
		val.resize(size);
		if(size > 0) {
			this->Read(&val.front(), size);
		}
		if(pad_length > size) {
			this->Position += pad_length - size;
		}
	}

	template<typename SizeType>
	inline void Put(const std::string& val)
	{
		SizeType size = val.size();
		this->Put(size);
		this->Write(&val.front(), size);
	}

	template<typename SizeType>
	inline void Get(std::string& val)
	{
		SizeType size;
		this->Get(size);
		val.resize(size);
		if(size > 0) {
			this->Read(&val.front(), size);
		}
	}

	/************************************************************************************/
	/*
	 * gmp整数和分数在存储时可以为定长和变长，当pad_size为0时为变长，大于0为定长
	 */

	inline void Put(const MP_INT& mpz, uint32_t pad_size = 0)
	{
		this->Put(mpz._mp_size);
		// 此处size和pad_size表示mp_limb_t长度的整数倍，因为如果_mp_size小于0表示负数，大于0表示正数，所以取绝对值
		uint32_t size = static_cast<uint32_t>(::abs(mpz._mp_size));
		if(pad_size > 0 && size > pad_size) {
			size = pad_size;
		}
		this->Write(mpz._mp_d, sizeof(mp_limb_t) * size);
		// 补剩余的空白字节
		if(pad_size > size) {
			std::string pad_str(sizeof(mp_limb_t) * static_cast<uint32_t>(pad_size - size), '\0');
			this->Write(&pad_str.front(), pad_str.size());
		}
	}

	inline void Get(MP_INT& mpz, uint32_t pad_size = 0)
	{
		if(pad_size > 0) {
			mpz_init2(&mpz, static_cast<mp_bitcnt_t>(pad_size * GMP_LIMB_BITS));
			this->Get(mpz._mp_size);
			uint32_t size = static_cast<uint32_t>(::abs(mpz._mp_size));
			this->Read(mpz._mp_d, sizeof(mp_limb_t) * size);
			if(pad_size > size) {
				this->Position += sizeof(mp_limb_t) * static_cast<uint32_t>(pad_size - size);
			}
		}
		else {
			int32_t mp_size;
			this->Get(mp_size);
			uint32_t size = static_cast<uint32_t>(::abs(mp_size));
			mpz_init2(&mpz, static_cast<mp_bitcnt_t>(size * GMP_LIMB_BITS));
			mpz._mp_size = mp_size;
			this->Read(mpz._mp_d, sizeof(mp_limb_t) * size);
		}
	}

	inline void Put(const boost::multiprecision::mpz_int& ap_int, uint32_t pad_size = 0)
	{
		this->Put(*ap_int.backend().data(), pad_size);
	}

	inline void Get(boost::multiprecision::mpz_int& ap_int, uint32_t pad_size = 0)
	{
		this->Get(*ap_int.backend().data(), pad_size);
	}

	inline void Put(const MP_RAT& mpq, uint32_t pad_size = 0)
	{
		this->Put(mpq._mp_num, pad_size);
		this->Put(mpq._mp_den, pad_size);
	}

	inline void Get(MP_RAT& mpq, uint32_t pad_size = 0)
	{
		this->Get(mpq._mp_num, pad_size);
		this->Get(mpq._mp_den, pad_size);
	}

	inline void Put(const boost::multiprecision::mpq_rational& ap_rat, uint32_t pad_size = 0)
	{
		this->Put(*ap_rat.backend().data(), pad_size);
	}

	inline void Get(boost::multiprecision::mpq_rational& ap_rat, uint32_t pad_size = 0)
	{
		this->Get(*ap_rat.backend().data(), pad_size);
	}

	/************************************************************************************/

	inline size_t GetSize() const noexcept
	{
		return Size;
	}

	inline void SetSize(size_t newsize) noexcept
	{
		if(newsize <= Capacity) {
			Size = newsize;
		}
		else {
			Size = Capacity;
		}
	}

protected:
	void* Cache;
	uint64_t Capacity;
	uint64_t Size;
	uint64_t Position;
	bool Internal;
};

}
