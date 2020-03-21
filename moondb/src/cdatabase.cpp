#include "cdatabase.h"

namespace MoonDb {

CDatabase::CDatabase(const string& path)
{
	Open(path);
}

CDatabase::~CDatabase()
{
	Close();
}

void CDatabase::Open(const string& path)
{
	Path = path;
	standardize_path(Path);
	if(!CFileSystem::IsDirectory(Path)) {
		ThrowError(ERR_DB_NOT_EXIST, "The database (" + Path + ") doesn't exist.");
		return;
	}

	string userfile = Path + DIRECTORY_SEPARATOR + "users.data";
	if(CFileSystem::IsFile(userfile)) {
		CPack pack;
		CFile file;
		file.Open(userfile, CFile::ONLY_READ);
		file.ReadAll(pack);
		file.Close();

		size_t packsize = pack.GetSize();
		if(packsize % UserDataSize != 0) {
			ThrowError(ERR_DB_USER_FILE_VALID, "Invalid user file (" + userfile + ").");
			return;
		}
		uint32_t num = static_cast<uint32_t>(packsize / UserDataSize);
		for(uint32_t i = 0; i < num; i ++) {
			string username;
			pack.Get<uint8_t>(username, 255);
			string password(16, '\0');
			pack.Read(&password.front(), 16);
			uint8_t status;
			pack.Get(status);
			string hostname;
			pack.Get<uint8_t>(hostname, 255);
			uint64_t privileges_int;
			pack.Get(privileges_int);
			bitset<64> privileges(privileges_int);
			Users[username] = CUser(password, status, hostname, privileges);
			//cout << username << ", " << base64_encode(password) << ", " << (int)status << ", " << hostname << ", " << privileges.num_to_string() << endl;
		}
	}

	vector<string> tablenames;
	CFileSystem::ReadDirectoryOnlyDirectory(Path, tablenames, true);
	for(size_t i = 0; i < tablenames.size(); i++) {
		LoadTable(tablenames[i]);
	}
}

void CDatabase::Close() noexcept
{
	Users.clear();
	for(auto it = Tables.begin(); it != Tables.end(); it++)	{
		delete it->second;
	}
	Tables.clear();
	Path.clear();
}

void CDatabase::Create(const string& path)
{
	Path = path;
	standardize_path(Path);
	if(!CFileSystem::IsDirectory(Path)) {
		if(!CFileSystem::CreateDirectories(Path)) {
			ThrowError(ERR_CREATE_DIRECTORY, "Can't create the directory (" + Path + ").");
			return;
		}
	}
	if(!CFileSystem::IsFile(Path + DIRECTORY_SEPARATOR + "users.data")) {
		CFile file;
		file.Open(Path + DIRECTORY_SEPARATOR + "users.data", CFile::ONLY_WRITE);
		file.Close();
	}
	else {
		ThrowError(ERR_DB_EXIST, "The database (" + Path + ") has existed.");
	}
}

void CDatabase::AddUser(const string& username, const string& password, uint8_t status, const string& hostname, const bitset<64>& privileges)
{
	if(Users.find(username) != Users.end()) {
		ThrowError(ERR_DB_USER_EXIST, "The database user(" + username + ") has existed.");
		return;
	}
	CPack pack(UserDataSize);
	pack.Put<uint8_t>(username, 255, '\0');
	pack.Write(&md5(Salt1 + password + Salt2, true).front(), 16);
	pack.Put(status);
	pack.Put<uint8_t>(hostname, 255, '\0');
	pack.Put(static_cast<uint64_t>(privileges.to_ullong()));

	CFile file;
	file.Open(Path + DIRECTORY_SEPARATOR + "users.data", CFile::ONLY_APPEND);
	file.Write(pack.GetPointer(), pack.GetSize());
	file.Close();
}

CTable* CDatabase::CreateTableObject(const string& name, TableType engine, FieldType rowidtype)
{
	CTable* table = nullptr;
	switch(rowidtype) {
	case FT_UINT8:
		switch(engine) {
		case TT_FIXMEMORY:
			table = new CFixedMemoryStorage<uint8_t>();
			break;
		default:
			ThrowError(ERR_WRONG_ENGINE, "The table engine " + CDefinition::TableTypeToString(engine) + " of the table " + name + " isn't correct.");
		}
		break;
	case FT_UINT16:
	case FT_SERIAL16:
		switch(engine) {
		case TT_FIXMEMORY:
			table = new CFixedMemoryStorage<uint16_t>();
			break;
		default:
			ThrowError(ERR_WRONG_ENGINE, "The table engine " + CDefinition::TableTypeToString(engine) + " of the table " + name + " isn't correct.");
		}
		break;
	case FT_UINT32:
	case FT_SERIAL32:
		switch(engine) {
		case TT_FIXMEMORY:
			table = new CFixedMemoryStorage<uint32_t>();
			break;
		default:
			ThrowError(ERR_WRONG_ENGINE, "The table engine " + CDefinition::TableTypeToString(engine) + " of the table " + name + " isn't correct.");
		}
		break;
	case FT_UINT64:
	case FT_SERIAL64:
		switch(engine) {
		case TT_FIXMEMORY:
			table = new CFixedMemoryStorage<uint64_t>();
			break;
		default:
			ThrowError(ERR_WRONG_ENGINE, "The table engine " + CDefinition::TableTypeToString(engine) + " of the table " + name + " isn't correct.");
		}
		break;
	case FT_UINT128:
	case FT_SERIAL128:
		switch(engine) {
		case TT_FIXMEMORY:
			table = new CFixedMemoryStorage<__uint128_t>();
			break;
		default:
			ThrowError(ERR_WRONG_ENGINE, "The table engine " + CDefinition::TableTypeToString(engine) + " of the table " + name + " isn't correct.");
		}
		break;
	default:
		ThrowError(ERR_WRONG_ROWID, "The rowid type " + CDefinition::FieldTypeToString(rowidtype) + " in the table " + name + " isn't correct.");
	}
	return table;
}

void CDatabase::CreateTable(const string& name, TableType engine, vector<CRawField>& fields, vector<CIndex>& indexes, uint64_t maxrows,
							uint64_t minrows, uint32_t lifetime)
{
	// 确定主键类型，如果主键不存在或者定义的主键为非整数型，那么程序自动创建一个rowid字段
	FieldType rowidtype = FT_SERIAL64;
	bool rowidfound = false;
	for(size_t i = 0; i < indexes.size(); i++) {
		CIndex* index = &indexes[i];
		if(IT_PRIMARY == index->Type) {
			if(index->Fields.size() == 1) {
				for(size_t j = 0; j < fields.size(); j++) {
					CRawField* field = &fields[j];
					if(field->Type == FT_UINT8 || field->Type == FT_UINT16 || field->Type == FT_UINT32 || field->Type == FT_UINT64 ||
						field->Type == FT_UINT128 || field->Type == FT_SERIAL16 || field->Type == FT_SERIAL32 || field->Type == FT_SERIAL64 ||
						field->Type == FT_SERIAL128) {
						rowidtype = field->Type;
						rowidfound = true;
					}
				}
			}
			break;
		}
	}
	if(!rowidfound) {
		fields.insert(fields.begin(), CRawField("rowid", FT_SERIAL64));
		indexes.insert(indexes.begin(), CIndex("rowid", IT_ROWID, IM_BTREE, vector<string>{"rowid"}));
	}
	// 创建表
	CTable* table = nullptr;
	try {
		table = CreateTableObject(name, engine, rowidtype);
		table->Create(Path, name, engine, rowidtype, fields, indexes, maxrows, minrows, lifetime);
		Tables[name] = table;
	}
	catch(exception& e) {
		if(table != nullptr) {
			delete table;
		}
		throw e;
		//cout << e.What() << endl;
	}
}

void CDatabase::LoadTable(const string& name)
{
	CTable* table = nullptr;
	TableType engine = TT_NONE;
	FieldType rowidtype = FT_NONE;
	if(!CTable::GetTableProperty(Path, name, engine, rowidtype)) {
		return;
	}
	table = CreateTableObject(name, engine, rowidtype);
	if(nullptr == table) {
		return;
	}
	if(!table->Open(Path, name)) {
		return;
	}
	Tables[name] = table;
}

}
