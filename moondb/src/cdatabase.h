#pragma once

#include "header.h"
#include "cfixedmemorystorage.hpp"

using namespace std;

namespace MoonDb {

class CDatabase
{
public:
	enum PrivilegeType {
		PT_SUPER = 1,
		PT_USER = 2,
		PT_MAMANGE = 4,
		PT_SCHEMA = 8,
		PT_SELECT = 16,
		PT_INSERT = 32,
		PT_UPDATE = 64,
		PT_DELETE = 128
	};

	struct CUser {
		string Password;
		uint8_t Status;
		string Hostname;
		bitset<64> Privileges;

		CUser() = default;

		CUser(const string& password, uint8_t status, const string& hostname, const bitset<64>& privileges) noexcept
		{
			Password = password;
			Status = status;
			Hostname = hostname;
			Privileges = privileges;
		}
	};

	CDatabase() = default;
	CDatabase(const string& path);
	~CDatabase();
	inline void SetMutex(shared_timed_mutex* mutex) noexcept
	{
		Mutex = mutex;
	}
	inline shared_timed_mutex* GetMutex() noexcept
	{
		return Mutex;
	}
	void Open(const string& path);
	void Close() noexcept;
	void Create(const string& path);
	void AddUser(const string& username, const string& password, uint8_t status, const string& hostname, const bitset<64>& privileges);
	void CreateTable(const string& name, TableType engine, vector<CRawField>& fields, vector<CIndex>& indexes, uint64_t maxrows = 0,
					 uint64_t minrows = 0, uint32_t lifetime = 0);
	inline CTable* GetTable(const string& name) noexcept
	{
		auto it = Tables.find(name);
		if(it == Tables.end()) {
			return nullptr;
		}
		else {
			return it->second;
		}
	}

protected:
	const uint16_t UserDataSize = 537;
	const string Salt1 = "s8P+@dki82~#^(g-]\\]fjdl65WD";
	const string Salt2 = "NN/.[~=hks`s7vp52019z,t8fd9}";

	string Path;
	map<string, CUser> Users;
	map<string, CTable*> Tables;

	CTable* CreateTableObject(const string& name, TableType engine, FieldType rowidtype);
	void LoadTable(const string& name);

	shared_timed_mutex* Mutex;
};

}
