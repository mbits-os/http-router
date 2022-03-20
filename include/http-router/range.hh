// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <cstdint>
#include <http-router/request.hh>
#include <string_view>
#include <vector>

namespace http_router {
	struct range_type {
		std::uint64_t start{};
		std::uint64_t end{};
	};

	enum class range_field { present, absent, invalid };

	range_field parse_byte_ranges(std::uint64_t size,
	                              std::string_view header,
	                              std::vector<range_type>& result);
	range_field parse_byte_ranges(std::uint64_t size,
	                              request const& req,
	                              std::vector<range_type>& result);
}  // namespace http_router
