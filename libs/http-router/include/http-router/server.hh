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

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace http_router {
	class router;
}
namespace http_router::server {
	struct local_endpoint {
		std::string interface{};
		unsigned short port{};
	};

	local_endpoint listen(
	    boost::asio::io_context& ioc,
	    boost::asio::ip::tcp::endpoint const& endpoint,
	    router const* handler,
	    std::string const& server_name = {};
}  // namespace http_router::server
