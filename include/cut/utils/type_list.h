#pragma once

namespace cut::utils
{

template <class... Ts>
struct TypeList
{
};

template <class T>
struct Visitor;

template <class Head, class... Tail>
struct Visitor<TypeList<Head, Tail...>>
{
    template <class F>
    static auto visit(F &&f)
    {
        f(Head{});

        Visitor<TypeList<Tail...>>::visit(std::forward<F>(f));
    }
};

template <class Head>
struct Visitor<TypeList<Head>>
{
    template <class F>
    static auto visit(F &&f)
    {
        f(Head{});
    }
};

template <class F, class... Ts>
auto visit(TypeList<Ts...>, F &&f)
{
    Visitor<TypeList<Ts...>>::visit(std::forward<F>(f));
}

}
