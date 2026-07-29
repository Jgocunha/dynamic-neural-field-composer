#pragma once

// Shared machine/environment capture for the manual perf tools (benchmark_main.cpp,
// profiler_main.cpp). Both tools report a machine-dependent metric (steps/sec, us/step),
// so every appended session records the environment it ran in alongside the numbers --
// otherwise runs from different machines silently land in the same table and look like
// regressions or wins.

#include <string>
#include <thread>

#include "tools/simd_dispatch.h"

#if defined(_WIN32)
#include <intrin.h>
#else
#include <cpuid.h>
#include <sys/utsname.h>
#endif

#ifndef BENCH_ENV_BUILD_TYPE
#define BENCH_ENV_BUILD_TYPE "unknown"
#endif
#ifndef BENCH_ENV_FFTW_VERSION
#define BENCH_ENV_FFTW_VERSION "unknown"
#endif
#ifndef BENCH_ENV_GIT_SHA
#define BENCH_ENV_GIT_SHA "nogit"
#endif

namespace bench_env {

struct Env
{
	std::string cpu;
	unsigned    logical = 0;
	std::string os;
	std::string compiler;
	std::string build_type;
	bool        avx2 = false;
	std::string fftw;
	std::string git;
};

namespace detail {

	inline std::string cpu_brand()
	{
#if defined(_WIN32) || defined(__GNUC__) || defined(__clang__)
		int regs[4] = {0, 0, 0, 0};
		char brand[49] = {};
#if defined(_WIN32)
		__cpuid(regs, 0x80000000);
		if (static_cast<unsigned>(regs[0]) < 0x80000004u)
			return "unknown CPU";
		__cpuid(reinterpret_cast<int*>(brand +  0), 0x80000002);
		__cpuid(reinterpret_cast<int*>(brand + 16), 0x80000003);
		__cpuid(reinterpret_cast<int*>(brand + 32), 0x80000004);
#else
		unsigned maxExt = 0;
		__get_cpuid(0x80000000, &maxExt, reinterpret_cast<unsigned*>(&regs[1]),
		            reinterpret_cast<unsigned*>(&regs[2]), reinterpret_cast<unsigned*>(&regs[3]));
		if (maxExt < 0x80000004u)
			return "unknown CPU";
		__get_cpuid(0x80000002, reinterpret_cast<unsigned*>(brand +  0),
		            reinterpret_cast<unsigned*>(brand +  4),
		            reinterpret_cast<unsigned*>(brand +  8),
		            reinterpret_cast<unsigned*>(brand + 12));
		__get_cpuid(0x80000003, reinterpret_cast<unsigned*>(brand + 16),
		            reinterpret_cast<unsigned*>(brand + 20),
		            reinterpret_cast<unsigned*>(brand + 24),
		            reinterpret_cast<unsigned*>(brand + 28));
		__get_cpuid(0x80000004, reinterpret_cast<unsigned*>(brand + 32),
		            reinterpret_cast<unsigned*>(brand + 36),
		            reinterpret_cast<unsigned*>(brand + 40),
		            reinterpret_cast<unsigned*>(brand + 44));
#endif
		std::string s(brand);
		// Brand strings are padded with leading/trailing spaces; trim both.
		const auto first = s.find_first_not_of(' ');
		const auto last  = s.find_last_not_of(' ');
		if (first == std::string::npos)
			return "unknown CPU";
		return s.substr(first, last - first + 1);
#else
		return "unknown CPU";
#endif
	}

	inline std::string os_name()
	{
#if defined(_WIN32)
		return "Windows";
#elif defined(__APPLE__)
		return "macOS";
#elif defined(__linux__)
		utsname u{};
		if (uname(&u) == 0)
			return std::string("Linux ") + u.release;
		return "Linux";
#else
		return "unknown OS";
#endif
	}

	inline std::string compiler_name()
	{
#if defined(_MSC_VER)
		return "MSVC " + std::to_string(_MSC_VER / 100) + "." + std::to_string(_MSC_VER % 100);
#elif defined(__clang__)
		return "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__);
#elif defined(__GNUC__)
		return "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#else
		return "unknown compiler";
#endif
	}

} // namespace detail

inline Env capture()
{
	Env e;
	e.cpu        = detail::cpu_brand();
	e.logical    = std::thread::hardware_concurrency();
	e.os         = detail::os_name();
	e.compiler   = detail::compiler_name();
	e.build_type = BENCH_ENV_BUILD_TYPE;
	e.avx2       = dnf_composer::tools::math::detail::avx2_fma_available();
	e.fftw       = BENCH_ENV_FFTW_VERSION;
	e.git        = BENCH_ENV_GIT_SHA;
	return e;
}

inline std::string to_markdown(const Env& e)
{
	std::string s = "**Env:** " + e.cpu;
	if (e.logical > 0)
		s += " (" + std::to_string(e.logical) + "T)";
	s += " | " + e.os
	   + " | " + e.compiler
	   + " | " + e.build_type
	   + " | AVX2: " + (e.avx2 ? "yes" : "no")
	   + " | FFTW " + e.fftw
	   + " | git " + e.git;
	return s;
}

} // namespace bench_env
