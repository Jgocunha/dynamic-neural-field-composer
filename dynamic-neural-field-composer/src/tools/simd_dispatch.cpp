// Compiled at the toolchain's DEFAULT baseline (no AVX2/FMA flag) — this is the
// file every other translation unit calls into to ask "is it safe to call the
// AVX2 kernel", so it must not itself require AVX2 to run.

#include "tools/simd_dispatch.h"

// AVX2 is an x86/x64-only ISA extension. __builtin_cpu_supports("avx2") and
// <intrin.h>'s cpuid intrinsics are unavailable/meaningless on other
// architectures (e.g. Apple Silicon's arm64) — gate the whole detection on
// the target architecture, not just the compiler, so this TU still compiles
// (and correctly reports "unavailable") on ARM hosts.
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
	#define DNF_COMPOSER_X86 1
#else
	#define DNF_COMPOSER_X86 0
#endif

#if DNF_COMPOSER_X86 && defined(_MSC_VER)
	#include <intrin.h>
#endif

namespace dnf_composer::tools::math::detail
{
	bool avx2_fma_available()
	{
		static const bool result = []() -> bool
		{
#if DNF_COMPOSER_X86 && defined(_MSC_VER)
			int regs[4] = { 0, 0, 0, 0 };
			__cpuid(regs, 0);
			const int maxLeaf = regs[0];
			if (maxLeaf < 7) return false;

			__cpuid(regs, 1);
			const bool osUsesXsave = (regs[2] & (1 << 27)) != 0; // ECX.OSXSAVE
			const bool cpuHasAvx   = (regs[2] & (1 << 28)) != 0; // ECX.AVX
			const bool cpuHasFma   = (regs[2] & (1 << 12)) != 0; // ECX.FMA
			if (!osUsesXsave || !cpuHasAvx || !cpuHasFma) return false;

			const unsigned long long xcr0 = _xgetbv(0);
			const bool osSavesYmm = (xcr0 & 0x6) == 0x6; // XCR0[2:1] = SSE + AVX state
			if (!osSavesYmm) return false;

			__cpuidex(regs, 7, 0);
			const bool cpuHasAvx2 = (regs[1] & (1 << 5)) != 0; // EBX.AVX2
			return cpuHasAvx2;
#elif DNF_COMPOSER_X86 && (defined(__GNUC__) || defined(__clang__))
			__builtin_cpu_init();
			return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
#else
			return false;
#endif
		}();
		return result;
	}
}
