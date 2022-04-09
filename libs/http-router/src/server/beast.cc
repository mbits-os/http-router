// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <server/beast.hh>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace http_router::server {
#ifdef _WIN32
	template <typename Char, size_t Length>
	struct char_buff {
		std::array<Char, Length> stack{};
		std::unique_ptr<Char[]> heap{};
		size_t used{};

		char_buff() = default;

		explicit char_buff(size_t size) : used{size} {
			if (size >= Length) heap = std::make_unique<Char[]>(size + 1);
			this->data()[size] = 0;
		}

		size_t size() const noexcept { return used; }
		Char* data() noexcept { return heap ? heap.get() : stack.data(); }
		Char const* data() const noexcept {
			return heap ? heap.get() : stack.data();
		}
		Char& operator[](size_t index) noexcept { return data()[index]; }
		std::basic_string_view<Char> view() const noexcept {
			return {data(), size()};
		}
	};

#define CHAR_FWD(NAME)       \
	template <typename Char> \
	struct NAME

	CHAR_FWD(opposite_t);
	CHAR_FWD(win32_conv);

#define OPPO(X, Y)         \
	template <>            \
	struct opposite_t<X> { \
		using type = Y;    \
	}

#define CONV(CHAR, FUNC, CP, ...)                                         \
	template <>                                                           \
	struct win32_conv<CHAR> {                                             \
		static size_t calc(std::basic_string_view<CHAR> input,            \
		                   opposite<CHAR>* output,                        \
		                   size_t size) noexcept {                        \
			auto const isize = static_cast<int>(size);                    \
			auto const result =                                           \
			    FUNC(CP, 0, input.data(), static_cast<int>(input.size()), \
			         output, __VA_ARGS__);                                \
			if (result < 1) return 0;                                     \
			return static_cast<size_t>(result);                           \
		}                                                                 \
	}

	template <typename Char>
	using opposite = typename opposite_t<Char>::type;

	OPPO(char, wchar_t);
	OPPO(wchar_t, char);

	CONV(char, MultiByteToWideChar, CP_ACP, isize);
	CONV(wchar_t, WideCharToMultiByte, CP_UTF8, isize, nullptr, nullptr);

	template <typename Char, size_t Length>
	bool conv(std::basic_string_view<Char> input,
	          char_buff<opposite<Char>, Length>& result) {
		using buff_type = char_buff<opposite<Char>, Length>;

		auto size = win32_conv<Char>::calc(input, nullptr, 0);
		if (size) {
			result = buff_type{size};
			size = win32_conv<Char>::calc(input, result.data(), size);
		}

		return size != 0;
	}

	std::string ui_to_utf8(std::string_view msg) {
		static constexpr auto small_size = 200;

		char_buff<wchar_t, small_size> wide_buffer{};
		if (!conv(msg, wide_buffer)) return {msg.data(), msg.size()};

		char_buff<char, small_size> narrow_buffer{};

		if (!conv(wide_buffer.view(), narrow_buffer))
			return {msg.data(), msg.size()};
		return {narrow_buffer.data(), narrow_buffer.size()};
	}
#endif
}  // namespace http_router::server
