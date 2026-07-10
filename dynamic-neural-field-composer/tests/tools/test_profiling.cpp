#include <gtest/gtest.h>
#include <sstream>
#include <string>

#include "tools/profiling.h"

using namespace dnf_composer::tools::profiling;

// ---------------------------------------------------------------------------
// Timer — RAII scope timer, writes a report line to its stream on destruction
// ---------------------------------------------------------------------------

TEST(Timer, WritesReportOnDestruction)
{
    std::ostringstream out;
    {
        Timer t("my-scope", out);
        (void)t;
    }
    EXPECT_FALSE(out.str().empty());
}

TEST(Timer, ReportContainsSignature)
{
    std::ostringstream out;
    {
        Timer t("unique-signature-123", out);
        (void)t;
    }
    EXPECT_NE(out.str().find("unique-signature-123"), std::string::npos);
}

TEST(Timer, ReportContainsDurationLabel)
{
    std::ostringstream out;
    {
        Timer t("scope", out);
        (void)t;
    }
    EXPECT_NE(out.str().find("Duration (us):"), std::string::npos);
}

TEST(Timer, DefaultSignatureConstructionDoesNotThrow)
{
    std::ostringstream out;
    EXPECT_NO_THROW({
        Timer t("something that takes time", out);
        (void)t;
    });
}

TEST(Timer, NothingWrittenBeforeDestruction)
{
    std::ostringstream out;
    {
        Timer t("scoped", out);
        (void)t;
        EXPECT_TRUE(out.str().empty());
    }
    EXPECT_FALSE(out.str().empty());
}
