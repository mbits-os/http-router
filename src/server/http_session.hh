// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <http-router/response.hh>
#include <memory>
#include <server/beast.hh>

namespace http_router {
	class router;
}

namespace http_router::server {
	class http_session : public std::enable_shared_from_this<http_session> {
		class queue final : public sender {
			enum { limit = 8 };
			struct work {
				virtual ~work() = default;
				virtual void operator()() = 0;
			};

			http_session& self_;
			std::vector<std::unique_ptr<work>> items_;

		public:
			explicit queue(http_session& self);
			bool is_full() const { return items_.size() >= limit; }

			bool on_write();

			template <class Body>
			void send(http::response<Body>&& msg);

			void send_response(
			    request const& req,
			    boost::beast::http::response<dynamic_body>&& msg) override;
			void send_response(
			    request const& req,
			    boost::beast::http::response<empty_body>&& msg) override;
			void send_response(
			    request const& req,
			    boost::beast::http::response<file_body>&& msg) override;
			void send_response(
			    request const& req,
			    boost::beast::http::response<string_body>&& msg) override;
		};

		beast::tcp_stream stream_;
		beast::flat_buffer buffer_;
		router const* router_;
		std::string const* server_name_;
		queue queue_;
		boost::optional<http::request_parser<http::string_body>> parser_;
		tcp::endpoint remote_;

	public:
		http_session(tcp::socket&& socket,
		             tcp::endpoint const& remote,
		             router const* router,
		             std::string const* server_name);

		void run() { do_read(); }

	private:
		void do_read();
		void on_read(beast::error_code ec, std::size_t bytes_transferred);
		void on_write(bool close,
		              beast::error_code ec,
		              std::size_t bytes_transferred);
		void do_close();
	};
}  // namespace http_router::server
