#include <iostream>
#include <print>
#include <string_view>

#include "cut/coro/task.h"
#include "cut/db/annotations.h"
#include "cut/db/db.h"
#include "cut/db/db_runner.h"
#include "cut/web/annotations.h"
#include "cut/web/app.h"
#include "cut/web/controller_base.h"
#include "cut/web/response.h"

using namespace std::literals;

struct Person
{
    std::string first_name;
    std::string last_name;
};

struct Record
{
    Person person;
    int id;
};

// clang-format off
struct ProductModel
{
    [[=cut::db::Id]]
    std::uint32_t id = 0;
    std::string name;
    std::uint32_t stock_count;
};

struct ProductCreateRequest
{
    std::string name;
};
// clang-format on

// clang-format off
class Foo : public cut::web::ControllerBase
{
  public:
    Foo() { }
    ~Foo() override = default;

    [[=cut::web::Get]]
    auto my_route() -> cut::coro::Task<cut::web::Response>
    {
        std::println("Foo get called");

        co_return cut::web::Ok("hello world");
    }

    [[=cut::web::Post]]
    auto greeting(Person person) -> cut::coro::Task<cut::web::Response>
    {
        std::println("Foo greeting called");

        co_return cut::web::Ok(std::format("hello {} {}", person.first_name, person.last_name));
    }

    [[=cut::web::Post]]
    auto record(Person person) -> cut::coro::Task<cut::web::Response>
    {
        std::println("Foo record called");
        co_return cut::web::Ok(Record{.person = person, .id = 1});
    }

    auto another_method() -> void
    {
    }
    
};

class AnotherController : public cut::web::ControllerBase
{
  public:
    AnotherController(){}
    ~AnotherController() override = default;

    [[=cut::web::Get]]
    auto users() -> cut::coro::Task<cut::web::Response>
    {
        std::println("AnotherController users called");

        co_return cut::web::Ok();
    }
};

class Product : public cut::web::ControllerBase
{
  public:
    Product(cut::db::Db *db)
        : db_(db) {};
    ~Product() override = default;

    [[=cut::web::Post]]
    auto add(ProductCreateRequest product_create_request) -> cut::coro::Task<cut::web::Response>
    {
        cut::utils::log::info("Product::add called");

        auto db_runner = cut::db::DbRunner{db_};
        co_await db_runner.insert(ProductModel{
            .name = product_create_request.name,
            .stock_count = 0
        });

        co_return cut::web::Created();
    }

  private:
    cut::db::Db *db_;
};
// clang-format on

auto main() -> int
{
    try
    {
        auto app = cut::web::App<Foo, AnotherController, Product>{};

        app.run();
    }
    catch (std::runtime_error &e)
    {
        std::println(std::cerr, "{}", e.what());
    }

    return 0;
}
