// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4619 4242)
#endif
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/string_body.hpp>
#include <http-router/beast/patched/file_body.hh>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace http_router {
	class request;

	using status = boost::beast::http::status;

	using dynamic_body = boost::beast::http::dynamic_body;
	using empty_body = boost::beast::http::empty_body;
	using file_body = boost::beast::http::patched::file_body;
	using string_body = boost::beast::http::string_body;

	class sender {
	public:
		virtual void send_response(
		    request const& req,
		    boost::beast::http::response<dynamic_body>&&) = 0;
		virtual void send_response(
		    request const& req,
		    boost::beast::http::response<empty_body>&&) = 0;
		virtual void send_response(
		    request const& req,
		    boost::beast::http::response<file_body>&&) = 0;
		virtual void send_response(
		    request const& req,
		    boost::beast::http::response<string_body>&&) = 0;

	protected:
		~sender() = default;
	};

	enum class content_length : bool { unset = true, set = false };

	class response
	    : public boost::beast::http::response<empty_body,
	                                          boost::beast::http::fields> {
	public:
		using beast_response =
		    boost::beast::http::response<empty_body,
		                                 boost::beast::http::fields>;
		response(unsigned version,
		         request const& req,
		         http_router::sender& out) noexcept
		    : beast_response{status::ok, version}, out_{out}, req_{req} {}

		http_router::sender& sender() noexcept { return out_; }
		request const& req() const noexcept { return req_; }

		template <typename Body>
		void send(Body::value_type&& body,
		          http_router::content_length prepare_payload =
		              content_length::unset) {
			if (keep_alive()) {
				set(boost::beast::http::field::connection, "keep-alive");
			}

			auto& header = static_cast<beast_response::header_type&>(*this);
			boost::beast::http::response<Body> msg{
			    std::piecewise_construct, std::make_tuple(std::move(body)),
			    std::make_tuple(std::move(header))};
			if (prepare_payload == content_length::unset) msg.prepare_payload();
			out_.send_response(req_, std::move(msg));
		}

		void stock_response(status st, std::string const& msg);
		void internal_error(std::string const& msg) {
			stock_response(status::internal_server_error, msg);
		}
		void bad_request(std::string const& msg) {
			stock_response(status::bad_request, msg);
		}

		void moved(status st, std::string const& location);

	private:
		http_router::sender& out_;
		request const& req_;
	};
}  // namespace http_router
