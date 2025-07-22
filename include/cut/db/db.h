#pragma once

#include <string>

namespace cut::db
{

/**
 * Interface for a database.
 */
class Db
{
  public:
    virtual ~Db() = default;

    /**
     * Execute SQL in an API specific way.
     *
     * @param sql
     *   The SQL to execute.
     */
    virtual auto execute(const std::string &sql) -> void = 0;
};

}
