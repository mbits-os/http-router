// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <http-router/filters/base.hh>
#include <http-router/route.hh>
#include <unordered_map>

namespace http_router {
	namespace http = boost::beast::http;

	class router {
	public:
		using filter = filters::base;
		using route_list =
		    std::unordered_map<http::verb, std::vector<std::unique_ptr<route>>>;
		using filter_list =
		    std::vector<std::pair<std::string, std::unique_ptr<filter>>>;

		route_list routes;
		filter_list filters;

		void print() const;
		void handle_request(response& resp) const;
		void before_send(request const& req, filter::header& resp) const;
		void after_send(request const& req,
		                filter::header& resp,
		                size_t transmitted) const;

		class cfg {
		public:
			cfg& add(std::string const& path,
			         route::delegate const& cb,
			         http::verb verb = http::verb::get,
			         COMPILE options = COMPILE::DEFAULT);
			cfg& get(std::string const& path,
			         route::delegate const& cb,
			         COMPILE options = COMPILE::DEFAULT) {
				return add(path, cb, http::verb::get, options);
			}
			cfg& getish(std::string const& path,
			            route::delegate const& cb,
			            COMPILE options = COMPILE::DEFAULT) {
				return add(path, cb, http::verb::get, options)
				    .add(path, cb, http::verb::head, options);
			}

			cfg& append(std::string const& path, std::unique_ptr<cfg>&& sub);

			template <typename Class, typename... Args>
			cfg& filter(Args&&... args) {
				return use(
				    std::make_unique<Class>(std::forward<Args>(args)...));
			}

			cfg& use(std::string const& path,
			         std::unique_ptr<router::filter>&& filt);
			cfg& use(std::unique_ptr<router::filter>&& filt) {
				return use({}, std::move(filt));
			}

			router compiled();

		private:
			struct sub {
				std::string mask{};
				std::unique_ptr<cfg> next{};
			};

			struct handler {
				std::string mask{};
				route::delegate callback{};
				COMPILE options{};

				std::unique_ptr<route> compile() && {
					return std::make_unique<route>(mask, std::move(callback),
					                               options);
				}
			};

			using handlers =
			    std::unordered_map<http::verb, std::vector<handler>>;
			void surrender(const std::string& prefix,
			               handlers& handlers,
			               filter_list& filters);

			handlers handlers_{};
			filter_list filters_{};
			std::vector<sub> routers_{};
		};
	};
}  // namespace http_router
