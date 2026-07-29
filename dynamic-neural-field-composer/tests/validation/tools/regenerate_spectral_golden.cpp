// dnf_composer_regen_spectral_golden — generates the committed reference CSVs
// under tests/validation/data/2d_spectral/expected/ from the sim JSONs under
// tests/validation/data/2d_spectral/simulations/.
//
// Reuses validation_common.h's runProtocol()/zeroAllStimuli() so the
// generator and test_spectral_golden_2d.cpp physically cannot drift on step
// counts or stimulus-zeroing semantics. Wraps the run in
// ScopedConvolutionMode(ForceDirect), so the committed CSVs are by
// construction the DIRECT path's answer -- test_spectral_golden_2d.cpp then
// checks both the direct path (must still match, unchanged) and the Auto
// path (which is expected to take the spectral branch on these fixtures,
// picked specifically to straddle the dispatch rule) against the same data.
//
// Usage: dnf_composer_regen_spectral_golden [simulations_dir] [expected_dir]
//   simulations_dir defaults to VALIDATION_DATA_DIR/2d_spectral/simulations
//   expected_dir    defaults to VALIDATION_DATA_DIR/2d_spectral/expected
// No hard-coded absolute paths, unlike examples/cross_platform_validation_runner*.cpp.

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>

#include "validation_common.h"
#include "tools/fft_convolution.h"

namespace fs = std::filesystem;
using namespace dnf_composer::test_validation;
using namespace dnf_composer::tools::math;

namespace
{
    void saveCsv(const fs::path& path, const std::vector<double>& values)
    {
        std::ofstream f(path);
        f << std::setprecision(17);
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (i) f << ',';
            f << values[i];
        }
    }
}

int main(int argc, char* argv[])
{
    silenceLogging();

    const fs::path simDir = (argc > 1) ? fs::path(argv[1])
                                        : fs::path(VALIDATION_DATA_DIR) / "2d_spectral" / "simulations";
    const fs::path outDir = (argc > 2) ? fs::path(argv[2])
                                        : fs::path(VALIDATION_DATA_DIR) / "2d_spectral" / "expected";

    if (!fs::exists(simDir))
    {
        std::fprintf(stderr, "Simulations directory not found: %s\n", simDir.string().c_str());
        return 1;
    }
    fs::create_directories(outDir);

    ScopedConvolutionMode forceDirect(ConvolutionMode::ForceDirect);

    int count = 0;
    bool anyNonFinite = false;
    for (const auto& entry : fs::directory_iterator(simDir))
    {
        if (entry.path().extension() != ".json") continue;
        const std::string stem = entry.path().stem().string();

        std::printf("Running %s ... ", stem.c_str());
        std::fflush(stdout);

        const ProtocolResult result = runProtocol(entry.path());

        bool finite = true;
        for (double v : result.with_stimulus)    finite = finite && std::isfinite(v);
        for (double v : result.without_stimulus) finite = finite && std::isfinite(v);
        if (!finite)
        {
            std::printf("NON-FINITE OUTPUT -- skipping write\n");
            anyNonFinite = true;
            continue;
        }

        saveCsv(outDir / (stem + "_with_stimulus.csv"), result.with_stimulus);
        saveCsv(outDir / (stem + "_without_stimulus.csv"), result.without_stimulus);
        std::printf("ok (%zu values)\n", result.with_stimulus.size());
        ++count;
    }

    std::printf("Wrote %d fixture(s) to %s\n", count, outDir.string().c_str());
    return anyNonFinite ? 1 : 0;
}
