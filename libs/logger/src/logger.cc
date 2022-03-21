// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <http-router/filters/logger.hh>
#include <http-router/request.hh>

namespace http_router::filters {
	void logger::postproc(request const& req,
	                      header& resp,
	                      size_t transmitted,
	                      std::string_view) {
		auto const& remote = req.remote();
		auto const ip = remote.address().to_string();
		auto const method = req.method_string();
		auto const resource = req.target_full();
		auto const status = resp.result_int();
		fmt::print("{} {} \"{}\" {} - {}\n", ip, method, resource, status,
		           transmitted);
	}
}  // namespace http_router::filters