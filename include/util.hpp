#pragma once

#include <string>
#include <vector>


#define __LOOP_CONCAT_INTERNAL(a, b) a ## b
#define __LOOP_CONCAT(a, b) __LOOP_CONCAT_INTERNAL(a, b)
#define LOOP(n) for (size_t __LOOP_CONCAT(_loop_nvar_43bor, __LINE__) = 0; __LOOP_CONCAT(_loop_nvar_43bor, __LINE__) < n; ++__LOOP_CONCAT(_loop_nvar_43bor, __LINE__))


// pads a string with spaces at the front
template <typename T>
std::string padString(T val, size_t size) {
	std::string raw = std::to_string(val);

	while (raw.size() != size) {
		raw.insert(0, " ");
	}

	return raw;
}

template<typename T, typename U>
bool isIn(const T& item, const U& list) {
	for (const auto& i : list) if (i == item) return true;
	return false;
}

std::vector<std::string> splitString(const std::string& str, const std::vector<char>& delim) {
	std::vector<std::string> splits{};

	std::string built = "";
	for (size_t i = 0; i < str.size(); ++i) {
		if (isIn(str[i], delim)) {
			splits.push_back(built);
			built = "";
		} else {
			built += str[i];
		}
	}
	splits.push_back(built);

	return splits;
}


// inclusive
template <typename T>
constexpr void clamp(T& num, const T& lower, const T& upper) {
		 if (num < lower) num = lower;
	else if (num > upper) num = upper;
}

// inclusive
// rolls values over (min -> max, vice versa)
template <typename T>
constexpr void clamp_rollover(T& num, const T& lower, const T& upper) {
		 if (num < lower) num = upper;
	else if (num > upper) num = lower;
}


template <typename T>
constexpr T max(const T& num1, const T& num2) {
	return num1 > num2 ? num1 : num2;
}
template <typename T>
constexpr T min(const T& num1, const T& num2) {
	return num1 < num2 ? num1 : num2;
}