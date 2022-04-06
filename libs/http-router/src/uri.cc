// Copyright (c) 2015 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <http-router/uri.hh>

namespace http_router::uri {
	namespace {
		size_t find_query(std::string_view request_target) {
			auto const size = request_target.size();
			auto data = request_target.data();

			auto query = 0;
			while (query < size) {
				switch (*data) {
					case '?':
					case '#':
						return query;
				}
				++query;
				++data;
			}

			return query;
		}

		inline bool issafe(unsigned char c) {
			return std::isalnum(c) || c == '-' || c == '.' || c == '_' ||
			       c == '~';
		}

		inline bool query_issafe(unsigned char c) {
			return issafe(c) || c == ' ';
		}

		inline bool path_issafe(unsigned char c) {
			return issafe(c) || c == '+';
		}

		inline char write_ch(char c) { return c; }
		inline char query_write_ch(char c) { return c == ' ' ? '+' : c; }

		template <typename SafePred, typename AppendPred>
		inline std::string urlencode_impl(const char* in,
		                                  size_t in_len,
		                                  SafePred&& safe,
		                                  AppendPred&& write_ch) {
			size_t length = in_len;

			auto b = in;
			auto e = b + in_len;

			for (auto it = b; it != e; ++it) {
				if (!safe(static_cast<unsigned char>(*it))) length += 2;
			}

			static constexpr char hexes[] = "0123456789ABCDEF";
			std::string out;
			out.reserve(length);

			for (auto it = b; it != e; ++it) {
				auto c = *it;
				if (safe(static_cast<unsigned char>(c))) {
					out += write_ch(c);
					continue;
				}
				out += '%';
				out += hexes[(c >> 4) & 0xF];
				out += hexes[(c)&0xF];
			}
			return out;
		}

		inline char read_ch(char c) { return c; }
		inline char query_read_ch(char c) { return c == '+' ? ' ' : c; }

		inline int hex(char c) {
			switch (c) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					return static_cast<unsigned char>(c - '0');
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					return c - 'a' + 10;
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					return c - 'A' + 10;
			}
			// Line excluded, as this function is ONLY called inside isxdigit
			// check
			return 0;  // GCOV_EXCL_LINE
		}

		template <typename AppendPred>
		std::string urldecode_impl(const char* in,
		                           size_t in_len,
		                           AppendPred&& read_ch) {
			std::string out;
			out.reserve(in_len);

			for (size_t i = 0; i < in_len; ++i) {
				// go inside only, if there is enough space
				if (in[i] == '%' && (i < in_len - 2) &&
				    std::isxdigit(in[i + 1]) && std::isxdigit(in[i + 2])) {
					auto c = (hex(in[i + 1]) << 4) | hex(in[i + 2]);
					out.push_back(static_cast<char>(c));
					i += 2;
					continue;
				}
				out += read_ch(in[i]);
			}
			return out;
		}

		inline std::string path_urlencode(std::string_view in) {
			return urlencode_impl(in.data(), in.size(), path_issafe, write_ch);
		}

		inline std::string query_urlencode(std::string_view in) {
			return urlencode_impl(in.data(), in.size(), query_issafe,
			                      query_write_ch);
		}

		inline std::string urldecode(const char* in, size_t in_len) {
			return urldecode_impl(in, in_len, read_ch);
		}

		inline std::string query_urldecode(const char* in, size_t in_len) {
			return urldecode_impl(in, in_len, query_read_ch);
		}
	}  // namespace

	std::pair<std::string_view, std::string_view> split(
	    std::string_view request_target) {
		std::pair<std::string_view, std::string_view> result{};

		result.first = request_target.substr(0, find_query(request_target));
		request_target = request_target.substr(result.first.length());

		if (!request_target.empty() && request_target.front() == '?')
			result.second =
			    request_target.substr(1, request_target.find('#', 1));

		return result;
	}

	std::string decoded_path(std::string_view path) {
		return urldecode(path.data(), path.size());
	}

	std::string query::string() const {
		std::string out;
		bool first = true;
		auto add_prefix = [&] {
			if (first) {
				first = false;
			} else
				out.push_back('&');
		};
		for (auto& pair : values_) {
			if (pair.second.empty()) {
				add_prefix();
				out += query_urlencode(pair.first);
				continue;
			}

			auto name = query_urlencode(pair.first) + "=";
			for (auto& value : pair.second) {
				add_prefix();

				out += name + query_urlencode(value);
			}
		}
		return out;
	}

	query::flat_list_type query::flat_list() const {
		size_t length = 0;
		for (auto& pair : values_) {
			if (pair.second.empty())
				++length;
			else
				length += pair.second.size();
		}

		query::flat_list_type out;
		out.reserve(length);

		for (auto& pair : values_) {
			if (pair.second.empty()) out.emplace_back(pair.first, std::nullopt);
			for (auto& value : pair.second)
				out.emplace_back(pair.first, value);
		}

		return out;
	}

	struct query_parser {
		const char* it;
		const char* end;

		query parse(std::string_view str) {
			query out{};

			if (!str.empty() && str.front() == '?') str = str.substr(1);
			it = str.data();
			end = it + str.length();
			while (it < end) {
				const char* name_start = it;
				look_for('=', '&');
				std::string name = query_urldecode(
				    name_start, static_cast<size_t>(it - name_start));

				if (is('=')) {
					++it;
					const char* value_start = it;
					look_for('&');
					out.add(name, query_urldecode(
					                  value_start,
					                  static_cast<size_t>(it - value_start)));
				} else
					out.set(name);

				if (!is('&')) break;

				++it;
			}
			return out;
		}

		bool valid() const noexcept { return it < end; }
		bool is(char ch) const noexcept { return valid() && *it == ch; }

		template <std::same_as<char>... Char>
		void look_for(Char... ch) noexcept {
			while ((valid() && ... && (ch != *it)))
				++it;
		}
	};

	query query::parse(std::string_view str) {
		return query_parser{}.parse(str);
	}

	query& query::add(const std::string& name, const std::string& value) {
		auto it = keys_.find(name);
		if (it != keys_.end()) {
			values_[it->second].second.push_back(value);
			return *this;
		}

		auto const index = values_.size();

		values_.push_back({name, {value}});
		keys_[name] = index;
		return *this;
	}

	query& query::set(const std::string& name) {
		auto it = keys_.find(name);
		if (it != keys_.end()) {
			values_[it->second].second.clear();
			return *this;
		}

		auto const index = values_.size();

		values_.push_back({name, {}});
		keys_[name] = index;
		return *this;
	}

	query& query::remove(const std::string& name) {
		auto it = keys_.find(name);
		if (it == keys_.end()) return *this;

		auto const index = it->second;
		values_.erase(
		    std::next(values_.begin(), static_cast<std::ptrdiff_t>(index)));

		keys_.erase(name);
		for (auto& key : keys_) {
			if (key.second > index) --key.second;
		}
		return *this;
	}
}  // namespace http_router::uri