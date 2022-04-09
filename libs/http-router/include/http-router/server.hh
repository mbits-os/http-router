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

	template <typename Tag>
	class explicit_bool {
	public:
		explicit_bool() = default;
		explicit explicit_bool(bool value) noexcept : value_{value} {}
		explicit operator bool() const noexcept { return value_; }
		bool value() const noexcept { return value_; }

	private:
		bool value_{};
	};

	using reuse_address = explicit_bool<class reuse_address_tag>;

	local_endpoint listen(boost::asio::io_context& ioc,
	                      boost::asio::ip::tcp::endpoint const& endpoint,
	                      router const* handler,
	                      std::string const& server_name = {},
	                      reuse_address reuse_address_opt = reuse_address{
	                          true});
}  // namespace http_router::server
