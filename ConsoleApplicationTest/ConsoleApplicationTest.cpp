// ConsoleApplicationTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "..\SQLit3\SQLit3.h"

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

	SqlStatement* stmt = db->StatementPrepare("SELECT a,c FROM t1 where b=? or b=$cv");
	if (stmt)
	{
		b = stmt->BindInt(0, 1);
		int idx = stmt->GetParamIndexByName("$cv");
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
		db->StatementFinalize(stmt);
	}


	b = db->Close();
	SQLit3::Ins().DestoryDatabase(db);
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
	return 0;
}

int main(int argc, char** argv)
{
	test_normal();
	test_blob(argv[0]);

    return 0;
}
