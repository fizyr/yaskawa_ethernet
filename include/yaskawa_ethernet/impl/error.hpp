#pragma once
#include <system_error>

namespace dr {
namespace yaskawa {

namespace impl {
	class YaskawaCategory : public std::error_category {
		/// Get the name of the error category
		char const * name() const noexcept override;

		/// Get a descriptive error message for an error code.
		std::string message(int error) const noexcept override;
	};
}

impl::YaskawaCategory const & yaskawa_category();

}}
