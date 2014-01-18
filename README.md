
Lightmetrica : A research-oriented renderer
====================

Introduction
--------------------

**Lightmetrica** is a research-oriented renderer mainly for implementing various rendering algorithms.
In order to concentrate the algorithm itself, the renderer is not optimized well, e.g. it does not contains neither parallel implementation nor SIMD optimization. Instead, the implementation is written for easy understanding, and easy to profile the performance of the algorithms. All applications are written in C++ language with C++11 standard, and are executable in Windows or Linux environment. The implemented algorithms are written so that you can grasp the relationship between the implementation and the formulation in the original paper.

<!--
How to build
--------------------

### Windows

### Linux
-->

Implementation detail
--------------------

The application contains three projects:

- **liblightmetrica** : The main library in which various rendering algorithms are implemented. The project is configured to compiled by the dynamic library (*.so in Linux, *.dll in Windows). The library is shared by the following projects.
- **lightmetrica** : The command line renderer interface.
- **lightmetrica.gui** : The GUI renderer interface.
- **lightmetrica.test** : Unit test project for **libnanon**.    

We *highly* recommend to read the Veach's thesis to understand the implementation.
