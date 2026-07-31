// Field-dynamics regression validation — 1D.
//
// Re-runs the two-phase cross-platform-validation protocol against the live
// library for every vendored 1D sim JSON and asserts the activation profile
// matches the vendored reference CSV within 1e-4. Guards that performance
// optimizations do not alter dynamics.

#include "validation_common.h"

using namespace dnf_composer::test_validation;

TEST(FieldDynamics1D, AllSimsMatchReference)
{
	silenceLogging();
	const auto stems = collectSimStems("1d");
	if (stems.empty())
		GTEST_SKIP() << "No 1D validation sims found under " << VALIDATION_DATA_DIR
		             << "/1d/simulations";

	for (const auto& stem : stems)
	{
		SCOPED_TRACE("1d sim: " + stem);
		try { expectSimMatchesReference("1d", stem); }
		catch (const std::exception& e) { ADD_FAILURE() << "1d sim " << stem << " threw: " << e.what(); }
	}
}
