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
While each of those libraries are perhaps the most famous in what they do, installing all of them just to get a sense of what HARE does might be overwhelming. Therefore, we have prepared a virtual machine that contains everything; source codes, binaries, benchmarks, script for running those benchmarks, as well as what you should expect to see at the end of running those benchmarks. All these can be found at https://uofi.app.box.com/v/HARE.

Prerequisites
-------------

