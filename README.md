Hybrid Abstraction Refinement Engine (HARE)
===========================================

HARE (Hybrid Abstraction-Refinement Engine) is a counterexample guided abstraction-refinement (CEGAR) based tool to verify safety properties of hybrid automata, whose continuous dynamics in each mode is non-linear, but initial values, invariants, and transition relations are specified using polyhedral constraints.
HARE works by abstracting non-linear hybrid automata into hybrid automata with polyhedral inclusion dynamics and uses [dReach](http://dreal.github.io/dReach/) to validate counterexamples. 
The CEGAR framework forming the theoretical basis of HARE makes provable progress in each iteration of the abstraction-refinement loop. The current version is a significant advance on previous versions of HARE; it considers a richer class of abstract models (polyhedral flows as opposed to rectangular flows) and can be applied to a larger class of concrete models (non-linear hybrid automata as opposed to affine hybrid automata).
These advances have led to better performance results for a broader class of examples.
In the [tool's page](https://nima-roohi.github.io/HARE), we report our experimental comparison of HARE against other state-of-the-art tools for affine models ([SpaceEx](http://spaceex.imag.fr/), [PHAVer](http://spaceex.imag.fr/phaver-8), and [SpaceEx AGAR](https://swt.informatik.uni-freiburg.de/tool/spaceex/agar)) and non-linear models ([FLOW*](https://flowstar.org/), [HSolver](http://hsolver.sourceforge.net/), and [C2E2](http://publish.illinois.edu/c2e2-tool/)).
In the rest of this file, we explain how to install HARE, run benchmarks, and different options for verifying a model.






Virtual Machine
---------------

HARE is written in C++ and uses a few libraries. 
While each of these libraries is perhaps the most famous in what they do, installing all of them to get a sense of what HARE does might be overwhelming. Therefore, we have prepared a virtual machine that contains everything; source codes, binaries, benchmarks, scripts for running those benchmarks, as well as what you should expect to see at the end of running those benchmarks. All these can are available at https://uofi.app.box.com/v/HARE. You need [7-Zip](https://www.7-zip.org/) to extract downloaded files, and virtualization software to load our virtual machine (we use [VirtualBox](https://www.virtualbox.org/)).

**_Note:_**
_We have tested the following steps on macOS and Ubuntu._ 








Prerequisites
-------------

**_Note:_**
_If you want to compile the source code, you are expected to have some experience working with C++ and compiling codes that use external libraries._

HARE uses the following libraries, and you need to install all of them. For each of these libraries, we also mention the version that we are currently using. However, there should be no problem with using more recent versions.

1. [GMP 6.1.2](https://gmplib.org/); this is a library for arbitrary precision arithmetic, operating on signed integers, rational numbers, and floating-point numbers.
1. [Z3-4.8.4](https://github.com/Z3Prover/z3/releases); this is a state-of-the-art theorem prover from [Microsoft Research](https://www.microsoft.com/en-us/research/). It can be used to check the satisfiability of logical formulas over one or more theories.
1. [Parma Polyhedra Library (PPL) 1.2](https://www.bugseng.com/parma-polyhedra-library); this is a powerful library for working with polyhedra (for us conjunction of affine constraints).
1. [dReach 3.16.06.02](http://dreal.github.io/dReach/); this is a tool for bounded safety verification of hybrid systems, and we use it for validating counterexamples. Unfortunately, at the time of writing this document, [dReal 4](https://github.com/dreal/dreal4) does not support integration. Therefore, make sure you download and install the right version. The good news is, we use dReach as an executable program and not a library. Therefore, one can download an available precompiled version.
2. [Boost 1.69](https://www.boost.org/users/history/version_1_69_0.html). We use the following libraries. You should compile the first five in this list (they are not header-only libraries).
    1. [File System](https://www.boost.org/doc/libs/1_69_0/libs/filesystem/doc/index.htm);
       Working with files and directories.
    2. [System](https://www.boost.org/doc/libs/1_69_0/libs/system/doc/html/system.html);
       Running [dReach](http://dreal.github.io/dReach/).
    3. [Stacktrace](https://www.boost.org/doc/libs/1_69_0/doc/html/stacktrace.html);
       Printing the call stack in case of an error.           
    4. [Program Options](https://www.boost.org/doc/libs/1_69_0/doc/html/program_options.html);
       Reading the input arguments and different options.    
    5. [Test](https://www.boost.org/doc/libs/1_69_0/libs/test/doc/html/index.html);
       Unit testing.
    6. [Predef](https://www.boost.org/doc/libs/1_69_0/doc/html/predef.html);
       Figuring out the current configuration (e.g., compiler, operating system, and more)
    7. [Preprocessing](https://www.boost.org/doc/libs/1_69_0/libs/preprocessor/doc/index.html);
       Helper C++ macros.
    8. [Algorithm](https://www.boost.org/doc/libs/1_69_0/libs/algorithm/doc/html/index.html);
       String manipulation.
    9. [MPL](https://www.boost.org/doc/libs/1_69_0/libs/mpl/doc/index.html);
       Manipulate types at compile time.
    10. [Property Tree](https://www.boost.org/doc/libs/1_69_0/doc/html/property_tree.html);
       Parsing the input model.
    11. [Iterator](https://www.boost.org/doc/libs/1_69_0/libs/iterator/doc/index.html);
       Also, parsing the input model.
    12. [Optional](https://www.boost.org/doc/libs/1_69_0/libs/optional/doc/html/index.html);
       Presenting optional values.
    13. [Interval](https://www.boost.org/doc/libs/1_69_0/libs/numeric/interval/doc/interval.htm);
       Interval arithmetic.
    14. [Lexical Cast](https://www.boost.org/doc/libs/1_69_0/doc/html/boost_lexical_cast.html);
       Converting strings to integers.

In addition to these libraries, we use [Boost Build](https://boostorg.github.io/build/) to compile the source code and run our tests. Last but not least, we are using [Clang 8.0.0](https://clang.llvm.org/get_started.html) as our compiler.

Before we use these libraries, we should set the environment variable `LD_LIBRARY_PATH` for Linux and `DYLD_LIBRARY_PATH` for macOS.
Here is an example of how it can be done (it is OK to set both `LD_LIBRARY_PATH` and `DYLD_LIBRARY_PATH`).
Make sure to replace paths in the following example with those you installed these libraries in them.
Also, remember that unless you put these in a shell script that is automatically executed every time you open a terminal (e.g., by putting them in `~/.bashrc` in Ubuntu or in `~.bash_profile` in macOS), you will most likely need to run them every time you open a new terminal.

```sh
~$ export GMP_HOME=/opt/c++/libs/gmp-6.1.2-clang-8.0.0
~$ export Z3_HOME=/opt/c++/libs/z3-4.8.4-clang-8.0.0 
~$ export PPL_HOME=/opt/c++/libs/ppl-1.2-clang-8.0.0
~$ export LD_LIBRARY_PATH=$GMP_HOME/lib:$Z3_HOME/lib:$PPL_HOME/lib:$BOOST_HOME/lib:$LD_LIBRARY_PATH
~$ export DYLD_LIBRARY_PATH=$GMP_HOME/lib:$Z3_HOME/lib:$PPL_HOME/lib:$BOOST_HOME/lib:$DYLD_LIBRARY_PATH
```

Since every time you work with HARE, you need to interact with [Boost Build](https://boostorg.github.io/build/), it would be easier if it is included in your `PATH` environment variable. Here is an example showing how this can be done.
```sh
~$ export BOOST_HOME=/opt/c++/libs/boost-1.68-clang-8.0.0
~$ export PATH=$BOOST_HOME/bin:$PATH
```

Finally, we follow a similar approach for dReach and add it to the `PATH` as well.
```sh
~$ export DREAL_HOME=/opt/dReal-3.16.06.02
~$ export PATH=$DREAL_HOME/bin:$PATH
```

Some these tools and libraries come with executable files.
We can use them to check if they are correctly installed.

```sh
~$ $Z3_HOME/bin/z3 --version
Z3 version 4.8.4 - 64 bit
```
```sh
~$ dReach
usage: /opt/dReal-3.16.06.02/bin/dReach options <*.drh> <options to dReal>
dReach: Bounded Model Checking for for Nonlinear Hybrid Systems

OPTIONS:
   -k <N> / -u <N>  specify the upperbound of unrolling steps (default: 3)
   -l <N>           specify the lowerbound of unrolling steps (default: 0)
   -b               use BMC heuristic with disjunctive path encoding (default: do not use)
   -r               -b and filter unreachable modes from SMT encoding (default: do not use)
   -e               -r and filter continuous variables from SMT encoding (default: do not use)
   -d               disjunctive path encoding (default: do not use)
   -z               apply exit codes (default: do not use):
                      51 if SAT,
                      52 if UNSAT,
                      1 abnormal termination
   -n               parse new .drh file format (default: do not use)
   -s               parse new .drh file format and use synchronous encoding (default: do not use)
   
EXAMPLE:
   dReach -k 10      bouncing_ball.drh --verbose --precision 0.001 --visualize
   dReach -l 3 -u 10 bouncing_ball.drh --verbose --precision 0.001 --visualize
```
```sh
~$ b2 -v
Boost.Jam  Version 2018.02. OS=MACOSX.
   Copyright 1993-2002 Christopher Seiwald and Perforce Software, Inc.
   Copyright 2001 David Turner.
   Copyright 2001-2004 David Abrahams.
   Copyright 2002-2015 Rene Rivera.
   Copyright 2003-2015 Vladimir Prus.
```






Compiling the Source Code
-------------------------

As we mentioned in [Prerequisite](#Prerequisite) section,
we use [Boost Build](https://boostorg.github.io/build/) to compile the source code and running our tests. 

1. Clone the source code on your local computer (if you don't have `git`, you can download the source code using your browser as well).
```sh
~$ git clone https://github.com/nima-roohi/HARE
```

1. Enter HARE folder.
```sh
~$ cd HARE
~/HARE$
```

1. File `boost-build.jam` contains exactly one non-empty line that is not a comment.
   Update that line with the correct folder of your Boost Build.
   
   
1. File `jamroot.jam` is the only other file that we need to update before compiling the code.
   1. Update all library path to where you installed yours.
   1. Update Boost's library file names as well.
   
We are ready to compile the code.
```sh
~/HARE$ b2 clang release tests
```
   1. `clang`   select the compiler,
   1. `release` chooses the build type,
   1. `tests`   chooses to build the source code and our unit tests.
    
If the compilation ends successfully, Boost immediately runs the tests. This generates lots of outputs, but a successful result is announced by `ff` at the end of the test. In our experience, the compilation takes about 60 seconds, and running the tests takes about 5 seconds.

 Running the Benchmarks
 ----------------------