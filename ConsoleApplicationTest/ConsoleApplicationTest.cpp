// ConsoleApplicationTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "..\SQLit3\SQLit3.h"
#include "ThreadPool.h"

int test_normal()
{
	Database* db = SQLit3::Ins().CreateDatabase();
	bool b = db->Open("1.db", "12345678", Database::SQLIT3_OPEN_DEFAULT);
	if (!b)
	{
		std::cout << "打开数据库失败" << db->GetLastErrorCode()
			<< ":" << db->GetLastErrorMsg() << std::endl;
		return 1;
	}
	//b = db->ResetPassword("12345678");

	b = db->Execute("CREATE TABLE IF NOT EXISTS t1 (\"a\" TEXT,\"b\" integer,\"c\" real);");
	b = db->Execute("INSERT INTO t1 (\"a\", \"b\", \"c\") VALUES ('abc', 1, 2.0);");
	__int64 changedRow = db->GetChangeRowCount();

	__int64 countRows = 0;
	b = db->QueryInt64("SELECT COUNT(1) FROM t1", &countRows);

	SqlStatement* stmt = db->StatementPrepare("SELECT a,c FROM t1 where b=? or b=$_v");
	if (stmt)
	{
		b = stmt->BindInt(0, 1);
		int idx = stmt->GetParamIndexByName("$_v");
		b = stmt->BindDouble(idx, 2);

		int colCount = stmt->GetColumnCount();
		while (stmt->Next() == SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			for (int i = 0; i < colCount; i++)
			{
				const char* columnName = stmt->GetColumnName(i);
				auto type = stmt->GetType(i);
				switch (type)
				{
				case SqlStatement::SQLIT3_VALUE_INTEGER:
				{
					__int64 v = stmt->GetInt64(i);
					std::cout << columnName << "=" << v << "\t";
					break;
				}
				case SqlStatement::SQLIT3_VALUE_FLOAT:
				{
					double v = stmt->GetDouble(i);
					std::cout << columnName << "=" << v << "\t";
					break;
				}
				case SqlStatement::SQLIT3_VALUE_BLOB:
				{
					int dataLen = 0;
					const void* data = stmt->GetBlob(i, &dataLen);
					char* pBuf = new char[dataLen + 1];
					memcpy(pBuf, data, dataLen);
					pBuf[dataLen] = '\0';
					std::cout << columnName << "=" << pBuf << "\t";
					delete[] pBuf;
					break;
				}
				case SqlStatement::SQLIT3_VALUE_NULL:
				{
					std::cout << columnName << "=<NULL>" << "\t";
					break;
				}
				case SqlStatement::SQLIT3_VALUE_TEXT:
				default:
				{
					int dataLen = 0;
					auto data = stmt->GetText(i, &dataLen);
					std::cout << columnName << "=" << data << "\t";
					break;
				}
				}
			}

			std::cout << std::endl;
		}

		//测试辅助函数
		std::vector<std::map<std::string, std::string>> outRows;
		stmt->ToStringRows(outRows);

		db->StatementFinalize(stmt);
	}


	b = db->Close();
	SQLit3::Ins().DestoryDatabase(db);

	DeleteFileA("1.db");

	return 0;
}

#include <fstream>

int test_blob(const char* exefile)
{
	Database* db = SQLit3::Ins().CreateDatabase();
	bool b = db->Open("2.db", "12345678", Database::SQLIT3_OPEN_DEFAULT);
	if (!b)
	{
		std::cout << "打开数据库失败" << db->GetLastErrorCode()
			<< ":" << db->GetLastErrorMsg() << std::endl;
		return 1;
	}
	//b = db->ResetPassword("12345678");

	b = db->Execute("CREATE TABLE IF NOT EXISTS test_blob (\"a\" integer NOT NULL,\"b\" blob, PRIMARY KEY (\"a\"));");

	__int64 id = 0;
	db->QueryInt64("SELECT MAX(a) FROM test_blob", &id);

	SqlStatement* stmt = db->StatementPrepare("INSERT INTO test_blob(\"a\", \"b\") VALUES (?, ?);");
	if (stmt)
	{
		stmt->BindInt64(0, id + 1);

		// 打开二进制文件
		std::ifstream inFile(exefile, std::ios::binary);
		// 检查文件是否打开成功
		if (inFile)
		{
			inFile.seekg(0, std::ios::end);
			std::streampos ps = inFile.tellg();
			inFile.seekg(0, std::ios_base::beg);
			// 读取文件内容
			char* buffer = new char[ps];
			inFile.read(buffer, ps);
			// 关闭文件
			inFile.close();

			stmt->BindBlob(1, buffer, ps);
			SqlStatement::ExecStatus es = stmt->Next();
			delete[] buffer;
			__int64 changedRow = db->GetChangeRowCount();
		}

		db->StatementFinalize(stmt);
	}
	
	const void* data = NULL;
	int dataLen = 0;
	b = db->QueryBlob("SELECT b FROM test_blob LIMIT 0,1", &data, &dataLen);

	b = db->Close();
	SQLit3::Ins().DestoryDatabase(db);

	DeleteFileA("2.db");

	return 0;
}

std::string test_a =
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
"auto stmt = db->StatementPrepare(INSERT INTO tab(a, b, c) VALUES($a, $b, $c); );\n"
;
int test_b = 123456;
double test_c = 123.456;
int insertCount = 10000;
SqlStatement::ExecStatus __test_stmt(Database* db)
{
	auto stmt = db->StatementPrepare("INSERT INTO tab (a, b, c) VALUES ($a, $b, $c);");
	if (stmt)
	{
		int pidx = stmt->GetParamIndexByName("$a");
		stmt->BindText(pidx, test_a.c_str(), test_a.size());

		pidx = stmt->GetParamIndexByName("$b");
		stmt->BindInt(pidx, test_b);

		pidx = stmt->GetParamIndexByName("$c");
		stmt->BindDouble(pidx, test_c);

		auto res = stmt->Next(-1);
		if (res != SqlStatement::SQLIT3_EXEC_DONE)
		{
			std::cout << "Insert error: " << (int)res << std::endl;
		}

		db->StatementFinalize(stmt);

		return res;
	}

	return SqlStatement::SQLIT3_EXEC_ERROR;
}


void _multiThreadInsert()
{
	Database* db = SQLit3::Ins().CreateDatabase();
	bool b = db->Open("testMultiThreadInserts.db", "", 
		(Database::SqliteOpenFlag)(
			Database::SQLIT3_OPEN_DEFAULT | 
			Database::SQLIT3_OPEN_FULLMUTEX
		)
	);
	
	__test_stmt(db);

	db->Close();
	SQLit3::Ins().DestoryDatabase(db);

	//std::cout << '.';
}
/*多线程插入数据会丢失，不建议多线程使用sqlite*/
void testMultiThreadInserts()
{
	remove("testMultiThreadInserts.db");
	Database* db = SQLit3::Ins().CreateDatabase();
	bool b = db->Open("testMultiThreadInserts.db", "", Database::SQLIT3_OPEN_DEFAULT);
	db->Execute("CREATE TABLE IF NOT EXISTS tab (a TEXT,b integer,c real);");
	db->Close();
	SQLit3::Ins().DestoryDatabase(db);

	tp::ThreadPool pool;

	DWORD t1 = ::GetTickCount();

	for (size_t i = 0; i < insertCount; i++)
	{
		pool.addJob([]()
		{
			_multiThreadInsert();
		});
	}

	pool.joinAll();

	DWORD t = ::GetTickCount() -  t1;

	{
		Database* db = SQLit3::Ins().CreateDatabase();
		bool b = db->Open("testMultiThreadInserts.db", "", Database::SQLIT3_OPEN_DEFAULT);
		
		auto stmt = db->StatementPrepare("SELECT COUNT(1) FROM tab;");
		if (stmt)
		{
			stmt->Next();
			int insertedCount = stmt->GetInt(0);
			std::cout << "testMultiThread inserted " << insertedCount << " 条数据 耗时 " << t << " ms" << std::endl;

			db->StatementFinalize(stmt);
		}

		db->Close();
		SQLit3::Ins().DestoryDatabase(db);
	}
}

void testNormalInserts()
{
	remove("testNormalInserts.db");
	Database* db = SQLit3::Ins().CreateDatabase();
	bool b = db->Open("testNormalInserts.db", "", Database::SQLIT3_OPEN_DEFAULT);
	db->Execute("CREATE TABLE IF NOT EXISTS tab (a TEXT,b integer,c real);");

	DWORD t1 = ::GetTickCount();

	for (size_t i = 0; i < insertCount; i++)
	{
		__test_stmt(db);
	}

	DWORD t = ::GetTickCount() - t1;
	auto stmt = db->StatementPrepare("SELECT COUNT(1) FROM tab;");
	if (stmt)
	{
		stmt->Next();
		int insertedCount = stmt->GetInt(0);
		std::cout << "testNormalInserts inserted " << insertedCount << " 条数据 耗时 " << t << " ms" << std::endl;

		db->StatementFinalize(stmt);
	}

	db->Close();
	SQLit3::Ins().DestoryDatabase(db);
}


void testTranscationInserts()
{
	remove("testTranscationInserts.db");
	Database* db = SQLit3::Ins().CreateDatabase();
	bool b = db->Open("testTranscationInserts.db", "", Database::SQLIT3_OPEN_DEFAULT);
	db->Execute("CREATE TABLE IF NOT EXISTS tab (a TEXT,b integer,c real);");

	DWORD t1 = ::GetTickCount();

	db->Execute("BEGIN TRANSACTION;");
	for (size_t i = 0; i < insertCount; i++)
	{
		__test_stmt(db);
	}
	db->Execute("COMMIT;");

	DWORD t = ::GetTickCount() - t1;
	auto stmt = db->StatementPrepare("SELECT COUNT(1) FROM tab;");
	if (stmt)
	{
		stmt->Next();
		int insertedCount = stmt->GetInt(0);
		std::cout << "testTranscationInserts inserted " << insertedCount << " 条数据 耗时 " << t << " ms" << std::endl;

		db->StatementFinalize(stmt);
	}

	db->Close();
	SQLit3::Ins().DestoryDatabase(db);
}


int main(int argc, char** argv)
{
	test_normal();
	test_blob(argv[0]);

	testMultiThreadInserts();
	testNormalInserts();
	testTranscationInserts();
    return 0;
}
