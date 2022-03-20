// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <http-router/filters/base.hh>

namespace http_router::filters {
	class headers : public base {
	public:
		headers() : base{"headers", supports::send} {}
		void sending(request const& req,
		             header& resp,
		             std::string_view prefix) override;
	};
}  // namespace http_router::filters
