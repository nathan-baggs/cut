#pragma once

#include <experimental/meta>

#include "cut/utils/type_list.h"

namespace cut::di
{
namespace details
{

template <class T>
struct Node
{
    inline static auto obj = T{};
};

template <class C, class T>
struct Create;

template <class C, class... T>
struct Create<C, utils::TypeList<T...>>
{
    static auto create()
    {
        return C{std::addressof(Node<T>::obj)...};
    }
};
}

template <class T, class S>
class Injector;

template <class P>
using Underlying = std::remove_pointer_t<P>;

/**
 * Perform dependency injection via reflection.
 *
 * Can create an object by automatically finding its ctor params, mapping them to derived classes and then passing them
 * in.
 *
 * Note that derived classes are only created once, each created object gets the same dependency.
 *
 * Not a massive fan of constructing with a TypeList of Base then TypeList of matching Derived - but it works for now.
 *
 * This was the first reflection code I wrote - so could be a bit shaky.
 */
template <class... T, class... S>
class Injector<utils::TypeList<T...>, utils::TypeList<S...>>
{
  public:
    /**
     * Create a C, dependency inject it's ctor params.
     */
    template <class C>
    constexpr auto create() const
    {
        return details::Create<C, std::decay_t<decltype(create_type_list<C>())>>::create();
    }

  private:
    /**
     * Convert the ctor params of C into a TypeList (mapping bae to derived).
     */
    template <class C>
    constexpr auto create_type_list() const
    {
        constexpr auto ctx = std::meta::access_context::current();
        constexpr auto ctor = (std::meta::members_of(^^C, ctx) |
                               std::views::filter([](const auto &e) { return std::meta::is_constructor(e); }))
                                  .front();
        constexpr auto params = std::define_static_array(std::meta::parameters_of(ctor));

        return [&]<std::size_t... Is>(std::index_sequence<Is...>) constexpr
        {
            return utils::TypeList<decltype(create_arg<std::remove_pointer_t<typename[:type_of(params[Is]):]>>())...>{};
        }(std::make_index_sequence<params.size()>());
    }

    template <class C>
    constexpr static auto create_arg()
    {
        constexpr auto element = std::ranges::find(base, ^^C);
        static_assert(element != std::ranges::cend(base));

        using d = typename[:derived[std::ranges::distance(std::ranges::cbegin(base), element)]:];

        // i'd like to not have to construct d here
        return d{};
    }

    // convert the typelists to arrays of meta infos
    static constexpr auto base = std::define_static_array(std::vector<std::meta::info>{^^T...});
    static constexpr auto derived = std::define_static_array(std::vector<std::meta::info>{^^S...});
};

}
