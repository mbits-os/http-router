#include <fmt/format.h>
#include <http-router/filters/static_files.hh>
#include <http-router/router.hh>
#include <http-router/server.hh>
#include <http-router/version.hh>
#include <thread>

namespace beast = boost::beast;
namespace net = boost::asio;
namespace server = http_router::server;

void run(net::ip::tcp::endpoint endpoint,
         http_router::router const* handler,
         std::string const& server_name) {
	int const thread_count = std::thread::hardware_concurrency() - 1;
	net::io_context ioc{thread_count + 1};

	auto [interface, port] = server::listen(ioc, endpoint, handler, server_name,
	                                        server::reuse_address{false});
	if (!port) {
		fmt::print("Error while seting up server...\n");
		return;
	}
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
	fmt::print("Listening on http://{}:{}/\n", interface, port);
	fmt::print("Started. Press ^C to stop.\n");
	ioc.run();

	for (auto& thread : threads)
		thread.join();

	fmt::print("Stopped.\n");
}

int main(int argc, char* argv[]) {
	using namespace http_router::filters;
	using tcp = net::ip::tcp;

#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif

	auto app =
	    http_router::router::cfg{}
	        .use(static_files<root_directory>::make(argc > 1 ? argv[0] : "."))
	        .compiled();

	using http_router::version;
	run(tcp::endpoint{tcp::v4(), 9000}, &app,
	    fmt::format("simple-server/{}.{}", version::major, version::minor));
}
