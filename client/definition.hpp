#pragma once

#include <unordered_map>
#include <vector>
#include <string>

namespace MoonDb {
	enum FieldType {
		FT_NONE,// 仅用于标记未识别的字段类型
		FT_BOOL,
		FT_BIT,//n位bit，n从1至128
		FT_INT8,
		FT_UINT8,
		FT_INT16,
		FT_UINT16,
		FT_INT32,
		FT_UINT32,
		FT_INT64,
		FT_UINT64,
		FT_INT128,
		FT_UINT128,
		FT_SERIAL16,//16位整数自增字段
		FT_SERIAL32,//32位整数自增字段
		FT_SERIAL64,//64位整数自增字段
		FT_SERIAL128,//128位整数自增字段
		FT_FLOAT32,
		FT_FLOAT64,
		FT_FLOAT128,
		FT_NUMERIC,// 任意精度浮点数，尚未实现
		FT_ENUM,//最多65535个选项
		FT_DATE,//从-16384-01-01至16383-12-31  3个字节
		FT_TIME,//8个字节有符号整数，精度为纳秒
		FT_DATETIME,//从-32768-01-01 00:00:00至32767-12-31 23:59:59  9个字节，精度为纳秒
		FT_TIMESTAMP,//1901年至2100年，时间截可正可负，用有符号64位整数存取，精度为纳秒
		FT_CHAR,//n，长度0-65535
		FT_VARCHAR,//n，长度0-65535
		FT_BINARY,
		FT_VARBINARY,
		FT_TEXT,// 长度占4个字节，不同于mysql长度为2个字节，能够存储4G-1个字节的数据
		FT_BLOB,// 二进制数据，其余同FT_TEXT
		FT_DECIMAL64,
		FT_DECIMAL128,//用128位有符号整数存储，有效数字的值从-170141183460469231731687303715884105728至170141183460469231731687303715884105727；Scale从-32767至32729，为有效数字乘以10的Scale次方，为0表示整数
		FT_SIZE,
		// 以下为非字段类型
		FT_ICONVSTRING = 65534,// 程序内部使用的多字节字符串类型
		FT_NULL = 65535,// NULL值
	};

	const uint16_t FT_STRING = 31;

	enum OperationType {
		OPER_NONE,
		OPER_SELECT,
		OPER_INSERT,
		OPER_UPDATE,
		OPER_DELETE,
		OPER_REPLACE
	};

	enum IndexType {
		IT_NONE,
		IT_ROWID,
		IT_PRIMARY,
		IT_UNIQUE,
		IT_KEY,
		IT_FULLTEXT
	};

	enum IndexMode {
		IM_BTREE = 1,
		IM_HASH
	};

	enum TableType {
		TT_NONE,
		TT_FIXMEMORY,	/**< 内存表，类似No-SQL的键值存储，键为主键，值为其他数据的集合 */
		TT_VARMEMORY,
		TT_HARDDISK,	/**< 存储在硬盘，固定长度的字段存储在主表文件，长度仅递增，不去除已删除的数据；变长度的字段统一存储在附表 */
		TT_SIZE
	};

	enum ResponseType{
		RT_NONE,
		RT_ERROR,
		RT_CONNECT,
		RT_QUERY,
		RT_LAST_INSERT_ID,
		RT_AFFECTED_ROWS,
		RT_EXECUTE,
	};

	class CDefinition
	{
	public:
		static FieldType StringToFieldType(std::string type)
		{
			static std::unordered_map<std::string, FieldType> ftmap;
			if(!ftmap.size()) {
				ftmap["BOOL"] = FT_BOOL;
				ftmap["BOOLEAN"] = FT_BOOL;
				ftmap["BIT"] = FT_BIT;
				ftmap["INT8"] = FT_INT8;
				ftmap["UINT8"] = FT_UINT8;
				ftmap["TINYINT"] = FT_INT8;
				ftmap["TINYINT UNSIGNED"] = FT_UINT8;
				ftmap["INT16"] = FT_INT16;
				ftmap["UINT16"] = FT_UINT16;
				ftmap["SMALLINT"] = FT_INT16;
				ftmap["SMALLINT UNSIGNED"] = FT_UINT16;
				ftmap["MEDIUMINT"] = FT_INT32;
				ftmap["MEDIUMINT UNSIGNED"] = FT_UINT32;
				ftmap["INT"] = FT_INT32;
				ftmap["INT UNSIGNED"] = FT_UINT32;
				ftmap["INT32"] = FT_INT32;
				ftmap["UINT32"] = FT_UINT32;
				ftmap["INTEGER"] = FT_INT32;
				ftmap["INT64"] = FT_INT64;
				ftmap["UINT64"] = FT_UINT64;
				ftmap["BIGINT"] = FT_INT64;
				ftmap["BIGINT UNSIGNED"] = FT_UINT64;
				ftmap["SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT"] = FT_SERIAL16;
				ftmap["SMALLSERIAL"] = FT_SERIAL16;
				ftmap["SERIAL16"] = FT_SERIAL16;
				ftmap["INT UNSIGNED NOT NULL AUTO_INCREMENT"] = FT_SERIAL32;
				ftmap["SERIAL"] = FT_SERIAL32;
				ftmap["SERIAL32"] = FT_SERIAL32;
				ftmap["BIGINT UNSIGNED NOT NULL AUTO_INCREMENT"] = FT_SERIAL64;
				ftmap["BIGSERIAL"] = FT_SERIAL64;
				ftmap["SERIAL64"] = FT_SERIAL64;
				ftmap["FLOAT"] = FT_FLOAT32;
				ftmap["REAL"] = FT_FLOAT32;
				ftmap["DOUBLE"] = FT_FLOAT64;
				ftmap["DOUBLE PRECISION"] = FT_FLOAT64;
				ftmap["FLOAT32"] = FT_FLOAT32;
				ftmap["FLOAT64"] = FT_FLOAT64;
				ftmap["DATE"] = FT_DATE;
				ftmap["TIME"] = FT_TIME;
				ftmap["DATETIME"] = FT_DATETIME;
				ftmap["TIMESTAMP"] = FT_TIMESTAMP;
				ftmap["YEAR"] = FT_INT32;
				ftmap["CHAR"] = FT_CHAR;
				ftmap["CHARACTER"] = FT_CHAR;
				ftmap["VARCHAR"] = FT_VARCHAR;
				ftmap["CHARACTER VARYING"] = FT_VARCHAR;
				ftmap["BINARY"] = FT_BINARY;
				ftmap["VARBINARY"] = FT_VARBINARY;
				ftmap["TINYTEXT"] = FT_TEXT;
				ftmap["TEXT"] = FT_TEXT;
				ftmap["MEDIUMTEXT"] = FT_TEXT;
				ftmap["LONGTEXT"] = FT_TEXT;
				ftmap["TINYBLOB"] = FT_BLOB;
				ftmap["BLOB"] = FT_BLOB;
				ftmap["MEDIUMBLOB"] = FT_BLOB;
				ftmap["LONGBLOB"] = FT_BLOB;
				ftmap["BYTEA"] = FT_BLOB;
				ftmap["DECIMAL"] = FT_NUMERIC;
				ftmap["NUMERIC"] = FT_NUMERIC;
				ftmap["DECIMAL64"] = FT_DECIMAL64;
				ftmap["DECIMAL128"] = FT_DECIMAL128;
				ftmap["ENUM"] = FT_ENUM;
				// 本数据库中的特殊字段类型
				ftmap["INT128"] = FT_INT128;
				ftmap["UINT128"] = FT_UINT128;
				ftmap["SERIAL128"] = FT_SERIAL128;
				ftmap["FLOAT128"] = FT_FLOAT128;
			}
			auto it = ftmap.find(type);
			if(it == ftmap.end()) {
				return FT_NONE;
			}
			return it->second;
		}

		static std::string FieldTypeToString(FieldType type)
		{
			static std::vector<std::string> fieldtypes;
			if(fieldtypes.size() == 0) {
				fieldtypes.resize(FT_SIZE);
				fieldtypes[FT_NONE] = "NONE";
				fieldtypes[FT_BOOL] = "BOOL";
				fieldtypes[FT_BIT] = "BIT";
				fieldtypes[FT_INT8] = "INT8";
				fieldtypes[FT_UINT8] = "UINT8";
				fieldtypes[FT_INT16] = "INT16";
				fieldtypes[FT_UINT16] = "UINT16";
				fieldtypes[FT_INT32] = "INT32";
				fieldtypes[FT_UINT32] = "UINT32";
				fieldtypes[FT_INT64] = "INT64";
				fieldtypes[FT_UINT64] = "UINT64";
				fieldtypes[FT_INT128] = "INT128";
				fieldtypes[FT_UINT128] = "UINT128";
				fieldtypes[FT_SERIAL16] = "SERIAL16";
				fieldtypes[FT_SERIAL32] = "SERIAL32";
				fieldtypes[FT_SERIAL64] = "SERIAL64";
				fieldtypes[FT_SERIAL128] = "SERIAL128";
				fieldtypes[FT_FLOAT32] = "FLOAT32";
				fieldtypes[FT_FLOAT64] = "FLOAT64";
				fieldtypes[FT_FLOAT128] = "FLOAT128";
				fieldtypes[FT_NUMERIC] = "NUMERIC";
				fieldtypes[FT_ENUM] = "ENUM";
				fieldtypes[FT_DATE] = "DATE";
				fieldtypes[FT_TIME] = "TIME";
				fieldtypes[FT_DATETIME] = "DATETIME";
				fieldtypes[FT_TIMESTAMP] = "TIMESTAMP";
				fieldtypes[FT_CHAR] = "CHAR";
				fieldtypes[FT_VARCHAR] = "VARCHAR";
				fieldtypes[FT_BINARY] = "BINARY";
				fieldtypes[FT_VARBINARY] = "VARBINARY";
				fieldtypes[FT_TEXT] = "TEXT";
				fieldtypes[FT_BLOB] = "BLOB";
				fieldtypes[FT_DECIMAL64] = "DECIMAL64";
				fieldtypes[FT_DECIMAL128] = "DECIMAL128";
			}
			if(type >= FT_SIZE) {
				return fieldtypes[FT_NONE];
			}
			return fieldtypes[type];
		}
	};
}
