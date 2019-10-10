Hybrid Abstraction Refinement Engine (HARE)
===========================================

HARE (Hybrid Abstraction-Refinement Engine) is a counterexample guided abstraction-refinement (CEGAR) based tool to verify safety properties of hybrid automata, whose continuous dynamics in each mode is non-linear, but initial values, invariants, and transition relations are specified using polyhedral constraints. 
HARE works by abstracting non-linear hybrid automata into hybrid automata with polyhedral inclusion dynamics, and uses [dReach](http://dreal.github.io/dReach/) to validate counterexamples. The CEGAR framework forming the theoretical basis of HARE, makes provable progress in each iteration of the abstraction-refinement loop. The current HARE tool is a significant advance on previous versions of HARE; it considers a richer class of abstract models (polyhedral flows as opposed to rectangular flows), and can be applied to a larger class of concrete models (non-linear hybrid automata as opposed to affine hybrid automata). These advances have led to better performance results for a wider class of examples. In the [tool's page](https://nima-roohi.github.io/HARE), we report our experimental comparison of HARE against other state of the art tools for affine models ([SpaceEx](http://spaceex.imag.fr/), [PHAVer](http://spaceex.imag.fr/phaver-8), and [SpaceEx AGAR](https://swt.informatik.uni-freiburg.de/tool/spaceex/agar)) and non-linear models ([FLOW*](https://flowstar.org/), [HSolver](http://hsolver.sourceforge.net/), and [C2E2](http://publish.illinois.edu/c2e2-tool/)).
In the rest of this file, we explain how to install HARE, run benchmarks, and different options for verifying a model.

Virtual Machine
---------------

HARE is written in C++ and uses a few libraries. 
While each of these libraries are perhaps the most famous in what they do, installing all of them just to get a sense of what HARE does might be overwhelming. Therefore, we have prepared a virtual machine that contains everything; source codes, binaries, benchmarks, script for running those benchmarks, as well as what you should expect to see at the end of running those benchmarks. All these can be found at https://uofi.app.box.com/v/HARE. You will need [7-Zip](https://www.7-zip.org/) to extract downloaded files, and a virtualization software to load our virtual machine (we use [VirtualBox](https://www.virtualbox.org/)).


**_Note:_**
_The following steps are tested on MacOS and Ubuntu._ 


Prerequisites
-------------

HARE uses the following libraries. For each of these libraries, we also mention the current version that we are currently using. However, more recent versions should be fine.

1. [GMP 6.1.2](https://gmplib.org/); this is a free library for arbitrary precision arithmetic, operating on signed integers, rational numbers, and floating-point numbers
1. [Z3-4.8.4](https://github.com/Z3Prover/z3/releases); this is a state-of-the art theorem prover from [Microsoft Research](https://www.microsoft.com/en-us/research/). It can be used to check the satisfiability of logical formulas over one or more theories
1. [Parma Polyhedra Library (PPL) 1.2](https://www.bugseng.com/parma-polyhedra-library); this wonderful library is for working with polyhedra (for us conjunction of affine constraint).
1. [dReach 3.16.06.02](http://dreal.github.io/dReach/); this is a tool for bounded safety verification of hybrid systems, and we use it for validating counterexamples. Unfortunately, at the time of writing this document, [dReal 4](https://github.com/dreal/dreal4) does not support integration. Therefore, make sure you download and install the right version. The good news is, we use dReach as an executable program and not a library. Therefore, one can download an available precompiled version.
1. [Boost 1.69](https://www.boost.org/users/history/version_1_69_0.html). We use the following libraries. Note that the first five in this list should be compiled (ie. the are not header only libraries).
    1. [File System](https://www.boost.org/doc/libs/1_69_0/libs/filesystem/doc/index.htm);
       Working with files and directories.
    1. [System](https://www.boost.org/doc/libs/1_69_0/libs/system/doc/html/system.html);
       Running [dReach](http://dreal.github.io/dReach/).
    1. [Stacktrace](https://www.boost.org/doc/libs/1_69_0/doc/html/stacktrace.html);
       Printing the call stack in case of an error.           
    1. [Program Options](https://www.boost.org/doc/libs/1_69_0/doc/html/program_options.html);
       Reading the input arguments and different options.    
    1. [Test](https://www.boost.org/doc/libs/1_69_0/libs/test/doc/html/index.html);
       Unit testing.
    1. [Predef](https://www.boost.org/doc/libs/1_69_0/doc/html/predef.html);
       Figuring out the current configuration (eg. compiler, operating system, ...)
    1. [Preprocessing](https://www.boost.org/doc/libs/1_69_0/libs/preprocessor/doc/index.html);
       Helper C++ macros.
    1. [Algorithm](https://www.boost.org/doc/libs/1_69_0/libs/algorithm/doc/html/index.html);
       String manipulation.
    1. [MPL](https://www.boost.org/doc/libs/1_69_0/libs/mpl/doc/index.html);
       Manipulate types at compile time.
    1. [Property Tree](https://www.boost.org/doc/libs/1_69_0/doc/html/property_tree.html);
       Parsing the input model.
    1. [Iterator](https://www.boost.org/doc/libs/1_69_0/libs/iterator/doc/index.html);
       Also, parsing the input model.
    1. [Optional](https://www.boost.org/doc/libs/1_69_0/libs/optional/doc/html/index.html);
       Presenting optional values.
    1. [Interval](https://www.boost.org/doc/libs/1_69_0/libs/numeric/interval/doc/interval.htm);
       Interval arithmetic.
    1. [Lexical Cast](https://www.boost.org/doc/libs/1_69_0/doc/html/boost_lexical_cast.html);
       Converting strings to integers.

In addition to these libraries, we use [Boost Build](https://boostorg.github.io/build/) to compile the source codes and run our tests. Last but not least, we are using [Clang 8.0.0](https://clang.llvm.org/get_started.html) as our compiler.

Before we use these libraries, we should set the environment variable `LD_LIBRARY_PATH` for linux and `DYLD_LIBRARY_PATH` for MacOS.
Here is an example of how it can be done (it is OK to set both `LD_LIBRARY_PATH` and `DYLD_LIBRARY_PATH`).
Make sure to replace paths in the following example with those you installed these libraries in them.
Also, remember that unless you put these in a shell script that is executed whenever you go to your terminal (eg. by putting them in `~/.bashrc` in Ubuntu or in `~.bash_profile` in MacOS), you will most likely need to run them every time you open a new terminal.

```sh
~$ export GMP_HOME=/opt/c++/libs/gmp-6.1.2-clang-6.0.1
~$ export Z3_HOME=/opt/c++/libs/z3-4.7.1-clang-6.0.1 
~$ export PPL_HOME=/opt/c++/libs/ppl-1.2-clang-6.0.1
~$ export BOOST_HOME=/opt/c++/libs/boost-1.68-clang-6.0.1
~$ export LD_LIBRARY_PATH=$GMP_HOME/lib:$Z3_HOME/lib:$PPL_HOME/lib:$BOOST_HOME/lib:$DYLD_LIBRARY_PATH
~$ export DYLD_LIBRARY_PATH=$GMP_HOME/lib:$Z3_HOME/lib:$PPL_HOME/lib:$BOOST_HOME/lib:$DYLD_LIBRARY_PATH
```

Here are a few tests to verify some of these libraries are correctly installed:
```sh
~$ $Z3_HOME/bin/z3 -v 
Z3 version 4.7.1 - 64 bit
```
```sh
~$ $DREAL_HOME/bin/dReach -v 
Z3 version 4.7.1 - 64 bit
```
```sh
~$ $BOOST_HOME/bin/b2 -v 
Boost.Jam  Version 2018.02. OS=LINUX.
   Copyright 1993-2002 Christopher Seiwald and Perforce Software, Inc.
   Copyright 2001 David Turner.
   Copyright 2001-2004 David Abrahams.
   Copyright 2002-2015 Rene Rivera.
   Copyright 2003-2015 Vladimir Prus.
```

Compiling the Source Code
-------------------------

We use [Boost Build](https://boostorg.github.io/build/) to compile the codes and running our tests. Therefore, we suggest that put it in your `PATH` environment variable.

```sh
~$ export PATH=$BOOST_HOME/bin:$PATH
~$ b2 -v
Boost.Jam  Version 2018.02. OS=LINUX.
   Copyright 1993-2002 Christopher Seiwald and Perforce Software, Inc.
   Copyright 2001 David Turner.
   Copyright 2001-2004 David Abrahams.
   Copyright 2002-2015 Rene Rivera.
   Copyright 2003-2015 Vladimir Prus.
```

**_Note:_**
_If you want to compile the source code, you are expected to have some experience working with C++ and compiling codes that use external libraries._

1. Clone the source code on your local computer (if you don't have git, you can download the source code using your browser as well).
```sh
~$ git clone https://github.com/nima-roohi/HARE
```

1. Enter HARE folder.
```sh
~$ cd HARE
~/HARE$
```

1. File `boost-build.jam` contains exactly one non-empty line that is not a comment.
   Update that line with the correct folder of you Boost Build.
   
   
1. File `jamroot.jam` is the only other file that we need to update before compiling the code.
   1. Update all library path to where you installed yours.
   1. Update Boost library file names as well.
   
We are ready to compile the code.
```sh
~/HARE$ b2 clang release tests
```
   1. `clang`   select the compiler,
   1. `release` chooses the build type,
   1. `tests`   chooses to build the source code and our unit tests.
    
If the compilation ends successfully, Boost immediately run the tests. This will generate a lot of outputs, but successful result will be announced by `ff` at the end of the test. In our experience, the compilation takes about 60 seconds and running the tests takes about 5 seconds.

 Running the Benchmarks
 ----------------------