#pragma once

#include "conditional.h"

#include <stdlib.h>

#include <tuple>
#include <type_traits>

namespace tl {

// Type List
template <typename... Types>
struct type_list {};

// Transform
template <template <typename...> typename F, typename List>
struct transform_impl;

template <template <typename...> typename F, typename... Types>
struct transform_impl<F, type_list<Types...>> {
  using type = type_list<F<Types>...>;
};

template <template <typename...> typename F, typename List>
using transform = transform_impl<F, List>::type;

// Apply
template <template <typename...> typename F, typename List>
struct apply_impl;

template <template <typename...> typename F, template <typename...> typename List, typename... Types>
struct apply_impl<F, List<Types...>> {
  using type = F<Types...>;
};

template <template <typename...> typename F, typename List>
using apply = apply_impl<F, List>::type;

// Push Back

template <typename Type, typename List>
struct push_back_impl;

template <typename Type, typename... Types>
struct push_back_impl<Type, type_list<Types...>> {
  using type = type_list<Types..., Type>;
};

template <typename Type, typename List>
using push_back = push_back_impl<Type, List>::type;

// Pop front
template <typename List>
struct pop_front_impl;

template <typename Head, typename... Types>
struct pop_front_impl<type_list<Head, Types...>> {
  using type = type_list<Types...>;
};

template <typename List>
using pop_front = pop_front_impl<List>::type;

// Front
template <typename List>
struct front_impl;

template <typename Head, typename... Types>
struct front_impl<type_list<Head, Types...>> {
  using type = Head;
};

template <typename List>
using front = front_impl<List>::type;

// Reverse
template <typename List>
struct reverse_impl;

template <typename List>
using reverse = reverse_impl<List>::type;

template <typename List>
struct reverse_impl {
  using type = push_back<front<List>, reverse<pop_front<List>>>;
};

template <>
struct reverse_impl<type_list<>> {
  using type = type_list<>;
};

// Pop back
template <typename List>
using pop_back = reverse<pop_front<reverse<List>>>;

// Concatenate an arbitrary number of typelists

template <typename Type>
struct identity {
  using type = Type;
};

template <
    template <typename...>
    typename List1,
    template <typename...>
    typename List2,
    typename... Types1,
    typename... Types2>
identity<List1<Types1..., Types2...>> operator+(identity<List1<Types1...>>, identity<List2<Types2...>>);

template <template <typename...> typename List, typename... Lists>
using concat_fast = decltype((identity<List<>>{} + ... + identity<Lists>{}))::type;

// Get nth element
template <std::size_t N, typename List>
struct get_nth_impl;

template <std::size_t N, typename List>
using get_nth = get_nth_impl<N, List>::type;

template <typename Head, typename... Rest>
struct get_nth_impl<0, type_list<Head, Rest...>> {
  using type = Head;
};

template <std::size_t N, typename List>
struct get_nth_impl {
  using type = get_nth<N - 1, pop_front<List>>;
};

// Index
template <std::size_t N>
using index = std::integral_constant<std::size_t, N>;

// Map Find
template <typename... Bases>
struct inherit : Bases... {};

template <typename Key, typename Map>
struct map_find_impl;

template <typename Key, typename... Lists, template <typename...> typename List>
struct map_find_impl<Key, List<Lists...>> {
  template <typename... Rest>
  static List<Key, Rest...> f(List<Key, Rest...>*);

  using derived = inherit<Lists...>;

  using type = decltype(f(static_cast<derived*>(nullptr)));
};

template <typename Key, typename Map>
using map_find = map_find_impl<Key, Map>::type;

// Second
template <typename List>
struct second_impl;

template <typename First, typename Second, typename... Rest, template <typename...> typename List>
struct second_impl<List<First, Second, Rest...>> {
  using type = Second;
};

template <typename List>
using second = second_impl<List>::type;

// Count
template <typename List>
struct count_impl;

template <typename... Types, template <typename...> typename List>
struct count_impl<List<Types...>> {
  static constexpr std::size_t value = sizeof...(Types);
};

template <typename List>
inline constexpr std::size_t count = count_impl<List>::value;

// Enumerate
template <typename List, typename Seq>
struct enumerate_impl;

template <typename... Types, std::size_t... Idxs, template <typename...> typename ListType>
struct enumerate_impl<ListType<Types...>, std::index_sequence<Idxs...>> {
  using type = ListType<ListType<index<Idxs>, Types>...>;
};

template <typename List>
using enumerate = typename enumerate_impl<List, std::make_index_sequence<count<List>>>::type;

// Get nth (fast)
template <std::size_t N, typename List>
using get_nth_fast = second<map_find<index<N>, enumerate<List>>>;

// Contains
template <typename Type, typename List>
struct contains_impl;

template <typename Type, template <typename...> typename List, typename... Types>
struct contains_impl<Type, List<Types...>> {
  static constexpr bool result = (std::is_same_v<Type, Types> || ...);
};

template <typename Key, typename List>
inline constexpr bool contains = contains_impl<Key, List>::result;

// Flip
template <typename List>
struct flip_impl {};

template <template <typename...> typename List>
struct flip_impl<List<>> {
  using type = List<>;
};

template <typename... L, typename... R, template <typename...> typename... Type, template <typename...> typename List>
struct flip_impl<List<Type<L, R>...>> {
  using type = List<Type<R, L>...>;
};

template <typename List>
using flip = typename flip_impl<List>::type;

// Index of unique
template <typename Type, typename List>
inline constexpr size_t index_of_unique = second<map_find<Type, flip<enumerate<List>>>>{};

// Flatten
template <typename Type>
struct flatten_impl {
  using type = type_list<Type>;
};

template <typename List>
using flatten = typename flatten_impl<List>::type;

template <typename... Types, template <typename...> typename ListType>
struct flatten_impl<ListType<Types...>> {
  using type = concat_fast<ListType, flatten<Types>...>;
};

// Merge
template <typename List1, typename List2, template <typename...> typename Cmp>
struct merge_two_impl;

template <template <typename...> typename List, template <typename...> typename Cmp>
struct merge_two_impl<List<>, List<>, Cmp> {
  using type = List<>;
};

template <typename... Type, template <typename...> typename List, template <typename...> typename Cmp>
struct merge_two_impl<List<>, List<Type...>, Cmp> {
  using type = List<Type...>;
};

template <typename... Type, template <typename...> typename List, template <typename...> typename Cmp>
struct merge_two_impl<List<Type...>, List<>, Cmp> {
  using type = List<Type...>;
};

template <typename List1, typename List2, template <typename...> typename Cmp>
using merge = typename merge_two_impl<List1, List2, Cmp>::type;

template <
    typename FirstL,
    typename... SecondL,
    typename FirstR,
    typename... SecondR,
    template <typename...>
    typename List,
    template <typename...>
    typename Cmp>
struct merge_two_impl<List<FirstL, SecondL...>, List<FirstR, SecondR...>, Cmp> {
  using left = concat_fast<List, List<FirstL>, merge<List<SecondL...>, List<FirstR, SecondR...>, Cmp>>;
  using right = concat_fast<List, List<FirstR>, merge<List<FirstL, SecondL...>, List<SecondR...>, Cmp>>;
  using type = conditional<Cmp<FirstL, FirstR>::value, left, right>;
};

// Split
template <typename _List, typename _Idxs>
struct split_impl;

template <template <typename...> typename _List, typename... _Type, size_t... Idx>
struct split_impl<_List<_Type...>, std::index_sequence<Idx...>> {
  static constexpr size_t cnt = sizeof...(_Type);
  static constexpr size_t middle = cnt / 2;
  using left = concat_fast < _List, conditional<Idx<middle, _List<_Type>, _List<>>...>;
  using right = concat_fast<_List, conditional<middle <= Idx, _List<_Type>, _List<>>...>;
};

template <typename _List>
using split = split_impl<_List, std::make_index_sequence<count<_List>>>;

// Merge Sort
template <typename List, template <typename...> typename Cmp>
struct merge_sort_impl;

template <template <typename...> typename List, template <typename...> typename Cmp>
struct merge_sort_impl<List<>, Cmp> {
  using type = List<>;
};

template <typename Type, template <typename...> typename List, template <typename...> typename Cmp>
struct merge_sort_impl<List<Type>, Cmp> {
  using type = List<Type>;
};

template <typename List, template <typename...> typename Cmp>
using merge_sort = typename merge_sort_impl<List, Cmp>::type;

template <typename Type, typename... Rest, template <typename...> typename List, template <typename...> typename Cmp>
struct merge_sort_impl<List<Type, Rest...>, Cmp> {
  using splitted = split<List<Type, Rest...>>;
  using first = merge_sort<typename splitted::left, Cmp>;
  using second = merge_sort<typename splitted::right, Cmp>;
  using type = merge<first, second, Cmp>;
};

} // namespace tl

// contains<T, List>
// flip<List>, List -- список пар
// index_of<T, List>
// flatten<List>
// merge_sort<List, Compare>
