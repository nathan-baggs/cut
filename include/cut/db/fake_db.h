#include "db/db.h"

#include <print>

namespace cut::db
{

class FakeDb : public Db
{
  public:
    ~FakeDb() override = default;

    auto test() -> Awaitable override
    {
        return {[] { return 101; }};
    }
};

}
