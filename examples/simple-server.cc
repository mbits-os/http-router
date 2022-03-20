#include <fmt/format.h>
#include <http-router/filters/static_files.hh>
#include <http-router/router.hh>
#include <http-router/server.hh>
#include <http-router/version.hh>

namespace beast = boost::beast;
namespace net = boost::asio;

void run(boost::asio::ip::tcp::endpoint endpoint,
         http_router::router const* handler,
         std::string const& server_name) {
	int const thread_count = std::thread::hardware_concurrency() - 1;
	boost::asio::io_context ioc{thread_count + 1};

	http_router::server::listen(ioc, endpoint, handler, server_name);
	net::signal_set signals{ioc, SIGINT, SIGTERM};
	signals.async_wait([&](beast::error_code const&, int) {
		fmt::print("Stopping...\n");
		ioc.stop();
	});

	std::vector<std::thread> threads{};
	threads.reserve(thread_count);
	for (auto i = thread_count; i > 0; --i) {
		threads.emplace_back([&] { ioc.run(); });
	}
	fmt::print("Started. Press ^C to stop.\n");
	ioc.run();

	for (auto& thread : threads)
		thread.join();

	fmt::print("Stopped.\n");
}

int main(int argc, char* argv[]) {
	using namespace http_router::filters;
	using tcp = boost::asio::ip::tcp;

	auto app =
	    http_router::router::cfg{}
	        .use(static_files<root_directory>::make(argc > 1 ? argv[0] : "."))
	        .compiled();

	using http_router::version;
	run(tcp::endpoint{tcp::v4(), 9000}, &app,
	    fmt::format("simple-server/{}.{}", version::major, version::minor));
}
