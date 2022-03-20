// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

// https://github.com/pillarjs/path-to-regexp/blob/master/index.js

#include <map>
#include <regex>
#include <string>
#include <variant>

#ifdef OPTIONAL
#undef OPTIONAL
#endif

#ifdef STRICT
#undef STRICT
#endif

namespace http_router {
	enum class KEY {
		NONE = 0x0000,
		IS_STRING = 0x0001,
		ASTERISK = 0x0004,
		OPTIONAL = 0x0008,
		REPEAT = 0x0010,
		PARTIAL = 0x0020,
	};

	inline KEY operator|(KEY lhs, KEY rhs) {
		return static_cast<KEY>(static_cast<std::underlying_type_t<KEY>>(lhs) |
		                        static_cast<std::underlying_type_t<KEY>>(rhs));
	}

	inline KEY operator&(KEY lhs, KEY rhs) {
		return static_cast<KEY>(static_cast<std::underlying_type_t<KEY>>(lhs) &
		                        static_cast<std::underlying_type_t<KEY>>(rhs));
	}

	enum COMPILE {
		STRICT = 0x0001,
		END = 0x0002,
		SENSITIVE = 0x0004,
		OPTIMIZE = 0x0008,
		TAKE_REST = 0x0010,
		DEFAULT = /*STRICT | */ END | SENSITIVE | OPTIMIZE
	};

	inline COMPILE operator|(COMPILE lhs, COMPILE rhs) {
		return static_cast<COMPILE>(
		    static_cast<std::underlying_type_t<COMPILE>>(lhs) |
		    static_cast<std::underlying_type_t<COMPILE>>(rhs));
	}

	inline COMPILE operator&(COMPILE lhs, COMPILE rhs) {
		return static_cast<COMPILE>(
		    static_cast<std::underlying_type_t<COMPILE>>(lhs) &
		    static_cast<std::underlying_type_t<COMPILE>>(rhs));
	}

	struct key {
		KEY flags{};
		std::variant<size_t, std::string> value{};
		std::string prefix{};
		std::string delimiter{};
		std::string pattern{};

		static key as_string(std::string&& value) {
			return {KEY::IS_STRING, std::move(value)};
		}

		bool has_flag(KEY flag) const noexcept {
			return (flags & flag) == flag;
		}

		key set_flags(bool asterisk,
		              bool optional,
		              bool repeat,
		              bool partial) noexcept {
			if (asterisk) flags = flags | KEY::ASTERISK;
			if (optional) flags = flags | KEY::OPTIONAL;
			if (repeat) flags = flags | KEY::REPEAT;
			if (partial) flags = flags | KEY::PARTIAL;
			return *this;
		}
		static key make(std::string&& value,
		                std::string&& prefix,
		                std::string&& delimiter,
		                std::string&& pattern) {
			return {KEY::NONE, std::move(value), std::move(prefix),
			        std::move(delimiter), std::move(pattern)};
		}
		static key make(size_t value,
		                std::string&& prefix,
		                std::string&& delimiter,
		                std::string&& pattern) {
			return {KEY::NONE, value, std::move(prefix), std::move(delimiter),
			        std::move(pattern)};
		}
	};

	struct description {
		std::string route;
		std::vector<key> keys;

		static description make(std::vector<key> const& tokens,
		                        COMPILE options = COMPILE::DEFAULT);
		static description make(const std::string& mask,
		                        COMPILE options = COMPILE::DEFAULT);
	};

	struct matcher {
		std::regex regex;
		std::vector<key> const keys;

		using params_type =
		    std::map<std::variant<size_t, std::string>, std::string>;

		static matcher make(description const& tokens,
		                    COMPILE options = COMPILE::DEFAULT);
		static matcher make(description&& tokens,
		                    COMPILE options = COMPILE::DEFAULT);
		static matcher make(const std::string& mask,
		                    COMPILE options = COMPILE::DEFAULT);

		bool matches(const std::string& route, params_type& params) const;
	};
}  // namespace http_router
