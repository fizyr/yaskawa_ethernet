#pragma once
#if defined(__has_include) && __cplusplus >= 201402
#  if __has_include(<string_view>)
#    define STD_STRING_VIEW
#  elif __has_include(<experimental/string_view>)
#    define EXPERIMENTAL_STRING_VIEW
#  else
#    define BOOST_STRING_VIEW
#  endif
#else
#  define BOOST_STRING_VIEW
#endif

#ifdef STD_STRING_VIEW
#  include <string_view>
#elif defined(EXPERIMENTAL_STRING_VIEW)
#  include <experimental/string_view>
#else
#  include <boost/utility/string_ref.hpp>
#endif

namespace dr {
namespace yaskawa {

#if defined(STD_STRING_VIEW)
using string_view = std::string_view;
#elif defined(EXPERIMENTAL_STRING_VIEW)
using string_view = std::experimental::string_view;
#else
using string_view = boost::string_ref;
#endif

}}
