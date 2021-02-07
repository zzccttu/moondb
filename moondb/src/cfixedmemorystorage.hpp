#pragma once

#include "cfixedmap.hpp"
#include "ctable.h"

namespace MoonDb {

template <typename IdType>
class CFixedMemoryStorage : public CTable
{
public:
	CFixedMemoryStorage() : AutoInc(0)
	{}

	~CFixedMemoryStorage(){}

	bool Open(const string& path, const string& name)
	{
		CTable::Open(path, name);
		// 如果不指定，最大行数为32位无符号整数的最大值，最小行数为100
		Contents.initialize(MaxRows > 0 ? MaxRows : num_limits<uint32_t>::max(), RowLength, MinRows > 0 ? MinRows : 100);
		return true;
	}

	void InsertData(unordered_map<string, CAny>& data, CPack& ret)
	{
		auto dit = data.find(RowIdField);
		//Mutex.lock();
		IdType id = GetRowId<IdType>(dit == data.end(), dit->second);
		void* dp = Contents.insert(id, LifeTime);
		if(nullptr == dp) {
			//Mutex.unlock();
			ThrowError(ERR_DUPLICATE_ID, "Duplicate rowid:" + num_to_string(static_cast<__uint128_t>(id)) + " when inserting data in the table " + Name + ".");
			return;
		}
		CPack pack(dp, RowLength);

		/*auto it = Contents.emplace(id, CPack());
		if(!it.second) {
			ThrowError(ERR_DUPLICATE_ID, "Duplicate rowid:" + num_to_string(static_cast<__uint128_t>(id)) + " when inserting data in the table " + Name + ".");
			return;
		}
		CPack& pack = it.first->second;
		pack.Allocate(RowLength);*/

		for(uint16_t i = 1; i < FieldNum; i ++) {
			const CField* field = &Fields[i];
			auto dit = data.find(field->Name);
			bool ifexist = dit != data.end();
			GetInputValue(pack, ifexist, ifexist ? &dit->second : nullptr, field->Name, field->Type, field->Length, field->Scale, field->Charset, field->DefaultDefined, field->DefaultValue, field->Values);
		}

		//Mutex.unlock();

		InsertResult<IdType>(ret, id);
	}

	void UpdateData(const CAny& rowid, unordered_map<string, CAny>& data, CPack& ret)
	{
		IdType id = GetRowId<IdType>(false, rowid);
		//Mutex.lock();
		void* dp = Contents.update(id, LifeTime);
		if(nullptr == dp) {
			//Mutex.unlock();
			ExecuteResult<IdType>(ret, 0);
			return;
		}
		CPack row(dp, RowLength);
		row.SetSize(RowLength);
		/*auto it = Contents.find(id);
		if(it == Contents.end()) {
			return;
		}
		CPack& pack = it->second;*/

		for(uint16_t i = 1; i < FieldNum; i ++) {
			const CField* field = &Fields[i];
			auto dit = data.find(field->Name);
			bool ifexist = dit != data.end();
			if(!ifexist && !field->OnUpdateDefined) {
				//pack.MoveAhead(GetFieldLength(*field));
				continue;
			}
			row.Seek(static_cast<int64_t>(field->Position));
			GetInputValue(row, ifexist, ifexist ? &dit->second : nullptr, field->Name, field->Type, field->Length, field->Scale, field->Charset, field->OnUpdateDefined, field->ValueOnUpdate, field->Values);
		}
		//Mutex.unlock();
		ExecuteResult<IdType>(ret, 1);
	}

	void ReplaceData(const CAny& rowid, unordered_map<string, CAny>& data, CPack& ret)
	{
		IdType id = GetRowId<IdType>(false, rowid);
		//Mutex.lock();
		CPack pack(Contents.replace(id, LifeTime), RowLength);
		for(uint16_t i = 1; i < FieldNum; i ++) {
			const CField* field = &Fields[i];
			auto dit = data.find(field->Name);
			bool ifexist = dit != data.end();
			GetInputValue(pack, ifexist, ifexist ? &dit->second : nullptr, field->Name, field->Type, field->Length, field->Scale, field->Charset, field->DefaultDefined, field->DefaultValue, field->Values);
		}
		//Mutex.unlock();
		ExecuteResult<IdType>(ret, 1);
	}

	void DeleteData(const CAny& rowid, CPack& ret)
	{
		IdType id = GetRowId<IdType>(false, rowid);
		IdType affectedrows;
		//Mutex.lock();
		if(Contents.erase(id)) {
			affectedrows = 1;
		}
		else {
			affectedrows = 0;
		}
		//Mutex.unlock();
		ExecuteResult<IdType>(ret, affectedrows);
	}

	void GetData(const CAny& rowid, CPack& ret)
	{
		IdType id = GetRowId<IdType>(false, rowid);
		//Mutex.lock_shared();
		void* dp = Contents.at(id);
		CPack row(dp, RowLength);
		if(nullptr == dp) {
			//Mutex.unlock_shared();
			GetResult<IdType>(ret, 0, row);
			return;
		}
		row.SetSize(RowLength);
		/*auto it = Contents.find(id);
		if(it == Contents.end()) {
			return;
		}
		CPack& pack = it->second;*/

		GetResult<IdType>(ret, id, row);

		/*map<string, CAny> data;
		for(uint16_t i = 1; i < FieldNum; i ++) {
			const CField* field = &Fields[i];
			data.emplace(field->Name, CAny(pack, field->Type, field->Length, field->FlipValues));
		}*/

		//Mutex.unlock_shared();
	}

protected:
	IdType AutoInc;		/**< 自增id数值 */

	inline IdType MaxIdValue() const noexcept
	{
		return num_limits<IdType>::max();
	}

	void IncreaseAutoInc(void* id) noexcept
	{
		++AutoInc;
		::memcpy(id, &AutoInc, sizeof(IdType));
	}

	CFixedMap<IdType> Contents;
	//map<IdType, CPack> Contents;
};

}
