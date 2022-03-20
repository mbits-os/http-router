// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <http-router/request.hh>
#include <http-router/router.hh>
#include <server/http_session.hh>

using namespace std::literals;

namespace http_router::server {
	http_session::queue::queue(http_session& self) : self_(self) {
		static_assert(limit > 0, "queue limit must be positive");
		items_.reserve(limit);
	}
	bool http_session::queue::on_write() {
		BOOST_ASSERT(!items_.empty());
		auto const was_full = is_full();
		items_.erase(items_.begin());
		if (!items_.empty()) (*items_.front())();
		return was_full;
	}

	template <class Body>
	void http_session::queue::send(http::response<Body>&& msg) {
		// This holds a work item
		struct work_impl : work {
			http_session& self_;
			http::response<Body> msg_;

			work_impl(http_session& self, http::response<Body>&& msg)
			    : self_{self}, msg_{std::move(msg)} {}

			void operator()() {
				http::async_write(
				    self_.stream_, msg_,
				    beast::bind_front_handler(&http_session::on_write,
				                              self_.shared_from_this(),
				                              msg_.need_eof()));
			}
		};

		items_.push_back(boost::make_unique<work_impl>(self_, std::move(msg)));

		if (items_.size() == 1) (*items_.front())();
	}
	void http_session::queue::send_response(
	    request const& req,
	    boost::beast::http::response<dynamic_body>&& msg) {
		self_.router_->before_send(req, msg);
		send(std::move(msg));
	}
	void http_session::queue::send_response(
	    request const& req,
	    boost::beast::http::response<empty_body>&& msg) {
		self_.router_->before_send(req, msg);
		send(std::move(msg));
	}
	void http_session::queue::send_response(
	    request const& req,
	    boost::beast::http::response<file_body>&& msg) {
		self_.router_->before_send(req, msg);
		send(std::move(msg));
	}
	void http_session::queue::send_response(
	    request const& req,
	    boost::beast::http::response<string_body>&& msg) {
		self_.router_->before_send(req, msg);
		send(std::move(msg));
	}

	http_session::http_session(tcp::socket&& socket,
	                           tcp::endpoint const& remote,
	                           router const* router,
	                           std::string const* server_name)
	    : stream_{std::move(socket)}
	    , router_{router}
	    , server_name_{server_name}
	    , queue_{*this}
	    , remote_{remote} {}

	void http_session::do_read() {
		parser_.emplace();
		parser_->body_limit(10000);
		stream_.expires_never();
		http::async_read(stream_, buffer_, *parser_,
		                 beast::bind_front_handler(&http_session::on_read,
		                                           shared_from_this()));
	}

	void http_session::on_read(beast::error_code ec,
	                           std::size_t bytes_transferred) {
		boost::ignore_unused(bytes_transferred);
		if (ec == http::error::end_of_stream) return do_close();
		if (ec) return fail(ec, "read");
		auto http_req = std::move(parser_->release());
		request req{http_req, remote_};
		response resp{http_req.version(), req, queue_};
		resp.set(http::field::server, *server_name_);
		resp.keep_alive(http_req.keep_alive() || http_req.version() > 10);
		router_->handle_request(resp);
		if (!queue_.is_full()) do_read();
	}

	void http_session::on_write(bool close,
	                            beast::error_code ec,
	                            std::size_t bytes_transferred) {
		boost::ignore_unused(bytes_transferred);

		if (ec) return fail(ec, "write");

		if (close) {
			// This means we should close the connection, usually because
			// the response indicated the "Connection: close" semantic.
			return do_close();
		}

		// Inform the queue that a write completed
		if (queue_.on_write()) {
			// Read another request
			do_read();
		}
	}

	void http_session::do_close() {
		// Send a TCP shutdown
		beast::error_code ec;
		stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

		// At this point the connection is closed gracefully
	}
}  // namespace http_router::server
