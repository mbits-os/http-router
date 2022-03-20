// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>

namespace http_router::filters {
	struct root_directory {
		std::filesystem::path root;
		std::filesystem::path path_for(
		    std::filesystem::path const& filename) const {
			return (root / filename).make_preferred();
		}

		std::string name() const {
			auto const u8 = root.generic_u8string();
			return {reinterpret_cast<char const*>(u8.data()), u8.size()};
		}
	};
}  // namespace http_router::filters
