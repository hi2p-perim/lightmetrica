
Lightmetrica
====================

Lightmetrica : A research-oriented renderer

[![Build Status](https://travis-ci.org/hi2p-perim/lightmetrica.svg?branch=master)](https://travis-ci.org/hi2p-perim/lightmetrica)

Introduction
--------------------

**Lightmetrica** is a research-oriented renderer mainly for implementing various rendering algorithms. Implementation is written for easy understanding, and easy to profile the performance of the algorithms. All applications are written in C++ language with C++11 standard, and are executable in Windows or Linux environment. Implemented algorithms are written so that users can grasp the relationship between the implementation and the formulation in the original paper.

Features
--------------------

- Implements various types of global illumination techniques
- Moderately unit tested
- Includes fast SIMD-based math library
- Various options for floating-point precisions (single, double, multi-precision)
- Clean and generalized design according to Veach's formulation
- Clean and easy-to-read log output
- Experiments support
- Plugin support

Dependencies
--------------------

Dependencies which requires additional installation step.  
Specially for VS2012 and VS2013 environement in Windows, we offer x64 binaries in the external submodule.

- **Boost** (1.53.0 or higher)
- **Assimp** (3.1.1 or higher)
- **FreeImage** (3.15.4 or higher)

The following dependencies are contained in the source or the external submodule,
so you don't need special setup for them.

- **Google test** (1.7.0 or higher)
- **Pugixml** (1.2 or higher)
- **SIMD-oriented Fast Mersenne Twister (SFMT)** (1.4.1 or higher)

<!--
How to build
--------------------

### Windows

### Linux
-->

Implementation detail
--------------------

This software contains the following projects:

- **Core libraries**
  - `liblightmetrica` : The main library in which various rendering algorithms are implemented. The project is configured to compiled by the dynamic library (*.so in Linux, *.dll in Windows). The library is shared by the following projects.
  - `liblightmetrica.test` : Static library for unit testing.
- **Applications**
  - `lightmetrica` : The command line renderer interface.
  - `lightmetrica.gui` : The GUI renderer interface. Yet to be implemented.
- **Tests**
  - `lightmetrica.test` : Unit test project for `liblightmetrica`.
  - `lightmetrica.perf` : Performance test project for `liblightmetrica`. Yet to be implemented.
- **Plugins**
  - `plugin.test` : An example plugin 

Licence
--------------------

This software is distributed under GPLv3. For details, see LICENCE file.