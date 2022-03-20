//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#pragma once

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/optional.hpp>
#include <cstdint>
#include <cstdio>
#include <utility>

namespace boost::beast::http::patched {
	template <class File>
	struct basic_file_body {
		static_assert(is_file<File>::value, "File type requirements not met");
		using file_type = File;
		class reader;
		class writer;

		class value_type {
			friend class reader;
			friend class writer;
			friend struct basic_file_body;

			File file_;
			std::uint64_t size_ = 0;  // cached file size
			std::uint64_t first_;     // starting offset of the range
			std::uint64_t last_;      // ending offset of the range

		public:
			~value_type() = default;
			value_type() = default;
			value_type(value_type&& other) = default;
			value_type& operator=(value_type&& other) = default;
			File& file() { return file_; }
			bool is_open() const { return file_.is_open(); }
			std::uint64_t size() const { return size_; }
			void close();
			void open(char const* path, file_mode mode, error_code& ec);
			void fragment(std::uint64_t first, std::uint64_t last);
			void reset(File&& file, error_code& ec);
		};

		class writer {
			value_type& body_;      // The body we are reading from
			std::uint64_t pos_;     // The current position in the file
			std::uint64_t remain_;  // The number of unread bytes
			char buf_[4096];        // Small buffer for reading

		public:
			using const_buffers_type = net::const_buffer;
			template <bool isRequest, class Fields>
			writer(header<isRequest, Fields>& h, value_type& b);
			void init(error_code& ec);
			boost::optional<std::pair<const_buffers_type, bool>> get(
			    error_code& ec);
		};

		class reader {
			value_type& body_;  // The body we are writing to

		public:
			template <bool isRequest, class Fields>
			explicit reader(header<isRequest, Fields>& h, value_type& b);

			void init(boost::optional<std::uint64_t> const&, error_code& ec);

			template <class ConstBufferSequence>
			std::size_t put(ConstBufferSequence const& buffers, error_code& ec);

			void finish(error_code& ec);
		};

		static std::uint64_t size(value_type const& body);
	};

	template <class File>
	void basic_file_body<File>::value_type::close() {
		error_code ignored;
		file_.close(ignored);
	}

	template <class File>
	void basic_file_body<File>::value_type::open(char const* path,
	                                             file_mode mode,
	                                             error_code& ec) {
		// Open the file
		file_.open(path, mode, ec);
		if (ec) return;

		// Cache the size
		size_ = file_.size(ec);
		if (ec) {
			close();
			return;
		}
		first_ = 0;
		last_ = size_;
	}

	template <class File>
	void basic_file_body<File>::value_type::fragment(std::uint64_t first,
	                                                 std::uint64_t last) {
		first_ = first;
		last_ = last;
	}

	template <class File>
	void basic_file_body<File>::value_type::reset(File&& file, error_code& ec) {
		// First close the file if open
		if (file_.is_open()) {
			error_code ignored;
			file_.close(ignored);
		}

		// Take ownership of the new file
		file_ = std::move(file);

		if (file_.is_open()) {
			size_ = file_.size(ec);
			if (ec) {
				close();
				return;
			}
			first_ = 0;
			last_ = size_;
		}
	}

	// This is called from message::payload_size
	template <class File>
	std::uint64_t basic_file_body<File>::size(value_type const& body) {
		// Forward the call to the body
		return body.size();
	}

	// ==================================================================================

	template <class File>
	template <bool isRequest, class Fields>
	basic_file_body<File>::writer::writer(header<isRequest, Fields>& h,
	                                      value_type& b)
	    : body_(b), pos_(body_.first_) {
		boost::ignore_unused(h);

		// The file must already be open
		BOOST_ASSERT(body_.file_.is_open());

		// Get the size of the file
		remain_ = body_.last_ - pos_;
	}

	// Initializer
	template <class File>
	void basic_file_body<File>::writer::init(error_code& ec) {
		ec = {};
	}

	template <class File>
	auto basic_file_body<File>::writer::get(error_code& ec)
	    -> boost::optional<std::pair<const_buffers_type, bool>> {
		auto const amount = remain_ > sizeof(buf_)
		                        ? sizeof(buf_)
		                        : static_cast<std::size_t>(remain_);

		if (amount == 0) {
			ec = {};
			return boost::none;
		}

		// Now read the next buffer
		body_.file_.seek(pos_, ec);
		if (ec) return boost::none;

		auto const nread = body_.file_.read(buf_, amount, ec);
		if (ec) return boost::none;

		if (nread == 0) {
			ec = error::short_read;
			return boost::none;
		}

		// Make sure there is forward progress
		BOOST_ASSERT(nread != 0);
		BOOST_ASSERT(nread <= remain_);

		// Update the amount remaining based on what we got
		pos_ += nread;
		remain_ -= nread;

		ec = {};
		return {{
		    const_buffers_type{buf_, nread},  // buffer to return.
		    remain_ > 0  // `true` if there are more buffers.
		}};
	}

	// ==================================================================================

	template <class File>
	template <bool isRequest, class Fields>
	basic_file_body<File>::reader::reader(header<isRequest, Fields>& h,
	                                      value_type& body)
	    : body_(body) {
		boost::ignore_unused(h);
	}

	template <class File>
	void basic_file_body<File>::reader::init(
	    boost::optional<std::uint64_t> const& content_length,
	    error_code& ec) {
		// The file must already be open for writing
		BOOST_ASSERT(body_.file_.is_open());

		// We don't do anything with this but a sophisticated
		// application might check available space on the device
		// to see if there is enough room to store the body.
		boost::ignore_unused(content_length);

		// The error_code specification requires that we
		// either set the error to some value, or set it
		// to indicate no error.
		//
		// We don't do anything fancy so set "no error"
		ec = {};
	}

	// This will get called one or more times with body buffers
	//
	template <class File>
	template <class ConstBufferSequence>
	std::size_t basic_file_body<File>::reader::put(
	    ConstBufferSequence const& buffers,
	    error_code& ec) {
		// This function must return the total number of
		// bytes transferred from the input buffers.
		std::size_t nwritten = 0;

		// Loop over all the buffers in the sequence,
		// and write each one to the file.
		for (auto it = net::buffer_sequence_begin(buffers);
		     it != net::buffer_sequence_end(buffers); ++it) {
			// Write this buffer to the file
			net::const_buffer buffer = *it;
			nwritten += body_.file_.write(buffer.data(), buffer.size(), ec);
			if (ec) return nwritten;
		}

		// Indicate success
		// This is required by the error_code specification
		ec = {};

		return nwritten;
	}

	// Called after writing is done when there's no error.
	template <class File>
	void basic_file_body<File>::reader::finish(error_code& ec) {
		// This has to be cleared before returning, to
		// indicate no error. The specification requires it.
		ec = {};
	}

	//]

#if !BOOST_BEAST_DOXYGEN
	// operator<< is not supported for file_body
	template <bool isRequest, class File, class Fields>
	std::ostream& operator<<(
	    std::ostream&,
	    message<isRequest, basic_file_body<File>, Fields> const&) = delete;
#endif

}  // namespace boost::beast::http::patched
