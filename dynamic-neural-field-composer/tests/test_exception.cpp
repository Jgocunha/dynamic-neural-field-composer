#include <gtest/gtest.h>
#include <string>

#include "exceptions/exception.h"

using namespace dnf_composer;

// ---------------------------------------------------------------------------
// String-message constructor
// ---------------------------------------------------------------------------

TEST(ExceptionStringCtor, WhatReturnsMessage)
{
    const Exception e("my custom message");
    EXPECT_STREQ(e.what(), "my custom message");
}

TEST(ExceptionStringCtor, ErrorCodeIsOK)
{
    const Exception e("msg");
    EXPECT_EQ(e.getErrorCode(), ErrorCode::OK);
}

// ---------------------------------------------------------------------------
// ErrorCode constructor
// ---------------------------------------------------------------------------

TEST(ExceptionErrorCodeCtor, WhatIsNonEmpty)
{
    const Exception e(ErrorCode::SIM_INVALID_PARAMETER);
    EXPECT_GT(std::string(e.what()).length(), 0u);
}

TEST(ExceptionErrorCodeCtor, GetErrorCodeReturnsCorrectCode)
{
    const Exception e(ErrorCode::SIM_INVALID_PARAMETER);
    EXPECT_EQ(e.getErrorCode(), ErrorCode::SIM_INVALID_PARAMETER);
}

// ---------------------------------------------------------------------------
// ErrorCode + element name constructor
// ---------------------------------------------------------------------------

TEST(ExceptionErrorCodeElementCtor, MessageContainsElementName)
{
    const Exception e(ErrorCode::SIM_ELEM_NOT_FOUND, std::string("my-element"));
    std::string msg = e.what();
    EXPECT_NE(msg.find("my-element"), std::string::npos);
}

TEST(ExceptionErrorCodeElementCtor, GetErrorCodeIsCorrect)
{
    const Exception e(ErrorCode::ELEM_INVALID_PARAMETER, std::string("el"));
    EXPECT_EQ(e.getErrorCode(), ErrorCode::ELEM_INVALID_PARAMETER);
}

// ---------------------------------------------------------------------------
// ErrorCode + index constructor
// ---------------------------------------------------------------------------

TEST(ExceptionErrorCodeIndexCtor, GetErrorCodeIsCorrect)
{
    const Exception e(ErrorCode::SIM_ELEM_INDEX, 42);
    EXPECT_EQ(e.getErrorCode(), ErrorCode::SIM_ELEM_INDEX);
}

TEST(ExceptionErrorCodeIndexCtor, MessageContainsIndex)
{
    const Exception e(ErrorCode::SIM_ELEM_INDEX, 42);
    std::string msg = e.what();
    EXPECT_NE(msg.find("42"), std::string::npos);
}

// ---------------------------------------------------------------------------
// ErrorCode + element + component constructor
// ---------------------------------------------------------------------------

TEST(ExceptionErrorCodeElemCompCtor, MessageContainsComponentName)
{
    const Exception e(ErrorCode::ELEM_COMP_NOT_FOUND, std::string("el"), std::string("activation"));
    std::string msg = e.what();
    EXPECT_NE(msg.find("activation"), std::string::npos);
}

TEST(ExceptionErrorCodeElemCompCtor, MessageContainsElementName)
{
    const Exception e(ErrorCode::ELEM_COMP_NOT_FOUND, std::string("my-field"), std::string("output"));
    std::string msg = e.what();
    EXPECT_NE(msg.find("my-field"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Is std::exception
// ---------------------------------------------------------------------------

TEST(ExceptionInheritance, IsStdException)
{
    const Exception e(ErrorCode::SIM_INVALID_PARAMETER);
    const std::exception* base = &e;
    EXPECT_NE(base->what(), nullptr);
    EXPECT_GT(std::string(base->what()).length(), 0u);
}

// ---------------------------------------------------------------------------
// getErrorMessage for all major error codes
// ---------------------------------------------------------------------------

struct ErrorCodeCase
{
    ErrorCode code;
    const char* name;
};

class ExceptionErrorMessageTest : public ::testing::TestWithParam<ErrorCode> {};

INSTANTIATE_TEST_SUITE_P(AllCodes, ExceptionErrorMessageTest, ::testing::Values(
    ErrorCode::APP_CONSTRUCTION,
    ErrorCode::APP_INIT,
    ErrorCode::APP_STEP,
    ErrorCode::APP_CLOSE,
    ErrorCode::APP_INVALID_SIM,
    ErrorCode::APP_INVALID_VIS,
    ErrorCode::APP_VIS_SIM_MISMATCH,
    ErrorCode::SIM_INVALID_PARAMETER,
    ErrorCode::SIM_RUNTIME_LESS_THAN_ZERO,
    ErrorCode::ELEM_COMP_NOT_FOUND,
    ErrorCode::ELEM_INPUT_IS_NULL,
    ErrorCode::ELEM_INPUT_ALREADY_EXISTS,
    ErrorCode::ELEM_INPUT_NOT_FOUND,
    ErrorCode::ELEM_INVALID_PARAMETER,
    ErrorCode::ELEM_INVALID_SIZE,
    ErrorCode::VIS_INVALID_SIM,
    ErrorCode::VIS_DATA_NOT_FOUND,
    ErrorCode::VIS_INVALID_PLOTTING_INDEX,
    ErrorCode::PLOT_INVALID_VIS_OBJ
));

TEST_P(ExceptionErrorMessageTest, MessageIsNonEmpty)
{
    const Exception e(GetParam());
    EXPECT_GT(std::string(e.what()).length(), 0u);
}

TEST_P(ExceptionErrorMessageTest, GetErrorCodeReturnsPassedCode)
{
    const Exception e(GetParam());
    EXPECT_EQ(e.getErrorCode(), GetParam());
}

// ---------------------------------------------------------------------------
// Can be caught as std::exception
// ---------------------------------------------------------------------------

TEST(ExceptionCatch, CanBeCaughtAsStdException)
{
    bool caught = false;
    try
    {
        throw Exception(ErrorCode::SIM_INVALID_PARAMETER);
    }
    catch (const std::exception& ex)
    {
        caught = true;
        EXPECT_GT(std::string(ex.what()).length(), 0u);
    }
    EXPECT_TRUE(caught);
}

TEST(ExceptionCatch, CanBeCaughtAsDnfException)
{
    bool caught = false;
    try
    {
        throw Exception("test error");
    }
    catch (const Exception& ex)
    {
        caught = true;
        EXPECT_STREQ(ex.what(), "test error");
    }
    EXPECT_TRUE(caught);
}
