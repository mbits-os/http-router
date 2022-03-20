// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4619 4242)
#endif
#include <boost/beast/http/fields.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <string_view>

namespace http_router {
	class request;
	class response;
}  // namespace http_router

namespace http_router::filters {
	enum class filtering { carry_on = true, finished = false };
	enum class supports {
		none = 0,
		pre = 1,
		send = 2,
		post = 4,
	};

	inline supports operator|(supports lhs, supports rhs) {
		using Int = std::underlying_type_t<supports>;
		return static_cast<supports>(static_cast<Int>(lhs) |
		                             static_cast<Int>(rhs));
	}

	inline supports operator&(supports lhs, supports rhs) {
		using Int = std::underlying_type_t<supports>;
		return static_cast<supports>(static_cast<Int>(lhs) &
		                             static_cast<Int>(rhs));
	}

	inline supports operator~(supports val) {
		using Int = std::underlying_type_t<supports>;
		return static_cast<supports>(~static_cast<Int>(val));
	}

	struct base {
		std::string_view name;
		filters::supports type;

		template <filters::supports bit>
		bool supports() const noexcept {
			return (type & bit) == bit;
		}
		bool supports_pre() const noexcept {
			return supports<filters::supports::pre>();
		}
		bool supports_send() const noexcept {
			return supports<filters::supports::send>();
		}
		bool supports_post() const noexcept {
			return supports<filters::supports::post>();
		}

		using header =
		    boost::beast::http::header<false, boost::beast::http::fields>;

		explicit base(std::string_view name, filters::supports type)
		    : name{name}, type{type} {}
		virtual ~base() = default;
		virtual filtering preproc([[maybe_unused]] response& resp,
		                          [[maybe_unused]] std::string_view prefix) {
			return filtering::carry_on;
		}

		virtual void sending([[maybe_unused]] request const& req,
		                     [[maybe_unused]] header& resp,
		                     [[maybe_unused]] std::string_view prefix) {}

		virtual void postproc([[maybe_unused]] request const& req,
		                      [[maybe_unused]] header& resp,
		                      [[maybe_unused]] size_t transmitted,
		                      [[maybe_unused]] std::string_view prefix) {}
	};
}  // namespace http_router::filters
