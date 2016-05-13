#pragma once
#include <boost/system/error_code.hpp>

#include <system_error>

namespace dr {
namespace yaskawa {

namespace impl {
	class YaskawaCategory : public std::error_category, public boost::system::error_category {
		/// Get the name of the error category
		char const * name() const noexcept override;

		/// Get a descriptive error message for an error code.
		std::string message(int error) const noexcept override;
	};
}

impl::YaskawaCategory const & yaskawa_category();

namespace impl {
	class YaskawaError {
	private:
		int value;

	public:
		YaskawaError(int value) : value(value) {}

		operator std::error_code()                const { return {value, yaskawa_category()}; }
		operator std::error_condition()           const { return {value, yaskawa_category()}; }
		operator boost::system::error_code()      const { return {value, yaskawa_category()}; }
		operator boost::system::error_condition() const { return {value, yaskawa_category()}; }
	};
}

}}
