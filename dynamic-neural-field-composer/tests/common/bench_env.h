#pragma once

// Shared machine/environment capture for the manual perf tools (benchmark_main.cpp,
// profiler_main.cpp). All of them report a machine-dependent metric (steps/sec, us/step,
// ns/cell/step), so every appended session records the environment it ran in alongside the
// numbers -- otherwise runs from different machines silently land in the same table and
// look like regressions or wins.
//
// capture() collects the environment; fingerprint() reduces the parts that must match for
// two runs to be comparable at all into one short token, so a baseline recorded elsewhere
// can be REFUSED rather than silently compared against.

#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <thread>

#include <nlohmann/json.hpp>

#include "tools/simd_dispatch.h"

// cpuid (via <intrin.h>/<cpuid.h>) is x86/x64-only — e.g. Apple Silicon's
// arm64 has no such instruction, and <cpuid.h> itself #errors on non-x86.
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define BENCH_ENV_X86 1
#else
#define BENCH_ENV_X86 0
#endif

#if defined(_WIN32)
#include <intrin.h>
#elif BENCH_ENV_X86
#include <cpuid.h>
#include <sys/utsname.h>
#include <unistd.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
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
// Full compiler flag string for the configured build type. /arch:AVX2 vs not is a large
// swing, so without this two runs of the same commit on the same machine can differ by
// tens of percent with nothing in the record explaining it.
#ifndef BENCH_ENV_CXX_FLAGS
#define BENCH_ENV_CXX_FLAGS "unknown"
#endif
// 1 when the working tree had uncommitted changes at configure time. Without it the
// recorded git SHA is a lie whenever someone benchmarks a work in progress.
#ifndef BENCH_ENV_GIT_DIRTY
#define BENCH_ENV_GIT_DIRTY 0
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

	std::string cxx_flags;
	bool        git_dirty = false;
	std::string dnfc_version;
	std::string hostname;

	// Machine state at run time, reported by the hygiene wrapper scripts rather than
	// probed here. "unrecorded" (not "none") when the tool was run outside a wrapper —
	// an unwrapped run must be visibly unrecorded, never silently claimed clean.
	std::string affinity;
	std::string priority;
	std::string power_state;
};

namespace detail {

	inline std::string cpu_brand()
	{
#if defined(_WIN32) || BENCH_ENV_X86
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

	// Reads an environment variable, returning `fallback` when unset or empty. MSVC
	// deprecates std::getenv, so use its _dupenv_s replacement there rather than
	// suppressing the warning.
	inline std::string env_or(const char* name, const char* fallback)
	{
#if defined(_MSC_VER)
		char* value = nullptr;
		std::size_t length = 0;
		if (_dupenv_s(&value, &length, name) == 0 && value != nullptr)
		{
			std::string result(value);
			std::free(value);
			if (!result.empty())
				return result;
		}
		return fallback;
#else
		const char* value = std::getenv(name);
		if (value != nullptr && *value != '\0')
			return value;
		return fallback;
#endif
	}

	inline std::string host_name()
	{
#if defined(_WIN32)
		// COMPUTERNAME is always set on Windows, which avoids pulling <windows.h> and its
		// macro pollution into a header every perf tool includes.
		return env_or("COMPUTERNAME", "unknown host");
#else
		char buffer[256] = {};
		if (gethostname(buffer, sizeof(buffer) - 1) == 0 && buffer[0] != '\0')
			return buffer;
		return "unknown host";
#endif
	}

	inline std::string version_string()
	{
#if defined(DNF_COMPOSER_VERSION_MAJOR)
		return std::to_string(DNF_COMPOSER_VERSION_MAJOR) + "."
		     + std::to_string(DNF_COMPOSER_VERSION_MINOR) + "."
		     + std::to_string(DNF_COMPOSER_VERSION_PATCH);
#else
		return "unknown";
#endif
	}

	inline std::uint64_t fnv1a64(std::string_view text)
	{
		std::uint64_t hash = 1469598103934665603ULL;
		for (const char c : text)
		{
			hash ^= static_cast<unsigned char>(c);
			hash *= 1099511628211ULL;
		}
		return hash;
	}

	inline std::string to_hex16(std::uint64_t value)
	{
		static constexpr char digits[] = "0123456789abcdef";
		std::string out(16, '0');
		for (int i = 15; i >= 0; --i)
		{
			out[static_cast<std::size_t>(i)] = digits[value & 0xFu];
			value >>= 4;
		}
		return out;
	}

} // namespace detail

inline Env capture()
{
	Env e;
	e.cpu          = detail::cpu_brand();
	e.logical      = std::thread::hardware_concurrency();
	e.os           = detail::os_name();
	e.compiler     = detail::compiler_name();
	e.build_type   = BENCH_ENV_BUILD_TYPE;
	e.avx2         = dnf_composer::tools::math::detail::avx2_fma_available();
	e.fftw         = BENCH_ENV_FFTW_VERSION;
	e.git          = BENCH_ENV_GIT_SHA;

	e.cxx_flags    = BENCH_ENV_CXX_FLAGS;
	e.git_dirty    = (BENCH_ENV_GIT_DIRTY != 0);
	e.dnfc_version = detail::version_string();
	e.hostname     = detail::host_name();

	e.affinity     = detail::env_or("DNFC_BENCH_AFFINITY", "unrecorded");
	e.priority     = detail::env_or("DNFC_BENCH_PRIORITY", "unrecorded");
	e.power_state  = detail::env_or("DNFC_BENCH_POWER",    "unrecorded");
	return e;
}

// Short token over exactly the properties that must match for two runs to be comparable.
//
// Deliberately EXCLUDES the git SHA (it changes every commit, and comparing across commits
// is the entire point) and the machine-state trio (it changes per run — a baseline must
// survive being re-run under the hygiene wrapper). Deliberately INCLUDES cxx_flags: the
// same source built with and without /arch:AVX2 is not the same thing to measure.
inline std::string fingerprint(const Env& e)
{
	const std::string material = e.cpu + '\x1f' + e.os + '\x1f' + e.compiler + '\x1f'
	                           + e.build_type + '\x1f' + e.cxx_flags + '\x1f'
	                           + (e.avx2 ? "avx2" : "noavx2");
	return detail::to_hex16(detail::fnv1a64(material));
}

inline nlohmann::json to_json(const Env& e)
{
	return nlohmann::json{
		{"cpu",          e.cpu},
		{"logical",      e.logical},
		{"os",           e.os},
		{"compiler",     e.compiler},
		{"build_type",   e.build_type},
		{"avx2",         e.avx2},
		{"fftw",         e.fftw},
		{"git",          e.git},
		{"git_dirty",    e.git_dirty},
		{"cxx_flags",    e.cxx_flags},
		{"dnfc_version", e.dnfc_version},
		{"hostname",     e.hostname},
		{"affinity",     e.affinity},
		{"priority",     e.priority},
		{"power_state",  e.power_state},
	};
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
