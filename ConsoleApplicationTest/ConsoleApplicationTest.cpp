// ConsoleApplicationTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "..\SQLit3\SQLit3.h"

int main()
{
    Database* db = SQLit3::Ins().CreateDatabase();
	bool b = db->Open("F:\\1.db", "12345678", Database::SQLIT3_OPEN_READWRITE);
    if (!b)
    {
        std::cout << "打开数据库失败"<< db->GetLastErrorCode()
			<< ":" << db->GetLastErrorMsg() << std::endl;
        return 1;
    }
	b = db->Execute("select 1");
	//b = db->ResetPassword("12345678");

    b = db->Execute("CREATE TABLE IF NOT EXISTS \"t1\" (\"a\" TEXT,\"b\" integer,\"c\" real);");
    b = db->Execute("INSERT INTO \"main\".\"t1\" (\"a\", \"b\", \"c\") VALUES ('abc', 1, 2.0);");
    __int64 changedRow = db->GetChangeRowCount();

    SqlStatement* stmt = db->StatementPrepare("SELECT a,c FROM \"main\".\"t1\" where b=? or b=$cv");
    if (stmt)
    {
		b = stmt->BindInt(0, 1);
		int idx = stmt->GetParamIndexByName("$cv");
		b = stmt->BindDouble(idx, 2);

		int colCount = stmt->GetColumnCount();
		while (stmt->Next() == SqlStatement::SQLIT3_HAS_ROW)
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
					std::cout << columnName << "=" << v << std::endl;
					break;
				}
				case SqlStatement::SQLIT3_VALUE_FLOAT:
				{
					double v = stmt->GetDouble(i);
					std::cout << columnName << "=" << v << std::endl;
					break;
				}
				case SqlStatement::SQLIT3_VALUE_BLOB:
				{
					int dataLen = 0;
					const void* data = stmt->GetBlob(i, &dataLen);
					char* pBuf = new char[dataLen + 1];
					memcpy(pBuf, data, dataLen);
					pBuf[dataLen] = '\0';
					std::cout << columnName << "=" << pBuf << std::endl;
					delete[] pBuf;
					break;
				}
				case SqlStatement::SQLIT3_VALUE_NULL:
				{
					std::cout << columnName << "=<NULL>" << std::endl;
					break;
				}
				case SqlStatement::SQLIT3_VALUE_TEXT:					
				default:
				{
					int dataLen = 0;
					auto data = stmt->GetText(i, &dataLen);
					std::cout << columnName << "=" << data << std::endl;
					break;
				}
				}
			}
		}
		db->StatementFinalize(stmt);
    }


    b = db->Close();

    return 0;
}
