// Golden regression suite for the hybrid direct/FFT convolution path.
//
// Unlike tests/validation/test_field_dynamics_2d.cpp (50x50 fixtures, always
// below the spectral dispatch floor), the sim JSONs under
// tests/validation/data/2d_spectral/ are 128x128 and were chosen to straddle
// tools::math::kFFTTapThreshold: some stay on the direct path under Auto
// dispatch, the rest cross into the spectral path. Running the SAME committed
// CSVs (generated once, under ForceDirect -- see
// tests/validation/tools/regenerate_spectral_golden.cpp) against both Auto
// and ForceDirect pins BOTH paths against absolute drift: SpectralGolden2D
// confirms Auto's spectral branch reproduces the direct-path reference within
// tolerance (no numerical or qualitative reliability lost by switching
// paths), and DirectPathRegression2D confirms the direct path itself is
// unchanged by this refactor.

#include "validation_common.h"
#include "tools/fft_convolution.h"

using namespace dnf_composer::test_validation;
using namespace dnf_composer::tools::math;

TEST(SpectralGolden2D, AutoPathMatchesReference)
{
    silenceLogging();
    const ScopedConvolutionMode mode(ConvolutionMode::Auto);

    const auto stems = collectSimStems("2d_spectral");
    if (stems.empty())
        GTEST_SKIP() << "No 2D spectral golden sims found under " << VALIDATION_DATA_DIR
                     << "/2d_spectral/simulations";

    for (const auto& stem : stems)
    {
        SCOPED_TRACE("2d_spectral sim (Auto): " + stem);
        try { expectSimMatchesReference("2d_spectral", stem); }
        catch (const std::exception& e) { ADD_FAILURE() << "2d_spectral sim " << stem << " threw: " << e.what(); }
    }
}

TEST(SpectralGolden2D, ForceDirectPathMatchesReference)
{
    silenceLogging();
    const ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);

    const auto stems = collectSimStems("2d_spectral");
    if (stems.empty())
        GTEST_SKIP() << "No 2D spectral golden sims found under " << VALIDATION_DATA_DIR
                     << "/2d_spectral/simulations";

    for (const auto& stem : stems)
    {
        SCOPED_TRACE("2d_spectral sim (ForceDirect): " + stem);
        try { expectSimMatchesReference("2d_spectral", stem); }
        catch (const std::exception& e) { ADD_FAILURE() << "2d_spectral sim " << stem << " threw: " << e.what(); }
    }
}
