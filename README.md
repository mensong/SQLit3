# SQLit3
基于SQLite3 Multiple Ciphers封装windows易用的sqlite3操作组件(https://github.com/mensong/SQLite3-Encryption)

# Usage
把SQLit3.dll放在应用程序所在的目录下，引入SQLit3.h文件到工程里，使用Database* db = SQLit3::Ins().CreateDatabase();即可操作sqlite3数据库。
