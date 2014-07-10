
Lightmetrica
====================

Lightmetrica : A research-oriented renderer

Introduction
--------------------

**Lightmetrica** is a research-oriented renderer mainly for implementing various rendering algorithms. Implementation is written for easy understanding, and easy to profile the performance of the algorithms. All applications are written in C++ language with C++11 standard, and are executable in Windows or Linux environment. Implemented algorithms are written so that users can grasp the relationship between the implementation and the formulation in the original paper.

Features
--------------------

- Implements various types of global illumination techniques
- Moderately unit tested
- Includes fast SIMD-based math library
- Various options for floating-point precisions (single, double, multi-precision)
- Clean and generalized design according Veach's formulation
- Clean and easy-to-read log output
- Experiments support
- Plugin support
<!--- Blender plugin-->

<!--
How to build
--------------------

### Windows

### Linux
-->

Implementation detail
--------------------

This software contains the following projects:

- Core libraries
  - **liblightmetrica** : The main library in which various rendering algorithms are implemented. The project is configured to compiled by the dynamic library (*.so in Linux, *.dll in Windows). The library is shared by the following projects.
  - **liblightmetrica.test** : Static library for unit testing.
- Applications
  - **lightmetrica** : The command line renderer interface.
  - **lightmetrica.gui** : The GUI renderer interface. Yet to be implemented.
- Tests
  - **lightmetrica.test** : Unit test project for **liblightmetrica**.
  - **lightmetrica.perf** : Performance test project for **liblightmetrica**. Yet to be implemented.
- Plugins
  - **plugin.test** : An example plugin 

Licence
--------------------

This software is distributed under GPLv3. For details, see LICENCE file.