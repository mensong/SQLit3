#include "pch.h"
#include "SQLit3.h"
#include <sqlite3mc.h>

class SqlStatementImp
	: public SqlStatement
{
	friend class DatabaseImp;
public:
	SqlStatementImp()
		: m_stmt(NULL)
	{

	}
	virtual ~SqlStatementImp()
	{
		if (m_stmt)
			sqlite3_finalize(m_stmt);
	}
	
	virtual bool BindBlob(int paramIdx, const void* data, int dataLen) override
	{
		return sqlite3_bind_blob(m_stmt, paramIdx + 1, data, dataLen, SQLITE_STATIC) == SQLITE_OK;
	}

	virtual bool BindBlob64(int paramIdx, const void* data, __int64 dataLen) override
	{
		return sqlite3_bind_blob64(m_stmt, paramIdx + 1, data, dataLen, SQLITE_STATIC) == SQLITE_OK;
	}

	virtual bool BindDouble(int paramIdx, double v) override
	{
		return sqlite3_bind_double(m_stmt, paramIdx + 1, v) == SQLITE_OK;
	}

	virtual bool BindInt(int paramIdx, int v) override
	{
		return sqlite3_bind_int(m_stmt, paramIdx + 1, v) == SQLITE_OK;
	}

	virtual bool BindInt64(int paramIdx, __int64 v) override
	{
		return sqlite3_bind_int64(m_stmt, paramIdx + 1, v) == SQLITE_OK;
	}

	virtual bool BindNull(int paramIdx) override
	{
		return sqlite3_bind_null(m_stmt, paramIdx + 1) == SQLITE_OK;
	}

	virtual bool BindText(int paramIdx, const char* text, int textLen) override
	{
		return sqlite3_bind_text(m_stmt, paramIdx + 1, text, textLen, SQLITE_STATIC) == SQLITE_OK;
	}

	virtual bool BindText16(int paramIdx, const void* text, int textLen) override
	{
		return sqlite3_bind_text16(m_stmt, paramIdx + 1, text, textLen, SQLITE_STATIC) == SQLITE_OK;
	}

	virtual bool BindText64(int paramIdx, const char* text, __int64 textLen, TextEncoding encoding) override
	{
		return sqlite3_bind_text64(m_stmt, paramIdx + 1, text, textLen, SQLITE_STATIC, encoding) == SQLITE_OK;
	}

	virtual bool BindPointer(int paramIdx, void* pointer, const char* valueType) override
	{
		return sqlite3_bind_pointer(m_stmt, paramIdx + 1, pointer, valueType, SQLITE_STATIC) == SQLITE_OK;
	}

	virtual bool BindZeroBlob(int paramIdx, int n) override
	{
		return sqlite3_bind_zeroblob(m_stmt, paramIdx + 1, n) == SQLITE_OK;
	}

	virtual bool BindZeroBlob64(int paramIdx, __int64 n) override
	{
		return sqlite3_bind_zeroblob64(m_stmt, paramIdx + 1, n) == SQLITE_OK;
	}

	virtual int GetParamIndexByName(const char* paramName) override
	{
		return sqlite3_bind_parameter_index(m_stmt, paramName) - 1;
	}

	virtual bool ClearBindings() override
	{
		return sqlite3_clear_bindings(m_stmt) == SQLITE_OK;
	}

	virtual int GetColumnCount() override
	{
		return sqlite3_column_count(m_stmt);
	}

	virtual const char* GetColumnName(int iCol) override
	{
		return sqlite3_column_name(m_stmt, iCol);
	}

	virtual ExecStatus Next(int timeout = 0) override
	{
		int res = sqlite3_step(m_stmt);

		if (res == SQLITE_BUSY)
		{
			DWORD st = ::GetTickCount();
			while (::GetTickCount() - st < (DWORD)timeout)
			{
				res = sqlite3_step(m_stmt);
				if (res != SQLITE_BUSY)
					break;
				Sleep(100);
			}
		}

		switch (res)
		{
		case SQLITE_ROW:
			return SQLIT3_EXEC_HAS_ROW;
			break;
		case SQLITE_OK:
		case SQLITE_DONE:
			return SQLIT3_EXEC_DONE;
			break;
		case SQLITE_BUSY:
			return SQLIT3_EXEC_TIMEOUT;
			break;
		case SQLITE_ERROR:
		default:
			return SQLIT3_EXEC_ERROR;
			break;
		}
	}

	virtual void Reset() override
	{
		sqlite3_reset(m_stmt);
	}

	virtual SqlValueType GetType(int iCol) override
	{
		int res = sqlite3_column_type(m_stmt, iCol);
		return (SqlValueType)res;
	}

	virtual const void* GetBlob(int iCol, int* size) override
	{
		*size = sqlite3_column_bytes(m_stmt, iCol);
		return sqlite3_column_blob(m_stmt, iCol);
	}

	virtual double GetDouble(int iCol) override
	{
		return sqlite3_column_double(m_stmt, iCol);
	}

	virtual int GetInt(int iCol) override
	{
		return sqlite3_column_int(m_stmt, iCol);
	}

	virtual __int64 GetInt64(int iCol) override
	{
		return sqlite3_column_int64(m_stmt, iCol);
	}

	virtual const unsigned char* GetText(int iCol, int* size) override
	{
		*size = sqlite3_column_bytes(m_stmt, iCol);
		return sqlite3_column_text(m_stmt, iCol);
	}

	virtual const void* GetText16(int iCol, int* size) override
	{
		*size = sqlite3_column_bytes16(m_stmt, iCol);
		return sqlite3_column_text16(m_stmt, iCol);
	}

private:
	sqlite3_stmt* m_stmt;
};

class DatabaseImp
	: public Database
{
public:
	DatabaseImp()
		: m_db(NULL)
	{

	}
	virtual ~DatabaseImp()
	{
		Close();
	}

	virtual bool Open(
		const char* dbFile,
		const char* password = 0, 
		SqliteOpenFlag flag = SQLIT3_OPEN_DEFAULT, 
		const char* dbname = "main") override
	{
		Close();

		int res = sqlite3_open_v2(dbFile, &m_db, flag, NULL);
		if (res != SQLITE_OK)
		{
			Close();
			return false;
		}

		if (password && password[0] != '\0')
		{
			res = sqlite3_key_v2(m_db, dbname, password, (int)strlen(password));
			if (res != SQLITE_OK)
			{
				Close();
				return false;
			}
		}

		return true;
	}

	virtual bool ResetPassword(const char* password, const char* dbname = "main") override
	{
		if (password)
			return sqlite3_rekey_v2(m_db, dbname, password, (int)strlen(password)) == SQLITE_OK;
		else
			return sqlite3_rekey_v2(m_db, dbname, "", 0) == SQLITE_OK;
	}

	virtual bool Close() override
	{
		if (m_db)
		{
			int res = sqlite3_close_v2(m_db);
			m_db = NULL;
			return res == SQLITE_OK;
		}
		return true;
	}

	virtual bool SetTimeout(int ms) override
	{
		return sqlite3_busy_timeout(m_db, ms) == SQLITE_OK;
	}

	virtual bool Execute(const char* sql) override
	{
		char* szErrMsg = NULL;
		int res = sqlite3_exec(m_db, sql, NULL, NULL, &szErrMsg);
		if (szErrMsg)
		{
			sqlite3_free(szErrMsg);
		}
		return res == SQLITE_OK;
	}

	virtual __int64 GetChangeRowCount() override
	{
		return sqlite3_changes64(m_db);
	}

	virtual __int64 GetLastInsertRowId() override
	{
		return sqlite3_last_insert_rowid(m_db);
	}

	virtual SqlStatement* StatementPrepare(const char* sql) override
	{
		sqlite3_stmt* stmt = NULL;
		int res = sqlite3_prepare_v2(m_db, sql, -1, &stmt, NULL);
		if (res != SQLITE_OK)
		{
			if (stmt)
				sqlite3_finalize(stmt);
			return NULL;
		}

		SqlStatementImp* imp = new SqlStatementImp();
		imp->m_stmt = stmt;
		return imp;
	}

	virtual void StatementFinalize(SqlStatement* pstmt) override
	{
		if (pstmt)
			delete pstmt;
	}

	virtual bool QueryInt(const char* sql, int* ret) override
	{
		SqlStatement* stmt = StatementPrepare(sql);
		if (!stmt)
			return false;
				
		SqlStatement::ExecStatus status = stmt->Next();
		if (status != SqlStatement::SQLIT3_EXEC_DONE && 
			status != SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			StatementFinalize(stmt);
			return false;
		}

		int nCount = stmt->GetColumnCount();
		if (status == SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			if (nCount > 0)
				*ret = stmt->GetInt(0);
		}

		StatementFinalize(stmt);
		return true;
	}

	virtual bool QueryInt64(const char* sql, __int64* ret) override
	{
		SqlStatement* stmt = StatementPrepare(sql);
		if (!stmt)
			return false;

		SqlStatement::ExecStatus status = stmt->Next();
		if (status != SqlStatement::SQLIT3_EXEC_DONE &&
			status != SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			StatementFinalize(stmt);
			return false;
		}

		int nCount = stmt->GetColumnCount();
		if (status == SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			if (nCount > 0)
				*ret = stmt->GetInt64(0);
		}

		StatementFinalize(stmt);
		return true;
	}

	virtual bool QueryDouble(const char* sql, double* ret) override
	{
		SqlStatement* stmt = StatementPrepare(sql);
		if (!stmt)
			return false;

		SqlStatement::ExecStatus status = stmt->Next();
		if (status != SqlStatement::SQLIT3_EXEC_DONE &&
			status != SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			StatementFinalize(stmt);
			return false;
		}

		int nCount = stmt->GetColumnCount();
		if (status == SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			if (nCount > 0)
				*ret = stmt->GetDouble(0);
		}

		StatementFinalize(stmt);
		return true;
	}

	virtual bool QueryText(const char* sql, const char** data, int* dataLen) override
	{
		SqlStatement* stmt = StatementPrepare(sql);
		if (!stmt)
			return false;

		SqlStatement::ExecStatus status = stmt->Next();
		if (status != SqlStatement::SQLIT3_EXEC_DONE &&
			status != SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			StatementFinalize(stmt);
			return false;
		}

		int nCount = stmt->GetColumnCount();
		if (status == SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			if (nCount > 0)
			{
				*dataLen = 0;
				const unsigned char* buff = stmt->GetText(0, dataLen);
				if (*dataLen > 0)
				{
					m_returnBuff.resize(*dataLen, 0);
					memcpy(&m_returnBuff[0], buff, *dataLen);
					*data = &m_returnBuff[0];
				}
			}
		}

		StatementFinalize(stmt);
		return true;
	}

	virtual bool QueryText16(const char* sql, const void** data, int* dataLen) override
	{
		SqlStatement* stmt = StatementPrepare(sql);
		if (!stmt)
			return false;

		SqlStatement::ExecStatus status = stmt->Next();
		if (status != SqlStatement::SQLIT3_EXEC_DONE &&
			status != SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			StatementFinalize(stmt);
			return false;
		}

		int nCount = stmt->GetColumnCount();
		if (status == SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			if (nCount > 0)
			{
				*dataLen = 0;
				const void* buff = stmt->GetText16(0, dataLen);
				if (*dataLen > 0)
				{
					m_returnBuff.resize((*dataLen) * 2, 0);
					memcpy(&m_returnBuff[0], buff, *dataLen * 2);
					*data = &m_returnBuff[0];
				}
			}
		}

		StatementFinalize(stmt);
		return true;
	}

	virtual bool QueryBlob(const char* sql, const void** data, int* dataLen) override
	{
		SqlStatement* stmt = StatementPrepare(sql);
		if (!stmt)
			return false;

		SqlStatement::ExecStatus status = stmt->Next();
		if (status != SqlStatement::SQLIT3_EXEC_DONE &&
			status != SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			StatementFinalize(stmt);
			return false;
		}

		int nCount = stmt->GetColumnCount();
		if (status == SqlStatement::SQLIT3_EXEC_HAS_ROW)
		{
			if (nCount > 0)
			{
				*dataLen = 0;
				const void* buff = stmt->GetBlob(0, dataLen);
				if (*dataLen > 0)
				{
					m_returnBuff.resize(*dataLen, 0);
					memcpy(&m_returnBuff[0], buff, *dataLen);
					*data = &m_returnBuff[0];
				}
			}
		}

		StatementFinalize(stmt);
		return true;
	}

	virtual ErrorCode GetLastErrorCode() override
	{
		return (ErrorCode)sqlite3_errcode(m_db);
	}

	virtual const char* GetLastErrorMsg() override
	{
		return sqlite3_errmsg(m_db);
	}

private:
	sqlite3* m_db;

	std::string m_returnBuff;
};

SQLIT3_API Database* CreateDatabase()
{
	return new DatabaseImp();
}

SQLIT3_API void DestoryDatabase(Database* db)
{
	if (!db)
		return;
	delete db;
}

