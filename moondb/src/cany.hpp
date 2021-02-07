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
#include "ctime.hpp"
#include "cdecimal64.h"
#include "cdecimal128.h"

namespace MoonDb {

class CAny
{
public:
	inline CAny() noexcept : Type(FT_NONE) {}

	inline CAny(const CAny& v)
	{
		Type = v.Type;
		switch(Type) {
		case FT_STRING:
			Data.String = new std::string(*v.Data.String);
			break;
		case FT_ICONVSTRING:
			Data.IconvString = new CString(*v.Data.IconvString);
			break;
		default:
			Data = v.Data;
			break;
		}
	}

	inline CAny(CAny&& v)
	{
		Type = v.Type;
		switch(Type) {
		case FT_STRING:
			Data.String = v.Data.String;
			break;
		case FT_ICONVSTRING:
			Data.IconvString = v.Data.IconvString;
			break;
		default:
			Data = v.Data;
			break;
		}
		v.Type = FT_NONE;
	}

	inline CAny(const bool& v) noexcept
	{
		Type = FT_BOOL;
		Data.Bool = v;
	}

	inline CAny(const int8_t& v) noexcept
	{
		Type = FT_INT8;
		Data.Int8 = v;
	}

	inline CAny(const uint8_t& v) noexcept
	{
		Type = FT_UINT8;
		Data.UInt8 = v;
	}

	inline CAny(const int16_t& v) noexcept
	{
		Type = FT_INT16;
		Data.Int16 = v;
	}

	inline CAny(const uint16_t& v) noexcept
	{
		Type = FT_UINT16;
		Data.UInt16 = v;
	}

	inline CAny(const int32_t& v) noexcept
	{
		Type = FT_INT32;
		Data.Int32 = v;
	}

	inline CAny(const uint32_t& v) noexcept
	{
		Type = FT_UINT32;
		Data.UInt32 = v;
	}

	inline CAny(const int64_t& v) noexcept
	{
		Type = FT_INT64;
		Data.Int64 = v;
	}

	inline CAny(const uint64_t& v) noexcept
	{
		Type = FT_UINT64;
		Data.UInt64 = v;
	}

	inline CAny(const __int128_t& v) noexcept
	{
		Type = FT_INT128;
		Data.Int128 = v;
	}

	inline CAny(const __uint128_t& v) noexcept
	{
		Type = FT_UINT128;
		Data.UInt128 = v;
	}

	inline CAny(const float& v) noexcept
	{
		Type = FT_FLOAT32;
		Data.Float32 = v;
	}

	inline CAny(const double& v) noexcept
	{
		Type = FT_FLOAT64;
		Data.Float64 = v;
	}

	inline CAny(const __float128& v) noexcept
	{
		Type = FT_FLOAT128;
		Data.FLOAT128 = v;
	}

	inline CAny(const CDate& v) noexcept
	{
		Type = FT_DATE;
		Data.Date = v;
	}

	inline CAny(const CDateTime& v) noexcept
	{
		Type = FT_DATETIME;
		Data.DateTime = v;
	}

	inline CAny(const std::string& v) noexcept
	{
		Type = FT_STRING;
		Data.String = new std::string(v);
	}

	inline CAny(const char* v) noexcept
	{
		Type = FT_STRING;
		Data.String = new std::string(v);
	}

	inline CAny(const CString& v) noexcept
	{
		Type = FT_ICONVSTRING;
		Data.IconvString = new CString(v);
	}

	// 用于处理接收到的字符串
	inline void Load(CPack& pack)
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
		case FT_FLOAT32:
			pack.Get(Data.Float32);
			break;
		case FT_FLOAT64:
			pack.Get(Data.Float64);
			break;
		case FT_FLOAT128:
			pack.Get(Data.FLOAT128);
			break;
		case FT_STRING:
			Data.String = new std::string;
			pack.Get<uint32_t>(*Data.String);
			break;
		case FT_NULL:
			break;
		default:
			ThrowError(ERR_WRONG_DATA_TYPE, "Can't identify the data type: " + num_to_string(Type));
		}
	}

	// 用于字段缺省值
	inline CAny(FieldType fieldtype, uint32_t length, uint32_t scale, CIconv::CharsetType charset, const std::string& defval, const std::unordered_map<std::string, uint16_t>& values) noexcept
	{
		switch(fieldtype) {
		case FT_BOOL:
			Type = FT_BOOL;
			if("1" == defval || "true" == to_lower_copy(defval)) {
				Data.Bool = true;
			}
			else {
				Data.Bool = false;
			}
			break;
		case FT_BIT:
			Type = FT_UINT128;
			Data.UInt128 = 0;
			for(uint16_t i = 0; i < defval.size(); ++i) {
				if('1' == defval[i]) {
					Data.UInt128 += 1 << i;
				}
			}
			break;
		case FT_INT8:
			Type = FT_INT8;
			Data.Int8 = static_cast<int8_t>(stol(defval));
			break;
		case FT_UINT8:
			Type = FT_UINT8;
			Data.UInt8 = static_cast<uint8_t>(stoul(defval));
			break;
		case FT_INT16:
			Type = FT_INT16;
			Data.Int16 = static_cast<int16_t>(stol(defval));
			break;
		case FT_UINT16:
			Type = FT_UINT16;
			Data.UInt16 = static_cast<uint16_t>(stoul(defval));
			break;
		case FT_INT32:
			Type = FT_INT32;
			Data.Int32 = static_cast<int32_t>(stol(defval));
			break;
		case FT_UINT32:
			Type = FT_UINT32;
			Data.UInt32 = static_cast<uint32_t>(stoul(defval));
			break;
		case FT_INT64:
			Type = FT_INT64;
			Data.Int64 = static_cast<int64_t>(stoll(defval));
			break;
		case FT_UINT64:
			Type = FT_UINT64;
			Data.UInt64 = static_cast<uint64_t>(stoull(defval));
			break;
		case FT_INT128:
			Type = FT_INT128;
			Data.Int128 = static_cast<__int128_t>(stolll(defval));
			break;
		case FT_UINT128:
			Type = FT_UINT128;
			Data.UInt128 = static_cast<__uint128_t>(stoulll(defval));
			break;
		//case FT_SERIAL16:
		//case FT_SERIAL32:
		//case FT_SERIAL64:
		//case FT_SERIAL128:
		case FT_FLOAT32:
			Type = FT_FLOAT32;
			Data.Float32 = static_cast<float>(stof(defval));
			break;
		case FT_FLOAT64:
			Type = FT_FLOAT64;
			Data.Float64 = static_cast<double>(stod(defval));
			break;
		case FT_FLOAT128:
			Type = FT_FLOAT128;
			Data.FLOAT128 = static_cast<__float128>(strtoflt128(defval.c_str(), NULL));
			break;
		case FT_DECIMAL64:
			Type = FT_INT64;
			{
				CDecimal64 dec(defval, scale);
				Data.Int64 = dec.GetData();
			}
			break;
		case FT_DECIMAL128:
			Type = FT_INT128;
			{
				CDecimal128 dec(defval, scale);
				Data.Int128 = dec.GetData();
			}
			break;
		case FT_ENUM:
			{
				Type = FT_UINT16;
				auto it = values.find(defval);
				if(it != values.end()) {
					Data.UInt16 = it->second + 1;
				}
				else {
					Data.UInt16 = uint16_t(0);
				}
			}
			break;
		case FT_DATE:
			{
				Type = FT_DATE;
				std::vector<std::string> rawdate = explode(defval, '-');
				Data.Date = CDate(static_cast<int16_t>(stol(rawdate[0])), static_cast<uint8_t>(stoul(rawdate[1])), static_cast<uint8_t>(stoul(rawdate[2])));
				break;
			}
		case FT_TIME:
			Type = FT_INT64;
			if(is_digit(defval)) {
				Data.Int64 = stoll(defval);
			}
			else {
				std::vector<std::string> rawtime = explode(defval, ':');
				if(defval[0] != '-') {
					Data.Int64 = (stoll(rawtime[0]) * 3600 + stoul(rawtime[1]) * 60) * CTime::NanoTime + static_cast<int64_t>(stod(rawtime[2]) * CTime::NanoTime);
				}
				else {
					Data.Int64 = (stoll(rawtime[0]) * 3600 - stoul(rawtime[1]) * 60) * CTime::NanoTime - static_cast<int64_t>(stod(rawtime[2]) * CTime::NanoTime);
				}
			}
			break;
		case FT_DATETIME:
			{
				Type = FT_DATETIME;
				std::vector<std::string> rawdatetime = explode(defval, ' ');
				std::vector<std::string> rawdate = explode(rawdatetime[0], '-');
				std::vector<std::string> rawtime = explode(rawdatetime[1], ':');
				double seconds = stod(rawtime[2]);
				uint32_t time = stoul(rawtime[0]) * 3600 + stoul(rawtime[1]) * 60 + static_cast<uint8_t>(::floor(seconds));
				uint32_t fraction = static_cast<uint32_t>((seconds - ::floor(seconds)) * CTime::NanoTime);
				Data.DateTime = CDateTime(static_cast<int16_t>(stol(rawdate[0])), static_cast<uint8_t>(stoul(rawdate[1])), static_cast<uint8_t>(stoul(rawdate[2])), time, fraction);
			}
			break;
		case FT_TIMESTAMP:
			if(to_upper_copy(defval) == "CURRENT_TIMESTAMP") {
				Type = FT_STRING;
				Data.String = new std::string("CURRENT_TIMESTAMP");
			}
			else {
				Type = FT_INT64;
				Data.Int64 = stoll(defval);
			}
			break;
		case FT_CHAR:
		case FT_VARCHAR:
			Type = FT_ICONVSTRING;
			Data.IconvString = new CString;
			Data.IconvString->Data = defval;
			truncate(Data.IconvString->Data, length, Data.IconvString->Chars, charset);
			break;
		case FT_TEXT:
			Type = FT_ICONVSTRING;
			Data.IconvString = new CString;
			Data.IconvString->Data = defval;
			truncate(Data.IconvString->Data, std::numeric_limits<uint32_t>::max(), Data.IconvString->Chars, charset);
			break;
		case FT_BINARY:
		case FT_VARBINARY:
			Type = FT_STRING;
			Data.String = new std::string(defval, 0, length);
			break;
		case FT_BLOB:
			Type = FT_STRING;
			Data.String = new std::string(defval, 0, std::numeric_limits<uint32_t>::max());
			break;
		default:
			break;
		}
	}

	// 用于内存表读取数据
	inline CAny(CPack& pack, FieldType fieldtype, uint32_t length, const std::vector<std::string>& flipvalues) noexcept
	{
		switch(fieldtype) {
		case FT_BOOL:
			Type = FT_BOOL;
			pack.Get(Data.Bool);
			break;
		case FT_BIT:
		{
			Type = FT_STRING;
			Data.String = new std::string(128, '\0');
			__uint128_t bitint = 0;
			pack.Get(bitint);
			for(uint16_t i = 0; i < 128; ++i) {
				if(bitint % 2 == 1) {
					Data.String->at(i) = '1';
				}
				else {
					Data.String->at(i) = '0';
				}
				bitint >>= 1;
			}
			rtrim(*Data.String, "0");
			break;
		}
		case FT_INT8:
			Type = FT_INT8;
			pack.Get(Data.Int8);
			break;
		case FT_UINT8:
			Type = FT_UINT8;
			pack.Get(Data.UInt8);
			break;
		case FT_INT16:
			Type = FT_INT16;
			pack.Get(Data.Int16);
			break;
		case FT_UINT16:
			Type = FT_UINT16;
			pack.Get(Data.UInt16);
			break;
		case FT_INT32:
			Type = FT_INT32;
			pack.Get(Data.Int32);
			break;
		case FT_UINT32:
			Type = FT_UINT32;
			pack.Get(Data.UInt32);
			break;
		case FT_INT64:
		case FT_DECIMAL64:
			Type = FT_INT64;
			pack.Get(Data.Int64);
			break;
		case FT_UINT64:
			Type = FT_UINT64;
			pack.Get(Data.UInt64);
			break;
		case FT_INT128:
		case FT_DECIMAL128:
			Type = FT_INT128;
			pack.Get(Data.Int128);
			break;
		case FT_UINT128:
			Type = FT_UINT128;
			pack.Get(Data.UInt128);
			break;
		case FT_FLOAT32:
			Type = FT_FLOAT32;
			pack.Get(Data.Float32);
			break;
		case FT_FLOAT64:
			Type = FT_FLOAT64;
			pack.Get(Data.Float64);
			break;
		case FT_FLOAT128:
			Type = FT_FLOAT128;
			pack.Get(Data.FLOAT128);
			break;
		case FT_ENUM:
			{
				Type = FT_STRING;
				uint16_t v;
				pack.Get(v);
				if(v > 0 && v <= flipvalues.size()) {
					Data.String = new std::string(flipvalues[v - 1]);
				}
				else {
					Data.String = new std::string;
				}
			}
			break;
		case FT_DATE:
			Type = FT_DATE;
			pack.Read(&Data.Date, sizeof(CDate));
			break;
		case FT_TIME:
			Type = FT_INT64;
			pack.Get(Data.Int64);
			break;
		case FT_DATETIME:
			Type = FT_DATETIME;
			pack.Read(&Data.DateTime, sizeof(CDate));
			break;
		case FT_TIMESTAMP:
			Type = FT_INT64;
			pack.Get(Data.Int64);
			break;
		case FT_CHAR:
			Type = FT_ICONVSTRING;
			Data.IconvString = new CString;
			pack.Get(Data.IconvString->Chars);
			pack.Get<uint16_t>(Data.IconvString->Data, length);
			break;
		case FT_VARCHAR:
			Type = FT_ICONVSTRING;
			Data.IconvString = new CString;
			pack.Get(Data.IconvString->Chars);
			pack.Get<uint16_t>(Data.IconvString->Data);
			break;
		case FT_TEXT:
			Type = FT_ICONVSTRING;
			Data.IconvString = new CString;
			pack.Get(Data.IconvString->Chars);
			pack.Get<uint32_t>(Data.IconvString->Data);
			break;
		case FT_BINARY:
			Type = FT_STRING;
			Data.String = new std::string;
			pack.Get<uint16_t>(*Data.String, length);
			break;
		case FT_VARBINARY:
			Type = FT_STRING;
			Data.String = new std::string;
			pack.Get<uint16_t>(*Data.String);
			break;
		case FT_BLOB:
			Type = FT_STRING;
			Data.String = new std::string;
			pack.Get<uint32_t>(*Data.String);
			break;
		default:
			break;
		}
	}

	inline ~CAny() noexcept
	{
		Reset();
	}

	inline uint16_t GetType() const noexcept
	{
		return Type;
	}

	// 用于存储缺省值
	inline void Store(CPack& pack, FieldType fieldtype, uint32_t length, uint32_t scale) const noexcept
	{
		switch(fieldtype) {
		case FT_BOOL:
			pack.Put(Data.Bool);
			break;
		case FT_BIT:
			pack.Put(Data.UInt128);
			//pack.Put(Data.UInt64);
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
		case FT_DECIMAL64:
			pack.Put(Data.Int64);
			break;
		case FT_UINT64:
			pack.Put(Data.UInt64);
			break;
		case FT_INT128:
		case FT_DECIMAL128:
			pack.Put(Data.Int128);
			break;
		case FT_UINT128:
			pack.Put(Data.UInt128);
			break;
		//case FT_SERIAL16:
		//case FT_SERIAL32:
		//case FT_SERIAL64:
		//case FT_SERIAL128:
		case FT_FLOAT32:
			pack.Put(Data.Float32);
			break;
		case FT_FLOAT64:
			pack.Put(Data.Float64);
			break;
		case FT_FLOAT128:
			pack.Put(Data.FLOAT128);
			break;
		case FT_ENUM:
			pack.Put(Data.UInt16);
			break;
		case FT_DATE:
			pack.Write(&Data.Date, sizeof(CDate));
			break;
		case FT_TIME:
			pack.Put(Data.Int64);
			break;
		case FT_DATETIME:
			pack.Write(&Data.DateTime, sizeof(CDateTime));
			break;
		case FT_TIMESTAMP:
			if(Type == FT_STRING && *Data.String == "CURRENT_TIMESTAMP") {
				pack.Put(CTime::Now());
			}
			else {
				pack.Put(Data.Int64);
			}
			break;
		case FT_CHAR:
			pack.Put(static_cast<uint16_t>(Data.IconvString->Chars));// 记录字符数
			pack.Put<uint16_t>(Data.IconvString->Data, length);// 记录字节数和内容
			break;
		case FT_VARCHAR:
			pack.Put(static_cast<uint16_t>(Data.IconvString->Chars));// 记录字符数
			pack.Put<uint16_t>(Data.IconvString->Data);// 记录字节数和内容
			break;
		case FT_TEXT:
			pack.Put(static_cast<uint32_t>(Data.IconvString->Chars));// 记录字符数
			pack.Put<uint32_t>(Data.IconvString->Data);// 记录字节数和内容
			break;
		case FT_BINARY:
			pack.Put<uint16_t>(*Data.String, length);
			break;
		case FT_VARBINARY:
			pack.Put<uint16_t>(*Data.String);
			break;
		case FT_BLOB:
			pack.Put<uint32_t>(*Data.String);
			break;
		default:
			break;
		}
	}

	// 用于存储输入值
	inline void Store(CPack& pack, FieldType fieldtype, uint32_t length, uint32_t scale, CIconv::CharsetType charset, const std::unordered_map<std::string, uint16_t>& values) const
	{
		switch(fieldtype) {
		case FT_BOOL:
			if(FT_BOOL == Type) {
				pack.Put(Data.Bool);
			}
			else if(FT_STRING == Type) {
				std::string str = to_lower_copy(*Data.String);
				bool value;
				if("true" == str || "1" == str) {
					value = true;
				}
				else {
					value = false;
				}
				pack.Put(value);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_BIT:
			if(FT_UINT128 == Type) {
				pack.Put(Data.UInt128);
				//pack.Put(Data.UInt64);
			}
			else if(FT_STRING == Type) {
				__uint128_t bitint = 0;
				//uint64_t bitint;
				for(uint16_t i = 0; i < Data.String->size(); ++i) {
					if('1' == (*Data.String)[i]) {
						bitint += 1 << i;
					}
				}
				pack.Put(bitint);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_INT8:
			if(FT_INT8 == Type) {
				pack.Put(Data.Int8);
			}
			else if(FT_INT64 == Type) {
				pack.Put(static_cast<int8_t>(Data.Int64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<int8_t>(std::stol(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_UINT8:
			if(FT_UINT8 == Type) {
				pack.Put(Data.UInt8);
			}
			else if(FT_INT64 == Type) {
				pack.Put(static_cast<uint8_t>(Data.Int64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<uint8_t>(std::stoul(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_INT16:
			if(FT_INT16 == Type) {
				pack.Put(Data.Int16);
			}
			else if(FT_INT64 == Type) {
				pack.Put(static_cast<int16_t>(Data.Int64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<int16_t>(std::stol(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_UINT16:
			if(FT_UINT16 == Type) {
				pack.Put(Data.UInt16);
			}
			else if(FT_INT64 == Type) {
				pack.Put(static_cast<uint16_t>(Data.Int64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<uint16_t>(std::stoul(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_INT32:
			if(FT_INT32 == Type) {
				pack.Put(Data.Int32);
			}
			else if(FT_INT64 == Type) {
				pack.Put(static_cast<int32_t>(Data.Int64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<int32_t>(std::stol(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_UINT32:
			if(FT_UINT32 == Type) {
				pack.Put(Data.UInt32);
			}
			else if(FT_INT32 == Type) {
				pack.Put(Data.Int32);
			}
			else if(FT_INT64 == Type) {
				pack.Put(static_cast<uint32_t>(Data.Int64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<uint32_t>(std::stoul(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_INT64:
			if(FT_INT64 == Type) {
				pack.Put(Data.Int64);
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<int64_t>(std::stoll(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_UINT64:
			if(FT_UINT64 == Type) {
				pack.Put(Data.UInt64);
			}
			else if(FT_INT64 == Type) {
				pack.Put(static_cast<uint64_t>(Data.Int64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<uint64_t>(std::stoull(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_INT128:
			if(FT_INT128 == Type) {
				pack.Put(Data.Int128);
			}
			else if(FT_INT64 == Type) {
				pack.Put(static_cast<__int128_t>(Data.Int64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<__int128_t>(stolll(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_UINT128:
			if(FT_UINT128 == Type) {
				pack.Put(Data.UInt128);
			}
			else if(FT_INT64 == Type) {
				pack.Put(static_cast<__uint128_t>(Data.Int64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<__uint128_t>(stoulll(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		//case FT_SERIAL16:
		//case FT_SERIAL32:
		//case FT_SERIAL64:
		//case FT_SERIAL128:
		case FT_FLOAT32:
			if(FT_FLOAT32 == Type) {
				pack.Put(Data.Float32);
			}
			else if(FT_FLOAT64 == Type) {
				pack.Put(static_cast<float>(Data.Float64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<float>(std::stof(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_FLOAT64:
			if(FT_FLOAT64 == Type) {
				pack.Put(Data.Float64);
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<double>(std::stod(*Data.String)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_FLOAT128:
			if(FT_FLOAT128 == Type) {
				pack.Put(Data.FLOAT128);
			}
			else if(FT_FLOAT64 == Type) {
				pack.Put(static_cast<__float128>(Data.Float64));
			}
			else if(FT_STRING == Type) {
				pack.Put(static_cast<__float128>(strtoflt128(Data.String->c_str(), NULL)));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_DECIMAL64:
			{
				CDecimal64 dec(scale);
				switch(Type) {
				case FT_STRING:
					dec.Set(*Data.String);
					break;
				case FT_FLOAT32:
					dec.Set(Data.Float32);
					break;
				case FT_FLOAT64:
					dec.Set(Data.Float64);
					break;
				case FT_FLOAT128:
					dec.Set(Data.FLOAT128);
					break;
				case FT_INT8:
					dec.Set(Data.Int8);
					break;
				case FT_UINT8:
					dec.Set(Data.UInt8);
					break;
				case FT_INT16:
					dec.Set(Data.Int16);
					break;
				case FT_UINT16:
					dec.Set(Data.UInt16);
					break;
				case FT_INT32:
					dec.Set(Data.Int32);
					break;
				case FT_UINT32:
					dec.Set(Data.UInt32);
					break;
				case FT_INT64:
					dec.Set(Data.Int64);
					break;
				case FT_UINT64:
					dec.Set(Data.UInt64);
					break;
				default:
					ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
				}
				pack.Put(dec.GetData());
			}
			break;
		case FT_DECIMAL128:
			{
				CDecimal128 dec(scale);
				switch(Type) {
				case FT_STRING:
					dec.Set(*Data.String);
					break;
				case FT_FLOAT32:
					dec.Set(Data.Float32);
					break;
				case FT_FLOAT64:
					dec.Set(Data.Float64);
					break;
				case FT_FLOAT128:
					dec.Set(Data.FLOAT128);
					break;
				case FT_INT8:
					dec.Set(Data.Int8);
					break;
				case FT_UINT8:
					dec.Set(Data.UInt8);
					break;
				case FT_INT16:
					dec.Set(Data.Int16);
					break;
				case FT_UINT16:
					dec.Set(Data.UInt16);
					break;
				case FT_INT32:
					dec.Set(Data.Int32);
					break;
				case FT_UINT32:
					dec.Set(Data.UInt32);
					break;
				case FT_INT64:
					dec.Set(Data.Int64);
					break;
				case FT_UINT64:
					dec.Set(Data.UInt64);
					break;
				case FT_INT128:
					dec.Set(Data.Int128);
					break;
				case FT_UINT128:
					dec.Set(Data.UInt128);
					break;
				default:
					ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
				}
				pack.Put(dec.GetData());
			}
			break;
		case FT_ENUM:
			if(FT_UINT16 == Type) {
				pack.Put(Data.UInt16);
			}
			else if(FT_STRING == Type) {
				uint16_t value = 0;
				auto it = values.find(*Data.String);
				if(it != values.end()) {
					value = it->second + 1;
				}
				pack.Put(value);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_DATE:
			if(FT_DATE == Type) {
				pack.Write(&Data.Date, sizeof(CDate));
			}
			else if(FT_STRING == Type) {
				std::vector<std::string> rawdate = explode(*Data.String, '-');
				CDate date(static_cast<int16_t>(std::stol(rawdate[0])), static_cast<uint8_t>(std::stoul(rawdate[1])), static_cast<uint8_t>(std::stoul(rawdate[2])));
				pack.Write(&date, sizeof(CDate));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_TIME:
			if(FT_TIME == Type) {
				pack.Put(Data.Int64);
			}
			else if(FT_STRING == Type) {
				int64_t time;
				if(is_digit(*Data.String)) {
					time = std::stoll(*Data.String);
				}
				else {
					std::vector<std::string> rawtime = explode(*Data.String, ':');
					if((*Data.String)[0] != '-') {
						time = (stoll(rawtime[0]) * 3600 + stoul(rawtime[1]) * 60) * CTime::NanoTime + static_cast<int64_t>(std::stod(rawtime[2]) * CTime::NanoTime);
					}
					else {
						time = (stoll(rawtime[0]) * 3600 - stoul(rawtime[1]) * 60) * CTime::NanoTime - static_cast<int64_t>(std::stod(rawtime[2]) * CTime::NanoTime);
					}
				}
				pack.Put(time);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_DATETIME:
			if(FT_DATETIME == Type) {
				pack.Write(&Data.DateTime, sizeof(CDateTime));
			}
			else if(FT_STRING == Type) {
				std::vector<std::string> rawdatetime = explode(*Data.String, ' ');
				std::vector<std::string> rawdate = explode(rawdatetime[0], '-');
				std::vector<std::string> rawtime = explode(rawdatetime[1], ':');
				double seconds = std::stod(rawtime[2]);
				uint32_t time = std::stoul(rawtime[0]) * 3600 + std::stoul(rawtime[1]) * 60 + static_cast<uint8_t>(::floor(seconds));
				uint32_t fraction = static_cast<uint32_t>((seconds - ::floor(seconds)) * CTime::NanoTime);
				CDateTime datetime(static_cast<int16_t>(std::stol(rawdate[0])), static_cast<uint8_t>(std::stoul(rawdate[1])), static_cast<uint8_t>(std::stoul(rawdate[2])), time, fraction);
				pack.Write(&datetime, sizeof(CDateTime));
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_TIMESTAMP:
			if(FT_INT64 == Type) {
				pack.Put(Data.Int64);
			}
			else if(FT_STRING == Type) {
				int64_t value;
				if(to_upper_copy(*Data.String) == "CURRENT_TIMESTAMP") {
					value = CTime::Now();
				}
				else {
					value = stoll(*Data.String);
				}
				pack.Put(value);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_CHAR:
			if(FT_STRING == Type) {
				uint32_t charnum;
				truncate(*Data.String, length, charnum, charset);
				pack.Put(static_cast<uint16_t>(charnum));// 记录字符数
				pack.Put<uint16_t>(*Data.String, length);
				//std::cout << *Data.String << std::endl;
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_VARCHAR:
			if(FT_STRING == Type) {
				uint32_t charnum;
				truncate(*Data.String, length, charnum, charset);
				pack.Put(static_cast<uint16_t>(charnum));// 记录字符数
				pack.Put<uint16_t>(*Data.String);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_TEXT:
			if(FT_STRING == Type) {
				uint32_t charnum;
				truncate(*Data.String, std::numeric_limits<uint32_t>::max(), charnum, charset);
				pack.Put(static_cast<uint32_t>(charnum));// 记录字符数
				pack.Put<uint32_t>(*Data.String);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_BINARY:
			if(FT_STRING == Type) {
				if(Data.String->size() > length) {
					Data.String->erase(length);
				}
				pack.Put<uint16_t>(*Data.String, length);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_VARBINARY:
			if(FT_STRING == Type) {
				if(Data.String->size() > length) {
					Data.String->erase(length);
				}
				pack.Put<uint16_t>(*Data.String);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		case FT_BLOB:
			if(FT_STRING == Type) {
				if(Data.String->size() > std::numeric_limits<uint32_t>::max()) {
					Data.String->erase(std::numeric_limits<uint32_t>::max());
				}
				pack.Put<uint32_t>(*Data.String);
			}
			else {
				ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong data type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(Type)));
			}
			break;
		default:
			break;
		}
	}

	inline bool& ToBool() noexcept
	{
		return Data.Bool;
	}

	inline const bool& ToBool() const noexcept
	{
		return Data.Bool;
	}

	inline int8_t& ToInt8() noexcept
	{
		return Data.Int8;
	}

	const int8_t& ToInt8() const noexcept
	{
		return Data.Int8;
	}

	inline uint8_t& ToUInt8() noexcept
	{
		return Data.UInt8;
	}

	inline const uint8_t& ToUInt8() const noexcept
	{
		return Data.UInt8;
	}

	inline int16_t& ToInt16() noexcept
	{
		return Data.Int16;
	}

	inline const int16_t& ToInt16() const noexcept
	{
		return Data.Int16;
	}

	inline uint16_t& ToUInt16() noexcept
	{
		return Data.UInt16;
	}

	inline const uint16_t& ToUInt16() const noexcept
	{
		return Data.UInt16;
	}

	inline int32_t& ToInt32() noexcept
	{
		return Data.Int32;
	}

	inline const int32_t& ToInt32() const noexcept
	{
		return Data.Int32;
	}

	inline uint32_t& ToUInt32() noexcept
	{
		return Data.UInt32;
	}

	inline const uint32_t& ToUInt32() const
	{
		return Data.UInt32;
	}

	inline int64_t& ToInt64() noexcept
	{
		return Data.Int64;
	}

	inline const int64_t& ToInt64() const noexcept
	{
		return Data.Int64;
	}

	inline uint64_t& ToUInt64() noexcept
	{
		return Data.UInt64;
	}

	inline const uint64_t& ToUInt64() const noexcept
	{
		return Data.UInt64;
	}

	inline __int128_t& ToInt128() noexcept
	{
		return Data.Int128;
	}

	inline const __int128_t& ToInt128() const noexcept
	{
		return Data.Int128;
	}

	inline __uint128_t& ToUInt128() noexcept
	{
		return Data.UInt128;
	}

	inline const __uint128_t& ToUInt128() const noexcept
	{
		return Data.UInt128;
	}

	inline float& ToFloat32() noexcept
	{
		return Data.Float32;
	}

	inline const float& ToFloat32() const noexcept
	{
		return Data.Float32;
	}

	inline double& ToFloat64() noexcept
	{
		return Data.Float64;
	}

	inline const double& ToFloat64() const noexcept
	{
		return Data.Float64;
	}

	inline __float128& ToFloat128() noexcept
	{
		return Data.FLOAT128;
	}

	inline const __float128& ToFloat128() const noexcept
	{
		return Data.FLOAT128;
	}

	inline CDate& ToDate() noexcept
	{
		return Data.Date;
	}

	inline const CDate& ToDate() const noexcept
	{
		return Data.Date;
	}

	inline CDateTime& ToDateTime() noexcept
	{
		return Data.DateTime;
	}

	inline const CDateTime& ToDateTime() const noexcept
	{
		return Data.DateTime;
	}

	inline std::string ToString() noexcept
	{
		return *Data.String;
	}

	inline const std::string& ToString() const noexcept
	{
		return *Data.String;
	}

	inline char* ToCharPointer() noexcept
	{
		return &Data.String->front();
	}

	const char* ToCharPointer() const noexcept
	{
		return &Data.String->front();
	}

	inline CString ToIconvString() noexcept
	{
		return *Data.IconvString;
	}

	const CString& ToIconvString() const noexcept
	{
		return *Data.IconvString;
	}

	inline CAny& operator = (const bool& v) noexcept
	{
		Reset();
		Type = FT_BOOL;
		Data.Bool = v;
		return *this;
	}

	inline CAny& operator = (const int8_t& v) noexcept
	{
		Reset();
		Type = FT_INT8;
		Data.Int8 = v;
		return *this;
	}

	inline CAny& operator = (const uint8_t& v) noexcept
	{
		Reset();
		Type = FT_UINT8;
		Data.UInt8 = v;
		return *this;
	}

	inline CAny& operator = (const int16_t& v) noexcept
	{
		Reset();
		Type = FT_INT16;
		Data.Int16 = v;
		return *this;
	}

	inline CAny& operator = (const uint16_t& v) noexcept
	{
		Reset();
		Type = FT_UINT16;
		Data.UInt16 = v;
		return *this;
	}

	inline CAny& operator = (const int32_t& v) noexcept
	{
		Reset();
		Type = FT_INT32;
		Data.Int32 = v;
		return *this;
	}

	inline CAny& operator = (const uint32_t& v) noexcept
	{
		Reset();
		Type = FT_UINT32;
		Data.UInt32 = v;
		return *this;
	}

	inline CAny& operator = (const int64_t& v) noexcept
	{
		Reset();
		Type = FT_INT64;
		Data.Int64 = v;
		return *this;
	}

	inline CAny& operator = (const uint64_t& v) noexcept
	{
		Reset();
		Type = FT_UINT64;
		Data.UInt64 = v;
		return *this;
	}

	inline CAny& operator = (const __int128_t& v) noexcept
	{
		Reset();
		Type = FT_INT128;
		Data.Int128 = v;
		return *this;
	}

	inline CAny& operator = (const __uint128_t& v) noexcept
	{
		Reset();
		Type = FT_UINT128;
		Data.UInt128 = v;
		return *this;
	}

	inline CAny& operator = (const float& v) noexcept
	{
		Reset();
		Type = FT_FLOAT32;
		Data.Float32 = v;
		return *this;
	}

	inline CAny& operator = (const double& v) noexcept
	{
		Reset();
		Type = FT_FLOAT64;
		Data.Float64 = v;
		return *this;
	}

	inline CAny& operator = (const __float128& v) noexcept
	{
		Reset();
		Type = FT_FLOAT128;
		Data.FLOAT128 = v;
		return *this;
	}

	inline CAny& operator = (const CDate& v) noexcept
	{
		Reset();
		Type = FT_DATE;
		Data.Date = v;
		return *this;
	}

	inline CAny& operator = (const CDateTime& v) noexcept
	{
		Reset();
		Type = FT_DATETIME;
		Data.DateTime = v;
		return *this;
	}

	inline CAny& operator = (const std::string& v) noexcept
	{
		Reset();
		Type = FT_STRING;
		Data.String = new std::string(v);
		return *this;
	}

	inline CAny& operator = (const char* v) noexcept
	{
		Reset();
		Type = FT_STRING;
		Data.String = new std::string(v);
		return *this;
	}

	inline CAny& operator = (const CString& v) noexcept
	{
		Reset();
		Type = FT_ICONVSTRING;
		Data.IconvString = new CString(v);
		return *this;
	}

	inline CAny& operator = (const CAny& v) noexcept
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
		case FT_FLOAT32:
			Data.Float32 = v.ToFloat32();
			break;
		case FT_FLOAT64:
			Data.Float64 = v.ToFloat64();
			break;
		case FT_FLOAT128:
			Data.FLOAT128 = v.ToFloat128();
			break;
		case FT_STRING:
			Data.String = new std::string(v.ToString());
			break;
		case FT_ICONVSTRING:
			Data.IconvString = new CString(v.ToIconvString());
			break;
		default:
			break;
		}
		return *this;
	}

	inline CAny& operator = (CAny&& v)
	{
		Reset();
		Type = v.Type;
		switch(Type) {
		case FT_STRING:
			Data.String = v.Data.String;
			break;
		case FT_ICONVSTRING:
			Data.IconvString = v.Data.IconvString;
			break;
		default:
			Data = v.Data;
			break;
		}
		v.Type = FT_NONE;
		return *this;
	}

	friend std::ostream & operator << (std::ostream & os, const CAny& v) noexcept
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
		case FT_FLOAT32:
			os << v.ToFloat32(); break;
		case FT_FLOAT64:
			os << v.ToFloat64(); break;
		case FT_FLOAT128:
			os << v.ToFloat128(); break;
		case FT_DATE:
			os << num_to_string(v.ToDate().Year) + "-" + num_to_string(v.ToDate().Month) + "-" + num_to_string(v.ToDate().Day); break;
		case FT_DATETIME:
			os << v.ToDateTime().to_string();
			break;
		case FT_STRING:
			os << v.ToString(); break;
		case FT_ICONVSTRING:
			os << v.ToIconvString().Data; break;
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
		float Float32;
		double Float64;
		__float128 FLOAT128;
		CDate Date;
		CDateTime DateTime;
		std::string* String;
		CString* IconvString;
	} Data;

protected:
	inline void Reset() noexcept
	{
		switch(Type) {
		case FT_STRING:
			delete Data.String;
			Data.String = nullptr;
			break;
		case FT_ICONVSTRING:
			delete Data.IconvString;
			Data.IconvString = nullptr;
			break;
		}
	}

	uint16_t Type;
};

}
