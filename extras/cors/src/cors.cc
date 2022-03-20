// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/chrono.h>
#include <http-router/filters/cors.hh>
#include <http-router/request.hh>
#include <http-router/response.hh>

using namespace std::literals;

namespace http_router::filters {
	using namespace std::chrono;

	filtering cors::preproc(response& resp, std::string_view) {
		resp.set(boost::beast::http::field::access_control_allow_origin, "*");
		return filtering::carry_on;
	}
}  // namespace http_router::filters
