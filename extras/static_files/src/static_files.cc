// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "static_files_types.hh"

#include <fmt/chrono.h>
#include <http-router/filters/static_files.hh>
#include <http-router/range.hh>
#include <http-router/request.hh>
#include <http-router/response.hh>

using namespace std::literals;

namespace http_router::filters {
	using namespace std::chrono;

	using namespace boost;
	using namespace boost::beast;

	namespace {
		boost::string_view as_boost(std::string_view view) {
			return {view.data(), view.size()};
		}

		std::string format_date(system_clock::time_point date) {
			auto const tm = fmt::gmtime(system_clock::to_time_t(date));

			static constexpr std::string_view wday[] = {
			    "Sun"sv, "Mon"sv, "Tue"sv, "Wed"sv, "Thu"sv, "Fri"sv, "Sat"sv,
			};
			static constexpr std::string_view month[] = {
			    "Jan"sv, "Feb"sv, "Mar"sv, "Apr"sv, "May"sv, "Jun"sv,
			    "Jul"sv, "Aug"sv, "Sep"sv, "Oct"sv, "Nov"sv, "Dec"sv,
			};

			return fmt::format("{}, {:02} {} {} {:02}:{:02}:{:02} GMT"sv,
			                   wday[tm.tm_wday], tm.tm_mday, month[tm.tm_mon],
			                   1900 + tm.tm_year, tm.tm_hour, tm.tm_min,
			                   tm.tm_sec);
		}

		bool modified_since(system_clock::time_point last_modified,
		                    request const& req) {
			if (last_modified == system_clock::time_point::min()) return false;
			auto it = req.find(http::field::if_modified_since);
			if (it == req.end()) return false;

			struct tm tm {};

			auto modified_date_str = std::string{it->value()};
			std::istringstream input{modified_date_str};
			input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
			input >> std::get_time(&tm, "%a, %d %b %Y %T GMT");
			auto const modified_date =
			    input.fail() ? system_clock::time_point::min()
			                 : system_clock::from_time_t(std::mktime(&tm));

			auto const cast = [](system_clock::time_point tp) {
				return system_clock::time_point{
				    duration_cast<seconds>(tp.time_since_epoch())};
			};

			return cast(last_modified) > cast(modified_date);
		}
	}  // namespace

	namespace fs = std::filesystem;
	filtering static_files_base::get_file(response& resp,
	                                      std::filesystem::path const& path) {
		auto const& req = resp.req();

		auto const not_found = [](beast::string_view target) {
			return fmt::format("The resource <code>'{}'</code> was not found.",
			                   target);
		};

		std::error_code ec1;
		auto const status = fs::status(path, ec1);
		if (!ec1 && fs::is_directory(status)) {
			return resp.moved(http::status::moved_permanently,
			                  std::string{req.target()} + "/"),
			       filtering::finished;
		}

		if (req.method() != http::verb::get &&
		    req.method() != http::verb::head) {
			return resp.bad_request("Unknown HTTP-method"), filtering::finished;
		}

		// Attempt to open the file
		beast::error_code ec;
		file_body::value_type body;
		body.open(path.string().c_str(), beast::file_mode::scan, ec);
		if (ec) return filtering::carry_on;

		// Cache the size since we need it after the move
		auto size = body.size();

		std::vector<range_type> ranges{};
		auto const range_status = parse_byte_ranges(size, req, ranges);

		auto const sys_date = [&] {
			std::error_code ec;
			auto const time = fs::last_write_time(path, ec);
			if (ec) return system_clock::time_point::min();
			return clock_cast<system_clock>(time);
		}();

		resp.set(http::field::content_type, as_boost(mime_type(path)));
		resp.set(http::field::accept_ranges, "bytes");
		resp.set(http::field::date, format_date(system_clock::now()));
		if (sys_date > system_clock::time_point::min())
			resp.set(http::field::last_modified, format_date(sys_date));
		if (range_status != range_field::absent) {
			if (range_status == range_field::invalid) {
				resp.result(http::status::range_not_satisfiable);
				resp.set(http::field::content_range,
				         fmt::format("bytes */{}", size));
				return resp.send<http::empty_body>({}, content_length::set),
				       filtering::finished;
			} else {
				resp.result(http::status::partial_content);
				resp.set(http::field::content_range,
				         fmt::format("bytes {}-{}/{}", ranges.front().start,
				                     ranges.front().end, size));
				size = ranges.front().end - ranges.front().start + 1;
				body.fragment(ranges.front().start, ranges.front().end + 1);
			}
		}
		resp.content_length(size);

		if (req.method() == http::verb::head)
			return resp.send<http::empty_body>({}, content_length::set),
			       filtering::finished;

		// Respond to GET request
		if (range_status != range_field::present &&
		    modified_since(sys_date, req)) {
			resp.result(http::status::not_modified);
			return resp.send<http::empty_body>({}, content_length::set),
			       filtering::finished;
		}

		return resp.send<file_body>(std::move(body), content_length::set),
		       filtering::finished;
	}

	std::string static_files_base::resource_from(response const& resp,
	                                             std::string_view prefix) {
		auto path =
		    std::string{resp.req().target().substr(prefix.length() + 1)};
		if (resp.req().target().back() == '/') path += "index.html";
		return path;
	}
}  // namespace http_router::filters