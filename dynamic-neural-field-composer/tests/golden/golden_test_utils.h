#pragma once
// ============================================================================
//  Golden-test harness  —  algebraic-equivalence regression net for DNF elements
// ============================================================================
//
//  Purpose
//  -------
//  Safeguard the *numerical / algebraic behaviour* of every DNF element. A change
//  that alters an element's dynamics (its output for a fixed input and parameter
//  set) MUST make a golden test fail. These tests are the contract that "feature
//  work did not silently change the maths".
//
//  Two complementary nets, both provided here:
//
//   1. ANALYTIC EQUIVALENCE  (checkAgainstReference)
//      For elements with a closed-form definition we re-derive the maths
//      INDEPENDENTLY in tests/golden/reference/*.h (the "golden implementation
//      reference") and assert the production element reproduces it element-wise
//      to a tight tolerance (kTol = 1e-9). If production drifts from the maths,
//      this fails immediately — no data file required.
//
//   2. FROZEN GOLDEN DATA  (checkCharacterization / the persisted CSV side of
//      checkAgainstReference)
//      Deterministic outputs/trajectories are serialised ONCE to CSV under
//      tests/golden/data/ and committed. Later runs load the CSV and compare.
//      This catches drift even for composed architectures where no closed form
//      is practical (e.g. a settled Amari bump after 500 steps), and it doubles
//      as the reusable validation dataset for new features.
//
//  Regenerating golden data
//  ------------------------
//  Set the environment variable  DNF_UPDATE_GOLDEN=1  and re-run the test binary.
//  Missing CSVs are always (re)written from the reference / production output;
//  existing CSVs are only overwritten in update mode. NEVER regenerate blindly to
//  make a red test pass — a diff in data/ is a signal that dynamics changed.
//
//  Conventions
//  -----------
//   * GOLDEN_DATA_DIR is injected by CMake and points at tests/golden/data.
//   * A "golden name" is a slug (e.g. "gauss_stimulus_1d_circular_s5") that maps
//     1:1 to a CSV file  <GOLDEN_DATA_DIR>/<name>.csv .
//   * 2D fields are stored y-major and flattened to a single row.
//   * Trajectories are stored one simulation step per CSV row.
// ============================================================================

#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <memory>

#include "simulation/simulation.h"
#include "elements/element.h"

namespace dnf_composer::golden
{
    // ---- tolerances -------------------------------------------------------
    // Tight tolerance for analytic equivalence (double maths reproduced exactly
    // bar floating-point reordering). Loosen per-call only with justification.
    inline constexpr double kTol = 1e-9;
    // Statistical tolerance for stochastic elements (means / variances).
    inline constexpr double kStatTol = 5e-2;

    using Row  = std::vector<double>;
    using Grid = std::vector<std::vector<double>>; // trajectory: rows of steps

    // ---- config -----------------------------------------------------------
    inline std::string dataDir()
    {
#ifdef GOLDEN_DATA_DIR
        return std::string(GOLDEN_DATA_DIR);
#else
        return std::string("golden/data");
#endif
    }

    inline bool updateMode()
    {
        const char* e = std::getenv("DNF_UPDATE_GOLDEN");
        return e != nullptr && std::string(e) != "0" && std::string(e).size() > 0;
    }

    inline std::string csvPath(const std::string& name)
    {
        return dataDir() + "/" + name + ".csv";
    }

    inline bool fileExists(const std::string& path)
    {
        std::ifstream f(path);
        return f.good();
    }

    // ---- CSV I/O (full precision, lossless round-trip) --------------------
    inline void writeCsv(const std::string& name, const Grid& rows)
    {
        const std::string path = csvPath(name);
        std::ofstream f(path, std::ios::trunc);
        ASSERT_TRUE(f.good()) << "cannot open golden file for write: " << path;
        f << std::setprecision(17);
        for (const auto& row : rows)
        {
            for (std::size_t i = 0; i < row.size(); ++i)
            {
                if (i) f << ',';
                f << row[i];
            }
            f << '\n';
        }
    }

    inline void writeCsv(const std::string& name, const Row& row)
    {
        writeCsv(name, Grid{ row });
    }

    inline Grid readCsv(const std::string& name)
    {
        const std::string path = csvPath(name);
        std::ifstream f(path);
        Grid rows;
        std::string line;
        while (std::getline(f, line))
        {
            if (line.empty()) continue;
            Row row;
            std::stringstream ss(line);
            std::string cell;
            while (std::getline(ss, cell, ','))
                row.push_back(std::stod(cell));
            rows.push_back(std::move(row));
        }
        return rows;
    }

    // ---- comparison with rich diagnostics ---------------------------------
    inline ::testing::AssertionResult vecNear(const Row& actual, const Row& expected,
                                              double tol, const std::string& tag = "")
    {
        if (actual.size() != expected.size())
            return ::testing::AssertionFailure()
                << tag << " size mismatch: actual " << actual.size()
                << " vs expected " << expected.size();

        double maxAbs = 0.0; std::size_t worst = 0;
        for (std::size_t i = 0; i < actual.size(); ++i)
        {
            const double d = std::abs(actual[i] - expected[i]);
            if (d > maxAbs) { maxAbs = d; worst = i; }
        }
        if (maxAbs > tol)
            return ::testing::AssertionFailure()
                << tag << " max |Δ| = " << std::setprecision(17) << maxAbs
                << " at index " << worst << " (tol " << tol << "); actual="
                << actual[worst] << " expected=" << expected[worst];
        return ::testing::AssertionSuccess();
    }

    inline ::testing::AssertionResult gridNear(const Grid& actual, const Grid& expected,
                                               double tol, const std::string& tag = "")
    {
        if (actual.size() != expected.size())
            return ::testing::AssertionFailure()
                << tag << " row-count mismatch: " << actual.size()
                << " vs " << expected.size();
        for (std::size_t r = 0; r < actual.size(); ++r)
        {
            auto res = vecNear(actual[r], expected[r], tol, tag + " row " + std::to_string(r));
            if (!res) return res;
        }
        return ::testing::AssertionSuccess();
    }

    // ---- the two primary entry points -------------------------------------
    //
    // ANALYTIC: assert production == independent reference (algebraic
    // equivalence) AND freeze/verify the committed golden CSV.
    inline void checkAgainstReference(const std::string& name,
                                      const Row& production,
                                      const Row& reference,
                                      double tol = kTol)
    {
        // 1) algebraic equivalence to the golden implementation
        EXPECT_TRUE(vecNear(production, reference, tol, name + " [production vs reference]"));

        // 2) frozen-data net: capture on first run / update, else compare
        if (updateMode() || !fileExists(csvPath(name)))
        {
            writeCsv(name, reference);
        }
        else
        {
            const Grid golden = readCsv(name);
            ASSERT_EQ(golden.size(), 1u) << name << ": expected single-row golden";
            EXPECT_TRUE(vecNear(production, golden[0], tol, name + " [production vs frozen golden]"));
        }
    }

    inline void checkAgainstReference(const std::string& name,
                                      const Grid& production,
                                      const Grid& reference,
                                      double tol = kTol)
    {
        EXPECT_TRUE(gridNear(production, reference, tol, name + " [production vs reference]"));
        if (updateMode() || !fileExists(csvPath(name)))
            writeCsv(name, reference);
        else
            EXPECT_TRUE(gridNear(production, readCsv(name), tol, name + " [production vs frozen golden]"));
    }

    // CHARACTERIZATION: no closed form — capture production once, then guard it.
    // Use for composed architectures and any output whose reference IS the code.
    inline void checkCharacterization(const std::string& name,
                                      const Grid& production,
                                      double tol = kTol)
    {
        if (updateMode() || !fileExists(csvPath(name)))
        {
            writeCsv(name, production);
            SUCCEED() << name << ": golden captured (" << production.size() << " rows)";
        }
        else
        {
            EXPECT_TRUE(gridNear(production, readCsv(name), tol, name + " [production vs frozen golden]"));
        }
    }

    inline void checkCharacterization(const std::string& name, const Row& production,
                                      double tol = kTol)
    {
        checkCharacterization(name, Grid{ production }, tol);
    }

    // ---- simulation helpers ----------------------------------------------
    // Run `sim` for `steps` steps, capturing `component` of `elementName` each
    // step. Returns a trajectory (one row per step). `sim.init()` must be called
    // by the caller before this (so callers control interactions / seeding).
    inline Grid captureTrajectory(Simulation& sim, const std::string& elementName,
                                  const std::string& component, int steps)
    {
        Grid traj;
        traj.reserve(steps);
        for (int i = 0; i < steps; ++i)
        {
            sim.step();
            traj.push_back(sim.getElement(elementName)->getComponent(component));
        }
        return traj;
    }
}
