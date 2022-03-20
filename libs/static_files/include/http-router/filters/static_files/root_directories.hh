// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <http-router/shared_roots.hh>

using namespace std::literals;

namespace http_router::filters {
	struct root_directories {
		shared_roots root;
		std::filesystem::path path_for(
		    std::filesystem::path const& filename) const {
			return (root / filename).make_preferred();
		}

		std::string_view name() const { return "<shared roots>"sv; }
	};

	struct named_root_directories : root_directories {
		std::string stored_name;
		std::string_view name() const { return stored_name; }
	};
}  // namespace http_router::filters