#include <iostream>
#include <print>
#include <string_view>

#include "cut/coro/task.h"
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
class Foo : public cut::web::ControllerBase
{
  public:
    ~Foo() override = default;

    [[=cut::web::Get]]
    auto my_route() -> cut::coro::Task<cut::web::Response>
    {
        std::println("Foo get called");

        co_return cut::web::Ok("hello world"sv);
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

        co_return cut::web::Ok(Record{.person = person, .id = 100});
    }

    auto another_method() -> void
    {
    }
};

class AnotherController : public cut::web::ControllerBase
{
  public:
    ~AnotherController() override = default;

    [[=cut::web::Get]]
    auto users() -> cut::coro::Task<cut::web::Response>
    {
        std::println("AnotherController users called");

        co_return cut::web::Ok();
    }
};
// clang-format on

auto main() -> int
{
    try
    {
        auto app = cut::web::App<Foo, AnotherController>{};

        app.run();
    }
    catch (std::runtime_error &e)
    {
        std::println(std::cerr, "{}", e.what());
    }

    return 0;
}
