// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <charconv>
#include <http-router/range.hh>

using namespace std::literals;

namespace http_router {
	namespace {
		bool parse(std::string_view data, std::uint64_t& result) {
			auto first_ptr = data.data();
			auto last_ptr = first_ptr + data.size();
			auto ret = std::from_chars(first_ptr, last_ptr, result);
			return ret.ec == std::errc{} && ret.ptr == last_ptr;
		}

		range_field parse_byte_range(std::uint64_t size,
		                             std::string_view data,
		                             range_type& result) {
			auto const pos = data.find('-');
			if (pos == std::string_view::npos) return range_field::invalid;

			auto const start = data.substr(0, pos);
			auto const end = data.substr(pos + 1);

			if (pos == 0) {
				if (!parse(end, result.end)) return range_field::invalid;
				result.start = size - result.end;
				result.end = size - 1;
			} else if (pos == data.length() - 1) {
				if (!parse(start, result.start)) return range_field::invalid;
				result.end = size - 1;
			} else {
				if (!parse(start, result.start)) return range_field::invalid;
				if (!parse(end, result.end)) return range_field::invalid;
			}

			if (result.end > size - 1) result.end = size - 1;
			return result.start <= result.end ? range_field::present
			                                  : range_field::invalid;
		}

		std::string_view strip(std::string_view s) {
			while (!s.empty() &&
			       std::isspace(static_cast<unsigned char>(s.back())))
				s = s.substr(0, s.length() - 1);
			while (!s.empty() &&
			       std::isspace(static_cast<unsigned char>(s.front())))
				s = s.substr(1);
			return s;
		}
	}  // namespace

	range_field parse_byte_ranges(std::uint64_t size,
	                              std::string_view header,
	                              std::vector<range_type>& result) {
		result.clear();

		auto pos = header.find('=');
		if (pos == std::string_view::npos) return range_field::invalid;
		if (strip(header.substr(0, pos)) != "bytes"sv)
			return range_field::invalid;

		header = strip(header.substr(pos + 1));
		pos = header.find(',');
		while (pos != std::string_view::npos) {
			range_type range{};
			auto const res =
			    parse_byte_range(size, strip(header.substr(0, pos)), range);
			if (res != range_field::invalid) result.push_back(range);

			header = strip(header.substr(pos + 1));
			pos = header.find(',');
		}
		range_type range{};
		auto const res =
		    parse_byte_range(size, strip(header.substr(0, pos)), range);
		if (res != range_field::invalid) result.push_back(range);

		return result.empty() ? range_field::invalid : range_field::present;
	}
	range_field parse_byte_ranges(std::uint64_t size,
	                              request const& req,
	                              std::vector<range_type>& result) {
		auto it = req.find(boost::beast::http::field::range);
		if (it == req.end()) return range_field::absent;
		return parse_byte_ranges(size, it->value(), result);
	}
}  // namespace http_router
