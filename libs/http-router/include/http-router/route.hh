// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <http-router/delegate.hh>
#include <http-router/path_compiler.hh>
#include <http-router/request.hh>
#include <http-router/response.hh>

namespace http_router {
	class route {
	public:
		using delegate =
		    ::delegate<void(response&, matcher::params_type const&)>;
		route(std::string const& mask, delegate&& callback, COMPILE options)
		    : mask_{mask}
		    , matcher_{http_router::matcher::make(mask, options)}
		    , callback_{std::move(callback)} {}

		std::string const& mask() const noexcept { return mask_; }
		http_router::matcher const& matcher() const noexcept {
			return matcher_;
		}

		void call(response& resp, matcher::params_type const& params) {
			if (callback_)
				callback_(resp, params);
			else
				resp.stock_response(status::internal_server_error,
				                    "Routing misconfigured");
		}

	private:
		friend class router;
		void mask(const std::string& value) { mask_ = value; }

		std::string mask_;
		http_router::matcher matcher_;
		delegate callback_;
	};
}  // namespace http_router
