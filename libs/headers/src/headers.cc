// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <http-router/filters/headers.hh>
#include <http-router/request.hh>

namespace http_router::filters {
	void headers::sending(request const& req, header& resp, std::string_view) {
		fmt::print("< {} {} HTTP/{}.{}\n", req.method_string(), req.target(),
		           req.version() / 10, req.version() % 10);
		for (auto const& field : req) {
			fmt::print("< {}: {}\n", field.name_string(), field.value());
		}

		if (std::distance(req.begin(), req.end()) > 0) fmt::print("\n");

		fmt::print("> HTTP/{}.{} {} {}\n", resp.version() / 10,
		           resp.version() % 10, resp.result_int(), resp.reason());
		for (auto const& field : resp) {
			fmt::print("> {}: {}\n", field.name_string(), field.value());
		}

		if (std::distance(resp.begin(), resp.end()) > 0) fmt::print("\n");
	}
}  // namespace http_router::filters