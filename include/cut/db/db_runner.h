#pragma once

#include <coroutine>
#include <experimental/meta>
#include <format>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include "db/db.h"
#include "utils/error.h"

using namespace std::literals;

namespace cut::db
{

namespace details
{

auto convert_type(std::string_view native_type) -> std::string_view
{
    if (native_type == "uint32_t"sv)
    {
        return "INTEGER"sv;
    }
    else if (native_type == "basic_string<char, char_traits<char>, allocator<char>>"sv)
    {
        return "TEXT"sv;
    }

    utils::ensure(false, "could not convert: {}", native_type);
    return {};
}

auto format_value(std::uint32_t value) -> std::string
{
    return std::to_string(value);
}

auto format_value(const std::string &value) -> std::string
{
    return std::format("'{}'", value);
}

template <class Row>
constexpr auto create_table_sql() -> std::string
{
    auto strm = std::stringstream{};

    strm << std::format("CREATE TABLE IF NOT EXISTS {}s", std::meta::display_string_of(^^Row));
    strm << "(";

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto member : std::define_static_array(std::meta::nonstatic_data_members_of(^^Row, ctx)))
    {
        strm << std::format("{} ", std::meta::display_string_of(member));
        strm << std::format("{} ", convert_type(std::meta::display_string_of(std::meta::type_of(member))));

        constexpr auto annotations = std::define_static_array(std::meta::annotations_of(member));

        if constexpr (!std::ranges::empty(annotations))
        {
            strm << "PRIMARY KEY";
        }
        strm << ",";
    }

    strm.seekp(-1, std::ios_base::end);
    strm << ");";

    return strm.str();
}

template <class Row>
constexpr auto insert_row_sql(const Row &row) -> std::string
{
    auto strm = std::stringstream{};

    strm << std::format("INSERT INTO {}s ", std::meta::display_string_of(^^Row));
    strm << "(";

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto member : std::define_static_array(std::meta::nonstatic_data_members_of(^^Row, ctx)))
    {
        constexpr auto annotations = std::define_static_array(std::meta::annotations_of(member));

        if constexpr (std::ranges::empty(annotations))
        {
            strm << std::format("{} ", std::meta::display_string_of(member));
            strm << ",";
        }
    }

    strm.seekp(-1, std::ios_base::end);
    strm << ") VALUES (";

    template for (constexpr auto member : std::define_static_array(std::meta::nonstatic_data_members_of(^^Row, ctx)))
    {
        constexpr auto annotations = std::define_static_array(std::meta::annotations_of(member));

        if constexpr (std::ranges::empty(annotations))
        {
            strm << format_value(row.[:member:]);
            strm << ",";
        }
    }

    strm.seekp(-1, std::ios_base::end);
    strm << ");";

    return strm.str();
}

}

class DbRunner
{
  public:
    DbRunner(Db *db)
        : db_(db)
    {
    }

    template <class Row>
    auto insert(Row &&row)
    {
        struct Awaitable
        {
            bool await_ready()
            {
                return true;
            }

            bool await_suspend(std::coroutine_handle<>)
            {
                return true;
            }

            auto await_resume()
            {
                db->execute(details::create_table_sql<Row>());
                db->execute(details::insert_row_sql(this->row));
            }

            Db *db;
            Row row;
        };

        return Awaitable{db_, std::forward<Row>(row)};
    }

  private:
    Db *db_;
};

}
