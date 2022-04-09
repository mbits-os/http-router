// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4619 4242)
#endif
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <fmt/format.h>

namespace http_router::server {
	namespace beast = boost::beast;
	namespace http = beast::http;
	namespace websocket = beast::websocket;
	namespace net = boost::asio;
	using tcp = boost::asio::ip::tcp;

#ifdef _WIN32
	std::string ui_to_utf8(std::string_view msg);
#else
	inline std::string_view ui_to_utf8(std::string_view native) {
		return native;
	}
#endif

	inline void fail(beast::error_code const& ec, char const* what) {
		fmt::print(stderr, "http-router: {}: {}\n", what,
		           ui_to_utf8(ec.message()));
	}
}  // namespace http_router::server
