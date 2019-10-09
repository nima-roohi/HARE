Hybrid Abstraction Refinement Engine (HARE)
===========================================

HARE (Hybrid Abstraction-Refinement Engine) is a counterexample guided abstraction-refinement (CEGAR) based tool to verify safety properties of hybrid automata, whose continuous dynamics in each mode is non-linear, but initial values, invariants, and transition relations are specified using polyhedral constraints. 
HARE works by abstracting non-linear hybrid automata into hybrid automata with polyhedral inclusion dynamics, and uses [dReach](http://dreal.github.io/dReach/) to validate counterexamples. The CEGAR framework forming the theoretical basis of HARE, makes provable progress in each iteration of the abstraction-refinement loop. The current HARE tool is a significant advance on previous versions of HARE; it considers a richer class of abstract models (polyhedral flows as opposed to rectangular flows), and can be applied to a larger class of concrete models (non-linear hybrid automata as opposed to affine hybrid automata). These advances have led to better performance results for a wider class of examples. In the tool's [page](https://nima-roohi.github.io/HARE), we report our experimental comparison of HARE against other state of the art tools for affine models ([SpaceEx](http://spaceex.imag.fr/), [PHAVer](http://spaceex.imag.fr/phaver-8), and [SpaceEx AGAR](https://swt.informatik.uni-freiburg.de/tool/spaceex/agar)) and non-linear models ([FLOW*](https://flowstar.org/), [HSolver](http://hsolver.sourceforge.net/), and [C2E2](http://publish.illinois.edu/c2e2-tool/)).
Here we explain how to install and run benchmarks as well as examples.

**_Note:_**
_The following steps are tested on MacOS and Ubuntu._ 


Virtual Machines
----------------

Hare is written in C++ and uses different libraries. 
While each of those libraries are perhaps the most famous in what they do, installing all of them just to get a sense of what HARE does might be overwhelming. Therefore, we have prepared a virtual machine that contains everything; source codes, binaries, benchmarks, script for running those benchmarks, as well as what you should expect to see at the end of running those benchmarks. All these can be found at https://uofi.app.box.com/v/HARE. You will need [7-Zip](https://www.7-zip.org/) to extract downloaded files, and a virtualization software to load our virtual machine (we use [VirtualBox](https://www.virtualbox.org/)).

Prerequisites
-------------

HARE uses the following libraries. For each of these libraries, we also mention the current version that we are using right now. However, using more recent versions should be fine.

1. [GMP 6.1.2](https://gmplib.org/); this is a free library for arbitrary precision arithmetic, operating on signed integers, rational numbers, and floating-point numbers
1. [Z3-4.8.4](https://github.com/Z3Prover/z3/releases); this is a state-of-the art theorem prover from [Microsoft Research](https://www.microsoft.com/en-us/research/). It can be used to check the satisfiability of logical formulas over one or more theories
1. [Parma Polyhedra Library (PPL) 1.2](https://www.bugseng.com/parma-polyhedra-library); this wonderful library is for working with polyhedra (for us conjunction of affine constraint).
1. [dReach 3.16.06.02](http://dreal.github.io/dReach/); this is a tool for bounded safety verification of hybrid systems, and we use it for validating counterexamples. Unfortunately, at the time of writing this document, [dReal 4](https://github.com/dreal/dreal4) does not support integration. Therefore, make sure you download and install the right version.
1. [Boost 1.69](https://www.boost.org/users/history/version_1_69_0.html). We use the following libraries. Note that the first five in this list should be compiled (ie. the are not header only libraries).
    1. [File System](https://www.boost.org/doc/libs/1_69_0/libs/filesystem/doc/index.htm)   
    1. [System](https://www.boost.org/doc/libs/1_69_0/libs/system/doc/html/system.html)        
    1. [Stacktrace](https://www.boost.org/doc/libs/1_69_0/doc/html/stacktrace.html)               
    1. [Program Options](https://www.boost.org/doc/libs/1_69_0/doc/html/program_options.html)      
    1. [Test](https://www.boost.org/doc/libs/1_69_0/libs/test/doc/html/index.html)
    1. [Predef](https://www.boost.org/doc/libs/1_69_0/doc/html/predef.html)
    1. [Preprocessing](https://www.boost.org/doc/libs/1_69_0/libs/preprocessor/doc/index.html)
    1. [Algorithm](https://www.boost.org/doc/libs/1_69_0/libs/algorithm/doc/html/index.html)
    1. [MPL](https://www.boost.org/doc/libs/1_69_0/libs/mpl/doc/index.html)
    1. [Property Tree](https://www.boost.org/doc/libs/1_69_0/doc/html/property_tree.html)
    1. [Iterator](https://www.boost.org/doc/libs/1_69_0/libs/iterator/doc/index.html)
    1. [optional](https://www.boost.org/doc/libs/1_69_0/libs/optional/doc/html/index.html)
    1. [Interval](https://www.boost.org/doc/libs/1_69_0/libs/numeric/interval/doc/interval.htm)
    1. [Lexical Cast](https://www.boost.org/doc/libs/1_69_0/doc/html/boost_lexical_cast.html)
