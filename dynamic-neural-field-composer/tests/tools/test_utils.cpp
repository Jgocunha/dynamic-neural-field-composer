#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "tools/utils.h"

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
