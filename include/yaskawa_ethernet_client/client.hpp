#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include <functional>

namespace dr {
namespace yaskawa {

class EthernetClient {
public:
	using Callback = std::function<void (boost::system::error_code const & error)>;

	template<typename T>
	using ResultCallback = std::function<void (boost::system::error_code const & error, T const & result)>;

private:
	boost::asio::ip::tcp::socket socket;
	boost::asio::streambuf read_buffer;

public:
	EthernetClient(boost::asio::io_service & ios);

	void start(int keep_alive, Callback const & callback);

	void readByteVariable(ResultCallback<std::uint8_t> const & callback);
	void readIntegerVariable(ResultCallback<int> const & callback);
	void readDoubleVariable(ResultCallback<double> const & callback);
	void readStringVariable(ResultCallback<double> const & callback);

	void writeVariable(Callback const & callback);
};

}}
