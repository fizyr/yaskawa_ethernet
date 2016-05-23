#pragma once
#include "types.hpp"

#include <cstdint>

namespace dr {
namespace yaskawa {

namespace impl {
	/// Generic ReadVariable command.
	template<typename T>
	struct ReadVariable {
		struct Request {
			int index;

			Request() {}
			Request(int index) : index{index} {}
			operator int() { return index; }
		};

		struct Response {
			T value;

			Response() {}
			Response(T value) : value{value} {}
			operator T       & ()       { return value; }
			operator T const & () const { return value; }
		};
	};

	/// Generic WriteVariable command.
	template<typename T>
	struct WriteVariable {
		struct Request { int index; T value; };
		using Response = void;
	};
}

using ReadInt8Variable      = impl::ReadVariable<std::uint8_t>;
using ReadInt16Variable     = impl::ReadVariable<std::int16_t>;
using ReadInt32Variable     = impl::ReadVariable<std::int32_t>;
using ReadFloat32Variable   = impl::ReadVariable<float>;
using ReadPositionVariable  = impl::ReadVariable<Position>;
using WriteInt8Variable     = impl::WriteVariable<std::uint8_t>;
using WriteInt16Variable    = impl::WriteVariable<std::int16_t>;
using WriteInt32Variable    = impl::WriteVariable<std::int32_t>;
using WriteFloat32Variable  = impl::WriteVariable<float>;
using WritePositionVariable = impl::WriteVariable<Position>;

}}
