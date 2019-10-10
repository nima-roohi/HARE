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
Make sure to replace paths in the following examples with those you installed these libraries in them.
Also, remember that unless you put these in a shell script that is automatically executed every time you open a terminal (e.g., by putting them in `~/.bashrc` in Ubuntu or in `~.bash_profile` in macOS), you will most likely need to run them every time you open a new terminal.

For the rest of this section, we assume you are in your home folder and use `~` to denote that folder.

```sh
~$ export GMP_HOME=/opt/c++/libs/gmp-6.1.2-clang-8.0.0
~$ export Z3_HOME=/opt/c++/libs/z3-4.8.4-clang-8.0.0 
~$ export PPL_HOME=/opt/c++/libs/ppl-1.2-clang-8.0.0
~$ export BOOST_HOME=/opt/c++/libs/boost-1.68-clang-8.0.0
~$ export LD_LIBRARY_PATH=$GMP_HOME/lib:$Z3_HOME/lib:$PPL_HOME/lib:$BOOST_HOME/lib:$BOOST_HOME/lib:$LD_LIBRARY_PATH
~$ export DYLD_LIBRARY_PATH=$GMP_HOME/lib:$Z3_HOME/lib:$PPL_HOME/lib:$BOOST_HOME/lib:$BOOST_HOME/lib:$DYLD_LIBRARY_PATH
```

Since every time you work with HARE, you need to interact with [Boost Build](https://boostorg.github.io/build/), it would be easier if it is included in your `PATH` environment variable. Here is an example showing how this can be done.
```sh
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

As we mentioned in [Prerequisites](#Prerequisites) Section,
we use [Boost Build](https://boostorg.github.io/build/) to compile the source code and run our tests. The following steps are the same for macOS and Ubuntu.

1. Open a terminal and go to a folder you want to download and compile HARE.
   For the rest of this section, we use `~` to denote that folder.

1. Clone the source code on your local computer (if you don't have `git`, you can download the source code using your browser as well).
    ```sh
    ~$ git clone https://github.com/nima-roohi/HARE
    Cloning into 'HARE'...
    remote: Enumerating objects: 655, done.
    remote: Counting objects: 100% (655/655), done.
    remote: Compressing objects: 100% (321/321), done.
    remote: Total 655 (delta 323), reused 583 (delta 258), pack-reused 0
    Receiving objects: 100% (655/655), 8.10 MiB | 4.89 MiB/s, done.
    Resolving deltas: 100% (323/323), done.
    ```

1. Enter HARE folder.
    ```sh
    ~$ cd ./HARE
    ~/HARE$
    ```

1. File `boost-build.jam` contains exactly one non-empty line that is not a comment.
   Update that line with the correct folder of your Boost Build.
   For us that line looks as follows. Notice that there is a space between `kernel` and `;`.
    ```sh
    boost-build /opt/c++/libs/boost-1.69-clang-8.0.0/share/boost-build/src/kernel ;
    ```   

1. `Jamroot.jam` is the only other file that we need to update before compiling the code.
   1. Update the Clang version (Line 17).
   1. Update the address to your Clang compiler (Line 18).
   1. Update all library paths to where you installed yours (Lines 28-35, 42-45, and 55-58).
   1. Update Boost's library file names as well (Lines 42-45).

1.  We are ready to compile the code and run our tests.
    ```sh
    ~/HARE$ b2 clang release tests
    ```
    1. `clang`   selects the compiler,
    2. `release` chooses the build type,
    3. `tests`   chooses to build the source code and our unit tests.
    
If the compilation ends successfully, Boost immediately runs the tests. This generates lots of outputs, but a successful result is announced by `No errors detected` at the end of execution. In our experience, the compilation takes about 4 minutes, and running the tests takes about 20 seconds. Note that if all tests pass and you want to rerun them, you have to find the file `release/tests.passed` in your `HARE/bin` folder first and remove it. After that, `~/HARE$ b2 clang release tests` runs the tests without recompiling the source code.




Running the Benchmarks
----------------------

If you have successfully run the tests, then running the benchmarks should be an easy process.

1. Compile the file that is going to run all benchmarks.
    ```sh
    ~/HARE$ b2 clang release bench
    ```
    In our experience, this takes about 40 seconds.
2. Run the benchmarks. 
   Make sure to replace `clang-darwin-8.0.0` with the right folder in your command.
    ```sh
    ~/HARE$ ./bin/clang-darwin-8.0.0/release/bench
    ```
    There are currently 85 examples set up to run.
    In our experience, this takes about 22 minutes.
    You are going to see different steps HARE takes to do its job. Also, a huge table, summarizing all the metrics HARE collected for each benchmark during the execution, is going to be printed at the end of the test.
    This repository contains an example of such [output](https://github.com/nima-roohi/HARE/blob/master/benchmarks/log-20191009).
    Also, [this](https://github.com/nima-roohi/HARE/blob/master/benchmarks/metrics-20191009) file contains the metrics we got during our experiments.




Running Other Examples
-----------------------

In order to run other examples, we still need to compile one more file; `command_line.cpp`. Fortunately, that is quite simple.
```sh
~/HARE$ b2 clang release hare
```
In our experience, this takes less than a minute (assuming you did not delete whatever that is compiled in the previous steps).

Next, we check if our build was successful.
```sh
~/HARE$ ./bin/clang-darwin-8.0.0/release/hare --version
Hybrid Abstraction Refinement Engine (HARE) version 0.3-2019.10.09
```

Execution of HARE must be adjusted using options and switches.
There are two mains categories:
Switches set in the command line and 
switches set in the input file.
In the rest of this file we explain different switches in each of these categories.

### Command line Switches

1. `--version`;
   Print version of the tool and exist.

1. `--help` or `h`; 
   Show the usage (i.e., list of command-line switches and a short description for each one of them). 

1. `--polyhedral`;
   Input file specifies a polyhedral hybrid automaton (everything is polyhedron).

1. `--non-linear`;
   Input file specifies a non-linear hybrid automaton (everything is polyhedron except flows that are either affine or non-linear ODEs).

1. `--verbosity arg` or `-v arg` (default value for the argument is `debug`);
   Set the log level.
   Possible values are `trace`, `debug`, `info`, `warn`, `error`, `fatal`, and `off`.    

1. `--file arg` or `-f arg`;
   Set the input file.

1. `--metrics` or `-m`;
   Print performance metrics after a successful termination.

1. `--output arg` or `-o arg` (the default value for the argument is `safety`);
   Kind of information to be generated on a successful termination. 
   Possible values are:
   1. `safety`: whether or not the system is safe.
   1. `counter-example`: in addition to the previous part, 
      in case the input was unsafe, a counter-example 
      (sequence of edges) will also be generated.
   1. `annotated-counter-example`: in addition to the previous part, 
      in case the input was unsafe, (abstract) states reachable along 
      with the generated counterexample will also be generated.
   1. `reachable-set`: (abstract) reachable states, 
      whether or not the system is found to be safe, will be printed out.

1. `--batch` or `-b`;
   Batch processing.
   Non-empty lines in the input file that are not started with the character `#`, each must contain four comma-separated values:
     1. name (need not be unique),
     1. type of input (`polyhedral` or `non-linear`),
     1. expected answer (`safe`, `unsafe`, `bounded-safe`, `unknown`, or `?`).
        1.  Expected answer `unknown` means the tool could not decide, and it 
            only makes sense if the input type is `non-linear`.
        1.  Expected answer `?` means there is no expected answer.
     2. (full) path to the model file.

#### Examples:
Don't forget to replace `clang-darwin-8.0.0` with the right folder in the following examples.

1. Simply model check a file.
    ```sh
    ~/HARE$ ./bin/clang-darwin-8.0.0/release/hare  \
    --file ./benchmarks/tanks/tank-01/tank.prb --polyhedral
    ```
2. Model check the same file, but print the counterexample.
    ```sh
    ~/HARE$ ./bin/clang-darwin-8.0.0/release/hare   \
    --file ./benchmarks/tanks/tank-01/tank.prb      \
    --polyhedral --output counter-example
    ```
3. Model check the same file, but print the counterexample along with the states that are reached using that counterexample.
    ```sh
    ~/HARE$ ./bin/clang-darwin-8.0.0/release/hare   \
    --file ./benchmarks/tanks/tank-01/tank.prb      \
    --polyhedral --output annotated-counter-example
    ```
4. Print the metrics.
    ```sh
    ~/HARE$ ./bin/clang-darwin-8.0.0/release/hare   \
    --file ./benchmarks/tanks/tank-01/tank.prb      \
    --polyhedral --metrics
    ```
5. Set verbosity to its highest value (we changed the input file in this example. It takes about 30 seconds to prove the system is safe).
    ```sh
    ~/HARE$ ./bin/clang-darwin-8.0.0/release/hare   \
    --file ./benchmarks/navigation/nav-01/nav.prb  \
    --non-linear --verbosity trace
    ```
6. Bach processing.
   Let's first create a sample input file.
    ```sh
    echo "ex1, polyhedral, unsafe, ./benchmarks/tanks/tank-01/tank.prb" >  ./hare.batch 
    echo "ex2, polyhedral,   ?   , ./benchmarks/tanks/tank-02/tank.prb" >> ./hare.batch 
    echo "ex3, polyhedral, unsafe, ./benchmarks/tanks/tank-03/tank.prb" >> ./hare.batch 
    echo "ex4, polyhedral,   ?   , ./benchmarks/tanks/tank-04/tank.prb" >> ./hare.batch 
    echo "ex5, non-linear,  safe , ./benchmarks/navigation/nav-01/nav.prb" >> ./hare.batch 
    echo "ex6, non-linear, unsafe, ./benchmarks/navigation/nav-02/nav.prb" >> ./hare.batch 
    echo "ex7, non-linear, unsafe, ./benchmarks/navigation/nav-03/nav.prb" >> ./hare.batch 
    echo "ex8, non-linear,   ?   , ./benchmarks/navigation/nav-04/nav.prb" >> ./hare.batch 
    echo "ex9, non-linear,  safe , ./benchmarks/navigation/nav-05/nav.prb" >> ./hare.batch 
    ```
    ```sh
    ~/HARE$ ./bin/clang-darwin-8.0.0/release/hare        \
    --file ./hare.batch --batch --verbosity info --metrics
    ```
    In our experience, this example takes about 70 seconds to terminate.

### Input File Options
Each input file ends in a section called `props`.
It contains different settings for how HARE should do its job.
Please look at `./benchmarks/navigation/nav-01/nav.prb` as an example.
In the following, we use `::` to write each property in a single line.
For example, we write `mn-poly::direction` to refer to the `direction` property inside `mc-poly`.

1.  `mc-poly::direction`.
    Possible values are `forward`, `backward`, `smaller-or-forward`, and `smaller-or-backward`.
    Default value is `smaller-or-forward`.
    Whether forward reachability should be used or backward reachability.

1. `mc-poly::separate-identity-resets`.
    Possible values are   
      `true` and
      `false`.
    Default value is `true`.
    Tells HARE that during interaction with PPL, do not introduce new variables for identity resets (i.e., a variable that is reset to itself on a discrete transition).

2. `mc-poly::check-unsafe-after-disc-edge`.
    Possible values are `true` and `false`.
    Default value is `false`.
    Tells HARE to test intersection with the unsafe set after every discrete transition in addition to after every continuous transition.


3. `mc-poly::add-to-visiteds-on-check`.
    Possible values are `true` and `false`.
    Default value is `false`.
    Tells HARE whether or not it should immediately adds input sets to the set of visited states whenever it checks if they have been reached before. `false` delays adding new states to the set of visited states a little bit.

4. `mc-poly::max-iter`.
    Possible values are non-negative integers. 
    Default value is `0`.
    How many iterations HARE should take before it gives up its goal for reaching to a fixed-point in the abstract system.

5. `mc-nlfpoly::bound-cont-trans`.
    Possible values are `true` and `false`.
    Default value is `true`.
    Tells HARE whether or not continuous transitions in the abstract system should be bounded. Setting the value of this parameter to `false` generates a warning because it becomes the user's responsibility to make sure that either there is a bound in the abstract automata anyway, or the concrete model checker can handle infinite time transitions.

6.  `mc-nlfpoly::use-empty-labels-for-bounding-time`.
    Possible values are `true` and `false`.
    Default value is `true`.
    Tells HARE whether or not an empty label should be used for transitions that bound time. In Parallel composition this reduces the number of discrete transitions.

7.  `mc-nlfpoly::connect-split-locs`.
    Possible values are `true` and `false`.
    Default value is `true`.
    Whether or not refinement should add edges between the split locations. 

    Duration of continuous transitions are bounded in the abstract system by initially adding self loops to the concrete system. When hybrid automata have those special-purpose self-loops with empty labels (so they won't sync with other edges) split locations remain connected. 

    Setting values of both `mc-nlfpoly::bound-cont-trans` and `connect-split-locs` to `false` generates a warning. Because it becomes the user's responsibility to make sure that after splitting locations of the automata under consideration, the split locations remain connected.
    
    At the time of writing this comment, this property will be ignored if `mc-nlfpoly::bound-cont-trans` and `mc-nlfpoly::use-empty-labels-for-bounding-time` are both set to `true`.

8.  `mc-nlfpoly::bound-cont-trans-by-eq`
    Possible values are `true` and `false`.
    Default value is `true`.
    Whether or not new edges that bound the time should use equality in their guards (as opposed to non-strict inequality).

9.  `mc-nlfpoly::cont-tran-duration`. 
    Possible values are positive rational numbers.
    Default value is `10`.
    The bound on duration on each continuous transition. 
    Even if `mc-nlfpoly::bound-cont-trans` is `false`, this parameter is used in verifying counterexamples. If `mc-nlfpoly::bound-cont-trans` is `true`, this parameter is also used to force the duration of continuous transitions in abstract models.
  
10. `mc-nlfpoly::linear-flow-abstraction`
    Possible values are `polyhedronize` and `rectangularize`.
    Default value is `polyhedronize`.
    When input dynamics is affine (and not linear), this option can be used to specify what should be used for abstract dynamics.
  
11. `mc-nlfpoly::initial-refinement-count`
    Possible values are non-negative integers.
    Default value is `0`.
    Sometimes it is very helpful to immediately split locations.
    This option tells HARE how many of those blind splitting should happen at the beginning.

12. `mc-nlfpoly::max-iter`
    Possible values are non-negative integers.
    Default value is `0`.
    Maximum number of times HARE tries to find a new counterexample before it gives up and tell the user result is `unknown`.

