
[comment]: ####################################################################
[comment]: #
[comment]: # Copyright (c) 2016-2018, Lawrence Livermore National Security, LLC.
[comment]: # Produced at the Lawrence Livermore National Laboratory
[comment]: # Written by Geoffrey M. Oxberry <oxberry1@llnl.gov>
[comment]: # LLNL-CODE-739313
[comment]: # All rights reserved.
[comment]: #
[comment]: # This file is part of gtest-mpi-listener. For details, see
[comment]: # https://github.com/LLNL/gtest-mpi-listener
[comment]: #
[comment]: # Please also see the LICENSE and COPYRIGHT files for the BSD-3
[comment]: # license and copyright information.
[comment]: #
[comment]: # Redistribution and use in source and binary forms, with or without
[comment]: # modification, are permitted provided that the following conditions
[comment]: # are met:
[comment]: #
[comment]: # * Redistributions of source code must retain the above copyright
[comment]: #   notice, this list of conditions and the disclaimer below.
[comment]: #
[comment]: # * Redistributions in binary form must reproduce the above copyright
[comment]: #   notice, this list of conditions and the disclaimer (as noted
[comment]: #   below) in the documentation and/or other materials provided with
[comment]: #   the distribution.
[comment]: #
[comment]: # * Neither the name of the LLNS/LLNL nor the names of its
[comment]: #   contributors may be used to endorse or promote products derived
[comment]: #   from this software without specific prior written permission.
[comment]: #
[comment]: # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
[comment]: # "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
[comment]: # LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
[comment]: # FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
[comment]: # LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF
[comment]: # ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
[comment]: # INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
[comment]: # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
[comment]: # OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
[comment]: # INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
[comment]: # WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
[comment]: # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
[comment]: # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
[comment]: #
[comment]: ####################################################################

# googletest-mpi-listener

The goals of this project are:

- to augment the googletest framework with listener(s) that yield
sensible output for testing MPI-based, distributed-memory parallel
software

- to keep this software as low-maintenance as possible

- ideally, to merge some version of this functionality into [Google Test](https://github.com/google/googletest)

# License

This software is BSD-3 licensed, and uses some example code from
Google Test, which has the following license that must be duplicated
in its entirety, per its terms:

```
 Copyright 2005, Google Inc.  All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

     * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above
 copyright notice, this list of conditions and the following disclaimer
 in the documentation and/or other materials provided with the
 distribution.
     * Neither the name of Google Inc. nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

# Requirements

- GNU-compatible Make or gmake
- [CMake](https://cmake.org/) (required for the build directions in
  this README; this is a soft requirement, and anyone sufficiently
  motivated and skilled could figure out how to build Google Test and
  this project with just GNU-compatible Make or gmake -- it would just
  take time)
- a C++98-standard-compliant C++ compiler
- all of the requirements for Google Test
- a standards-conforming MPI-1.x implementation
  (e.g.,
  [MPICH](http://www.mpich.org/),
  [Open MPI](https://www.open-mpi.org/),
  [MVAPICH](http://mvapich.cse.ohio-state.edu/),
  [Intel MPI](https://software.intel.com/en-us/intel-mpi-library))

# Building the example code

1) In the root directory of this repository, clone the googletest GitHub repo:

`git clone https://github.com/google/googletest.git`

2) Change to the new `googletest` directory created via cloning:

`pushd googletest`

3) Within this directory, make a `build` directory; this step is
standard for CMake-based buildsystems:

`mkdir build`

4) Change to this `build` directory:

`cd build`

5) Run CMake to generate a `Makefile` that will build `googletest`:

`cmake ..`

You should see some CMake output indicating compiler detection, etc.

6) Assuming step 5 completed successfully, call `make` to build the
googletest library:

`make`

7) Return to the root directory of the repository:

`popd` (if you followed this directions)

8) Build the example test runner:

`make`

9) Assuming the test runner build proceeds successfully, run the test
runner example:

`bin/example.exe`

# Usage

Please read the
[Google Test Primer](https://github.com/google/googletest/blob/master/googletest/docs/Primer.md)
and
[Google Test Advanced Guide](https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md)
for background before reading the remainder of this section.

Usage of this listener is illustrated by the example in
`src/example.cpp`, which includes some very simple-minded MPI unit
tests that I've used for basic testing. To use this listener, you will
need to write your own `int main(int argc, char** argv)` function --
you cannot use the stock `gtest_main` supplied with Google Test. Using
a custom `main` function enables us to:

- initialize MPI
- add an `Environment` object that will finalize MPI when `main`
  terminates
- remove the stock Google Test `TestEventListener`, which emits output on tests
- add a `TestEventListener` that will emit sensible output (that is,
  rank-ordered output to stdout) for MPI-based unit tests

all of which is necessary, and not possible using the stock `gtest_main` function.

Directions for writing tests can be found in
[Google Test Primer](https://github.com/google/googletest/blob/master/googletest/docs/Primer.md)
and
[Google Test Advanced Guide](https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md). My
design assumption is that these tests will be executed in distributed
fashion using the MPI distributed-memory parallel programming model
(that is, a shared-nothing, process-local memory address space). This
assumption implies that tests will be run by all MPI processes, or
disabled on all processes. Conditionals can be used to execute parts
(or all) of the body of a test on a given MPI process or set of
processes.

After writing tests, your `int main(int argc, char** argv)` function should look like the example test runner:

```
int main(int argc, char** argv) {
// Filter out Google Test arguments
::testing::InitGoogleTest(&argc, argv);

// Initialize MPI
MPI_Init(&argc, &argv);

// Add object that will finalize MPI on exit; Google Test owns this pointer
::testing::AddGlobalTestEnvironment(new MPIEnvironment);

// Get the event listener list.
::testing::TestEventListeners& listeners =
::testing::UnitTest::GetInstance()->listeners();

// Remove default listener
delete listeners.Release(listeners.default_result_printer());

// Adds MPI listener; Google Test owns this pointer
listeners.Append(new MPIMinimalistPrinter);

// Run tests, then clean up and exit
RUN_ALL_TESTS();

return 0;
}
```

Comments in this example describe what each line does. The ordering is
also important, and should be preserved if you augment this example
with additional code.

# Design considerations

The most important design consideration was to write something
portable, quickly. This meant using MPI-1.x calls (because
supercomputers sometimes run older MPI implementations), and relying
on the public API calls of a portable unit testing library that is
currently being maintained. As a result of these decisions:

- this software communicates unit test results to MPI process 0 and
  then writes them to standard out (because MPI I/O is defined in
  MPI-2)

- consequently, this software will have an output bottleneck at
  extreme scale, because the output is, to borrow from
  [Bill Gropp's MPI I/O slides (see slide 3)](http://wgropp.cs.illinois.edu/courses/cs598-s16/lectures/lecture32.pdf),
  "worse than sequential"

- this software doesn't have the awesome, fancy, color-coded output
  that stock Google Test uses, because all of those functions are
  implemented as `static` in `googletest/googletest/src/gtest.cc`, and
  thus are inaccessible from outside that translation unit because
  they are not part of the Google Test public API

- this software is header-only, to make vendoring it into software
  packages no more painful than vendoring in Google Test

The current setup should be usable for small numbers of MPI processes
-- I've tested it on 256 MPI processes and it seems to work fine. If
there is a need to write infrastructure for testing on 100,000 MPI
processes, then an MPI I/O-based `TestEventListener` makes more sense,
and would relax the output bottleneck. This new code should be written
in a second header to isolate MPI-2-conforming code from
MPI-1-conforming code. Although I haven't done it yet, I suspect this
modification could be done in days.

Adding fancy, color-coded output is also possible, but it means
reimplementing private functions from Google Test that could change at
any moment. I'd rather not reimplement that myself, because it will
also mean that this software can no longer be header-only due to the
way Google Test implements terminal color-coding.

If you want to hack on this code, the most important things to note are:

- `::testing::TestEventListener::OnTestPartResult` is only called
  after assertion failures or invocation of the `SUCCESS()` macro, so
  it may not be called on some MPI processes, and there should be no
  communication in this function

- In `TestEventListener::OnTestEnd`, although `::testing::TestInfo`
  has methods to query `::testing::TestPartResult` objects within it,
  when I called those methods, the information I wanted (namely,
  `::testing::TestPartResult` objects) was no longer present, which is
  why I instead store that information within a class member instead

As stated above, I aim to keep this software low-maintenance, because
this package was written in a couple days in order to make the
parallel software development for my work easier. I'm happy to accept
bug-fixes, and I'm willing to discuss or consider feature
requests/additions, but I'd like any feature to be easy to maintain. I
do not get paid to write testing frameworks, which is why any
responses to bug-fixes, feature requests, documentation improvements,
or questions may be delayed; it is also why I want this software to be
low-maintenance. **In general, even though I intend to respond to
correspondence about this software (I would like it to improve), users
should assume I may not get back to them in a timely fashion, or at
all.**
