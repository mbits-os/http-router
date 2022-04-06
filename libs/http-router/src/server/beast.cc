// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <server/beast.hh>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace http_router::server {
#ifdef _WIN32
	std::string ui_to_utf8(std::string_view msg) {
		int wide_size =
		    MultiByteToWideChar(CP_ACP, 0, msg.data(),
		                        static_cast<DWORD>(msg.length()), nullptr, 0);
		if (wide_size < 1) return {msg.data(), msg.size()};

		auto wide_buffer =
		    std::make_unique<wchar_t[]>(static_cast<size_t>(wide_size) + 1);
		int result = MultiByteToWideChar(CP_ACP, 0, msg.data(),
		                                 static_cast<DWORD>(msg.length()),
		                                 wide_buffer.get(), wide_size);
		if (result < 1) return {msg.data(), msg.size()};
		wide_buffer[wide_size] = 0;

		int narrow_size = WideCharToMultiByte(CP_UTF8, 0, wide_buffer.get(), -1,
		                                      nullptr, 0, nullptr, nullptr);
		if (narrow_size < 1) return {msg.data(), msg.size()};

		auto narrow_buffer =
		    std::make_unique<char[]>(static_cast<size_t>(narrow_size) + 1);
		result = WideCharToMultiByte(CP_UTF8, 0, wide_buffer.get(), -1,
		                             narrow_buffer.get(), narrow_size, nullptr,
		                             nullptr);
		if (result < 1) return {msg.data(), msg.size()};
		narrow_buffer[narrow_size] = 0;

		return narrow_buffer.get();
	}
#endif
}  // namespace http_router::server
