#pragma once

using namespace std::literals;

namespace cut::db
{

namespace annotations
{

// simple annotations for database classes

// member must be the ID column in the database
struct Id
{
};

}

static constexpr auto Id = annotations::Id{};

}
