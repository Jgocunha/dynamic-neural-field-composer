#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "tools/utils.h"
#include "exceptions/exception.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"

using namespace dnf_composer;
using namespace dnf_composer::tools::utils;
namespace fs = std::filesystem;

// Fixture: per-test temporary directory.
class UtilsFileTest : public ::testing::Test
{
protected:
    std::string tempDir;

    void SetUp() override
    {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        tempDir = (fs::temp_directory_path() / "dnf_utils_tests" / info->name()).string();
        fs::create_directories(tempDir);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(tempDir, ec);
    }

    std::string path(const std::string& name) const { return tempDir + "/" + name; }
};

// ---------------------------------------------------------------------------
// countNumOfLinesInFile
// ---------------------------------------------------------------------------

TEST_F(UtilsFileTest, CountLinesNonExistentFileReturnsMinusOne)
{
    EXPECT_EQ(countNumOfLinesInFile(path("no_such_file.txt")), -1);
}

TEST_F(UtilsFileTest, CountLinesEmptyFileReturnsZero)
{
    const std::string f = path("empty.txt");
    std::ofstream{ f };
    EXPECT_EQ(countNumOfLinesInFile(f), 0);
}

TEST_F(UtilsFileTest, CountLinesSingleLineReturnsOne)
{
    const std::string f = path("one.txt");
    std::ofstream{ f } << "hello\n";
    EXPECT_EQ(countNumOfLinesInFile(f), 1);
}

TEST_F(UtilsFileTest, CountLinesMultipleLinesCountsCorrectly)
{
    const std::string f = path("multi.txt");
    std::ofstream ofs{ f };
    ofs << "line1\nline2\nline3\n";
    ofs.close();
    EXPECT_EQ(countNumOfLinesInFile(f), 3);
}

// ---------------------------------------------------------------------------
// saveVectorToFile
// ---------------------------------------------------------------------------

TEST_F(UtilsFileTest, SaveVectorToFileReturnsTrueOnSuccess)
{
    const std::vector<double> v{ 1.0, 2.0, 3.0 };
    EXPECT_TRUE(saveVectorToFile(v, path("vec.txt")));
}

TEST_F(UtilsFileTest, SaveVectorToFileCreatesFile)
{
    const std::string f = path("vec2.txt");
    saveVectorToFile({ 1.0, 2.5 }, f);
    EXPECT_TRUE(fs::exists(f));
}

TEST_F(UtilsFileTest, SaveVectorToFileReturnsFalseOnBadPath)
{
    const std::vector<double> v{ 1.0 };
    EXPECT_FALSE(saveVectorToFile(v, "/nonexistent_dir/file.txt"));
}

TEST_F(UtilsFileTest, SaveVectorToFileRoundTrip)
{
    const std::vector<double> original{ 1.1, 2.2, 3.3, 4.4 };
    const std::string f = path("roundtrip.txt");
    saveVectorToFile(original, f);

    std::ifstream ifs(f);
    ASSERT_TRUE(ifs.is_open());
    std::vector<double> loaded;
    double val;
    while (ifs >> val)
        loaded.push_back(val);

    ASSERT_EQ(loaded.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i)
        EXPECT_NEAR(loaded[i], original[i], 1e-9);
}

// ---------------------------------------------------------------------------
// replaceForwardSlashesWithBackslashes
// ---------------------------------------------------------------------------

TEST(UtilsTest, ReplaceForwardSlashesNoSlashesIsUnchanged)
{
    EXPECT_EQ(replaceForwardSlashesWithBackslashes("hello"), "hello");
}

TEST(UtilsTest, ReplaceForwardSlashesSingleSlash)
{
    EXPECT_EQ(replaceForwardSlashesWithBackslashes("a/b"), "a\\b");
}

TEST(UtilsTest, ReplaceForwardSlashesMultipleSlashes)
{
    EXPECT_EQ(replaceForwardSlashesWithBackslashes("a/b/c/d"), "a\\b\\c\\d");
}

TEST(UtilsTest, ReplaceForwardSlashesEmptyStringIsUnchanged)
{
    EXPECT_EQ(replaceForwardSlashesWithBackslashes(""), "");
}

// ---------------------------------------------------------------------------
// resizeMatrix
// ---------------------------------------------------------------------------

TEST(UtilsTest, ResizeMatrixSetsCorrectDimensions)
{
    std::vector<std::vector<int>> m;
    resizeMatrix(m, 3, 5);
    ASSERT_EQ(static_cast<int>(m.size()), 3);
    for (const auto& row : m)
        EXPECT_EQ(static_cast<int>(row.size()), 5);
}

TEST(UtilsTest, ResizeMatrixToZeroRows)
{
    std::vector<std::vector<double>> m(4, std::vector<double>(4));
    resizeMatrix(m, 0, 4);
    EXPECT_TRUE(m.empty());
}

// ---------------------------------------------------------------------------
// generateRandomNumber
// ---------------------------------------------------------------------------

TEST(UtilsTest, GenerateRandomDoubleIsWithinRange)
{
    for (int i = 0; i < 50; ++i)
    {
        const double v = generateRandomNumber(0.0, 1.0);
        EXPECT_GE(v, 0.0);
        EXPECT_LE(v, 1.0);
    }
}

TEST(UtilsTest, GenerateRandomDoubleNegativeRange)
{
    for (int i = 0; i < 50; ++i)
    {
        const double v = generateRandomNumber(-5.0, -1.0);
        EXPECT_GE(v, -5.0);
        EXPECT_LE(v, -1.0);
    }
}

// ---------------------------------------------------------------------------
// fillMatrixWithRandomValues
// ---------------------------------------------------------------------------

TEST(UtilsTest, FillMatrixWithRandomValuesIsInRange)
{
    std::vector<std::vector<double>> m(4, std::vector<double>(4));
    fillMatrixWithRandomValues(m, -1.0, 1.0);
    for (const auto& row : m)
        for (const double v : row)
        {
            EXPECT_GE(v, -1.0);
            EXPECT_LE(v,  1.0);
        }
}

TEST(UtilsTest, FillMatrixWithRandomValuesModifiesMatrix)
{
    std::vector<std::vector<double>> m(3, std::vector<double>(3, 0.0));
    fillMatrixWithRandomValues(m, -10.0, 10.0);
    // Very unlikely that all 9 values remain exactly 0.0
    bool anyNonZero = false;
    for (const auto& row : m)
        for (const double v : row)
            if (v != 0.0) { anyNonZero = true; break; }
    EXPECT_TRUE(anyNonZero);
}

// ---------------------------------------------------------------------------
// describeElementCreationFailure
//
// Regression tests for issue #146.
//
// The library deliberately fails loudly now: ElementDimensions (#96), the
// Element base constructor (#118) and ElementFactory (#113) all throw on bad
// input. But the element-creation forms in SimulationWindow run inside the
// ImGui render loop, so an exception escaping one of them unwinds straight out
// of the frame and terminates the application -- a user typing a bad size into
// the form could kill the process.
//
// describeElementCreationFailure() is the seam that stops that: it runs the
// construct-and-add step and converts any failure into a message the form can
// display, returning an empty string on success. It is deliberately free of any
// ImGui dependency so it can be tested exactly like this, with no window, no
// context and no render loop.
// ---------------------------------------------------------------------------

TEST(UtilsTest, DescribeElementCreationFailureSuccessfulCreationReportsNoError)
{
    const auto sim = std::make_shared<Simulation>("creation-ok", 1.0, 0.0, 0.0);

    const std::string error = describeElementCreationFailure([&]
    {
        const element::ElementCommonParameters common{ "nf", 20 };
        const element::NeuralFieldParameters nfp{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } };
        sim->addElement(std::make_shared<element::NeuralField>(common, nfp));
    });

    EXPECT_TRUE(error.empty()) << "unexpected error: " << error;
    EXPECT_EQ(sim->getNumberOfElements(), 1);
}

TEST(UtilsTest, DescribeElementCreationFailureInvalidSizeIsReportedInsteadOfEscaping)
{
    // The live trigger today: ElementDimensions rejects a non-positive size
    // (#96, already on main). Before this seam existed the throw propagated out
    // of the render loop.
    const auto sim = std::make_shared<Simulation>("creation-bad-size", 1.0, 0.0, 0.0);

    const std::string error = describeElementCreationFailure([&]
    {
        const element::ElementCommonParameters common{ element::ElementIdentifiers{ "nf" },
                                                       element::ElementDimensions{ 0, 1.0 } };
        const element::NeuralFieldParameters nfp{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } };
        sim->addElement(std::make_shared<element::NeuralField>(common, nfp));
    });

    EXPECT_FALSE(error.empty());
    EXPECT_EQ(sim->getNumberOfElements(), 0) << "nothing should have been added";
}

TEST(UtilsTest, DescribeElementCreationFailureLibraryExceptionMessageIsPreserved)
{
    // The Exception carries the element name and ErrorCode, so the form can show
    // something specific rather than a generic "could not add element".
    const std::string error = describeElementCreationFailure([]
    {
        throw Exception(ErrorCode::ELEM_INVALID_SIZE, std::string("my field"));
    });

    ASSERT_FALSE(error.empty());
    EXPECT_NE(error.find("my field"), std::string::npos)
        << "the element name should survive into the message, got: " << error;
}

TEST(UtilsTest, DescribeElementCreationFailureNonLibraryExceptionIsAlsoCaught)
{
    // Backstop: std::bad_alloc and anything else a constructor might throw must
    // not escape the render loop either.
    const std::string error = describeElementCreationFailure([]
    {
        throw std::runtime_error("something else went wrong");
    });

    ASSERT_FALSE(error.empty());
    EXPECT_NE(error.find("something else went wrong"), std::string::npos);
}

TEST(UtilsTest, DescribeElementCreationFailureUnknownExceptionIsCaught)
{
    // Nothing at all may escape; a non-std throw still yields a usable message.
    const std::string error = describeElementCreationFailure([] { throw 42; });

    EXPECT_FALSE(error.empty());
}

// ---------------------------------------------------------------------------
// resolveResourceRoot -- the decision logic behind getResourceRoot(), pulled
// out into a pure function so it can be exercised against a real (but
// temporary) filesystem layout instead of the actual running executable's
// path. See getResourceRoot()'s doc comment for why a dev-build fallback
// exists at all (#126): the build tree does not copy resources/ next to the
// test binary the way `cmake --install` does, so an uninstalled binary must
// fall back to something -- these tests pin exactly when that fallback is
// used, and that the exe-relative path never contains it when it wasn't.
// ---------------------------------------------------------------------------

TEST_F(UtilsFileTest, ResolveResourceRootPrefersExeRelativeDirectoryWhenResourcesExists)
{
    const auto exeDir = fs::path(tempDir) / "bin";
    fs::create_directories(exeDir);
    fs::create_directories(fs::path(tempDir) / "resources");

    const std::string devFallback = "C:/some/configure-time/source/tree";
    const std::string root = resolveResourceRoot(exeDir, devFallback);

    EXPECT_EQ(root, fs::weakly_canonical(tempDir).string());
    EXPECT_NE(root, devFallback);
}

TEST_F(UtilsFileTest, ResolveResourceRootFallsBackWhenNoResourcesDirNextToExe)
{
    const auto exeDir = fs::path(tempDir) / "bin";
    fs::create_directories(exeDir);
    // Deliberately do not create a "resources" sibling directory.

    const std::string devFallback = "C:/some/configure-time/source/tree";
    EXPECT_EQ(resolveResourceRoot(exeDir, devFallback), devFallback);
}

TEST_F(UtilsFileTest, ResolveResourceRootFallbackCanBeEmpty)
{
    // A release build configured with DNF_COMPOSER_DEV_FALLBACK_PATHS=OFF has
    // no compile-time path to fall back to at all; the fallback is "", not a
    // build-machine path, once no exe-relative resources dir is found.
    const auto exeDir = fs::path(tempDir) / "bin";
    fs::create_directories(exeDir);
    EXPECT_EQ(resolveResourceRoot(exeDir, ""), "");
}
