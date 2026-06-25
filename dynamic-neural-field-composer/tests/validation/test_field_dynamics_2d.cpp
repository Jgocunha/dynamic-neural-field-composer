// Field-dynamics regression validation — 2D.
//
// Re-runs the two-phase cross-platform-validation protocol against the live
// library for every vendored 2D sim JSON and asserts the (row-major flattened)
// activation profile matches the vendored reference CSV within 1e-4. Guards that
// performance optimizations do not alter dynamics.

#include "validation_common.h"

using namespace dnf_composer::test_validation;

TEST(FieldDynamics2D, AllSimsMatchReference)
{
	silenceLogging();
	const auto stems = collectSimStems("2d");
	if (stems.empty())
		GTEST_SKIP() << "No 2D validation sims found under " << VALIDATION_DATA_DIR
		             << "/2d/simulations";

	for (const auto& stem : stems)
	{
		SCOPED_TRACE("2d sim: " + stem);
		try { expectSimMatchesReference("2d", stem); }
		catch (const std::exception& e) { ADD_FAILURE() << "2d sim " << stem << " threw: " << e.what(); }
	}
}
