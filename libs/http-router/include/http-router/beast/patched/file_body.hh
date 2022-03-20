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
#include <boost/beast/core/file.hpp>
#include <boost/optional.hpp>
#include <cstdint>
#include <cstdio>
#include <http-router/beast/patched/basic_file_body.hh>
#include <utility>

namespace boost::beast::http::patched {
	using file_body = basic_file_body<file>;
}  // namespace boost::beast::http::patched

#ifndef BOOST_BEAST_NO_FILE_BODY_WIN32
#include <http-router/beast/patched/file_body_win32.hh>
#endif
