#pragma once

#include "conditional.h"

#include <stdlib.h>

#include <tuple>
#include <type_traits>

namespace tl {

// Type List
template <typename... Types>
struct type_list {};

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
struct merge_impl;

template <template <typename...> typename List, template <typename...> typename Cmp>
struct merge_impl<List<>, List<>, Cmp> {
  using type = List<>;
};

template <typename... Type, template <typename...> typename List, template <typename...> typename Cmp>
struct merge_impl<List<>, List<Type...>, Cmp> {
  using type = List<Type...>;
};

template <typename... Type, template <typename...> typename List, template <typename...> typename Cmp>
struct merge_impl<List<Type...>, List<>, Cmp> {
  using type = List<Type...>;
};

template <typename List1, typename List2, template <typename...> typename Cmp>
using merge = typename merge_impl<List1, List2, Cmp>::type;

template <
    typename FirstL,
    typename... SecondL,
    typename FirstR,
    typename... SecondR,
    template <typename...>
    typename List,
    template <typename...>
    typename Cmp>
struct merge_impl<List<FirstL, SecondL...>, List<FirstR, SecondR...>, Cmp> {
  template <bool Condition>
  using select_type = std::conditional_t<
      Condition,
      concat_fast<List, List<FirstL>, merge<List<SecondL...>, List<FirstR, SecondR...>, Cmp>>,
      concat_fast<List, List<FirstR>, merge<List<FirstL, SecondL...>, List<SecondR...>, Cmp>>>;

  using type = select_type<Cmp<FirstL, FirstR>::value>;
};

// Split
template <typename List, typename Idxs>
struct split_impl;

template <template <typename...> typename List, typename... Type, size_t... Idx>
struct split_impl<List<Type...>, std::index_sequence<Idx...>> {
  static constexpr size_t count = sizeof...(Type);
  static constexpr size_t middle = count / 2;
  using left = concat_fast<List, conditional<(Idx < middle), List<Type>, List<>>...>;
  using right = concat_fast<List, conditional<(middle <= Idx), List<Type>, List<>>...>;
};

template <typename List>
using split = split_impl<List, std::make_index_sequence<count<List>>>;

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
  using left = merge_sort<typename splitted::left, Cmp>;
  using right = merge_sort<typename splitted::right, Cmp>;
  using type = merge<left, right, Cmp>;
};

} // namespace tl

// contains<T, List>
// flip<List>, List -- список пар
// index_of<T, List>
// flatten<List>
// merge_sort<List, Compare>
