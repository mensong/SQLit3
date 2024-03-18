#pragma once
#ifndef _AFX
#include <windows.h>
#endif
#include <string>
#include <stdint.h>

#ifdef SQLIT3_EXPORTS
#define SQLIT3_API extern "C" __declspec(dllexport)
#else
#define SQLIT3_API extern "C" __declspec(dllimport)
#endif

class SqlStatement
{
public:
	enum SqlValueType
	{
		SQLIT3_VALUE_INTEGER = 1,
		SQLIT3_VALUE_FLOAT = 2,
		SQLIT3_VALUE_TEXT = 3,
		SQLIT3_VALUE_BLOB = 4,
		SQLIT3_VALUE_NULL = 5,
	};

	enum TextEncoding
	{
		SQLIT3_ENCODING_UTF16LE = 2,    /* IMP: R-03371-37637 */
		SQLIT3_ENCODING_UTF16BE = 3,    /* IMP: R-51971-34154 */
		SQLIT3_ENCODING_UTF16   = 4,    /* Use native byte order */
		//SQLIT3_ENCODING_ANY     = 5,    /* Deprecated */
		SQLIT3_ENCODING_UTF16_ALIGNED = 8,    /* sqlite3_create_collation only */
	};

	enum ExecStatus
	{
		SQLIT3_EXEC_HAS_ROW = 100,	//有数据

		SQLIT3_EXEC_DONE = 101,		//已读取完毕

		SQLIT3_EXEC_TIMEOUT = 5,	//读取数据超时，数据库正在忙
		SQLIT3_EXEC_ERROR = 1,		//读取错误
	};

public:
	virtual bool BindBlob(int paramIdx, const void* data, int dataLen) = 0;
	virtual bool BindBlob64(int paramIdx, const void* data, __int64 dataLen) = 0;
	virtual bool BindDouble(int paramIdx, double v) = 0;
	virtual bool BindInt(int paramIdx, int v) = 0;
	virtual bool BindInt64(int paramIdx, __int64 v) = 0;
	virtual bool BindNull(int paramIdx) = 0;
	virtual bool BindText(int paramIdx, const char* text, int textLen) = 0;
	virtual bool BindText16(int paramIdx, const void* text, int textLen) = 0;
	virtual bool BindText64(int paramIdx, const char* text, __int64 textLen, TextEncoding encoding) = 0;
	virtual bool BindPointer(int paramIdx, void* pointer, const char* valueType/*="ValueList"*/) = 0;
	virtual bool BindZeroBlob(int paramIdx, int n) = 0;
	virtual bool BindZeroBlob64(int paramIdx, __int64 n) = 0;
	virtual int GetParamIndexByName(const char* paramName) = 0;
	virtual bool ClearBindings() = 0;

	virtual int GetColumnCount() = 0;
	virtual const char* GetColumnName(int iCol) = 0;
	virtual ExecStatus Next(int timeout = 0) = 0;
	virtual void Reset() = 0;

	virtual SqlValueType GetType(int iCol) = 0;
	virtual const void* GetBlob(int iCol, int* size) = 0;
	virtual double GetDouble(int iCol) = 0;
	virtual int GetInt(int iCol) = 0;
	virtual __int64 GetInt64(int iCol) = 0;
	virtual const unsigned char* GetText(int iCol, int* size) = 0;
	virtual const void* GetText16(int iCol, int* size) = 0;
};

class Database
{
public:
	enum SqliteOpenFlag
	{		
		SQLIT3_OPEN_READONLY = 0x00000001,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_READWRITE = 0x00000002,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_CREATE = 0x00000004,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_DELETEONCLOSE = 0x00000008,  /* VFS only */
		SQLIT3_OPEN_EXCLUSIVE = 0x00000010,  /* VFS only */
		SQLIT3_OPEN_AUTOPROXY = 0x00000020,  /* VFS only */
		SQLIT3_OPEN_URI = 0x00000040,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_MEMORY = 0x00000080,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_MAIN_DB = 0x00000100,  /* VFS only */
		SQLIT3_OPEN_TEMP_DB = 0x00000200,  /* VFS only */
		SQLIT3_OPEN_TRANSIENT_DB = 0x00000400,  /* VFS only */
		SQLIT3_OPEN_MAIN_JOURNAL = 0x00000800,  /* VFS only */
		SQLIT3_OPEN_TEMP_JOURNAL = 0x00001000,  /* VFS only */
		SQLIT3_OPEN_SUBJOURNAL = 0x00002000,  /* VFS only */
		SQLIT3_OPEN_SUPER_JOURNAL = 0x00004000,  /* VFS only */
		SQLIT3_OPEN_NOMUTEX = 0x00008000,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_FULLMUTEX = 0x00010000,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_SHAREDCACHE = 0x00020000,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_PRIVATECACHE = 0x00040000,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_WAL = 0x00080000,  /* VFS only */
		SQLIT3_OPEN_NOFOLLOW = 0x01000000,  /* Ok for sqlite3_open_v2() */
		SQLIT3_OPEN_EXRESCODE = 0x02000000,  /* Extended result codes */

		/* Reserved:                         0x00F00000 */
		/* Legacy compatibility: */
		SQLIT3_OPEN_MASTER_JOURNAL = 0x00004000,  /* VFS only */

		SQLIT3_OPEN_DEFAULT = SQLIT3_OPEN_READWRITE | SQLIT3_OPEN_CREATE,
	};

	enum ErrorCode
	{
		SQLIT3_STATUS_OK          =  0,   /* Successful result */
								   /* beginning-of-error-codes */
		SQLIT3_STATUS_ERROR       =  1,   /* Generic error */
		SQLIT3_STATUS_INTERNAL    =  2,   /* Internal logic error in SQLite */
		SQLIT3_STATUS_PERM        =  3,   /* Access permission denied */
		SQLIT3_STATUS_ABORT       =  4,   /* Callback routine requested an abort */
		SQLIT3_STATUS_BUSY        =  5,   /* The database file is locked */
		SQLIT3_STATUS_LOCKED      =  6,   /* A table in the database is locked */
		SQLIT3_STATUS_NOMEM       =  7,   /* A malloc() failed */
		SQLIT3_STATUS_READONLY    =  8,   /* Attempt to write a readonly database */
		SQLIT3_STATUS_INTERRUPT   =  9,   /* Operation terminated by sqlite3_interrupt()*/
		SQLIT3_STATUS_IOERR       = 10,   /* Some kind of disk I/O error occurred */
		SQLIT3_STATUS_CORRUPT     = 11,   /* The database disk image is malformed */
		SQLIT3_STATUS_NOTFOUND    = 12,   /* Unknown opcode in sqlite3_file_control() */
		SQLIT3_STATUS_FULL        = 13,   /* Insertion failed because database is full */
		SQLIT3_STATUS_CANTOPEN    = 14,   /* Unable to open the database file */
		SQLIT3_STATUS_PROTOCOL    = 15,   /* Database lock protocol error */
		SQLIT3_STATUS_EMPTY       = 16,   /* Internal use only */
		SQLIT3_STATUS_SCHEMA      = 17,   /* The database schema changed */
		SQLIT3_STATUS_TOOBIG      = 18,   /* String or BLOB exceeds size limit */
		SQLIT3_STATUS_CONSTRAINT  = 19,   /* Abort due to constraint violation */
		SQLIT3_STATUS_MISMATCH    = 20,   /* Data type mismatch */
		SQLIT3_STATUS_MISUSE      = 21,   /* Library used incorrectly */
		SQLIT3_STATUS_NOLFS       = 22,   /* Uses OS features not supported on host */
		SQLIT3_STATUS_AUTH        = 23,   /* Authorization denied */
		SQLIT3_STATUS_FORMAT      = 24,   /* Not used */
		SQLIT3_STATUS_RANGE       = 25,   /* 2nd parameter to sqlite3_bind out of range */
		SQLIT3_STATUS_NOTADB      = 26,   /* File opened that is not a database file */
		SQLIT3_STATUS_NOTICE      = 27,   /* Notifications from sqlite3_log() */
		SQLIT3_STATUS_WARNING     = 28,   /* Warnings from sqlite3_log() */
	};

public:
	virtual bool Open(
		const char* dbFile, 
		const char* password = 0, 
		SqliteOpenFlag flag = SQLIT3_OPEN_DEFAULT,
		const char* dbname = "main") = 0;
	virtual bool ResetPassword(const char* password, const char* dbname = "main") = 0;

	virtual bool SetTimeout(int ms) = 0;

	virtual bool Execute(const char* sql) = 0;

	virtual SqlStatement* StatementPrepare(const char* sql) = 0;
	virtual void StatementFinalize(SqlStatement* pstmt) = 0;

	virtual __int64 GetChangeRowCount() = 0;
	virtual __int64 GetLastInsertRowId() = 0;

	virtual bool QueryInt(const char* sql, int* ret) = 0;
	virtual bool QueryInt64(const char* sql, __int64* ret) = 0;
	virtual bool QueryDouble(const char* sql, double* ret) = 0;
	virtual bool QueryText(const char* sql, const char** data, int* dataLen) = 0;
	virtual bool QueryText16(const char* sql, const void** data, int* dataLen) = 0;
	virtual bool QueryBlob(const char* sql, const void** data, int* dataLen) = 0;

	virtual ErrorCode GetLastErrorCode() = 0;
	virtual const char* GetLastErrorMsg() = 0;

	virtual bool Close() = 0;
};

SQLIT3_API Database* CreateDatabase();
SQLIT3_API void DestoryDatabase(Database* db);

class SQLit3
{
#define DEF_PROC(name) \
	decltype(::name)* name

#define SET_PROC(hDll, name) \
	this->name = (decltype(::name)*)::GetProcAddress(hDll, #name)

public:
	SQLit3()
	{
		hDll = LoadLibraryFromCurrentDir("SQLit3.dll");
		if (!hDll)
			return;

		SET_PROC(hDll, CreateDatabase);
		SET_PROC(hDll, DestoryDatabase);
	}


	DEF_PROC(CreateDatabase);
	DEF_PROC(DestoryDatabase);


public:
	static SQLit3& Ins()
	{
		static SQLit3 s_ins;
		return s_ins;
	}

	static HMODULE LoadLibraryFromCurrentDir(const char* dllName)
	{
		char selfPath[MAX_PATH];
		MEMORY_BASIC_INFORMATION mbi;
		HMODULE hModule = ((::VirtualQuery(LoadLibraryFromCurrentDir, 
			&mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
		::GetModuleFileNameA(hModule, selfPath, MAX_PATH);
		std::string moduleDir(selfPath);
		size_t idx = moduleDir.find_last_of('\\');
		moduleDir = moduleDir.substr(0, idx);
		std::string modulePath = moduleDir + "\\" + dllName;
		char curDir[MAX_PATH];
		::GetCurrentDirectoryA(MAX_PATH, curDir);
		::SetCurrentDirectoryA(moduleDir.c_str());
		HMODULE hDll = LoadLibraryA(modulePath.c_str());
		::SetCurrentDirectoryA(curDir);
		if (!hDll)
		{
			DWORD err = ::GetLastError();
			char buf[10];
			sprintf_s(buf, "%u", err);
			::MessageBoxA(NULL, ("找不到" + modulePath + "模块:" + buf).c_str(), "找不到模块", MB_OK | MB_ICONERROR);
		}
		return hDll;
	}
	~SQLit3()
	{
		if (hDll)
		{
			FreeLibrary(hDll);
			hDll = NULL;
		}
	}

private:
	HMODULE hDll;
};

