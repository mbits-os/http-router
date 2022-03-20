// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <http-router/filters/base.hh>

namespace http_router::filters {
	class logger : public base {
	public:
		logger() : base{"logger", supports::post} {}
		void postproc(request const& req,
		              header& resp,
		              size_t transmitted,
		              std::string_view prefix) override;
	};
}  // namespace http_router::filters
