#pragma once

#include <cstddef>
#include <print>
#include <utility>

namespace cut::web
{

template <class... Controllers>
class App
{
  public:
    App()
    {
    }

    ~App() = default;
    App(const App &) = delete;
    auto operator=(const App &) -> App & = delete;
    App(App &&) = delete;
    auto operator=(App &&) -> App & = delete;

    auto run()
    {
        []<class F, std::size_t... Is>(F &&f, std::index_sequence<Is...>)
        {
            (f(Controllers{}), ...);
        }([](auto e) { std::println("{}", e.name()); }, std::make_index_sequence<sizeof...(Controllers)>());

        std::println("running");
    }
};

}
