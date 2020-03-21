#include "ctable.h"

namespace MoonDb {

CTable::~CTable()
{
}

bool CTable::GetTableProperty(const string& path, const string& name, TableType& engine, FieldType& rowidtype)
{
	string schemafile = path + DIRECTORY_SEPARATOR + name + DIRECTORY_SEPARATOR + "schema.moon";
	if(!CFileSystem::Exists(schemafile)) {
		return false;
	}
	CFile file;
	file.Open(schemafile, CFile::ONLY_READ);
	uint16_t engine_code;
	file.Get(engine_code);
	engine = static_cast<TableType>(engine_code);
	uint16_t rowidtype_code;
	file.Get(rowidtype_code);
	rowidtype = static_cast<FieldType>(rowidtype_code);
	file.Close();
	return true;
}

bool CTable::Open(const string& path, const string& name)
{
	Path = path + DIRECTORY_SEPARATOR + name;
	Name = name;

	string schemafile = Path + DIRECTORY_SEPARATOR + "schema.moon";
	if(!CFileSystem::IsFile(schemafile)) {
		ThrowError(ERR_TABLE_NOT_EXIST, "The table " + Name + " doesn't exist.");
		return false;
	}

	CPack pack;
	CFile file;
	file.Open(schemafile, CFile::ONLY_READ);
	file.ReadAll(pack);
	file.Close();

	uint16_t engine;
	pack.Get(engine);
	Engine = static_cast<TableType>(engine);
	uint16_t rowidtype;
	pack.Get(rowidtype);
	RowIdType = static_cast<FieldType>(rowidtype);
	pack.Get(MaxRows);
	pack.Get(MinRows);
	pack.Get(LifeTime);

	// 读取字段
	pack.Get(FieldNum);
	Fields.reserve(FieldNum);
	for(uint16_t i = 0; i < FieldNum; ++i) {
		string fieldname;
		pack.Get<uint8_t>(fieldname);
		CField& field = Fields.insert(fieldname);
		field.Name = fieldname;
		uint16_t type;
		pack.Get(type);
		field.Type = static_cast<FieldType>(type);
		pack.Get(field.NotNull);
		pack.Get(field.DefaultDefined);
		string defval;
		pack.Get<uint32_t>(defval);
		pack.Get(field.OnUpdateDefined);
		string updval;
		pack.Get<uint32_t>(updval);
		pack.Get(field.Length);
		pack.Get(field.Scale);
		uint16_t charset;
		pack.Get(charset);
		field.Charset = static_cast<CIconv::CharsetType>(charset);
		string rawvalues;
		pack.Get<uint32_t>(rawvalues);
		vector<string> flipvalues;
		EnumValuesToVector(rawvalues, field.Values, flipvalues);
		pack.Get<uint32_t>(field.Comment);
		if(field.DefaultDefined) {
			field.DefaultValue = CAny(field.Type, field.Length, field.Charset, defval, field.Values);
		}
		if(field.OnUpdateDefined) {
			field.ValueOnUpdate = CAny(field.Type, field.Length, field.Charset, updval, field.Values);
		}
	}

	RowLength = ComputeFixedRowLength();

	// 读取索引
	uint8_t indexnum = 0;
	pack.Get(indexnum);
	for(uint8_t i = 0; i < indexnum; ++i) {
		string name;
		pack.Get<uint8_t>(name);
		uint16_t type;
		pack.Get(type);
		uint16_t mode;
		pack.Get(mode);
		uint8_t fieldnum;
		pack.Get(fieldnum);
		string fieldkey;
		vector<string> fields;
		fields.reserve(fieldnum);
		for(uint8_t j = 0; j < fieldnum; ++j) {
			string field;
			pack.Get<uint8_t>(field);
			fields.push_back(field);
			fieldkey += field + ",";
		}
		fieldkey = substr(fieldkey, 0, -1);
		Indexes[fieldkey] = CIndex(name, static_cast<IndexType>(type), static_cast<IndexMode>(mode), fields);
		if(RowIdField.empty()) {
			if(type == IT_ROWID) {
				RowIdField = "rowid";
			}
			else if(type == IT_PRIMARY) {
				RowIdField = fields[0];
			}
		}
	}

	return true;
}

bool CTable::Create(const string& path, const string& name, TableType engine, FieldType rowidtype, const vector<CRawField>& fields,
					const vector<CIndex>& indexes, uint64_t maxrows, uint64_t minrows, uint32_t lifetime)
{
	Path = path	+ DIRECTORY_SEPARATOR + name;
	if(!CFileSystem::IsDirectory(Path)) {
		if(!CFileSystem::CreateDirectories(Path)) {
			ThrowError(ERR_CREATE_DIRECTORY, "Can't create the directory (" + Path + ").");
			return false;
		}
	}
	Name = name;

	string schemafile = Path + DIRECTORY_SEPARATOR + "schema.moon";
	if(CFileSystem::IsFile(schemafile)) {
		ThrowError(ERR_TABLE_EXIST, "The table " + Name + " has existed.");
		return false;
	}

	Engine = engine;
	RowIdType = rowidtype;
	MaxRows = maxrows;
	MinRows = minrows;
	LifeTime = lifetime;

	FieldNum = static_cast<uint16_t>(fields.size());
	Fields.reserve(FieldNum);
	for(size_t i = 0; i < FieldNum; ++i) {
		const CRawField* rawfield = &fields[i];
		CField field;
		field.Name = rawfield->Name;
		field.Type = rawfield->Type;
		field.NotNull = rawfield->NotNull;
		field.Length = rawfield->Length;
		field.Scale = rawfield->Scale;
		field.Charset = rawfield->Charset;
		EnumValuesToVector(rawfield->Values, field.Values, field.FlipValues);
		field.DefaultDefined = rawfield->DefaultDefined;
		if(field.DefaultDefined) {
			field.DefaultValue = CAny(field.Type, field.Length, field.Charset, rawfield->DefaultValue, field.Values);
		}
		field.OnUpdateDefined = rawfield->OnUpdateDefined;
		if(field.OnUpdateDefined) {
			field.ValueOnUpdate = CAny(field.Type, field.Length, field.Charset, rawfield->ValueOnUpdate, field.Values);
		}
		field.Comment = rawfield->Comment;
		Fields.insert(rawfield->Name, field);
	}

	RowLength = ComputeFixedRowLength();

	for(size_t i = 0; i < indexes.size(); ++i) {
		const CIndex* index = &indexes[i];
		if(index->Type == IT_ROWID) {
			RowIdField = "rowid";
			break;
		}
		else if(index->Type == IT_PRIMARY) {
			RowIdField = index->Fields[0];
			break;
		}
	}

	CPack pack(4096);

	pack.Put(static_cast<uint16_t>(Engine));// 引擎类型
	pack.Put(static_cast<uint16_t>(RowIdType));//rowid类型
	pack.Put(MaxRows);
	pack.Put(MinRows);
	pack.Put(LifeTime);

	// 字段定义
	pack.Put(FieldNum);// 字段数量
	for(size_t i = 0; i < FieldNum; ++i) {
		const CRawField* rawfield = &fields[i];
		pack.Put<uint8_t>(rawfield->Name);
		pack.Put(static_cast<uint16_t>(rawfield->Type));
		pack.Put(rawfield->NotNull);
		pack.Put(rawfield->DefaultDefined);
		pack.Put<uint32_t>(rawfield->DefaultValue);
		pack.Put(rawfield->OnUpdateDefined);
		pack.Put<uint32_t>(rawfield->ValueOnUpdate);
		pack.Put(rawfield->Length);
		pack.Put(rawfield->Scale);
		pack.Put(static_cast<uint16_t>(rawfield->Charset));
		pack.Put<uint32_t>(rawfield->Values);
		pack.Put<uint32_t>(rawfield->Comment);
	}

	// 索引定义
	pack.Put(static_cast<uint8_t>(indexes.size()));
	for(size_t i = 0; i < indexes.size(); ++i) {
		const CIndex* index = &indexes[i];
		pack.Put<uint8_t>(index->Name);
		pack.Put(static_cast<uint16_t>(index->Type));
		pack.Put(static_cast<uint16_t>(index->Mode));
		pack.Put(static_cast<uint8_t>(index->Fields.size()));
		for(size_t j = 0; j < index->Fields.size(); ++j) {
			pack.Put<uint8_t>(index->Fields[j]);
		}
	}

	CFile file;
	file.Open(schemafile, CFile::ONLY_WRITE);
	file.Write(pack.GetPointer(), pack.GetSize());
	file.Close();

	return true;
}

size_t CTable::GetFieldLength(const CField& field) const noexcept
{
	switch(field.Type) {
	case FT_BOOL:
		return 1;
	case FT_BIT:// Length最多为128，返回最大为16
		switch(static_cast<size_t>(::ceil(static_cast<double>(field.Length) / static_cast<double>(8)))) {
		case 1:
			return 1;
		case 2:
			return 2;
		case 3:
		case 4:
			return 4;
		case 5:
		case 6:
		case 7:
		case 8:
			return 8;
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
			return 16;
		default:
			return 0;
		}
	case FT_INT8:
		return 1;
	case FT_UINT8:
		return 1;
	case FT_INT16:
		return 2;
	case FT_UINT16:
		return 2;
	case FT_INT32:
		return 4;
	case FT_UINT32:
		return 4;
	case FT_INT64:
		return 8;
	case FT_UINT64:
		return 8;
	case FT_INT128:
		return 16;
	case FT_UINT128:
		return 16;
	case FT_SERIAL16:
		return 2;
	case FT_SERIAL32:
		return 4;
	case FT_SERIAL64:
		return 8;
	case FT_SERIAL128:
		return 16;
	case FT_FLOAT:
		return 4;
	case FT_DOUBLE:
		return 8;
	case FT_LONGDOUBLE:
		return 16;
	case FT_DECIMAL:
		return field.Length + 2;// 加上负号和小数点的1个字节
	case FT_ENUM:
		return 2;
	case FT_DATE:
		return 3;
	case FT_TIME:
		return 8;
	case FT_DATETIME:
		return 9;
	case FT_TIMESTAMP:
		return 8;
	case FT_CHAR:// 2个字节实际长度，2个字节字符数（根据字符集不同而不同）
		return 4 + field.Length;
	case FT_VARCHAR:
		return 4 + field.Length;
	case FT_BINARY://2个字节长度
		return 2 + field.Length;
	case FT_VARBINARY:
		return 2 + field.Length;
	case FT_TEXT:// 4个字节实际长度，4个字节字符数（根据字符集不同而不同）
		return 8 + numeric_limits<uint32_t>::max();
	case FT_BLOB://4个字节长度
		return 4 + numeric_limits<uint32_t>::max();
	case FT_MPINT:
		return 4 + sizeof(mp_limb_t) * field.Length;
	case FT_MPRATIONAL:
		return (4 + sizeof(mp_limb_t) * field.Length) * 2;
	default:
		return 0;
	}
}

size_t CTable::ComputeFixedRowLength() noexcept
{
	size_t length = 0;
	size_t fieldsize = Fields.size();
	for(size_t i = 1; i < fieldsize; ++i) {
		CField* field = &Fields[i];
		//if(FT_VARCHAR == field->Type || FT_VARBINARY == field->Type || FT_TEXT == field->Type || FT_BLOB == field->Type || ((FT_MPINT == field->Type || FT_MPRATIONAL) && field->Length == 0)) {
		//	continue;
		//}
		field->Position = length;
		length += GetFieldLength(*field);
	}
	return length;
}

void CTable::GetInputValue(CPack& pack, bool ifexist, CAny* data, const string& fieldname, FieldType fieldtype, uint32_t length, uint32_t scale, CIconv::CharsetType charset, bool defdef, const CAny& defval, const map<string, uint16_t>& values)
{
	if(!ifexist) {
		if(defdef) {
			defval.Store(pack, fieldtype, length, scale);
		}
		else {
			ThrowError(ERR_MISSING_DATA, "The value	of the field " + fieldname + " is missing when inserting at the table " + Name + ".");
		}
	}
	else {
		data->Store(pack, fieldtype, length, scale, charset, values);
	}
}

void CTable::EnumValuesToVector(const string& rawvalues, map<string, uint16_t>& values, vector<string>& flipvalues)
{
	if(rawvalues[0] != '\'' && rawvalues[0] != '"') {
		vector<string>flipvalues = explode(rawvalues, ',');
		for(uint16_t i = 0; i < flipvalues.size(); ++i) {
			values[flipvalues[i]] = i + 1;
		}
	}
	else {
		size_t size = rawvalues.size();
		uint16_t key = 1;
		size_t i = 0;
		while(i < size) {
			if(rawvalues[i] == '\'') {
				bool found = false;
				for(size_t j = i + 1; j < size; ++j) {
					// 如果遇到转义字符\'，那么跳过，继续匹配字符串
					if(j + 1 < size && rawvalues[j] == '\\' && rawvalues[j + 1] == '\'') {
						j++;
						continue;
					}
					if(rawvalues[j] == '\'') {
						string value = rawvalues.substr(i + 1, j - i - 1);
						str_replace("\\'", "'", value);
						flipvalues.emplace_back(value);
						values[value] = key;
						i = j;
						key++;
						found = true;
						break;
					}
				}
				if(!found) {
					string value = rawvalues.substr(i + 1, size - i);
					flipvalues.emplace_back(value);
					values[value] = key;
					break;
				}
			}
			else if(rawvalues[i] == '"') {
				bool found = false;
				for(size_t j = i + 1; j < size; ++j) {
					// 如果遇到转义字符\"，那么跳过，继续匹配字符串
					if(j + 1 < size && rawvalues[j] == '\\' && rawvalues[j + 1] == '"') {
						j++;
						continue;
					}
					if(rawvalues[j] == '"') {
						string value = rawvalues.substr(i + 1, j - i - 1);
						str_replace("\\\"", "\"", value);
						flipvalues.emplace_back(value);
						values[value] = key;
						i = j;
						key++;
						found = true;
						break;
					}
				}
				if(!found) {
					string value = rawvalues.substr(i + 1, size - i);
					flipvalues.emplace_back(value);
					values[value] = key;
					break;
				}
			}
			i++;
		}
	}
}

}
