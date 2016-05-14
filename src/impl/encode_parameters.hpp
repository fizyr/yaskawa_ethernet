#pragma once
#include <ostream>

namespace dr {
namespace yaskawa {

namespace {
	template<typename... T>
	struct EncodeParametersImp;

	template<typename Head, typename... Tail>
	struct EncodeParametersImp<Head, Tail...> {
		static void addParameters(std::ostream & stream, Head const & head, Tail const & ... tail) {
			stream << head << ' ';
			EncodeParametersImp<Tail...>::addParameters(stream, tail...);
		}
	};

	template<typename Head>
	struct EncodeParametersImp<Head> {
		static void addParameters(std::ostream & stream, Head const & head) {
			stream << head << '\r';
		}
	};

	/// Encode a number of parameters.
	/**
	 * Each parameter is serialized writing it to the output stream.
	 * Paremeters are seperated by a space, and the list is terminated by a '\r'.
	 */
	template<typename... T>
	void encodeParameters(std::ostream & stream, T const & ... params) {
		EncodeParametersImp<T...>::addParameters(stream, params...);
	}
}

}}
