// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <http-router/router.hh>

using namespace std::literals;

namespace http_router {
	namespace {
		bool prefixes(std::string_view prefix, std::string_view resource) {
			if (prefix.empty() && resource.starts_with('/')) return true;

			if (resource.length() < prefix.length()) return false;
			if (!resource.starts_with(prefix)) return false;
			if (resource.length() == prefix.length()) return true;

			auto const resource_continues_with_a_slash =
			    resource[prefix.length()] == '/';
			auto const prefix_ends_with_a_slash = prefix.back() == '/';

			return resource_continues_with_a_slash || prefix_ends_with_a_slash;
		}

		bool has_up_dir(std::string_view resource) {
			return resource.ends_with("/.."sv) ||
			       resource.find("/../"sv) != std::string_view::npos;
		}
	}  // namespace

	void router::print() const {
		for (auto& [prefix, filter] : filters) {
			using filters::supports;
			fmt::print(stderr, "[FILTER] '{}/' -> {}", prefix, filter->name);
			if (filter->type == supports::none) {
				fmt::print("NONE");
			} else {
#define IF(FLAG, NAME)                \
	if ((flags & FLAG) == FLAG) {     \
		if (!first) fmt::print(" |"); \
		first = false;                \
		fmt::print(" " NAME);         \
		flags = flags & ~FLAG;        \
	}
				auto flags = filter->type;
				auto first = true;
				IF(supports::pre, "PRE-PROC");
				IF(supports::send, "PRE-SEND");
				IF(supports::post, "POST-PROC");
				if (flags != supports::none) {
					if (!first) fmt::print(" |");
					fmt::print(
					    " {}",
					    static_cast<std::underlying_type_t<supports>>(flags));
				}
#undef IF
			}
			fmt::print("\n");
		}

		std::vector<std::pair<std::string, std::string>> list;
		auto add_route = [&](std::string const& path, auto const& method) {
			auto it = std::find_if(
			    list.begin(), list.end(),
			    [&](auto const& pair) { return pair.first == path; });
			if (it == list.end())
				list.emplace_back(path, method);
			else {
				it->second.push_back('|');
				it->second.append(method);
			}
		};

		for (auto& [verb, handlers] : routes) {
			std::string_view method = to_string(verb);
			for (auto& handler : handlers)
				add_route(handler->mask(), method);
		}

		for (auto const& [path, methods] : list)
			fmt::print(stderr, "[ROUTE] {} '{}'\n", methods, path);
	}

	void router::handle_request(response& resp) const {
		auto const resource = resp.req().decoded_path();
		if (resource.empty() || resource[0] != '/' || has_up_dir(resource))
			return resp.bad_request("Illegal resource path");

		for (auto& [prefix, filter] : filters) {
			if (!filter->supports_pre()) continue;
			if (!prefixes(prefix, resource)) continue;
			if (filter->preproc(resp, prefix) == filters::filtering::finished)
				return;
		}

		if (!routes.empty()) {
			auto it = routes.find(resp.req().method());
			if (it == routes.end())
				return resp.bad_request("Unknown HTTP-method");

			matcher::params_type params;
			for (auto& route : it->second) {
				if (!route->matcher().matches(std::string{resource}, params))
					continue;
				return route->call(resp, params);
			}
		}

		return resp.stock_response(
		    http::status::not_found,
		    fmt::format("The resource <code>'{}'</code> was not found.",
		                resource));
	}

	void router::before_send(request const& req, filter::header& resp) const {
		auto const resource = req.decoded_path();
		for (auto& [prefix, filter] : filters) {
			if (!filter->supports_send()) continue;
			if (!prefixes(prefix, resource)) continue;
			filter->sending(req, resp, prefix);
		}
	}

	void router::after_send(request const& req,
	                        filter::header& resp,
	                        size_t transmitted) const {
		auto const resource = req.decoded_path();
		for (auto& [prefix, filter] : filters) {
			if (!filter->supports_post()) continue;
			if (!prefixes(prefix, resource)) continue;
			filter->postproc(req, resp, transmitted, prefix);
		}
	}

	router::cfg& router::cfg::add(std::string const& path,
	                              route::delegate const& cb,
	                              http::verb verb,
	                              COMPILE options) {
		handlers_[verb].push_back({path, cb, options});
		return *this;
	}

	router::cfg& router::cfg::append(std::string const& path,
	                                 std::unique_ptr<cfg>&& sub) {
		routers_.push_back({path, std::move(sub)});
		return *this;
	}

	router::cfg& router::cfg::use(std::string const& path,
	                              std::unique_ptr<router::filter>&& filt) {
		filters_.push_back({path.empty() || path.back() != '/'
		                        ? path
		                        : path.substr(0, path.length() - 1),
		                    std::move(filt)});
		return *this;
	}

	void router::cfg::surrender(std::string const& prefix,
	                            handlers& handlers,
	                            filter_list& filters) {
		for (auto& [key, list] : handlers_) {
			auto& dst = handlers[key];
			for (auto& handler : list) {
				handler.mask = prefix + handler.mask;
				dst.push_back(std::move(handler));
			}
		}
		handlers_.clear();

		filters.reserve(filters.size() + filters_.size());
		for (auto& [path, filter] : filters_) {
			filters.emplace_back(prefix + path, std::move(filter));
		}
		filters_.clear();

		for (auto& subrouter : routers_) {
			subrouter.next->surrender(subrouter.mask, handlers, filters);
		}
		routers_.clear();
	}

	router router::cfg::compiled() {
		router result{};

		for (auto& subrouter : routers_) {
			subrouter.next->surrender(subrouter.mask, handlers_, filters_);
		}
		routers_.clear();

		for (auto& [verb, handlers] : handlers_) {
			auto& dst = result.routes[verb];
			dst.reserve(handlers.size());
			for (auto& handler : handlers)
				dst.push_back(std::move(handler).compile());
		}
		handlers_.clear();

		result.filters = std::move(filters_);
		filters_.clear();

		return result;
	}
}  // namespace http_router
