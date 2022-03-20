// Copyright (c) 2016 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <filesystem>
#include <http-router/filters/base.hh>
#include <http-router/filters/static_files/root_directories.hh>
#include <http-router/filters/static_files/root_directory.hh>

namespace http_router::filters {
	struct static_files_name {
		std::string constructed;
		explicit static_files_name(std::string_view dir)
		    : constructed{fmt::format("[{}]", dir)} {}
	};

	class static_files_base : private static_files_name, public base {
	public:
		explicit static_files_base(std::string_view dir)
		    : static_files_name{dir}
		    , base{static_files_name::constructed, supports::pre} {}
		static filtering get_file(response& resp, std::filesystem::path const&);
		static std::string resource_from(response const& req,
		                                 std::string_view prefix);
	};

	template <typename Mapper>
	class static_files : public static_files_base, public Mapper {
	public:
		using base = static_files_base;

		static_files(Mapper const& mapper)
		    : static_files_base{mapper.name()}, Mapper{mapper} {}

		filtering preproc(response& resp, std::string_view prefix) override {
			return base::get_file(
			    resp, Mapper::path_for(base::resource_from(resp, prefix)));
		}

		template <typename... Arguments>
		static std::unique_ptr<base> make(Arguments&&... args) {
			return std::make_unique<static_files<Mapper>>(
			    Mapper{std::forward<Arguments>(args)...});
		}
	};
}  // namespace http_router::filters