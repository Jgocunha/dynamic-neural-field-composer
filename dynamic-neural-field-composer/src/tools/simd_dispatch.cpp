// Compiled at the toolchain's DEFAULT baseline (no AVX2/FMA flag) — this is the
// file every other translation unit calls into to ask "is it safe to call the
// AVX2 kernel", so it must not itself require AVX2 to run.

#include "tools/simd_dispatch.h"

#if defined(_MSC_VER)
	#include <intrin.h>
#endif

namespace dnf_composer::tools::math::detail
{
	bool avx2_fma_available()
	{
		static const bool result = []() -> bool
		{
#if defined(_MSC_VER)
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
#elif defined(__GNUC__) || defined(__clang__)
			__builtin_cpu_init();
			return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
#else
			return false;
#endif
		}();
		return result;
	}
}
