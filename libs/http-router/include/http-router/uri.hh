// Copyright (c) 2015 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace http_router::uri {
	std::pair<std::string_view, std::string_view> split(
	    std::string_view request_target);

	std::string decoded_path(std::string_view path);

	/**
	Helper class for parsing, updating and setting
	query component.
	*/
	struct query {
	public:
		/**
		Parses a query component
		\param query a query component to be parsed
		\result a parsed query with decoded name/value pairs
		*/
		static query parse(std::string_view query);

		/**
		Adds a new name/value pair.
		\param name a name of the field to add
		\param value a value of the field
		\result a builder reference to chain the calls together
		*/
		query& add(const std::string& name, const std::string& value);

		/**
		Sets a value-less param name.
		\param name a name of the field to set
		\result a builder reference to chain the calls together
		*/
		query& set(const std::string& name);

		/**
		Removes all fields with given name
		\param name a name of the field to remove
		\result a builder reference to chain the calls together
		*/
		query& remove(const std::string& name);

		/**
		Builds a resulting query string for URI or for form request.
		\result encoded string created from all fields in the builder
		*/
		std::string string() const;

		/**
		 Represents a pair on param list. The optional value will be set, if
		 corresponding param had value.
		*/
		using flat_param_type =
		    std::pair<std::string, std::optional<std::string>>;

		/**
		 Represents a flat list of all the parameters on a query string. The
		 order of the params will follow the order of add(), set() and
		 remove() calls, and params with the same name preserve the order of
		 values.
		*/
		using flat_list_type = std::vector<flat_param_type>;

		/**
		Creates a list of all fields for individual access.
		\result a vector of name/value pairs
		*/
		flat_list_type flat_list() const;

		/**
		 Represents a param with all values attached. The vector will not be
		 empty, if corresponding param had value.
		*/
		using multi_param_type =
		    std::pair<std::string, std::vector<std::string>>;

		/**
		 Represents a list of all the parameters with all values on a query
		 string. The order of the params will follow the order of add(),
		 set() and remove() calls, with order within sub-vectors also
		 preserved.
		*/
		using multi_list_type = std::vector<multi_param_type>;

		multi_list_type const& multi_list() const noexcept { return values_; }

		auto begin() const { return values_.begin(); }
		auto end() const { return values_.end(); }
		auto size() const { return values_.size(); }
		auto empty() const { return values_.empty(); }
		auto find(std::string const& name) const {
			auto it = keys_.find(name);
			if (it == keys_.end()) return end();
			return std::next(begin(), static_cast<std::ptrdiff_t>(it->second));
		}

	private:
#ifndef USING_DOXYGEN
		multi_list_type values_;
		std::unordered_map<std::string, size_t> keys_;
#endif
	};
};  // namespace http_router::uri
