#include "db/db.h"

#include <memory>
#include <string>

#include <sqlite3/sqlite3.h>

#include "utils/error.h"
#include "utils/log.h"

namespace cut::db
{

class Sqlite3Db : public Db
{
  public:
    ~Sqlite3Db() override = default;

    auto execute(const std::string &sql) -> void override
    {
        utils::log::info("will execute: {}", sql);

        const auto close_db = [](const auto &e) { ::sqlite3_close(e); };
        auto db = std::unique_ptr<::sqlite3, decltype(close_db)>{};
        utils::ensure(::sqlite3_open("test.db", std::out_ptr(db)) == SQLITE_OK, "failed to open database");

        char *err = nullptr;
        utils::ensure(
            ::sqlite3_exec(db.get(), sql.c_str(), 0, 0, &err) == SQLITE_OK, "failed to execute query: {}", err);
    }
};

}
