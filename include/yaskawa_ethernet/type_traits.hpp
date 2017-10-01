#include <type_traits>
#include <utility>

namespace dr {
namespace yaskawa {

/// Map a type T from type From to to To.
template<typename T, typename From, typename To>
using map_type_t = std::conditional_t<std::is_same<T, From>::value, To, T>;

namespace detail {
	/// Unpack a tuple and map the type T of each tuple element to Converter<T>::type.
	template<template<typename> typename Converter, typename... T>
	auto map_tuple_elements(std::tuple<T...>) -> std::tuple<typename Converter<T>::type...>;
}

/// Map the type T of each tuple element to Converter<T>::type.
template<typename Tuple, template<typename> typename Converter>
using map_tuple_t = decltype(detail::map_tuple_elements<Converter>(std::declval<Tuple>()));

/// Empty type.
struct Empty {};

}}
