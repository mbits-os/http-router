// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <http-router/shared_roots.hh>

namespace http_router {
	namespace fs = std::filesystem;

	fs::path shared_roots::expand(fs::path const& filename) const {
		fs::file_time_type best_mtime{fs::file_time_type::max()};
		fs::path best_candidate{};
		bool found{false};
		for (auto const& root : roots_) {
			auto candidate = root / filename;
			if (verbose_) fmt::print("TRY {}\n", candidate.generic_string());

			std::error_code ec{};
			auto mtime = fs::last_write_time(candidate, ec);
			if (ec || mtime >= best_mtime) continue;
			best_mtime = mtime;
			best_candidate = std::move(candidate);
			found = true;
		}

		if (!found) best_candidate = roots_.front() / filename;
		return best_candidate;
	}
}  // namespace http_router
