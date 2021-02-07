#pragma once

#include "header.h"
#include <shared_mutex>

namespace MoonDb {

/*
 * 注意：sql在添加和更新数据时将CURRENT_TIMESTAMP()和NOW()都映射为将CURRENT_TIMESTAMP
 */

class CTable
{
public:
	CTable() = default;

	static bool GetTableProperty(const string& path, const string& name, TableType& engine, FieldType& rowidtype);
	virtual bool Open(const string& path, const string& name);

	virtual ~CTable();
	virtual void InsertData(unordered_map<string, CAny>& data, CPack& ret) = 0;
	virtual void ReplaceData(const CAny& rowid, unordered_map<string, CAny>& data, CPack& ret) = 0;
	virtual void UpdateData(const CAny& rowid, unordered_map<string, CAny>& data, CPack& ret) = 0;
	virtual void DeleteData(const CAny& rowid, CPack& ret) = 0;
	virtual void GetData(const CAny& rowid, CPack& ret) = 0;

	bool Create(const string& path, const string& name, TableType engine, FieldType rowidtype, const vector<CRawField>& fields,
				const vector<CIndex>& indexes, uint64_t maxrows = 0, uint64_t minrows = 0, uint32_t lifetime = 0);

	const string& GetName() const noexcept
	{
		return Name;
	}

	void Test()
	{
		string str = "'a','b\\'', 'c',\"d\\\"\",'e'";
		unordered_map<string, uint16_t> values;
		vector<string> flipvalues;
		this->EnumValuesToVector(str, values, flipvalues);
		cout << endl;
		for(auto it = values.begin(); it != values.end(); it++) {
			cout << it->first << ":" << it->second << endl;
		}
		cout << endl;
	}

protected:
	class CField {
	public:
		string Name;						/**< 字段名称 */
		FieldType Type;						/**< 字段类型 */
		bool NotNull;						/**< 是否允许为NULL */
		bool DefaultDefined;				/**< 是否定义缺省值 */
		CAny DefaultValue;					/**< 缺省值 */
		bool OnUpdateDefined;				/**< 是否定义更新数据时值 */
		CAny ValueOnUpdate;					/**< 更新数据时的值 */
		uint32_t Length;					/**< 长度或精度 */
		uint32_t Scale;						/**< 小数点后的位数 */
		CIconv::CharsetType Charset;		/**< 字符集 */
		unordered_map<string, uint16_t> Values;		/**< ENUM的选项值 */
		vector<string> FlipValues;			/**< ENUM的选项值（反转） */
		string Comment;						/**< 备注 */
		uint64_t Position;					/**< 固定长度字段的位置 */
		CField() = default;
		CField(const string& name, FieldType type, bool notnull = true, bool defdef = false, const CAny& defval = "", bool onupdatedef = false,
			   const CAny& valonupdate = "", uint32_t length = 0, uint32_t scale = 0, CIconv::CharsetType charset = CIconv::CHARSET_NONE,
			   const unordered_map<string, uint16_t>& values = {}, const vector<string>& flipvalues = {}, const string& comment = "") noexcept
			: Name(name), Type(type), NotNull(notnull), DefaultDefined(defdef), OnUpdateDefined(onupdatedef), Length(length), Scale(scale),
			  Charset(charset), Values(values), FlipValues(flipvalues), Comment(comment), Position(0)
		{
			DefaultValue = defval;
			ValueOnUpdate = valonupdate;
		}
	};

	size_t GetFieldLength(const CField& field) const noexcept;

	size_t ComputeFixedRowLength() noexcept;

	void GetInputValue(CPack& pack, bool ifexist, CAny* data, const string& fieldname, FieldType fieldtype, uint32_t length, uint32_t scale, CIconv::CharsetType charset, bool defdef, const CAny& defval, const unordered_map<string, uint16_t>& values);

	void EnumValuesToVector(const string& rawvalues, unordered_map<string, uint16_t>& values, vector<string>& flipvalues);

	virtual void IncreaseAutoInc(void* id) noexcept = 0;

	template <typename IdType>
	IdType GetRowId(bool ifmissing, const CAny& v)
	{
		IdType id;
		if(ifmissing) {
			switch(Fields[0].Type) {
			case FT_SERIAL16:
			case FT_SERIAL32:
			case FT_SERIAL64:
			case FT_SERIAL128:
				IncreaseAutoInc(&id);
				break;
			default:
				ThrowError(ERR_MISSING_DATA, "The value	of the field " + RowIdField + " is missing when inserting at the table " + Name + ".");
			}
		}
		else {
			switch(Fields[0].Type) {
			case FT_UINT8:
				switch(v.GetType()) {
				case FT_UINT8:
					id = v.ToUInt8();
					break;
				case FT_STRING:
					id = static_cast<uint8_t>(stoul(v.ToString()));
					break;
				case FT_UINT128:
					id = static_cast<uint8_t>(v.ToUInt128());
					break;
				case FT_INT32:
					id = static_cast<uint8_t>(v.ToInt32());
					break;
				case FT_INT64:
					id = static_cast<uint8_t>(v.ToInt64());
					break;
				default:
					ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong rowid type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(v.GetType())));
				}
				break;
			case FT_UINT16:
			case FT_SERIAL16:
				switch(v.GetType()) {
				case FT_UINT16:
					id = v.ToUInt16();
					break;
				case FT_STRING:
					id = static_cast<uint16_t>(stoul(v.ToString()));
					break;
				case FT_UINT128:
					id = static_cast<uint16_t>(v.ToUInt128());
					break;
				case FT_INT32:
					id = static_cast<uint16_t>(v.ToInt32());
					break;
				case FT_INT64:
					id = static_cast<uint16_t>(v.ToInt64());
					break;
				default:
					ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong rowid type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(v.GetType())));
				}
				break;
			case FT_UINT32:
			case FT_SERIAL32:
				switch(v.GetType()) {
				case FT_UINT32:
					id = v.ToUInt32();
					break;
				case FT_STRING:
					id = static_cast<uint32_t>(stoul(v.ToString()));
					break;
				case FT_UINT128:
					id = static_cast<uint32_t>(v.ToUInt128());
					break;
				case FT_INT32:
					id = static_cast<uint32_t>(v.ToInt32());
					break;
				case FT_INT64:
					id = static_cast<uint32_t>(v.ToInt64());
					break;
				default:
					ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong rowid type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(v.GetType())));
				}
				break;
			case FT_UINT64:
			case FT_SERIAL64:
				switch(v.GetType()) {
				case FT_UINT64:
					id = v.ToUInt64();
					break;
				case FT_STRING:
					id = static_cast<uint64_t>(stoull(v.ToString()));
					break;
				case FT_UINT128:
					id = static_cast<uint64_t>(v.ToUInt128());
					break;
				case FT_INT32:
					id = static_cast<uint64_t>(v.ToInt32());
					break;
				case FT_INT64:
					id = static_cast<uint64_t>(v.ToInt64());
					break;
				default:
					ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong rowid type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(v.GetType())));
				}
				break;
			case FT_UINT128:
			case FT_SERIAL128:
				switch(v.GetType()) {
				case FT_UINT128:
					id = v.ToUInt128();
					break;
				case FT_STRING:
					id = static_cast<__uint128_t>(stoulll(v.ToString()));
					break;
				case FT_INT32:
					id = static_cast<__uint128_t>(v.ToInt32());
					break;
				case FT_INT64:
					id = static_cast<__uint128_t>(v.ToInt64());
					break;
				default:
					ThrowError(ERR_WRONG_DATA_TYPE, std::string("Wrong rowid type:") + CDefinition::FieldTypeToString(static_cast<FieldType>(v.GetType())));
				}
				break;
			default:
				ThrowError(ERR_WRONG_FIELD_TYPE, "The type(" + CDefinition::FieldTypeToString(Fields[0].Type) + ") of the field " + RowIdField + " is wrong when inserting at the table " + Name + ".");
			}
		}
		return id;
	}

	FieldType GetIdType() const
	{
		FieldType ft = Fields[0].Type;
		switch(ft) {
		case FT_SERIAL16:
			ft = FT_UINT16; break;
		case FT_SERIAL32:
			ft = FT_UINT32; break;
		case FT_SERIAL64:
			ft = FT_UINT64; break;
		case FT_SERIAL128:
			ft = FT_UINT128; break;
		default:
			break;
		}
		return ft;
	}

	template <typename IdType>
	void InsertResult(CPack& ret, IdType id)
	{
		ret.Put(static_cast<int64_t>(4 + sizeof(IdType)));
		ret.Put(static_cast<uint16_t>(RT_LAST_INSERT_ID));
		ret.Put(static_cast<uint16_t>(GetIdType()));
		ret.Put(id);
//		string idstr = num_to_string(id);
//		ret.Put(static_cast<int64_t>(2 + idstr.size()));
//		ret.Put(static_cast<uint16_t>(RT_LAST_INSERT_ID));
//		ret.Write(idstr.c_str(), idstr.size());
	}

	template <typename IdType>
	void ExecuteResult(CPack& ret, IdType affectedrows)
	{
		ret.Put(static_cast<int64_t>(4 + sizeof(IdType)));
		ret.Put(static_cast<uint16_t>(RT_AFFECTED_ROWS));
		ret.Put(static_cast<uint16_t>(GetIdType()));
		ret.Put(affectedrows);
//		string rowsstr = num_to_string(affectedrows);
//		ret.Put(static_cast<int64_t>(2 + rowsstr.size()));
//		ret.Put(static_cast<uint16_t>(RT_AFFECTED_ROWS));
//		ret.Write(rowsstr.c_str(), rowsstr.size());
	}

	template <typename IdType>
	void GetResult(CPack& ret, IdType id, CPack& row)
	{
		// 初始长度
		ret.Put(static_cast<int64_t>(4));
		// 返回类型
		ret.Put(static_cast<uint16_t>(RT_QUERY));
		// 如果id为0返回
		if(id == 0) {
			ret.Put(static_cast<uint16_t>(0));
			return;
		}
		// 数据条数
		ret.Put(static_cast<uint16_t>(1));
		// 写入id
		ret.Put(static_cast<uint16_t>(GetIdType()));
		ret.Put(id);
		// 按字段名逐一写入
		ret.Put(static_cast<uint16_t>(FieldNum - 1));
		for(uint16_t i = 1; i < FieldNum; i ++) {
			const CField* field = &Fields[i];
			ret.Put<uint16_t>(field->Name);
			switch(field->Type) {
			case FT_BOOL:
			{
				bool v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_BOOL));
				ret.Put(static_cast<uint8_t>(v));
				break;
			}
			case FT_BIT:
			{
				string v(128, '\0');
				__uint128_t bitint = 0;
				row.Get(bitint);
				for(uint16_t i = 0; i < 128; ++i) {
					if(bitint % 2 == 1) {
						v[i] = '1';
					}
					else {
						v[i] = '0';
					}
					bitint >>= 1;
				}
				rtrim(v, "0");
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(v);
				break;
			}
			case FT_INT8:
			{
				int8_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_INT8));
				ret.Put(v);
				break;
			}
			case FT_UINT8:
			{
				uint8_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_UINT8));
				ret.Put(v);
				break;
			}
			case FT_INT16:
			{
				int16_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_INT16));
				ret.Put(v);
				break;
			}
			case FT_UINT16:
			{
				uint16_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_UINT16));
				ret.Put(v);
				break;
			}
			case FT_INT32:
			{
				int32_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_INT32));
				ret.Put(v);
				break;
			}
			case FT_UINT32:
			{
				uint32_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_UINT32));
				ret.Put(v);
				break;
			}
			case FT_INT64:
			{
				int64_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_INT64));
				ret.Put(v);
				break;
			}
			case FT_UINT64:
			{
				uint64_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_UINT64));
				ret.Put(v);
				break;
			}
			case FT_INT128:
			{
				__int128_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_INT128));
				ret.Put(v);
				//ret.Put(static_cast<uint16_t>(FT_STRING));
				//ret.Put<int32_t>(num_to_string(v));
				break;
			}
			case FT_UINT128:
			{
				__uint128_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_UINT128));
				ret.Put(v);
				//ret.Put(static_cast<uint16_t>(FT_STRING));
				//ret.Put<int32_t>(num_to_string(v));
				break;
			}
			case FT_FLOAT32:
			{
				float v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_FLOAT32));
				ret.Put(v);
				break;
			}
			case FT_FLOAT64:
			{
				double v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_FLOAT64));
				ret.Put(v);
				break;
			}
			case FT_FLOAT128:
			{
				__float128 v;
				row.Get(v);
				//ret.Put(static_cast<uint16_t>(FT_FLOAT128));
				//ret.Put(v);
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(num_to_string(v));
				break;
			}
			case FT_DECIMAL64:
			{
				int64_t v;
				row.Get(v);
				CDecimal64 dec(field->Scale);
				dec.SetData(v);
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(dec.ToString(false));
				break;
			}
			case FT_DECIMAL128:
			{
				__int128_t v;
				row.Get(v);
				CDecimal128 dec(field->Scale);
				dec.SetData(v);
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(dec.ToString(false));
				break;
			}
			case FT_ENUM:
			{
				uint16_t v;
				row.Get(v);
				string sv;
				if(v > 0 && v <= field->FlipValues.size()) {
					sv = string(field->FlipValues[v - 1]);
				}
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(sv);
				break;
			}
			case FT_DATE:
			{
				CDate v;
				row.Read(&v, sizeof(CDate));
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(num_to_string(v.Year) + "-" + num_to_string(v.Month) + "-" + num_to_string(v.Day));
				break;
			}
			case FT_TIME:
			{
				int64_t v;
				row.Get(v);
				__float128 ldv = static_cast<__float128>(v) / CTime::NanoTime;
				v = static_cast<int64_t>(ldv);
				double fraction = static_cast<double>(fabsq(ldv - v));
				int64_t hour = v / 3600;
				int32_t leftseconds = v % 3600;
				int32_t minute = leftseconds / 60;
				int32_t second = leftseconds % 60;
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(num_to_string(hour) + ":" + pad_left_copy(num_to_string(::abs(minute)), 2, '0') + ":" + pad_left_copy(num_to_string(::abs(second)), 2, '0') + "." + num_to_string(fraction));
				break;
			}
			case FT_DATETIME:
			{
				CDateTime v;
				row.Read(&v, sizeof(CDate));
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(v.to_string());
				break;
			}
			case FT_TIMESTAMP:
			{
				int64_t v;
				row.Get(v);
				ret.Put(static_cast<uint16_t>(FT_INT64));
				ret.Put(v);
				break;
			}
			case FT_CHAR:
			{
				uint16_t chars;
				row.Get(chars);
				string v;
				row.Get<uint16_t>(v, field->Length);
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(v);
				break;
			}
			case FT_VARCHAR:
			{
				uint16_t chars;
				row.Get(chars);
				string v;
				row.Get<uint16_t>(v);
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(v);
				break;
			}
			case FT_TEXT:
			{
				uint32_t chars;
				row.Get(chars);
				string v;
				row.Get<uint32_t>(v);
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(v);
				break;
			}
			case FT_BINARY:
			{
				string v;
				row.Get<uint16_t>(v, field->Length);
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(v);
				break;
			}
			case FT_VARBINARY:
			{
				string v;
				row.Get<uint16_t>(v);
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(v);
				break;
			}
			case FT_BLOB:
			{
				string v;
				row.Get<uint32_t>(v);
				ret.Put(static_cast<uint16_t>(FT_STRING));
				ret.Put<uint32_t>(v);
				break;
			}
			default:
				break;
			}
		}
		ret.Seek(0);
		ret.Put(static_cast<int64_t>(ret.GetSize() - 8));
	}

	string Path;				/**< 表所在目录 */
	string Name;				/**< 表名称 */
	uint16_t FieldNum;			/**< 字段数量 */
	CMap<string, CField> Fields;/**< 字段 */
	unordered_map<string, CIndex> Indexes;/**< 索引 */
	TableType Engine;			/**< 表引擎 */
	string RowIdField;			/**< rowid字段名 */
	FieldType RowIdType;		/**< rowid类型 */
	uint64_t RowLength;			/**< 当此表为TT_MEMORY类型时，RowLength为每行数据的最大长度；当表为TT_HARDDISK类型时，RowLength为每行固定长度数据的总长度。 */
	uint64_t MaxRows;			/**< 最大行数 */
	uint64_t MinRows;			/**< 最小行数 */
	uint32_t LifeTime;			/**< 数据的有效时间，为0表示长期有效，单位为秒 */
	//vector<bool> RowLockState;/**< 行锁定的状态，true为被锁定，false为未被锁定 */
	//vector<bool> RowLockType;	/**< 行锁定的类型，true为读，false为写 */
};

}
