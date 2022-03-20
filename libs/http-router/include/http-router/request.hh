// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4619 4242)
#endif
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/string_body.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace http_router {
	class request {
	public:
		using tcp = boost::asio::ip::tcp;
		using beast_request =
		    boost::beast::http::request<boost::beast::http::string_body,
		                                boost::beast::http::fields>;
		request(beast_request& orig, tcp::endpoint const& remote) noexcept
		    : req_{orig}, remote_{remote} {}

		auto& req() const noexcept { return req_; }
		auto const& remote() const noexcept { return remote_; }
		auto method() const noexcept { return req_.method(); }
		auto method_string() const noexcept { return req_.method_string(); }
		auto target() const noexcept { return req_.target(); }
		auto version() const noexcept { return req_.version(); }
		auto keep_alive() const noexcept { return req_.keep_alive(); }
		auto find(auto key) const noexcept { return req_.find(key); }
		auto begin() const noexcept { return req_.begin(); }
		auto end() const noexcept { return req_.end(); }

	private:
		beast_request& req_;
		tcp::endpoint remote_;
	};
}  // namespace http_router
