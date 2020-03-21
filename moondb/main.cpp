#include "src/cservice.h"
#include <getopt.h>

//多线程测试
void ja()
{
  double a = 0.0;
  for(uint64_t i = 0; i < 500000000; i ++) {
	  a += sin(i * 1.05);
  }
  cout << "ja:" << a << endl;
}
void bar(int x)
{
	double a = 0.0;
	for(uint64_t i = 0; i < 300000000; i ++) {
		a += cos(i * 1.05);
	}
	cout << "bar:" << x << " " << a << endl;
}

double sin_a(double a) {
	return sin(a);
}

// 函数调用
int64_t increment (const int64_t& x) {return x + 1;}


void PrepareLunarCalendarData()
{
	CFile file;
	std::string path = "D:\\MyProgram\\VCS\\trunk\\LunarCalendarData\\";
	char buf[11];
	std::string data;
	for(uint16_t i = 1901; i < 1902; i ++) {
		sprintf(buf, "T%dc.txt", i);
		file.Open(path + buf, CFile::ONLY_READ);
		file.ReadAll(data);
		cout << data.c_str() << endl;
		file.Close();
	}
}

void test()
{
	cout << strlen("Failed to open the file (%s) as '%s' mode. ") << endl;

//        const char FileOpenModes[4][4] = {"rb", "wb+", "rb+", "ab+"};
//        FILE* fp = fopen("./aa.txt", FileOpenModes[0]);
//        char buf[10];
//        fread(buf, 1, 10, fp);
//        fclose(fp);
//        cout << buf << endl;

//        fstream fs;
//        fs.open("./data.txt", ios::in);
//        char buf[20];
//        fs.read(buf, 10);
//        fs.close();
//        cout << fs.gcount() << endl;

	//写文件
//		CFile file;
//		file.Open("./aa.txt", CFile::ONLYWRITE);
//		file.Write(string("hgjghj[]"));
//		file.Close();

	// 读文件
//        CFile file;
//		string filedata;
//		file.Open("./aa.txt", CFile::ONLYREAD);
//		//file.ReadAll(filedata);
//		file.GetLine(filedata,100);
//		file.Close();
//		cout << filedata << ", " << filedata.size() << endl;


	/*VCString a(5);
	a = "abcdefgh";
	VCString b("14");
	a = b + "??";cout << a.GetString() << endl;
	a = "";
	a += "6d;;sgfd[fl";
	string c = VCString(":abcd:");
	a += c;
	cout << a.GetString() << endl;*/
	//cout << a[2344] << endl;

	//多线程测试
//        std::thread first (ja);     // spawn new thread that calls ja()
//        std::thread second (bar,0);  // spawn new thread that calls bar(0)

//        std::cout << "main, ja and bar now execute concurrently...\n";

//        // synchronize threads:
//        first.join();                // pauses until first finishes
//        second.join();               // pauses until second finishes

//        std::cout << "ja and bar completed.\n";

	//cout << (double)(CTime::Duration(ja)) * CTime::Ratio() << endl;

	/*auto t1 = CTime::Now();
	double a = 1.0;
	for (uint64_t i = 0; i < 10000000; i++)
		a = sin_a(a);
	auto t2 = CTime::Now();
	cout << a << endl;
	cout << (t2 - t1) * CTime::TimeRatio << endl;*/

	CTime::DateTime dt;
	dt.Year = 2100;
	dt.Month = 12;
	dt.Day = 31;
	CTime::DateTime time = CTime::LunarCalendar(CTime::MakeSolarTime(dt));
	cout << endl << CTime::ToString(time) << endl;
	cout << CTime::ToString(CTime::SolarCalendar((int64_t)CTime::CurrentTime() + CTime::GetLocalTimeDifference())) << endl;
	cout << CTime::GetLocalTimeDifference() << endl;
	CTime::DateTime lunardt;
	lunardt.Year = 2019;
	lunardt.Month = 6;
	lunardt.Day = 1;
	lunardt.Leap = 0;
	cout << CTime::ToString(CTime::SolarCalendar(CTime::MakeLunarTime(lunardt))) << endl;

	vector<string> exts(explode("jpg|png|gif", '|'));
	for (size_t i = 0; i < exts.size(); i++)
		cout << exts[i] << endl;
	vector<string> tags(explode("html, body; header; table", "; "));
	for (size_t i = 0; i < tags.size(); i++)
		cout << tags[i] << endl;
	set<std::string> exts2(explode_set("jpg|png|gif", '|'));
	for (auto it = exts2.begin(); it != exts2.end(); it++)
		cout << *it << endl;
	set<std::string> tags2(explode_set("html, menu; meta; div; p", "; "));
	for (auto it = tags2.begin(); it != tags2.end(); it++)
		cout << *it << endl;

	// 测试GetLine函数
	/*CFile file;
	string data;
	file.Open("aaaa.txt", CFile::ONLY_READ);
	file.ReadAll(data);
	cout << data.data() << endl;*/

	//PrepareLunarCalendarData();

	// 转码
	/*VCharset cs(VCharset::UTF8, VCharset::GBK);
	VCString source = "生成";
	VCString des;
	cs.Convert(source, des);
	cout << des.GetString() << endl;*/

	// sqlite
//		sqlite3 * pDB = NULL;
//		int nRes = sqlite3_open("./test.db", &pDB);
//		if (nRes != SQLITE_OK) {
//			cout << "Open database fail: " << sqlite3_errmsg(pDB);
//		}
//		else {
//			sqlite3_close(pDB);
//		}

	// 测试zlib
/*		int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[16384];
	unsigned char out[16384];
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, 9);*/

	// 测试数组交换数据
	/*VVector<double> x(5), y(10);
	x[0] = 1.1;
	y[0] = 2.2;
	x.Swap(y);
	cout << x[0] << " " << y[0] << endl;*/

	/*VCharset cs(VCharset::UTF8, VCharset::BIG5);
	string udata;
	wstring data;
	VCString path("D:\\MyProgram\\VCS\\trunk\\LunarCalendarData\\");
	for (uint16_t i = 1901; i < 1902; ++i) {
		VCString filename(11);
		sprintf((char*)filename.GetPointer(), "T%dc.txt", i);
		filename.SetLength(10);
		CFile file;
		file.Open(path + filename, CFile::ONLYREAD);
		file.ReadAll(udata);
		//cs.Convert(udata, data);
		data = VCodeConversion::utf8_to_wstring(udata);
		file.Close();
		cout << data.size() << endl;
	}*/

	cout << CRandom()(0, 100) << endl;
	cout << CRandom()(10, 'a', 'z') << endl;

	cout << endl;
	//cout << CFile::FileSize("./main.obj") << endl;
	//cout << CTime::ToString(CTime::SolarCalendar(CFileSystem::LastModifyTime("./main.obj") + CTime::GetLocalTimeDifference())) << endl;
	//cout << CFileSystem::RemoveFile("D:\\MyProgram\\Release\\VCS\\.qmake.stash") << endl;
	//cout << CFileSystem::CreateDirectories("D:\\tt\\tt2");
	//CFileSystem::Rename("D:\\MyProgram\\Release\\VCS\\aa.txt", "D:\\MyProgram\\Release\\VCS\\bb.txt");
	/*cout << endl;
	vector<string> files;
	CFileSystem::ReadDirectoryOnlyFile("D:\\MyProgram\\VCS\\trunk\\x64\\Release\\", files, false, "pdb|log");
	for (auto it = files.begin(); it != files.end(); it++) {
		cout << *it << endl;
	}
	cout << endl;
	vector<string> allfiles;
	CFileSystem::ReadDirectoryAllFiles("D:\\MyProgram\\VCS\\trunk\\", allfiles, false, "cpp");
	for (auto it = allfiles.begin(); it != allfiles.end(); it++) {
		cout << *it << endl;
	}
	cout << endl;
	vector<string> allfilesdirs;
	CFileSystem::ReadDirectoryAll("D:\\MyProgram\\VCS\\trunk\\x64\\Release\\", allfilesdirs, false);
	for (auto it = allfilesdirs.begin(); it != allfilesdirs.end(); it++) {
		cout << *it << endl;
	}*/
	//CFileSystem::CopyDirectory("D:\\MyProgram\\VCS\\trunk\\x64\\Release\\aaa", "D:\\MyProgram\\VCS\\trunk\\x64\\Release\\bbb");

	/*CString str = " \n1 2 3  ";
	CString str_2 = str.trim();
	cout << str_2 << ", length " << str_2.size() << endl;
	CString str_3 = str.ltrim();
	cout << str_3 << ", length " << str_3.size() << endl;
	CString str_4 = str_3.rtrim();
	cout << str_4 << ", length " << str_4.size() << endl;*/
	string str_5("\n\n a b  c \t");
	ltrim(str_5);
	rtrim(str_5);
	cout << str_5 << ", length" << str_5.size() << endl;
	string str_6("\t aa bb \n");
	trim(str_6);
	cout << str_6 << ", length" << str_6.size() << endl;
	string str_7("  \t\na1 b2 c3\n\r ");
	cout << trim_copy(str_7) << endl;

	cout << pad_left_copy("Alien", 10, "-=") << endl;
	cout << pad_right_copy("Alien", 6, "___") << endl;

	cout << sizeof(long long) << " " << sizeof(double) << " " << sizeof(long double) << " " << sizeof(__int128_t) << " " << sizeof(__uint128_t) << endl;
	// long double常量浮点数后面应当加上L，float后面加上F，不加为double
	long double ld_num = 1.99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999L;
	cout << sin(ld_num) << endl;
	/*uint1024_t myUnsignedInt1024 = 1024;
	uint512_t myUnsignedInt512 = 512;
	uint256_t myUnsignedInt256("5343643654645643439579347502302730823740174032875032874032847085");
	cout << myUnsignedInt1024 + myUnsignedInt512 << " " << myUnsignedInt256 << endl;
	cout << setprecision(numeric_limits<long double>::max_digits10) << numeric_limits<long double>::max_digits10 << ", " << numeric_limits<long double>::min() << ", " << numeric_limits<long double>::max() << endl;
	cout << sizeof(chrono::high_resolution_clock::rep) << " " << chrono::high_resolution_clock::period::num << " " << chrono::high_resolution_clock::period::den << endl;
	//dynamic_bitset aaaa(4096);
	__uint128_t testnum = 110156546734858ULL;
	testnum *= testnum;
	cout << num_to_string(testnum) << endl;
	__int128_t testnum_neg = -110156546734858LL;
	cout << num_to_string(testnum_neg) << endl;
	number<cpp_dec_float<200> > high_prec_num("435804385043495727493284793274932857439569479238759465942750284370259340535.043580439504395043");
	cout << std::setprecision(std::numeric_limits<number<cpp_dec_float<200> >>::max_digits10) << log(high_prec_num) << endl;*/

	/*chrono::high_resolution_clock::rep startime, endtime;
	startime = CTime::Now();
	mpf_float_100 aa = 2;
	for (uint64_t i = 0; i < 100000; i++)
		aa = sin(aa + 0.1);
	endtime = CTime::Now();
	cout << std::setprecision(std::numeric_limits<mpf_float_100>::max_digits10) << aa << endl;
	cout << "  " << (endtime - startime) * CTime::TimeRatio << endl;
	cout << "    " << aa.backend().data()->_mp_size << endl;

	cpp_dec_float_100 bb = 2;
	for (uint64_t i = 0; i < 100000; i++)
		bb = sin(bb + 0.1);
	endtime = CTime::Now();
	cout << std::setprecision(std::numeric_limits<cpp_dec_float_100>::max_digits10) << bb << endl;
	cout << "  " << (endtime - startime) * CTime::TimeRatio << endl;

	cpp_bin_float_100 cc = 2;
	for (uint64_t i = 0; i < 100000; i++)
		cc = sin(cc + 0.1);
	endtime = CTime::Now();
	cout << std::setprecision(std::numeric_limits<cpp_bin_float_100>::max_digits10) << cc << endl;
	cout << "  " << (endtime - startime) * CTime::TimeRatio << endl;*/

	mpz_int v = 1;
	// Do some arithmetic:
	for(unsigned i = 1; i <= 1000; ++i)
		v *= i;
	std::cout << v << std::endl; // prints 1000!
	// Access the underlying representation:
	mpz_t z;
	mpz_init(z);
	mpz_set(z, v.backend().data());
	mpz_clear(z);

	mpq_rational gmprational("2/3");
	std::cout << numerator(gmprational) << std::endl;
	std::cout << denominator(gmprational) << std::endl;

	{
		// Operations at variable precision and limited standard library support:
		mpf_float_100 a = 2;
		std::cout << std::numeric_limits<mpf_float_100>::max_digits10 << "  " << sqrt(a) << std::endl; // print root-2
	}

	mpf_float_100 mpf_float_test_a = 1;
	for(int i = 1; i <= 10000; i ++)
		mpf_float_test_a *= i;
	mpf_float_test_a /= 1000;
	cout << std::setprecision(std::numeric_limits<mpf_float_100>::max_digits10) << mpf_float_test_a << endl;

	CFile file2;
	file2.Open("./test.txt", CFile::ONLY_WRITE);
	file2.Write(iconv("的非官方大哥", CIconv::CHARSET_UTF8,  CIconv::CHARSET_GBK));
	file2.Close();

	cout << sizeof(wchar_t) << endl;
	string str_8 = "a汉字b";
	string str_9 = iconv(str_8, CIconv::CHARSET_UTF8,  CIconv::CHARSET_GBK);
	cout << str_8.size() << " " << str_9.size() << " " << strlen(str_8, CIconv::CHARSET_UTF8) << " " << strlen(str_9, CIconv::CHARSET_GBK) << endl;
	cout << std::string::npos << " " << substr("abcefg", 2, -2) << endl;
	cout << iconv(substr("的非官方大哥", 1, -1, CIconv::CHARSET_UTF8), CIconv::CHARSET_UTF8,  CIconv::CHARSET_GBK) << endl;
	cout << substr(iconv("他人让他和规范化", CIconv::CHARSET_UTF8,  CIconv::CHARSET_GBK), 3, 50, CIconv::CHARSET_GBK) << endl;

	cout << base64_encode(sha1("abc", true)) << endl;
	cout << base64_encode(md5("abc", true)) << endl;
	cout << sha1("abcdefg") << endl;

	cout << num_to_string(stolll("123456789")) << "," << num_to_string(stolll("-123456789")) << "," << num_to_string(stoulll("1234567890987654321")) << endl;

	CFixedMemoryStorage<uint32_t> table;
	table.Test();

	map<string, int> test_map;
	test_map["a"] = 1;
	test_map["b"] = 2;
	test_map["c"] = 3;
	test_map["d"] = 4;
	test_map["e"] = 5;
	test_map["f"] = 6;
	for(auto it = test_map.begin(); it != test_map.end(); )
	{
		if(it->second == 2 || it->second == 5)
	   {
			test_map.erase(it++);
	   }else
	   {
			it->second += 2;
			++it;
	   }
	}
	for(auto it = test_map.begin(); it != test_map.end(); it++) {
		cout << it->first << ":" << it->second << endl;
	}

	/*CPack pack;
	mpz_int test_mpzint("-123456789");
	pack.Put(test_mpzint);
	mpz_int test_mpzint2;
	pack.Get(test_mpzint2);
	cout << "test_mpzint:" << test_mpzint2 << endl;
	pack.Clear();
	mpq_rational test_mpqrat("123/-456");
	pack.Put(test_mpqrat);
	mpq_rational test_mpqrat2;
	pack.Get(test_mpqrat2);
	cout << "test_mpqrat:" << test_mpqrat2 << endl;*/

	//BigDecimal bd("-12345678.1256");
	//cout << table.BigDecimalToString(bd, 10, 2) << endl;

	/*uint32_t charnum;
	string trunstr("的非官方大哥");
	truncate(trunstr, 14, charnum, CIconv::CHARSET_UTF8);
	cout << iconv(trunstr, CIconv::CHARSET_UTF8,  CIconv::CHARSET_GBK) << ":" << charnum << "," << strlen(trunstr, CIconv::CHARSET_UTF8) << "," << trunstr.size() << endl;*/

	cout << num_limits<__int128_t>::min() << "," << num_limits<__int128_t>::max() << endl;
	//cout << numeric_limits<int128_t>::min() << "," << numeric_limits<int128_t>::max() << endl;
	cout << num_limits<__uint128_t>::min() << "," << num_limits<__uint128_t>::max() << endl;
	//cout << numeric_limits<uint128_t>::min() << "," << numeric_limits<uint128_t>::max() << endl;
	cout << setiosflags(ios::scientific) << setprecision(numeric_limits<float>::digits10) << numeric_limits<float>::min() << "," << numeric_limits<float>::max() << endl;
	cout << setiosflags(ios::scientific) << setprecision(numeric_limits<double>::digits10) << numeric_limits<double>::min() << "," << numeric_limits<double>::max() << endl;
	cout << setiosflags(ios::scientific) << setprecision(numeric_limits<long double>::digits10) << numeric_limits<long double>::min() << "," << numeric_limits<long double>::max() << endl;

	cout << "uint32 product:" << uint64_t(num_limits<uint32_t>::max()) * uint64_t(num_limits<uint32_t>::max()) << endl;
	cout << thread::hardware_concurrency() << endl;

	//cout << regex_match("20M", regex("\\d+(K|M|G)?")) << "," << regex_match("20", regex("\\d+(K|M|G)?")) << "," << regex_match("20aa", regex("\\d+(K|M|G)?")) << endl;

	CAny any(static_cast<int8_t>(8));
	cout << "any:" << (int)any.ToInt8() << endl;
	cout << sizeof(CAny::Data) << endl;
	mpz_int mpint = 45345435345879;
	any = *mpint.backend().data();
	cout << mpint << " " << any.ToMPZInt() << endl;
	mpq_rational mprat("4534543/5345879");
	any = *mprat.backend().data();
	cout << mprat << " " << any.ToMPQRational() << endl;

	/*auto t1 = CTime::Now();
	int a;
	//boost::any b = 233423;
	//CAny b = 233423;
	for(int i = 0; i < 10000000; i ++) {
		//boost::any a = 233423 + i;
		CAny a = 233423 + i;
		//num_to_string(233423 + i);
		//int a = 233423 + i;
		//int a = stol("233423");
		//a = static_cast<int>(b);
		//a = boost::any_cast<int>(b);
	}
	auto t2 = CTime::Now();
	cout << a << endl;
	cout << (t2 - t1) * CTime::TimeRatio << endl;*/

	mpf_float a = 2;
	mpf_float::default_precision(2000);
	std::cout << mpf_float::default_precision() << std::endl;
	std::cout << sqrt(a) << std::endl;

	{
		CDateTime v(2019, 6, 27, 7561, 2001);
		uint16_t hour = static_cast<uint16_t>(v.Time / 3600);
		uint16_t leftseconds = v.Time % 3600;
		uint16_t minute = leftseconds / 60;
		uint16_t second = leftseconds % 60;
		cout << num_to_string(v.Year) + "-" + num_to_string(v.Month) + "-" + num_to_string(v.Day) + " " +
				num_to_string(hour) + ":" + num_to_string(minute) + ":" + num_to_string(second) + "." +
				num_to_string(v.Fraction) << endl;
	}
	{
		int64_t v = 7561ULL * CTime::NanoTime + 4534545ULL;
		long double ldv = static_cast<long double>(v) / CTime::NanoTime;
		v = static_cast<int64_t>(ldv);
		double fraction = static_cast<double>(::fabs(ldv - v));
		int64_t hour = v / 3600;
		int32_t leftseconds = v % 3600;
		int32_t minute = leftseconds / 60;
		int32_t second = leftseconds % 60;
		cout << num_to_string(hour) + ":" + pad_left_copy(num_to_string(::abs(minute)), 2, '0') + ":" + pad_left_copy(num_to_string(::abs(second)), 2, '0') + "." + num_to_string(fraction) << endl;
	}

	CDatabase db;
	/*// 创建数据库
	db.Create("./testdb");*/
	/*
	// 添加用户
	db.Open("./testdb");
	db.AddUser("mzt", "123456", 1, "127.0.0.1", bitset<8>(255));
	*/

	// 创建表
	/*db.Open("./testdb/nosql/test");
	vector<CRawField> fields;
	fields.emplace_back(CRawField("id", FT_SERIAL32));
	fields.emplace_back(CRawField("createdtime", FT_TIMESTAMP, true, true, "CURRENT_TIMESTAMP"));
	fields.emplace_back(CRawField("updatedtime", FT_TIMESTAMP, true, true, "CURRENT_TIMESTAMP", true, "CURRENT_TIMESTAMP"));
	fields.emplace_back(CRawField("title", FT_CHAR, true, false, "", false, "", 200));
	fields.emplace_back(CRawField("content", FT_CHAR, true, false, "", false, "", 10000));
	fields.emplace_back(CRawField("price", FT_DOUBLE));
	fields.emplace_back(CRawField("hits", FT_UINT32, true, true, "0"));
	vector<CIndex> indexes;
	indexes.emplace_back(CIndex("id", IT_PRIMARY, IM_BTREE, vector<string>{"id"}));
	db.CreateTable("testtable", TT_FIXMEMORY, fields, indexes);
	db.Close();*/
	/*db.Open("./testdb");
	db.LoadTable("testtable");
	db.Close();*/

	/*if (std::regex_match ("-234.3", std::regex("^[+-]?(\\d+|(\\d*[\\.]\\d+)|(\\d+[\\.]\\d*))([eE][+-]?\\d+)?$") ))
		std::cout << "string literal matched\n";
	if (std::regex_match ("-234.3e-2", std::regex("^[+-]?(\\d+|(\\d*[\\.]\\d+)|(\\d+[\\.]\\d*))([eE][+-]?\\d+)?$") ))
		std::cout << "string literal matched\n";*/

	cout << std::numeric_limits<float>::digits10 << "," << std::numeric_limits<double>::digits10 << "," << std::numeric_limits<long double>::digits10 << endl;
	cout << std::to_string((float)1.1) << "," << num_to_string((float)1.1) << "," << num_to_string((double)1.1) << "," << num_to_string((long double)1.1) << endl;

	list<int> li;
	for(int i = 0; i < 6; i++) {
		li.push_back(i);
	}
	cout << li.size() << endl;
	for(auto it = li.begin(); it != li.end();) {
		auto del_it = it;
		it++;
		li.erase(del_it);
	}
	cout << li.size() << endl;

	map<int, int> intmap;
	for(int i = 0; i < 10; i++) {
		intmap.insert(pair<int, int>(i + 1, i));
	}
	cout << intmap.size() << endl;
	for(auto it = intmap.begin(); it != intmap.end();) {
		auto del_it = it;
		it++;
		intmap.erase(del_it);
	}
	cout << intmap.size() << endl;

//	auto threadstarttime = CTime::Now();
	/*vector<string> teststringvector(4);
	thread thpool[4];
	for(size_t i = 0; i < 4; i++) {
		string* str = &teststringvector[i];
		thpool[i] = thread([str]() {
			for(size_t j = 0; j < 10000000; j++) {
				*str = "teststring_" + to_string(j);
			}
		});
	}
	for(size_t i = 0; i < 4; i++) {
		thpool[i].join();
	}*/
	/*string test1, test2, test3, test4;
	thread th1, th2, th3, th4;
	th1 = thread([&test1]() {
		for(size_t j = 0; j < 10000000; j++) {
			test1 = "teststring_" + to_string(j);
		}
	});
	th2 = thread([&test2]() {
		for(size_t j = 0; j < 10000000; j++) {
			test2 = "teststring_" + to_string(j);
		}
	});
	th3 = thread([&test3]() {
		for(size_t j = 0; j < 10000000; j++) {
			test3 = "teststring_" + to_string(j);
		}
	});
	th4 = thread([&test4]() {
		for(size_t j = 0; j < 10000000; j++) {
			test4 = "teststring_" + to_string(j);
		}
	});
	th1.join();
	th2.join();
	th3.join();
	th4.join();*/
//	auto threadendtime = CTime::Now();
//	cout << "  " << (threadendtime - threadstarttime) * CTime::TimeRatio << endl;
}

int main(int argc, char *argv[])
{
	try {
		string programdir = boost::filesystem::initial_path().string();
		string programpath = argv[0];
		if(programpath.substr(0, programdir.size()) != programdir) {
			if(programpath.substr(0, 2) == string(".") + DIRECTORY_SEPARATOR) {
				programpath.erase(0, 2);
			}
			size_t pos = programpath.find_last_of(DIRECTORY_SEPARATOR);
			bool isroot;
#if defined(_WIN32)
			isroot = ':' == programpath[1];
#else
			isroot = '/' == programpath[0];
#endif
			if(isroot) {
				programdir = programpath.substr(0, pos);
			}
			else {
				if(string::npos != pos) {
					programdir += DIRECTORY_SEPARATOR + programpath.substr(0, pos);
					programpath = programdir + DIRECTORY_SEPARATOR + programpath.substr(pos + 1);
				}
				else {
					programpath = programdir + DIRECTORY_SEPARATOR + programpath;
				}
			}
		}
		CService::ProgramDir = programdir;
		CLog::Instance(programdir);

		int opt;
		char short_options[] = "hi:s";
		static struct option long_options[] =
		{
			{"help", no_argument, nullptr, 'h'},
			{"inifile", required_argument, nullptr, 'i'},
			{"showinfo",  no_argument, nullptr, 's'},
#if defined(_WIN32)
			{"install",  required_argument, nullptr, 1},
			{"uninstall",  required_argument, nullptr, 2},
			{"ntservice",  required_argument, nullptr, 3},
#endif
			{nullptr, 0, nullptr, 0}
		};

#if defined(_WIN32)
		int oper = 0;
		string servicename;
#endif
		while((opt = getopt_long(argc, argv, short_options, long_options, nullptr))!= -1)
		{
			switch(opt) {
			case 'h':
				cout << endl << "-h or --help: show command list" << endl
					 << "-i or --inifile file: default ini file is under the path of moondb file." << endl
					 << "--install: install windows service" << endl
					 << "--uninstall: uninstall windows service" << endl;
				exit(0);
			case 'i':
				CService::ConfigFile = optarg;
				break;
			case 's':
				CService::ShowInfo = true;
				break;
#if defined(_WIN32)
			case 1:
			case 2:
			case 3:
				oper = opt;
				servicename = optarg;
				break;
#endif
			}
		}
		if(CService::ConfigFile.size() == 0) {
			TriggerError("Please input ini file (eg. moondb.xml) in the command line.");
		}
#if defined(_WIN32)
		if(oper == 1) {
			CService::Install(programpath, CService::ConfigFile, servicename);
			return 0;
		}
		else if(oper == 2) {
			CService::Uninstall(servicename);
			return 0;
		}
		else if(oper == 3) {
			CService::ServiceName = servicename;
			CService::IsService = true;
			CService::ShowInfo = false;
			CService::Start();
			return 0;
		}
#endif
		CService::Main(static_cast<unsigned long>(argc), argv);
	}
	catch(invalid_argument& e) {
		//TriggerError("Invalid command line parameter (invalid_argument: " + string(optarg) + " " + e.what() + ").");
		cout << "Invalid command line parameter (invalid_argument: " + string(optarg) + " " + e.what() + ")." << endl;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
	return 0;
}

