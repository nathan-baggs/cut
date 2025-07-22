#pragma once

#include <string>

namespace cut::db
{

class Db
{
  public:
    virtual ~Db() = default;
    virtual auto execute(const std::string &sql) -> void = 0;
};

}
