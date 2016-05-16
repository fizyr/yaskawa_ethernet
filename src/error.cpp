#include "error.hpp"

#include <string>

namespace dr {
namespace yaskawa {

DetailedError DetailedError::empty = DetailedError{};

namespace {
	impl::YaskawaCategory yaskawa_category_;
}

char const * impl::YaskawaCategory::name() const noexcept {
	return "yaskawa";
}

std::string impl::YaskawaCategory::message(int error) const noexcept {
	switch (errc::errc_t(error)) {
		case errc::malformed_response:    return "malformed message";
		case errc::command_failed:        return "command failed";
	}

	return "unknown error: " + std::to_string(error);
}

/// The error category for protocol errors.
impl::YaskawaCategory const & yaskawa_category() {
	return yaskawa_category_;
}

}}
