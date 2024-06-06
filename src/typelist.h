#pragma once

#include <cstdio>
#include <utility>

#include "conditional.h"

namespace tl {

// Type List
template <typename... Types>
struct type_list
{};

///////////////////// TRANSFROM /////////////////////

template <template <typename...> typename F, typename List>
struct transform_impl;

template <template <typename...> typename F, typename... Types>
struct transform_impl<F, type_list<Types...>>
{
    using type = type_list<F<Types>...>;
};

template <template <typename...> typename F, typename List>
using transform = transform_impl<F, List>::type;

///////////////////// APPLY /////////////////////

template <template <typename...> typename F, typename List>
struct apply_impl;

template <template <typename...> typename F, template <typename...> typename List, typename... Types>
struct apply_impl<F, List<Types...>>
{
    using type = F<Types...>;
};

template <template <typename...> typename F, typename List>
using apply = apply_impl<F, List>::type;

///////////////////// PUSH_BACK /////////////////////

template <typename Type, typename List>
struct push_back_impl;

template <typename Type, typename... Types>
struct push_back_impl<Type, type_list<Types...>>
{
    using type = type_list<Types..., Type>;
};

template <typename Type, typename List>
using push_back = push_back_impl<Type, List>::type;

///////////////////// CONCATENATE 2 /////////////////////

template <typename List1, typename List2>
struct concat2_impl;

template <typename... Types1, typename... Types2>
struct concat2_impl<type_list<Types1...>, type_list<Types2...>>
{
    using type = type_list<Types1..., Types2...>;
};

template <typename List1, typename List2>
using concat2 = concat2_impl<List1, List2>::type;

///////////////////// PUSH_BACK_ALT /////////////////////

template <typename Type, typename List>
using push_back_alt = concat2<List, type_list<Type>>;

///////////////////// PUSH_FRONT /////////////////////

template <typename Type, typename List>
using push_front = concat2<type_list<Type>, List>;

///////////////////// POP_FRONT /////////////////////

template <typename List>
struct pop_front_impl;

template <typename Head, typename... Types>
struct pop_front_impl<type_list<Head, Types...>>
{
    using type = type_list<Types...>;
};

template <typename List>
using pop_front = pop_front_impl<List>::type;

///////////////////// REPLACE_FRONT /////////////////////

template <typename Type, typename List>
using replace_front = push_front<Type, pop_front<List>>;

///////////////////// FRONT /////////////////////

template <typename List>
struct front_impl;

template <typename Head, typename... Types>
struct front_impl<type_list<Head, Types...>>
{
    using type = Head;
};

template <typename List>
using front = front_impl<List>::type;

///////////////////// REVERSE /////////////////////

template <typename List>
struct reverse_impl;

template <typename List>
using reverse = reverse_impl<List>::type;

template <typename List>
struct reverse_impl
{
    using type = push_back<front<List>, reverse<pop_front<List>>>;
};

template <>
struct reverse_impl<type_list<>>
{
    using type = type_list<>;
};

///////////////////// POP_BACK /////////////////////

template <typename List>
using pop_back = reverse<pop_front<reverse<List>>>;

///////////////////// CONCATENATE ANY NUMBER OF LISTS /////////////////////

template <typename Type>
struct identity
{
    using type = Type;
};

template <template <typename...> typename List1, template <typename...> typename List2, typename... Types1, typename... Types2>
identity<List1<Types1..., Types2...>> operator+(identity<List1<Types1...>>, identity<List2<Types2...>>);

template <typename... Lists>
using concat_fast = decltype((identity<Lists>{} + ... + identity<type_list<>>{}))::type;

///////////////////// FILTER /////////////////////

template <template <typename...> typename Pred, typename List>
struct filter_impl;

template <template <typename...> typename Pred, typename List>
using filter = filter_impl<Pred, List>::type;

template <template <typename...> typename Pred, typename... Types>
struct filter_impl<Pred, type_list<Types...>>
{
    template <typename Type>
    using single = conditional<Pred<Type>::value, type_list<Type>, type_list<>>;

    using type = concat_fast<single<Types>...>;
};

template <template <typename...> typename Pred>
struct filter_impl<Pred, type_list<>>
{
    using type = type_list<>;
};

///////////////////// GET N-th /////////////////////

template <std::size_t N, typename List>
struct get_nth_impl;

template <std::size_t N, typename List>
using get_nth = get_nth_impl<N, List>::type;

template <typename Head, typename... Rest>
struct get_nth_impl<0, type_list<Head, Rest...>>
{
    using type = Head;
};

template <std::size_t N, typename List>
struct get_nth_impl
{
    using type = get_nth<N - 1, pop_front<List>>;
};

///////////////////// INDEX /////////////////////

template <std::size_t N>
using index = std::integral_constant<std::size_t, N>;

///////////////////// MAP_FIND /////////////////////

template <typename... Bases>
struct inherit : Bases...
{};

template <typename Key, typename Map>
struct map_find_impl;

template <typename Key, typename... Lists>
struct map_find_impl<Key, type_list<Lists...>>
{
    template <typename... Rest>
    static type_list<Key, Rest...> f(type_list<Key, Rest...> *);

    using derived = inherit<Lists...>;

    using type = decltype(f(static_cast<derived *>(nullptr)));
};

template <typename Key, typename Map>
using map_find = map_find_impl<Key, Map>::type;

///////////////////// SECOND /////////////////////
template <typename List>
struct second_impl;

template <typename First, typename Second, typename... Rest>
struct second_impl<type_list<First, Second, Rest...>>
{
    using type = Second;
};

template <typename List>
using second = second_impl<List>::type;

///////////////////// COUNT /////////////////////
template <typename List>
struct count_impl;

template <typename... Types>
struct count_impl<type_list<Types...>>
{
    static constexpr std::size_t value = sizeof...(Types);
};

template <typename List>
inline constexpr std::size_t count = count_impl<List>::value;

///////////////////// ENUMERATE /////////////////////
template <typename List, typename Seq>
struct enumerate_impl;

template <typename... Types, std::size_t... Idxs>
struct enumerate_impl<type_list<Types...>, std::index_sequence<Idxs...>>
{
    using type = type_list<type_list<index<Idxs>, Types>...>;
};

template <typename List>
using enumerate = enumerate_impl<List, std::make_index_sequence<count<List>>>::type;

///////////////////// GET N-th FAST /////////////////////

template <std::size_t N, typename List>
using get_nth_fast = second<map_find<index<N>, enumerate<List>>>;

} // namespace tl
