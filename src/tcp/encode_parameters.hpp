#pragma once
#include <boost/asio/streambuf.hpp>

#include <ostream>
#include <utility>

namespace dr {
namespace yaskawa {
namespace tcp {

template<typename T>
void encode(std::ostream & stream, T const & value) {
	stream << value;
}

namespace {
	template<typename... T>
	struct EncodeParametersImp;

	template<typename Head, typename... Tail>
	struct EncodeParametersImp<Head, Tail...> {
		static void addParameters(std::ostream && stream, Head const & head, Tail const & ... tail) {
			stream << head << ',';
			EncodeParametersImp<Tail...>::addParameters(std::move(stream), tail...);
		}
	};

	template<typename Head>
	struct EncodeParametersImp<Head> {
		static void addParameters(std::ostream && stream, Head const & head) {
			encode(stream, head);
			stream.put('\r');
		}
	};

	/// Encode a number of parameters.
	/**
	 * Each parameter is serialized writing it to the output stream.
	 * Paremeters are seperated by a space, and the list is terminated by a '\r'.
	 */
	template<typename... T>
	void encodeParameters(boost::asio::streambuf & buffer, T const & ... params) {
		EncodeParametersImp<T...>::addParameters(std::ostream{&buffer}, params...);
	}
}

}}}
