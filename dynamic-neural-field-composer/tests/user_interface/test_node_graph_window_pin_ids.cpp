// Unit tests for NodeGraphWindow's pin/link id encoding.
//
// The additive scheme (base 1000/2000/3000 + never-reset global uid) collides
// once any element's uid reaches 1000, and the stoull(concat) link id formula
// collides for multi-digit uids (stoull("11"+"1") == stoull("1"+"11")). This
// suite pins down the multiplicative replacement so it cannot regress.

#include <gtest/gtest.h>

#include "user_interface/node_graph_window.h"

using namespace dnf_composer::user_interface;

TEST(NodeGraphWindowPinIds, InputOutputTargetPinsNeverCollideAcrossUids)
{
    const std::vector<int> uids{ 0, 1, 999, 1000, 999999 };
    std::vector<uint64_t> allIds;

    for (const int uid : uids)
    {
        allIds.push_back(NodeGraphWindow::PinIdEncoding::inputPin(uid));
        allIds.push_back(NodeGraphWindow::PinIdEncoding::targetPin(uid));
        allIds.push_back(NodeGraphWindow::PinIdEncoding::outputPin(uid));
        allIds.push_back(NodeGraphWindow::PinIdEncoding::activationPin(uid));
    }

    for (std::size_t i = 0; i < allIds.size(); ++i)
        for (std::size_t j = i + 1; j < allIds.size(); ++j)
            EXPECT_NE(allIds[i], allIds[j]) << "collision at indices " << i << " and " << j;
}

TEST(NodeGraphWindowPinIds, DecodeRoundTripsForEveryKindAndUid)
{
    const std::vector<int> uids{ 0, 1, 999, 1000, 999999 };

    for (const int uid : uids)
    {
        const auto in = NodeGraphWindow::PinIdEncoding::decode(NodeGraphWindow::PinIdEncoding::inputPin(uid));
        EXPECT_EQ(in.kind, NodeGraphWindow::PinIdEncoding::Kind::Input);
        EXPECT_EQ(in.uid, uid);

        const auto tgt = NodeGraphWindow::PinIdEncoding::decode(NodeGraphWindow::PinIdEncoding::targetPin(uid));
        EXPECT_EQ(tgt.kind, NodeGraphWindow::PinIdEncoding::Kind::Target);
        EXPECT_EQ(tgt.uid, uid);

        const auto out = NodeGraphWindow::PinIdEncoding::decode(NodeGraphWindow::PinIdEncoding::outputPin(uid));
        EXPECT_EQ(out.kind, NodeGraphWindow::PinIdEncoding::Kind::Output);
        EXPECT_EQ(out.uid, uid);

        const auto act = NodeGraphWindow::PinIdEncoding::decode(NodeGraphWindow::PinIdEncoding::activationPin(uid));
        EXPECT_EQ(act.kind, NodeGraphWindow::PinIdEncoding::Kind::Activation);
        EXPECT_EQ(act.uid, uid);
    }
}

TEST(NodeGraphWindowPinIds, LinkIdsDoNotCollideForMultiDigitUids)
{
    // Regression for stoull(std::to_string(dst)+std::to_string(src)):
    // stoull("11"+"1") == stoull("1"+"11") == 111.
    const uint64_t a = NodeGraphWindow::PinIdEncoding::linkId(11, 1, false);
    const uint64_t b = NodeGraphWindow::PinIdEncoding::linkId(1, 11, false);
    EXPECT_NE(a, b);
}

TEST(NodeGraphWindowPinIds, LinkIdDistinguishesTargetSlot)
{
    const uint64_t normal = NodeGraphWindow::PinIdEncoding::linkId(5, 7, false);
    const uint64_t target = NodeGraphWindow::PinIdEncoding::linkId(5, 7, true);
    EXPECT_NE(normal, target);
}

TEST(NodeGraphWindowPinIds, LinkIdDistinguishesActivationSource)
{
    const uint64_t fromOutput     = NodeGraphWindow::PinIdEncoding::linkId(5, 7, false, false);
    const uint64_t fromActivation = NodeGraphWindow::PinIdEncoding::linkId(5, 7, false, true);
    EXPECT_NE(fromOutput, fromActivation);
}

TEST(NodeGraphWindowPinIds, LinkIdDistinguishesAllFourSlotCombinations)
{
    const uint64_t outputToInput      = NodeGraphWindow::PinIdEncoding::linkId(5, 7, false, false);
    const uint64_t outputToTarget     = NodeGraphWindow::PinIdEncoding::linkId(5, 7, true,  false);
    const uint64_t activationToInput  = NodeGraphWindow::PinIdEncoding::linkId(5, 7, false, true);
    const uint64_t activationToTarget = NodeGraphWindow::PinIdEncoding::linkId(5, 7, true,  true);

    const std::vector<uint64_t> ids{ outputToInput, outputToTarget, activationToInput, activationToTarget };
    for (std::size_t i = 0; i < ids.size(); ++i)
        for (std::size_t j = i + 1; j < ids.size(); ++j)
            EXPECT_NE(ids[i], ids[j]) << "collision at indices " << i << " and " << j;
}
