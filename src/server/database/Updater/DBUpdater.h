
#ifndef DBUpdater_h__
#define DBUpdater_h__

#include "Define.h"
#include "DatabaseEnvFwd.h"
#include <string>

template <class T>
class DatabaseWorkerPool;

namespace boost
{
    namespace filesystem
    {
        class path;
    }
}

class TC_DATABASE_API UpdateException : public std::exception
{
public:
    UpdateException(std::string const& msg) : _msg(msg) { }
    ~UpdateException() throw() { }

    char const* what() const throw() override { return _msg.c_str(); }

private:
    std::string const _msg;
};

enum BaseLocation
{
    LOCATION_REPOSITORY,
    LOCATION_DOWNLOAD
};

class DBUpdaterUtil
{
public:
    static std::string GetCorrectedMySQLExecutable();

    static bool CheckExecutable();

private:
    static std::string& corrected_path();
};

template <class T>
class TC_DATABASE_API DBUpdater
{
public:
    using Path = boost::filesystem::path;

    static inline std::string GetConfigEntry();

    static inline std::string GetTableName();

    static std::string GetBaseFile();

    static bool IsEnabled(uint32 const updateMask);

    static BaseLocation GetBaseLocationType();

    static bool Create(DatabaseWorkerPool<T>& pool);

    static bool Update(DatabaseWorkerPool<T>& pool);

    static bool Populate(DatabaseWorkerPool<T>& pool);

private:
    static QueryResult Retrieve(DatabaseWorkerPool<T>& pool, std::string const& query);
    static void Apply(DatabaseWorkerPool<T>& pool, std::string const& query);
    static void ApplyFile(DatabaseWorkerPool<T>& pool, Path const& path);
    static void ApplyFile(DatabaseWorkerPool<T>& pool, std::string const& host, std::string const& user,
        std::string const& password, std::string const& port_or_socket, std::string const& database, Path const& path);
};

#endif // DBUpdater_h__
