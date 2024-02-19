#pragma once

#include <istream>
#include <iterator>
#include <tuple>

namespace csv_reader {

// Default case for validation check
template<typename, typename = void>
struct is_callable: std::false_type {};

// Parser is valid because has `operator()`.
template<typename T>
struct is_callable<
	T,
	std::enable_if_t<std::is_member_function_pointer_v<decltype(&T::operator())>>
>: std::true_type {};

// Alias for parser validation
template<typename T>
inline constexpr bool is_callable_v = is_callable<T>::value;

template<template<typename> typename P, typename... Ts>
class Deserializer {
	private:
		static const char _DELIMITER = ',';

		std::tuple<Ts..., std::vector<std::string>> _result;

	public:
		Deserializer() = default;
		~Deserializer() = default;

		template<size_t I = 0>
		std::enable_if_t<I >= sizeof...(Ts), decltype(_result)&>
		deserialize(std::istream& s) {
			// Read token
			std::string token;
			if (!std::getline(s, token, this->_DELIMITER)) {
				return this->_result;
			}
			// Push new token to vector
			std::get<sizeof...(Ts)>(this->_result).push_back(token);
			return this->deserialize<I>(s);
		}

		template<size_t I = 0>
		std::enable_if_t<I < sizeof...(Ts), decltype(_result)&>
		deserialize(std::istream& s) {
			// Read token
			std::string token;
			if (!std::getline(s, token, this->_DELIMITER)) {
				throw std::runtime_error("There is no " + std::to_string(I) + " token.");
			}
			// Get type of element with index `I` and validate parser for this type
			using T = std::tuple_element_t<I, decltype(this->_result)>;
			static_assert(is_callable_v<P<T>>, "Parser do not handle operator().");
			// Create parser for type `T` and parse to result tuple
			P<T> parser;
			std::get<I>(this->_result) = parser(token);
			return this->deserialize<I + 1>(s);
		}
};

// Iterator of CSV rows.
template<template<typename> typename P, typename... Ts>
class Iterator {
	public:
		/* Iterator traits */
		using difference_type = void;
		using iterator_category = std::input_iterator_tag;
		using value_type = std::tuple<Ts..., std::vector<std::string>>;
		using pointer = value_type*;
		using reference = value_type&;

		Iterator() = default;
		Iterator(const Iterator& other) noexcept: _stream(other._stream) {}
		Iterator(Iterator&& other) noexcept: _stream(other._stream) {}
		~Iterator() = default;

		Iterator(std::istream& stream, size_t skip_lines): _stream(&stream) {
			++*this;
			while (skip_lines-- > 0) {
				++*this;
			}
		}

		typename std::iterator_traits<Iterator>::reference operator*() {
			std::istringstream ss(this->_line);
			return this->_deserializer.deserialize(ss);
		}

		Iterator& operator=(const Iterator& other) noexcept {
			Iterator temp(other);
			swap(*this, temp);
			return *this;
		}

		Iterator& operator=(Iterator&& other) noexcept {
			Iterator temp(std::move(other));
			swap(*this, temp);
			return *this;
		}

		bool operator==(const Iterator& other) const noexcept {
			return this->_equal(other);
		}

		bool operator!=(const Iterator& other) const noexcept {
			return !this->_equal(other);
		}

		Iterator& operator++() {
			if (this->_stream && !std::getline(*this->_stream, this->_line)) {
				this->_stream = nullptr;
			}
			return *this;
		}

		Iterator operator++(int) {
			Iterator old = *this;
			operator++();
			return old;
		}

		friend void swap(Iterator& x, Iterator& y) noexcept {
			using std::swap;
			swap(x->_stream, y->_stream);
		}

	private:
		std::istream* _stream;
		std::string _line;
		Deserializer<P, Ts...> _deserializer;

		bool _equal(const Iterator& other) const noexcept {
			// Does not matter if lines readed or not
			return this->_stream == other._stream;
		}
};

template<template<typename> typename P, typename... Ts>
class Reader {
	public:
		Reader(std::istream& stream, size_t skip_lines) noexcept:
			_stream(stream), _skip_lines(skip_lines) {}
		~Reader() = default;

		Iterator<P, Ts...> begin() const noexcept {
			return Iterator<P, Ts...>(this->_stream, this->_skip_lines);
		}

		Iterator<P, Ts...> end() const noexcept {
			return Iterator<P, Ts...>();
		}

	private:
		std::istream& _stream;
		size_t _skip_lines;

};

}
