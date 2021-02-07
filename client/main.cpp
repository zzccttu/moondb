#include <future>
#include <thread>

#include "cmoondbclient.h"
#include "crandom.hpp"

using namespace MoonDb;

__uint128_t insert()
{
	CMoonDbClient client("127.0.0.1", 8888, "test");
	map<string, CAny> data;
	data["title"] = "abc";
	data["content"] = "hgdfgd";
	data["price"] = 10.0;
	data["hits"] = 2;
	__uint128_t lastinsertid = 0;
	for(uint32_t i = 0; i < 50000;i ++)
		lastinsertid = client.InsertData("testtable", data);
	return lastinsertid;
}

void select()
{
	CMoonDbClient client("127.0.0.1", 8888, "test");
	for(uint32_t i = 0; i < 50000; i++) {
		map<string, CAny> data;
		client.GetData("testtable", i + 1, data);
	}
}

void generaltest()
{
	try {
		CMoonDbClient client("127.0.0.1", 8888, "test");
		map<string, CAny> data;
		map<string, CAny> data2;

		data["title"] = "abc";
		data["content"] = "hgdfgd";
		data["price"] = 10.0;
		data["hits"] = 2;
		__uint128_t lastinsertid = client.InsertData("testtable", data);

		client.GetData("testtable", lastinsertid, data2);
		client.UpdateData("testtable", lastinsertid, data);
		client.DeleteData("testtable", lastinsertid);

		data["title"] = "abc3";
		data["content"] = "replace2";
		data["price"] = 30.0;
		data["hits"] = 6;
		client.ReplaceData("testtable", lastinsertid, data);

		uint32_t times = 50;
		for(uint32_t i = 0; i < times; i++)
			client.GetData("testtable", lastinsertid, data2);
	}
	catch(runtime_error& e) {
		cout << e.what() << endl;
	}
}

int main()
{
//	string str = "ab";
//	str.push_back('\0');
//	str += "'\"\b\n\r\t\\ghggh";
//	cout << CMoonDbClient::Quote(str) << endl;
//	return 0;

#if defined(_WIN32)
	WSADATA ws;
	if (::WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
		ThrowError(ERR_SOCKET, "Init Windows Socket Failed:" + MoonLastError());
	}
#endif

	try {
//		// insert
//		CMoonDbClient client("127.0.0.1", 8888, "test");
//		map<string, CAny> data;
//		data["title"] = "abc";
//		data["content"] = "hgdfgd";
//		data["price"] = 10.0;
//		data["hits"] = 2;
//		__uint128_t lastinsertid = client.InsertData("testtable", data);
//		cout <<  lastinsertid << endl;

		//get
//		CMoonDbClient client("127.0.0.1", 8888, "test");
//		map<string, CAny> data;
//		client.GetData("testtable", 2, data);
//		cout << data << endl;

		//delete
//		CMoonDbClient client("127.0.0.1", 8888, "test");
//		cout << client.DeleteData("testtable", 2) << endl;

		//update
//		CMoonDbClient client("127.0.0.1", 8888, "test");
//		map<string, CAny> data;
//		data["title"] = "abc3";
//		data["content"] = "hgdfgd3";
//		data["price"] = 30.0;
//		data["hits"] = 6;
//		cout << client.UpdateData("testtable", 1, data) << endl;

		//replace
//		CMoonDbClient client("127.0.0.1", 8888, "test");
//		map<string, CAny> data;
//		data["title"] = "abc33";
//		data["content"] = "replace22";
//		data["price"] = 31.0;
//		data["hits"] = 62;
//		cout << client.ReplaceData("testtable", 2, data) << endl;



		auto time1 = CTime::Now();
		thread thpool[1000];
		for(uint32_t i = 0; i < 250; i++) {
			thpool[i] = thread(&generaltest);
		}
		timespec t;
		t.tv_sec = 0;
		t.tv_nsec = 1;
		for(uint32_t i = 250; i < 500; i++) {
			thpool[i] = thread(&generaltest);
			::nanosleep(&t, nullptr);
		}
		for(uint32_t i = 0; i < 500; i++) {
			thpool[i].join();
		}
		auto time2 = CTime::Now();
		cout << (time2 - time1) * CTime::TimeRatio << endl;


//		insert();
//		auto time1 = CTime::Now();
//		thread th1([]() {
//			select();
//		});
//		thread th2([]() {
//			select();
//		});
//		thread th3([]() {
//			select();
//		});
//		thread th4([]() {
//			select();
//		});
//		th1.join();
//		th2.join();
//		th3.join();
//		th4.join();
//		auto time2 = CTime::Now();
//		cout << (time2 - time1) * CTime::TimeRatio << endl;

//		auto time1 = CTime::Now();
//		future<__uint128_t> fut1 = async(insert);
//		future<__uint128_t> fut2 = async(insert);
//		future<__uint128_t> fut3 = async(insert);
//		future<__uint128_t> fut4 = async(insert);
//		__uint128_t lastinsertid1 = fut1.get();
//		__uint128_t lastinsertid2 = fut2.get();
//		__uint128_t lastinsertid3 = fut3.get();
//		__uint128_t lastinsertid4 = fut4.get();
//		auto time2 = CTime::Now();
//		cout << lastinsertid1 << "," << lastinsertid2 << "," << lastinsertid3 << "," << lastinsertid4 << endl;
//		cout << (time2 - time1) * CTime::TimeRatio << endl;

		/*auto time1 = CTime::Now();
		CMoonDbClient client("127.0.0.1", 8888, "test");
		//CMoonDbClient client("::1", 8888, "test");
		auto time2 = CTime::Now();
		cout << (time2 - time1) * CTime::TimeRatio << endl;

		map<string, CAny> retdata;
		{
			map<string, CAny> data;
			data["title"] = "abc";
			data["content"] = "hgdfgd";
			data["price"] = 10.0;
			data["hits"] = 2;
			cout << "insert: " << client.InsertData("testtable", data) << endl;
		}
		if(client.GetData("testtable", 1, retdata)) {
			cout << retdata << endl;
		}

		{
			map<string, CAny> data;
			data["title"] = "abc2";
			data["content"] = "hgdfgd2";
			data["price"] = 20.0;
			data["hits"] = 4;
			cout << "update: " << client.UpdateData("testtable", 1, data) << endl;
		}
		if(client.GetData("testtable", 1, retdata)) {
			cout << retdata << endl;
		}

		cout << "delete: " << client.DeleteData("testtable", 1) << endl;
		if(client.GetData("testtable", 1, retdata)) {
			cout << retdata << endl;
		}

		{
			map<string, CAny> data;
			data["title"] = "abc_r";
			data["content"] = "replace";
			data["price"] = 30.0;
			data["hits"] = 6;
			cout << "replace: " << client.ReplaceData("testtable", 1, data) << endl;
		}
		if(client.GetData("testtable", 1, retdata)) {
			cout << retdata << endl;
		}*/
	}
	catch(runtime_error& e) {
		cout << e.what() << endl;
	}

#if defined(_WIN32)
	::WSACleanup();
#endif
	return 0;
}
