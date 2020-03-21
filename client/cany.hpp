#pragma once

#include <string>
#include <limits>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cpack.hpp"
#include "crunningerror.hpp"
#include "definition.hpp"
#include "functions.hpp"

namespace MoonDb {

class CAny
{
public:
	CAny() : Type(FT_NONE) {}

	CAny(const bool& v)
	{
		Type = FT_BOOL;
		Data.Bool = v;
	}

	CAny(const int8_t& v)
	{
		Type = FT_INT8;
		Data.Int8 = v;
	}

	CAny(const uint8_t& v)
	{
		Type = FT_UINT8;
		Data.UInt8 = v;
	}

	CAny(const int16_t& v)
	{
		Type = FT_INT16;
		Data.Int16 = v;
	}

	CAny(const uint16_t& v)
	{
		Type = FT_UINT16;
		Data.UInt16 = v;
	}

	CAny(const int32_t& v)
	{
		Type = FT_INT32;
		Data.Int32 = v;
	}

	CAny(const uint32_t& v)
	{
		Type = FT_UINT32;
		Data.UInt32 = v;
	}

	CAny(const int64_t& v)
	{
		Type = FT_INT64;
		Data.Int64 = v;
	}

	CAny(const uint64_t& v)
	{
		Type = FT_UINT64;
		Data.UInt64 = v;
	}

	CAny(const __int128_t& v)
	{
		Type = FT_INT128;
		Data.Int128 = v;
	}

	CAny(const __uint128_t& v)
	{
		Type = FT_UINT128;
		Data.UInt128 = v;
	}

	CAny(const float& v)
	{
		Type = FT_FLOAT;
		Data.Float = v;
	}

	CAny(const double& v)
	{
		Type = FT_DOUBLE;
		Data.Double = v;
	}

	CAny(const long double& v)
	{
		Type = FT_LONGDOUBLE;
		Data.LongDouble = v;
	}

	CAny(const std::string& v)
	{
		Type = FT_STRING;
		Data.String = new std::string(v);
	}

	CAny(const char* v)
	{
		Type = FT_STRING;
		Data.String = new std::string(v);
	}

	// 用于处理接收到的字符串
	void Load(CPack& pack)
	{
		pack.Get(Type);
		switch(Type) {
		case FT_BOOL:
			pack.Get(Data.Bool);
			break;
		// 读取后类型改为字符串
		case FT_BIT:
			Data.String = new std::string;
			pack.Get<uint8_t>(*Data.String);
			Type = FT_STRING;
			break;
		case FT_INT8:
			pack.Get(Data.Int8);
			break;
		case FT_UINT8:
			pack.Get(Data.UInt8);
			break;
		case FT_INT16:
			pack.Get(Data.Int16);
			break;
		case FT_UINT16:
			pack.Get(Data.UInt16);
			break;
		case FT_INT32:
			pack.Get(Data.Int32);
			break;
		case FT_UINT32:
			pack.Get(Data.UInt32);
			break;
		case FT_INT64:
			pack.Get(Data.Int64);
			break;
		case FT_UINT64:
			pack.Get(Data.UInt64);
			break;
		case FT_INT128:
			pack.Get(Data.Int128);
			break;
		case FT_UINT128:
			pack.Get(Data.UInt128);
			break;
		case FT_FLOAT:
			pack.Get(Data.Float);
			break;
		case FT_DOUBLE:
			pack.Get(Data.Double);
			break;
		case FT_LONGDOUBLE:
			pack.Get(Data.LongDouble);
			break;
		case FT_STRING:
			Data.String = new std::string;
			pack.Get<int32_t>(*Data.String);
			break;
		case FT_NULL:
			break;
		default:
			ThrowError(ERR_WRONG_DATA_TYPE, "Can't identify the data type: " + num_to_string(Type));
		}
	}

	~CAny()
	{
		Reset();
	}

	uint16_t GetType() const
	{
		return Type;
	}

	// 用于存储缺省值
	void Store(CPack& pack, FieldType fieldtype = FT_NONE) const
	{
		if(FT_NONE == fieldtype) {
			fieldtype = static_cast<FieldType>(Type);
		}
		switch(fieldtype) {
		case FT_BOOL:
			pack.Put(Data.Bool);
			break;
		case FT_BIT:
			pack.Put<uint8_t>(*Data.String);
			break;
		case FT_INT8:
			pack.Put(Data.Int8);
			break;
		case FT_UINT8:
			pack.Put(Data.UInt8);
			break;
		case FT_INT16:
			pack.Put(Data.Int16);
			break;
		case FT_UINT16:
			pack.Put(Data.UInt16);
			break;
		case FT_INT32:
			pack.Put(Data.Int32);
			break;
		case FT_UINT32:
			pack.Put(Data.UInt32);
			break;
		case FT_INT64:
			pack.Put(Data.Int64);
			break;
		case FT_UINT64:
			pack.Put(Data.UInt64);
			break;
		case FT_INT128:
			pack.Put(Data.Int128);
			break;
		case FT_UINT128:
			pack.Put(Data.UInt128);
			break;
		//case FT_SERIAL16:
		//case FT_SERIAL32:
		//case FT_SERIAL64:
		//case FT_SERIAL128:
		case FT_FLOAT:
			pack.Put(Data.Float);
			break;
		case FT_DOUBLE:
			pack.Put(Data.Double);
			break;
		case FT_LONGDOUBLE:
			pack.Put(Data.LongDouble);
			break;
		case FT_STRING:
			pack.Put<int32_t>(*Data.String);
			break;
		case FT_NULL:
			break;
		default:
			break;
		}
	}

	bool& ToBool()
	{
		return Data.Bool;
	}

	const bool& ToBool() const
	{
		return Data.Bool;
	}

	int8_t& ToInt8()
	{
		return Data.Int8;
	}

	const int8_t& ToInt8() const
	{
		return Data.Int8;
	}

	uint8_t& ToUInt8()
	{
		return Data.UInt8;
	}

	const uint8_t& ToUInt8() const
	{
		return Data.UInt8;
	}

	int16_t& ToInt16()
	{
		return Data.Int16;
	}

	const int16_t& ToInt16() const
	{
		return Data.Int16;
	}

	uint16_t& ToUInt16()
	{
		return Data.UInt16;
	}

	const uint16_t& ToUInt16() const
	{
		return Data.UInt16;
	}

	int32_t& ToInt32()
	{
		return Data.Int32;
	}

	const int32_t& ToInt32() const
	{
		return Data.Int32;
	}

	uint32_t& ToUInt32()
	{
		return Data.UInt32;
	}

	const uint32_t& ToUInt32() const
	{
		return Data.UInt32;
	}

	int64_t& ToInt64()
	{
		return Data.Int64;
	}

	const int64_t& ToInt64() const
	{
		return Data.Int64;
	}

	uint64_t& ToUInt64()
	{
		return Data.UInt64;
	}

	const uint64_t& ToUInt64() const
	{
		return Data.UInt64;
	}

	__int128_t& ToInt128()
	{
		return Data.Int128;
	}

	const __int128_t& ToInt128() const
	{
		return Data.Int128;
	}

	__uint128_t& ToUInt128()
	{
		return Data.UInt128;
	}

	const __uint128_t& ToUInt128() const
	{
		return Data.UInt128;
	}

	float& ToFloat()
	{
		return Data.Float;
	}

	const float& ToFloat() const
	{
		return Data.Float;
	}

	double& ToDouble()
	{
		return Data.Double;
	}

	const double& ToDouble() const
	{
		return Data.Double;
	}

	long double& ToLongDouble()
	{
		return Data.LongDouble;
	}

	const long double& ToLongDouble() const
	{
		return Data.LongDouble;
	}

	std::string ToString()
	{
		return *Data.String;
	}

	const std::string& ToString() const
	{
		return *Data.String;
	}

	char* ToCharPointer()
	{
		return &Data.String->front();
	}

	const char* ToCharPointer() const
	{
		return &Data.String->front();
	}

	CAny& operator = (const bool& v)
	{
		Reset();
		Type = FT_BOOL;
		Data.Bool = v;
		return *this;
	}

	CAny& operator = (const int8_t& v)
	{
		Reset();
		Type = FT_INT8;
		Data.Int8 = v;
		return *this;
	}

	CAny& operator = (const uint8_t& v)
	{
		Reset();
		Type = FT_UINT8;
		Data.UInt8 = v;
		return *this;
	}

	CAny& operator = (const int16_t& v)
	{
		Reset();
		Type = FT_INT16;
		Data.Int16 = v;
		return *this;
	}

	CAny& operator = (const uint16_t& v)
	{
		Reset();
		Type = FT_UINT16;
		Data.UInt16 = v;
		return *this;
	}

	CAny& operator = (const int32_t& v)
	{
		Reset();
		Type = FT_INT32;
		Data.Int32 = v;
		return *this;
	}

	CAny& operator = (const uint32_t& v)
	{
		Reset();
		Type = FT_UINT32;
		Data.UInt32 = v;
		return *this;
	}

	CAny& operator = (const int64_t& v)
	{
		Reset();
		Type = FT_INT64;
		Data.Int64 = v;
		return *this;
	}

	CAny& operator = (const uint64_t& v)
	{
		Reset();
		Type = FT_UINT64;
		Data.UInt64 = v;
		return *this;
	}

	CAny& operator = (const __int128_t& v)
	{
		Reset();
		Type = FT_INT128;
		Data.Int128 = v;
		return *this;
	}

	CAny& operator = (const __uint128_t& v)
	{
		Reset();
		Type = FT_UINT128;
		Data.UInt128 = v;
		return *this;
	}

	CAny& operator = (const float& v)
	{
		Reset();
		Type = FT_FLOAT;
		Data.Float = v;
		return *this;
	}

	CAny& operator = (const double& v)
	{
		Reset();
		Type = FT_DOUBLE;
		Data.Double = v;
		return *this;
	}

	CAny& operator = (const long double& v)
	{
		Reset();
		Type = FT_LONGDOUBLE;
		Data.LongDouble = v;
		return *this;
	}

	CAny& operator = (const std::string& v)
	{
		Reset();
		Type = FT_STRING;
		Data.String = new std::string(v);
		return *this;
	}

	CAny& operator = (const char* v)
	{
		Reset();
		Type = FT_STRING;
		Data.String = new std::string(v);
		return *this;
	}

	CAny& operator = (const CAny& v)
	{
		Reset();
		Type = v.GetType();
		switch(Type) {
		case FT_BOOL:
			Data.Bool = v.ToBool();
			break;
		case FT_INT8:
			Data.Int8 = v.ToInt8();
			break;
		case FT_UINT8:
			Data.UInt8 = v.ToUInt8();
			break;
		case FT_INT16:
			Data.Int16 = v.ToInt16();
			break;
		case FT_UINT16:
			Data.UInt16 = v.ToUInt16();
			break;
		case FT_INT32:
			Data.Int32 = v.ToInt32();
			break;
		case FT_UINT32:
			Data.UInt32 = v.ToUInt32();
			break;
		case FT_INT64:
			Data.Int64 = v.ToInt64();
			break;
		case FT_UINT64:
			Data.UInt64 = v.ToUInt64();
			break;
		case FT_INT128:
			Data.Int128 = v.ToInt128();
			break;
		case FT_UINT128:
			Data.UInt128 = v.ToUInt128();
			break;
		case FT_FLOAT:
			Data.Float = v.ToFloat();
			break;
		case FT_DOUBLE:
			Data.Double = v.ToDouble();
			break;
		case FT_LONGDOUBLE:
			Data.LongDouble = v.ToLongDouble();
			break;
		case FT_STRING:
			Data.String = new std::string(v.ToString());
			break;
		default:
			break;
		}
		return *this;
	}

	friend std::ostream & operator << (std::ostream & os, const CAny& v)
	{
		switch(v.GetType()) {
		case FT_BOOL:
			os << (v.ToBool() ? "true" : "false"); break;
		case FT_INT8:
			os << static_cast<int>(v.ToInt8()); break;
		case FT_UINT8:
			os << static_cast<int>(v.ToUInt8()); break;
		case FT_INT16:
			os << v.ToInt16(); break;
		case FT_UINT16:
			os << v.ToUInt16(); break;
		case FT_INT32:
			os << v.ToInt32(); break;
		case FT_UINT32:
			os << v.ToUInt32(); break;
		case FT_INT64:
			os << v.ToInt64(); break;
		case FT_UINT64:
			os << v.ToUInt64(); break;
		case FT_INT128:
			os << v.ToInt128(); break;
		case FT_UINT128:
			os << v.ToUInt128(); break;
		case FT_FLOAT:
			os << v.ToFloat(); break;
		case FT_DOUBLE:
			os << v.ToDouble(); break;
		case FT_LONGDOUBLE:
			os << v.ToLongDouble(); break;
		case FT_STRING:
			os << v.ToString(); break;
		}
		return os;
	}

	union {
		bool Bool;
		int8_t Int8;
		uint8_t UInt8;
		int16_t Int16;
		uint16_t UInt16;
		int32_t Int32;
		uint32_t UInt32;
		int64_t Int64;
		uint64_t UInt64;
		__int128_t Int128;
		__uint128_t UInt128;
		float Float;
		double Double;
		long double LongDouble;
		std::string* String;
	} Data;

protected:
	inline void Reset()
	{
		switch(Type) {
		case FT_STRING:
			delete Data.String;
			Data.String = nullptr;
			break;
		}
	}

	uint16_t Type;
};

}
