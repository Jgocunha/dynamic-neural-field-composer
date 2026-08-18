# dnf-composer threading contract

This documents what is and isn't safe to do with dnf-composer across threads, after
the thread-safety hardening.

## Supported model: one Simulation per thread

**You may run independent `dnf_composer::Simulation` objects on separate threads
simultaneously.** Each thread must own its Simulation (and all of that Simulation's
elements) exclusively — no two threads may touch the same Simulation or Element at
the same time.

This is exactly the pattern used by consumers like **neat-dnfs**, which evaluates a
population of phenotypes in parallel: each genome owns one `Simulation`, and
`std::async` runs `phenotype.step()` for each on its own thread. That usage is safe
and requires **no changes to neat-dnfs**.

The regression test `tests/simulation/test_thread_safety.cpp`
(`ThreadSafety.ParallelIndependentSimulationsMatchSerial`) proves that stepping N
independent Simulations in parallel yields the same per-Simulation result as stepping
them serially.

### What makes this safe

- **Element / plot IDs** — `ElementIdentifiers::uniqueIdentifierCounter` and
  `Plot::uniqueIdentifierCounter` are `std::atomic<int>` incremented with `fetch_add`,
  so constructing elements/plots concurrently (e.g. building many Simulations at once)
  assigns unique, non-torn IDs.
- **Logging** — `tools::logger::log()` no longer mutates a shared global Logger (it uses
  a local instance), `minLogLevel` is `std::atomic`, and the actual console/GUI emit is
  serialized by an internal mutex. Logging from many threads is safe; lines from
  different threads are not ordered relative to each other (interleaving is possible but
  each line is intact).
- **Per-Simulation state** — each `Simulation` owns its `elements` vector and each
  `Element` owns its `components` map; these are never shared between Simulations.
- **RNG / conv scratch** — the noise PRNG (`tools::math::fillNormal`) and the
  convolution kernel scratch are `thread_local`, so each thread has its own. Note: this
  means noise is **not bit-reproducible across a different thread count** — each thread
  seeds its own engine. Deterministic (noise-free or amplitude-0) simulations are fully
  reproducible.

## Not supported

- **Sharing a single Simulation or Element across threads** (e.g. stepping one
  Simulation from two threads, or parallelizing the element `step()` calls *within* one
  `Simulation::step()`). Elements update in place and read each other's outputs during a
  step (a Gauss-Seidel sweep over a cyclic field↔kernel graph), so concurrent element
  stepping would both data-race and change the dynamics. `Simulation::step()` is
  deliberately single-threaded.

- **Concurrent mutation of one Simulation's topology** (addElement/removeElement) from
  multiple threads.

## For integrators / debugging

If you embed dnf-composer in a multithreaded application, keep each Simulation on one
thread. For debugging, a single-threaded driver is easiest — there is no parallelism
inside the library to disable, so single-threaded stepping is the default and the
behavior is identical to any parallel multi-Simulation run.
