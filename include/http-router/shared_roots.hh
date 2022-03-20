// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <vector>

namespace http_router {
	class shared_roots {
	public:
		explicit shared_roots(std::vector<std::filesystem::path>&& roots,
		                      bool verbose)
		    : roots_{std::move(roots)}, verbose_{verbose} {}

		friend std::filesystem::path operator/(
		    shared_roots const& root,
		    std::filesystem::path const& filename) {
			return root.expand(filename);
		}

		std::vector<std::filesystem::path> const& roots() const noexcept {
			return roots_;
		}

		std::filesystem::path expand(
		    std::filesystem::path const& filename) const;

	private:
		std::vector<std::filesystem::path> roots_{};
		bool verbose_{};
	};
}  // namespace http_router
