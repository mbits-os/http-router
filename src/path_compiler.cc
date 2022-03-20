// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <http-router/path_compiler.hh>

using namespace std::literals;

namespace std {
	inline std::sregex_iterator begin(std::sregex_iterator const& it) {
		return it;
	}
	inline std::sregex_iterator end(std::sregex_iterator const&) { return {}; }
}  // namespace std

namespace http_router {
	namespace {
		template <bool (*checker)(char)>
		inline std::string escape(const std::string& s) {
			size_t length = s.length();
			for (auto& c : s) {
				if (checker(c)) ++length;
			}

			std::string out;
			out.reserve(length + 1);
			for (auto& c : s) {
				if (checker(c)) out.push_back('\\');
				out.push_back(c);
			}

			return out;
		}

		inline bool is_string(char c) {
			// ".+*?=^!:${}()[]|/\"
			switch (c) {
				case '.':
				case '+':
				case '*':
				case '?':
				case '=':
				case '^':
				case '!':
				case '$':
				case '{':
				case '}':
				case '(':
				case ')':
				case '[':
				case ']':
				case ':':
				case '|':
				case '/':
				case '\\':
					return true;
				default:
					break;
			};
			return false;
		}

		inline std::string escape_string(const std::string& s) {
			return escape<is_string>(s);
		}

		inline bool is_group(char c) {
			// "=!:$/()"
			switch (c) {
				case '=':
				case '!':
				case ':':
				case '$':
				case '/':
				case '(':
				case ')':
					return true;
				default:
					break;
			};
			return false;
		}

		inline static std::string escape_group(const std::string& s) {
			return escape<is_group>(s);
		}

		inline static std::vector<key> parse_matcher(const std::string& mask) {
#if 1
			static const std::regex path_regex{
			    // Match escaped characters that would otherwise appear in
			    // future matches. This allows the user to escape special
			    // characters that won't transform.
			    R"((\\.)|)"
			    // Match Express-style parameters and un-named parameters with a
			    // prefix and optional suffixes. Matches appear as:
			    //
			    // "/:test(\\d+)?" =>
			    //   ["/", "test", "\d+", undefined, "?", undefined]
			    // "/route(\\d+)"  =>
			    //   [undefined, undefined, undefined, "\d+", undefined,
			    //   undefined]
			    // "/*"            =>
			    //   ["/", undefined, undefined, undefined, undefined, "*"]
			    R"(([\/.])?(?:(?:\:(\w+)(?:\(((?:\\.|[^\\()])+)\))?|\(((?:\\.|[^\\()])+)\))([+*?])?|(\*)))"};
#else
			static const std::regex path_regex{
			    R"(((\\.)|(([\/.])?(?:(?:\:(\w+)(?:\(((?:\\.|[^\\()])+)\))?|\(((?:\\.|[^\\()])+)\))([+*?])?|(\*)))))"};
#endif

			std::vector<key> out;

			std::string path;
			size_t key = 0;
			size_t index = 0;

			for (auto const& res :
			     std::sregex_iterator{mask.begin(), mask.end(), path_regex}) {
				size_t const offset = res.position();
				path += mask.substr(index, offset - index);
				index = offset + res[0].length();

				// Ignore already escaped sequences.
				std::string const escaped = res[1];
				if (!escaped.empty()) {
					path += escaped[1];
					continue;
				}

				// std::string const m = res[0];

				auto const next = index < mask.length()
				                      ? std::string{1, mask[index]}
				                      : std::string{};
				auto prefix = std::string{res[2]};
				auto name = std::string{res[3]};
				auto capture = std::string{res[4]};
				auto group = std::string{res[5]};
				auto const modifier = std::string{res[6]};
				auto const asterisk = std::string{res[7]} == "*";

				// Push the current path onto the tokens.
				if (!path.empty())
					out.push_back(key::as_string(std::move(path)));

				auto const partial =
				    !prefix.empty() && !next.empty() && next != prefix;
				auto const repeat = modifier == "+" || modifier == "*";
				auto const optional = modifier == "?" || modifier == "*";
				auto delimiter = prefix.empty() ? "/" : prefix;
				auto pattern = [&, asterisk] {
					// JS: capture || group
					auto pattern = std::move(capture);
					if (pattern.empty()) pattern = std::move(group);

					if (!pattern.empty()) return escape_group(pattern);
					if (asterisk) return ".*"s;
					return fmt::format("[^{}]+?", escape_string(delimiter));
				}();

				if (name.empty()) {
					out.push_back(
					    key::make(key++, std::move(prefix),
					              std::move(delimiter), std::move(pattern))
					        .set_flags(asterisk, optional, repeat, partial));
				} else {
					out.push_back(
					    key::make(std::move(name), std::move(prefix),
					              std::move(delimiter), std::move(pattern))
					        .set_flags(asterisk, optional, repeat, partial));
				}
			}

			// Match any characters still remaining.
			if (index < mask.length()) path += mask.substr(index);

			if (!path.empty()) out.push_back(key::as_string(std::move(path)));

			return out;
		}

	}  // namespace
	description description::make(std::vector<key> const& tokens,
	                              COMPILE options) {
		auto strict = (options & COMPILE::STRICT) == COMPILE::STRICT;
		auto end = (options & COMPILE::END) == COMPILE::END;
		auto take_rest = (options & COMPILE::TAKE_REST) == COMPILE::TAKE_REST;

		std::string route;
		std::vector<key> keys;

		auto ends_with_slash =
		    !tokens.empty() && tokens.back().has_flag(KEY::IS_STRING) &&
		    std::holds_alternative<std::string>(tokens.back().value) &&
		    (std::get<std::string>(tokens.back().value).back() == '/');

		auto it = tokens.begin();
		auto end_it = tokens.end();

		auto rest = [&]() -> key const* {
			if (tokens.empty()) return nullptr;
			auto& last = tokens.back();
			if (last.has_flag(KEY::IS_STRING)) return nullptr;
			return &last;
		}();

		if (!take_rest || ends_with_slash) rest = nullptr;
		if (rest) --end_it;

		for (; it != end_it; ++it) {
			auto& token = *it;
			if (token.has_flag(KEY::IS_STRING)) {
				route += escape_string(std::get<std::string>(token.value));
				continue;
			}

			auto prefix = escape_string(token.prefix);
			auto capture = fmt::format("(?:{})", token.pattern);

			if (token.has_flag(KEY::REPEAT))
				capture += fmt::format("(?:{}{})*", prefix, capture);

			if (token.has_flag(KEY::OPTIONAL)) {
				if (token.has_flag(KEY::PARTIAL))
					capture = fmt::format("{}({})?", prefix, capture);
				else
					capture = fmt::format("(?:{}({}))?", prefix, capture);
			} else {
				capture = fmt::format("{}({})", prefix, capture);
			}

			route += capture;
			keys.push_back(token);
		}

		if (rest) {
			route += escape_string(rest->prefix) + "(.*)";
			keys.push_back(*rest);
		} else {
			// In non-strict mode we allow a slash at the end of match. If the
			// path to match already ends with a slash, we remove it for
			// consistency. The slash is valid at the end of a path match, not
			// in the middle. This is important in non-ending mode, where
			// "/test/" shouldn't match "/test//route".
			if (!strict) {
				if (ends_with_slash) {
					route.pop_back();  // '\'
					route.pop_back();  // '/'
				}
				route += R"((?:\/(?=$))?)";
			}

			if (end) {
				route.push_back('$');
			} else if (!strict || !ends_with_slash) {
				// In non-ending mode, we need the capturing groups to match as
				// much as possible by using a positive lookahead to the end or
				// next path segment.
				route += "(?=\\/|$)";
			}
		}

		route = "^" + route;
		return {std::move(route), std::move(keys)};
	};

	description description::make(const std::string& mask, COMPILE options) {
		return make(parse_matcher(mask), options);
	}

	matcher matcher::make(description const& tokens, COMPILE options) {
		auto flags = std::regex_constants::ECMAScript;
		if ((options & COMPILE::SENSITIVE) != COMPILE::SENSITIVE)
			flags |= std::regex_constants::icase;
		if ((options & COMPILE::OPTIMIZE) == COMPILE::OPTIMIZE)
			flags |= std::regex_constants::optimize;

		return {std::regex{tokens.route, flags}, tokens.keys};
	}

	matcher matcher::make(description&& tokens, COMPILE options) {
		auto flags = std::regex_constants::ECMAScript;
		if ((options & COMPILE::SENSITIVE) != COMPILE::SENSITIVE)
			flags |= std::regex_constants::icase;
		if ((options & COMPILE::OPTIMIZE) == COMPILE::OPTIMIZE)
			flags |= std::regex_constants::optimize;

		return {std::regex{tokens.route, flags}, std::move(tokens.keys)};
	}

	matcher matcher::make(const std::string& mask, COMPILE options) {
		return make(description::make(mask, options), options);
	}

	bool matcher::matches(const std::string& route, params_type& params) const {
		std::smatch match;
		auto matched = std::regex_match(route, match, regex);
		if (!matched) return false;

		params.clear();

		size_t id = 0;
		for (auto& key : keys) {
			params[key.value] = match[++id];
		}

		return true;
	}
}  // namespace http_router
