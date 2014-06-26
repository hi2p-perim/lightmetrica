
Lightmetrica
====================

Lightmetrica : A research-oriented renderer

Introduction
--------------------

**Lightmetrica** is a research-oriented renderer mainly for implementing various rendering algorithms. Implementation is written for easy understanding, and easy to profile the performance of the algorithms. All applications are written in C++ language with C++11 standard, and are executable in Windows or Linux environment. Implemented algorithms are written so that users can grasp the relationship between the implementation and the formulation in the original paper.

Features
--------------------

- Implements various types of global illumination techniques (PT, LT, BPT, ~~MLT~~, PM, ~~SPPM~~, ~~UPS/VCM~~, etc.)
- Unit tested
- Includes fast SIMD-based math library
- Various precision for floating-point (single, double, multi-precision)
- Clean and generalized design according Veach's formulation
- Experiments support
- Easily extendable via plugins
- Clean and easy-to-read log output
- Blender plugin

Implementation detail
--------------------

The application contains three projects:

- **liblightmetrica** : The main library in which various rendering algorithms are implemented. The project is configured to compiled by the dynamic library (*.so in Linux, *.dll in Windows). The library is shared by the following projects.
- **lightmetrica** : The command line renderer interface.
- **lightmetrica.gui** : The GUI renderer interface.
- **lightmetrica.test** : Unit test project for **liblightmetrica**.

We *highly* recommend to read the Veach's thesis to understand the implementation.
