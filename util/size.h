/* Copyright © 2014 cat <cat@wolfgirl.org>
 * This program is free software. It comes without any warranty, to the extent
 * permitted by applicable law. You can redistribute it and/or modify it under
 * the terms of the Do What The Fuck You Want To Public License, Version 2, as
 * published by Sam Hocevar. See http://www.wtfpl.net/ for more details.
 */

#ifndef UTIL_SIZE_H
#define UTIL_SIZE_H

/**
 * \file size.h
 * \brief Various size and math utility functions
 */

/**
 * \namespace util::size
 * \brief Size utilities
 */

#include <cstddef>
#include <QString>
#include <QApplication>

namespace util {
namespace size {

	/// Pre-C++17 alternative to std::size() 
	template<class T, std::size_t N>
	constexpr std::size_t array_size(T (&)[N]) noexcept { return N; }

	/// Compute whole number of kilobytes (base 2 aka KiB) in \p bytes
	inline std::size_t to_kib(std::size_t bytes)
	{
		return bytes / 1024;
	}

	/// Compute number of megabytes (base 2 aka MiB) in \p bytes
	inline double to_mib(std::size_t bytes)
	{
		return static_cast<double>(bytes) / 1048576.0;
	}

	/// Human-readable file size string for \p bytes.
	inline QString printable(std::size_t bytes)
	{
		return bytes < 1024*1024 // if less than 1 Mb show exact KB size
			? qApp->translate("PrintableFileSize", "%1 KiB").arg(to_kib(bytes))
			: qApp->translate("PrintableFileSize", "%1 MiB").arg(to_mib(bytes), 0,'f', 3);
	}

	/// Compute percentage of \p value between \p max and \p min.
	inline std::int64_t percent(std::int64_t value, std::int64_t max, std::int64_t min = 0ll)
	{
		if(value == 0 || max == 0 || max == min) return 0;
		return static_cast<int64_t>(static_cast<double>(value-min) * 100.0 / (max - min));
	}
}
}

#endif
