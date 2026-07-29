#include "tools/math.h"

#include <cstdint>



	
		namespace dnf_composer::tools::math
		{
			std::array<int, 2> computeKernelRange(double sigma, int cutOfFactor, int fieldSize, bool circular)
			{
				const int ceilSigmaCutOfFactor = static_cast<int>(std::ceil(sigma * cutOfFactor));
				const int fieldSizeMinus1 = (fieldSize - 1);

				if (circular)
				{
					const double halfFieldSizeMinus1 = (static_cast<double>(fieldSize) - 1) / 2;
					const int floorHalfFieldSizeMinus1 = static_cast<int>(std::floor(halfFieldSizeMinus1));
					const int ceilHalfFieldSizeMinus1 = static_cast<int>(std::ceil(halfFieldSizeMinus1));

					return std::min(std::array<int, 2>{ceilSigmaCutOfFactor, ceilSigmaCutOfFactor},
						std::array<int, 2>{floorHalfFieldSizeMinus1, ceilHalfFieldSizeMinus1});
				}

				return std::min(std::array<int, 2>{ceilSigmaCutOfFactor, ceilSigmaCutOfFactor},
					std::array<int, 2>{fieldSizeMinus1, fieldSizeMinus1});
			}

			std::vector<int> createExtendedIndex(int fieldSize, const std::array<int, 2>& kernelRange)
			{
				const int startingValue = fieldSize - kernelRange[1] + 1;
				const int initialVectorSize = fieldSize - startingValue + 1;

				std::vector<int> initialVector(initialVectorSize);
				std::iota(initialVector.begin(), initialVector.end(), startingValue);

				std::vector<int> secondVector(fieldSize);
				std::iota(secondVector.begin(), secondVector.end(), 1);

				std::vector<int> thirdVector(kernelRange[0]);
				std::iota(thirdVector.begin(), thirdVector.end(), 1);

				std::vector<int> extendedVector;
				extendedVector.reserve(initialVector.size() + secondVector.size() + thirdVector.size());

				extendedVector.insert(extendedVector.end(), initialVector.begin(), initialVector.end());
				extendedVector.insert(extendedVector.end(), secondVector.begin(), secondVector.end());
				extendedVector.insert(extendedVector.end(), thirdVector.begin(), thirdVector.end());

				return extendedVector;
			}

			namespace
			{
				// ---- xoshiro256+ PRNG (fast, high quality) ----------------------
				// Seeded once per thread from std::random_device (same lifetime model
				// as the previous std::mt19937). Not for cryptographic use.
				struct Xoshiro256
				{
					std::array<std::uint64_t, 4> s;

					static std::uint64_t rotl(std::uint64_t x, int k)
					{
						return (x << k) | (x >> (64 - k));
					}

					std::uint64_t next()
					{
						const std::uint64_t result = s[0] + s[3];
						const std::uint64_t t = s[1] << 17;
						s[2] ^= s[0];
						s[3] ^= s[1];
						s[1] ^= s[2];
						s[0] ^= s[3];
						s[2] ^= t;
						s[3] = rotl(s[3], 45);
						return result;
					}

					// uniform double in [0, 1) from the top 53 bits.
					double nextDouble()
					{
						return static_cast<double>(next() >> 11) * (1.0 / 9007199254740992.0);
					}
				};

				Xoshiro256 makeSeededEngine()
				{
					std::random_device rd;
					Xoshiro256 e{};
					for (auto& v : e.s) {
						v = (static_cast<std::uint64_t>(rd()) << 32) ^ static_cast<std::uint64_t>(rd());
					}
					// Avoid the degenerate all-zero state.
					if ((e.s[0] | e.s[1] | e.s[2] | e.s[3]) == 0) {
						e.s[0] = 0x9E3779B97F4A7C15ULL;
					}
					return e;
				}

				Xoshiro256& threadEngine()
				{
					static thread_local Xoshiro256 eng = makeSeededEngine();
					return eng;
				}

				// ---- Ziggurat normal sampler (Marsaglia & Tsang, 256 layers) -----
				// Exact standard normal. Tables built once (thread-safe static init).
				struct ZigguratTables
				{
					std::array<std::uint32_t, 256> k;
					std::array<double, 256> w;
					std::array<double, 256> f;

					ZigguratTables()
					{
						constexpr double r = 3.6541528853610088; // start of the tail
						constexpr double v = 4.92867323399e-3;    // area of each layer
						double dn = r;
						double tn = r;
						const double m1 = 2147483648.0; // 2^31

						const double q = v / std::exp(-0.5 * r * r);
						k[0] = static_cast<std::uint32_t>((dn / q) * m1);
						k[1] = 0;
						w[0] = q / m1;
						w[255] = dn / m1;
						f[0] = 1.0;
						f[255] = std::exp(-0.5 * dn * dn);

						for (int i = 254; i >= 1; --i)
						{
							dn = std::sqrt(-2.0 * std::log(v / dn + std::exp(-0.5 * dn * dn)));
							k[i + 1] = static_cast<std::uint32_t>((dn / tn) * m1);
							tn = dn;
							f[i] = std::exp(-0.5 * dn * dn);
							w[i] = dn / m1;
						}
					}
				};

				const ZigguratTables& zigTables()
				{
					static const ZigguratTables t;
					return t;
				}

				// Slow path: sample from the Gaussian tail beyond r.
				double zigguratTail(Xoshiro256& eng, double r, bool negative)
				{
					double x;
					double y;
					do {
						x = -std::log(1.0 - eng.nextDouble()) / r;
						y = -std::log(1.0 - eng.nextDouble());
					} while (y + y < x * x);
					const double val = r + x;
					return negative ? -val : val;
				}

				double zigguratNormal(Xoshiro256& eng, const ZigguratTables& z)
				{
					constexpr double r = 3.6541528853610088;
					for (;;)
					{
						const std::uint64_t u = eng.next();
						const int i = static_cast<int>(u & 0xFF);
						// signed 32-bit value from the next bits
						const auto hz = static_cast<std::int32_t>(static_cast<std::uint32_t>(u >> 32));
						const auto iz = static_cast<std::uint32_t>(i);
						const double x = hz * z.w[iz];
						if (static_cast<std::uint32_t>(std::abs(hz)) < z.k[iz]) {
							return x;
						}
						if (iz == 0) {
							return zigguratTail(eng, r, hz < 0);
						}
						if (z.f[iz] + eng.nextDouble() * (z.f[iz - 1] - z.f[iz]) < std::exp(-0.5 * x * x)) {
							return x;
						}
						// else reject, loop again
					}
				}
			} // namespace

			void fillNormal(double* dst, std::size_t n)
			{
				Xoshiro256& eng = threadEngine();
				// Fetch the ziggurat tables once for the whole batch. zigTables() is a
				// function-local static whose thread-safe-init guard was otherwise
				// checked on every single sample (measured as the largest cost inside
				// zigguratNormal); hoisting it here removes that per-sample overhead.
				const ZigguratTables& z = zigTables();
				for (std::size_t i = 0; i < n; ++i) {
					dst[i] = zigguratNormal(eng, z);
				}
			}

			std::vector<double> generateNormalVector(int size)
			{
				std::vector<double> vec(size);
				fillNormal(vec.data(), static_cast<std::size_t>(size));
				return vec;
			}

			void seedNormal(std::uint64_t seed)
			{
				// SplitMix64 expansion of a single seed into the four 64-bit words
				// Xoshiro256's state needs. Same seed => same subsequent fillNormal
				// sequence on the calling thread.
				std::uint64_t state = seed;
				auto splitmix64 = [&state]() {
					state += 0x9E3779B97F4A7C15ULL;
					std::uint64_t z = state;
					z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
					z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
					return z ^ (z >> 31);
				};

				Xoshiro256& eng = threadEngine();
				for (auto& v : eng.s) {
					v = splitmix64();
				}
				// Avoid the degenerate all-zero state (see makeSeededEngine).
				if ((eng.s[0] | eng.s[1] | eng.s[2] | eng.s[3]) == 0) {
					eng.s[0] = 0x9E3779B97F4A7C15ULL;
				}
			}

			void embedWrapped1D(std::vector<double>& out, std::span<const double> window, int kR0)
			{
				const int M = static_cast<int>(window.size());
				const int N = static_cast<int>(out.size());
				if (N <= 0) {
					return;
				}
				for (int j = 0; j < M; ++j)
				{
					const int offset = j - kR0;
					const int idx = ((offset % N) + N) % N;
					out[idx] += window[j];
				}
			}

			std::vector<double> buildWrappedSeparableKernel2D(
				int size_x, int size_y, std::span<const SeparableKernelTerm2D> terms)
			{
				if (size_x <= 0 || size_y <= 0) {
					return {};
				}

				std::vector<double> combined(static_cast<std::size_t>(size_x) * size_y, 0.0);
				std::vector<double> wx(size_x);
				std::vector<double> wy(size_y);
				for (const auto& term : terms)
				{
					if (term.taps_x.empty() || term.taps_y.empty()) {
						continue;
					}

					std::ranges::fill(wx, 0.0);
					std::ranges::fill(wy, 0.0);
					embedWrapped1D(wx, term.taps_x, term.kR0_x);
					embedWrapped1D(wy, term.taps_y, term.kR0_y);

					for (int y = 0; y < size_y; ++y) {
						for (int x = 0; x < size_x; ++x) {
							combined[static_cast<std::size_t>(y) * size_x + x] += term.sign * wx[x] * wy[y];
						}
					}
				}
				return combined;
			}
		}
	
