// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "static_files_types.hh"

namespace http_router::filters {
	inline constexpr uint32_t ext_tmplt(char const* e) {
		auto shl = 24;
		uint32_t result = 0;
		for (size_t index = 0; index < 4; ++index) {
			auto const c = e[index];
			auto const ch = c == 0 ? 0 : c < 'a' ? c + ('a' - 'A') : c;
			result |= static_cast<uint32_t>(ch) << shl;
			shl -= 8;
		}

		return result;
	}

	inline consteval uint32_t ext_u32(char const (&e)[4]) {
		return ext_tmplt(e);
	}

	inline consteval uint32_t ext_u32(char const (&e)[5]) {
		return ext_tmplt(e);
	}

	namespace {
		inline consteval auto create_types() {
			using P = std::pair<uint32_t, std::string_view>;
			std::array data = {
#include "static_files_types.inl"
			};
			std::sort(std::begin(data), std::end(data));
			return data;
		}

	}  // namespace
	std::string_view mime_type(std::filesystem::path const& path) {
		static constinit auto const mimes = create_types();

		auto const extension = [&path] {
			auto const ext = path.extension();
			if (ext.empty()) return 0u;
			auto const ext_str = ext.string();
			auto const view = std::string_view{ext_str}.substr(1);
			return ext_tmplt(view.data());
		}();

		for (auto [ext, mime] : mimes) {
			if (ext == extension) return mime;
		}

		return "application/octet-stream"sv;
	}
}  // namespace http_router::filters
