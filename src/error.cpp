#include "error.hpp"

#include <string>

namespace dr {
namespace yaskawa {

namespace {
	class : public std::error_category {
		/// Get the name of the error category
		char const * name() const noexcept override {
			return "yaskawa";
		}

		/// Get a descriptive error message for an error code.
		std::string message(int error) const noexcept override {
			switch (errc::errc_t(error)) {
				case errc::malformed_response:    return "malformed message";
				case errc::command_failed:        return "command failed";
			}
		}
	} yaskawa_category_;
}

/// The error category for protocol errors.
std::error_category const & yaskawa_category() {
	return yaskawa_category_;
}

}}
