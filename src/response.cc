// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4619 4242)
#endif
#include <boost/beast/version.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <fmt/format.h>
#include <http-router/response.hh>

namespace http_router {
	namespace http = boost::beast::http;
	void response::stock_response(status st, std::string const& msg) {
		result(st);
		set(http::field::content_type, "text/html; charset=UTF-8");
		send<string_body>(fmt::format(R"(<html>
  <head>
    <title>{0} {1}</title>
    <link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Montserrat:400,400i,700">
    <style>
      body, h1 {{ font-family: Montserrat, sans-serif; }}
      body {{ font-weight: 400 }}
      h1 {{ font-weight: 700 }}
      code {{
        font-family: monospace, monospace;
        padding: .1em;
        color: #800;
        background: #fee;
        border-radius: 4px;
      }}
    </style>
  </head>
  <body>
    <h1>{0} {1}</h1>
    <p>{2}</p>
  </body>
</html>)",
		                              static_cast<unsigned>(st),
		                              http::obsolete_reason(st), msg));
	}

	void response::moved(status st, std::string const& location) {
		set(http::field::location, location);
		stock_response(
		    st, fmt::format(R"(Moved <a href="{}">here</a>.)", location));
	}
}  // namespace http_router
