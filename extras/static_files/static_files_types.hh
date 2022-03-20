// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <array>
#include <filesystem>
#include <string_view>

using namespace std::literals;

namespace http_router::filters {
	std::string_view mime_type(std::filesystem::path const& path);
}  // namespace router::filters
