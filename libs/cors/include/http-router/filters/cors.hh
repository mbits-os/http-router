// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <http-router/filters/base.hh>

namespace http_router::filters {
	class cors : public base {
	public:
		explicit cors() : base{"cors", supports::pre} {}
		filtering preproc(response& resp, std::string_view prefix) override;
	};
}  // namespace http_router::filters