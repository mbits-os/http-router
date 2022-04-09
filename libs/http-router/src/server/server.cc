// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4619 4242)
#endif
#include <boost/asio/ip/host_name.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <filesystem>
#include <http-router/server.hh>
#include <memory>
#include <server/http_session.hh>

using namespace std::literals;

namespace http_router::server {
	namespace {
		inline std::string host_name() {
			boost::system::error_code ec{};
			auto result = net::ip::host_name(ec);
			if (ec) {
				fail(ec, "host_name");
				result = "localhost"s;
			}
			return result;
		}

		class listener : public std::enable_shared_from_this<listener> {
			net::io_context& ioc_;
			tcp::acceptor acceptor_;
			std::string server_name_;
			router const* router_;

		public:
			listener(net::io_context& ioc,
			         tcp::endpoint const& endpoint,
			         std::string const& user_agent,
			         router const* handler,
			         reuse_address reuse_address_opt);

			// Start accepting incoming connections
			local_endpoint run() {
				auto const port = [this]() -> net::ip::port_type {
					try {
						return acceptor_.local_endpoint().port();
					} catch (boost::system::system_error const& sys) {
						fail(sys.code(), "local_endpoint");
						return 0;
					} catch (...) {
						return 0;
					}
				}();
				if (!port) return {};
				next_accept();
				return {
				    .interface = host_name(),
				    .port = port,
				};
			}

		private:
			void next_accept() {
				// The new connection gets its own strand
				acceptor_.async_accept(
				    net::make_strand(ioc_),
				    beast::bind_front_handler(&listener::on_accept,
				                              shared_from_this()));
			}

			void on_accept(beast::error_code ec, tcp::socket socket);
		};

		listener::listener(net::io_context& ioc,
		                   tcp::endpoint const& endpoint,
		                   std::string const& server_name,
		                   router const* handler,
		                   reuse_address reuse_address_opt)
		    : ioc_{ioc}
		    , acceptor_{net::make_strand(ioc)}
		    , server_name_{server_name}
		    , router_{handler} {
			beast::error_code ec;

			// Open the acceptor
			acceptor_.open(endpoint.protocol(), ec);
			if (ec) {
				fail(ec, "open");
				return;
			}

			// Allow address reuse
			acceptor_.set_option(
			    net::socket_base::reuse_address{reuse_address_opt.value()}, ec);
			if (ec) {
				fail(ec, "set_option");
				return;
			}

			// Bind to the server address
			acceptor_.bind(endpoint, ec);
			if (ec) {
				fail(ec, "bind");
				return;
			}

			// Start listening for connections
			acceptor_.listen(net::socket_base::max_listen_connections, ec);
			if (ec) {
				fail(ec, "listen");
				return;
			}
		}

		void listener::on_accept(beast::error_code ec, tcp::socket socket) {
			if (ec) {
				fail(ec, "accept");
			} else {
				// Create the http session and run it
				auto remote = socket.remote_endpoint(ec);
				if (ec) {
					fail(ec, "accept/remote_endpoint");
				} else {
					std::make_shared<http_session>(std::move(socket), remote,
					                               router_, &server_name_)
					    ->run();
				}
			}

			next_accept();
		}
	}  // namespace

	local_endpoint listen(net::io_context& ioc,
	                      tcp::endpoint const& endpoint,
	                      router const* handler,
	                      std::string const& server_name,
	                      reuse_address reuse_address_opt) {
		return std::make_shared<listener>(
		           ioc, endpoint,
		           !server_name.empty()
		               ? fmt::format("{} {}", server_name,
		                             BOOST_BEAST_VERSION_STRING)
		               : BOOST_BEAST_VERSION_STRING,
		           handler, reuse_address_opt)
		    ->run();
	}
}  // namespace http_router::server
