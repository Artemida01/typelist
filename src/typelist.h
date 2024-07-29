#pragma once

#include <cstdio>
#include <tuple>
#include <type_traits>
#include <utility>

#include "conditional.h"

namespace tl {

///////////////////// TYPE_LIST /////////////////////

template <typename... Types>
struct type_list {
    template <typename... OtherTypes>
    using rebind = type_list<OtherTypes...>;
};

///////////////////// OTHER_LIST /////////////////////

template <typename... Types>
struct other_list {
    template <typename... OtherTypes>
    using rebind = other_list<OtherTypes...>;
};

///////////////////// TRANSFROM /////////////////////

template <template <typename...> typename F, typename List>
struct transform_impl;

template <template <typename...> typename F, template <typename...> typename ListT, typename... Types>
struct transform_impl<F, ListT<Types...>> {
    using type = ListT<F<Types>...>;
};

template <template <typename...> typename F, typename List>
using transform = typename transform_impl<F, List>::type;


///////////////////// APPLY /////////////////////

template <template <typename...> typename F, typename List>
struct apply_impl;

template <template <typename...> typename F, template <typename...> typename ListT, typename... Types>
struct apply_impl<F, ListT<Types...>> {
    using type = F<Types...>;
};

template <template <typename...> typename F, typename List>
using apply = typename apply_impl<F, List>::type;


///////////////////// PUSH_BACK /////////////////////

template <typename Type, typename List>
struct push_back_impl;

template <typename Type, template <typename...> typename ListT, typename... Types>
struct push_back_impl<Type, ListT<Types...>> {
    using type = ListT<Types..., Type>;
};

template <typename Type, typename List>
using push_back = typename push_back_impl<Type, List>::type;

///////////////////// CONCATENATE 2 /////////////////////

template <typename List1, typename List2>
struct concat2_impl;

template <template <typename...> typename ListT, typename... Types1, typename... Types2>
struct concat2_impl<ListT<Types1...>, ListT<Types2...>> {
    using type = ListT<Types1..., Types2...>;
};

template <typename List1, typename List2>
using concat = typename concat2_impl<List1, List2>::type;

///////////////////// PUSH_BACK_ALT /////////////////////

template <typename Type, typename List>
using push_back_alt = concat<List, type_list<Type>>;

///////////////////// PUSH_FRONT /////////////////////

template <typename Type, typename List>
using push_front = concat<typename List::template rebind<Type>, List>;

///////////////////// POP_FRONT /////////////////////

template <typename List>
struct pop_front_impl;

template <template <typename...> typename ListT, typename Head, typename... Types>
struct pop_front_impl<ListT<Head, Types...>> {
    using type = ListT<Types...>;
};

template <typename List>
using pop_front = typename pop_front_impl<List>::type;


///////////////////// REPLACE_FRONT /////////////////////

template <typename Type, typename List>
using replace_front = push_front<Type, pop_front<List>>;

///////////////////// FRONT /////////////////////

template <typename List>
struct front_impl;

template <template <typename...> typename ListT, typename Head, typename... Types>
struct front_impl<ListT<Head, Types...>> {
    using type = Head;
};

template <typename List>
using front = typename front_impl<List>::type;


///////////////////// REVERSE /////////////////////

template <typename List>
struct reverse_impl;

template <template <typename...> typename ListT>
struct reverse_impl<ListT<>> {
    using type = ListT<>;
};

template <template <typename...> typename ListT, typename Head, typename... Types>
struct reverse_impl<ListT<Head, Types...>> {
    using type = typename push_back_impl<Head, typename reverse_impl<ListT<Types...>>::type>::type;
};

template <typename List>
using reverse = typename reverse_impl<List>::type;

///////////////////// POP_BACK /////////////////////

template <typename List>
using pop_back = reverse<pop_front<reverse<List>>>;

///////////////////// CONCATENATE ANY NUMBER OF LISTS /////////////////////

template <typename... Lists>
struct concat_fast_impl;

template <typename List>
struct concat_fast_impl<List> {
    using type = List;
};

template <typename List1, typename List2, typename... Rest>
struct concat_fast_impl<List1, List2, Rest...> {
    using type = typename concat_fast_impl<typename concat2_impl<List1, List2>::type, Rest...>::type;
};

template <typename... Lists>
using concat_fast = typename concat_fast_impl<Lists...>::type;

///////////////////// FILTER /////////////////////

template <template <typename...> typename Pred, typename List>
struct filter_impl;

template <template <typename...> typename Pred, template <typename...> typename ListT, typename... Types>
struct filter_impl<Pred, ListT<Types...>> {
    template <typename Type>
    using single = std::conditional_t<Pred<Type>::value, ListT<Type>, type_list<>>;

    using type = concat_fast<single<Types>...>;
};

template <template <typename...> typename Pred, typename List>
using filter = typename filter_impl<Pred, List>::type;

///////////////////// GET N-th /////////////////////

template <std::size_t N, typename List>
struct get_nth_impl;

template <std::size_t N, template <typename...> typename ListT, typename Head, typename... Rest>
struct get_nth_impl<N, ListT<Head, Rest...>> {
    using type = typename get_nth_impl<N - 1, ListT<Rest...>>::type;
};

template <template <typename...> typename ListT, typename Head, typename... Rest>
struct get_nth_impl<0, ListT<Head, Rest...>> {
    using type = Head;
};

template <std::size_t N, typename List>
using get_nth = typename get_nth_impl<N, List>::type;


///////////////////// INDEX /////////////////////

template <std::size_t N>
using index = std::integral_constant<std::size_t, N>;

///////////////////// MAP_FIND /////////////////////

template <typename Key, typename Map>
struct map_find_impl;

template <typename Key, template <typename...> typename ListT, typename... Types>
struct map_find_impl<Key, ListT<Types...>> {
    using type = std::conditional_t<(std::is_same_v<Key, Types> || ...), tl::type_list<Key>, tl::type_list<>>;
};

template <typename Key, typename Map>
using map_find = typename map_find_impl<Key, Map>::type;



///////////////////// SECOND /////////////////////

template <typename List>
struct second_impl;

template <template <typename...> typename ListT, typename First, typename Second, typename... Rest>
struct second_impl<ListT<First, Second, Rest...>> {
    using type = Second;
};

template <typename List>
using second = typename second_impl<List>::type;


///////////////////// COUNT /////////////////////

template <typename List>
struct count_impl;

template <template <typename...> typename ListT, typename... Types>
struct count_impl<ListT<Types...>> {
    static constexpr std::size_t value = sizeof...(Types);
};

template <typename List>
inline constexpr std::size_t count = count_impl<List>::value;


///////////////////// ENUMERATE /////////////////////

template <typename List, typename Seq>
struct enumerate_impl;

template <template <typename...> typename ListT, typename... Types, std::size_t... Idxs>
struct enumerate_impl<ListT<Types...>, std::index_sequence<Idxs...>> {
    using type = ListT<tl::type_list<tl::index<Idxs>, Types>...>;
};

template <typename List>
using enumerate = typename enumerate_impl<List, std::make_index_sequence<count<List>>>::type;


///////////////////// GET N-th FAST /////////////////////

template <std::size_t N, typename List>
using get_nth_fast = second<map_find<index<N>, enumerate<List>>>;


///////////////////// CONTAINS /////////////////////

template<typename Type, typename List>
constexpr bool contains = !std::is_same_v<tl::type_list<>, tl::map_find<Type, List>>;

///////////////////// FLIP /////////////////////

//template<typename... Lists>
//struct flip_impl;
//
//template<template<typename...> typename ListT, typename... Types1, typename... Types2, typename... Rest>
//struct flip_impl<ListT<ListT<Types1...>, ListT<Types2...>, Rest...>> {
//    using type = concat_fast<type_list<ListT<Types2, Types1>>..., typename flip_impl<ListT<Rest...>>::type>;
//};
//
//template<template<typename...> typename ListT>
//struct flip_impl<ListT<>> {
//    using type = ListT<>;
//};
//
//template<typename... Lists>
//using flip = typename flip_impl<Lists...>::type;
template<typename... Lists>
struct flip_impl;

template<template<typename...> typename ListT, typename... Types1, typename... Types2, typename... Rest>
struct flip_impl<ListT<ListT<Types1...>, ListT<Types2...>, Rest...>> {
    using type = concat_fast<type_list<ListT<Types2, Types1>>..., typename flip_impl<Rest...>::type>;
};

template<template<typename...> typename ListT>
struct flip_impl<ListT<>> {
    using type = ListT<>;
};

template<typename... Lists>
using flip = typename flip_impl<Lists...>::type;














} // namespace tl
