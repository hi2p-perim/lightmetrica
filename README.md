
nanon : A research-oriented renderer
====================

Introduction
--------------------

**nanon** is a research-oriented renderer mainly for implementing various rendering algorithms.
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

- **libnanon** : The main library in which various rendering algorithms are implemented. The project is configured to compiled by the dynamic library (*.so in Linux, *.dll in Windows). The library is shared by the following projects.
- **nanon** : The command line renderer interface.
- **nanon.gui** : The GUI renderer interface.
- **nanon.test** : Unit test project for **libnanon**.    

We *highly* recommend to read the Veach's thesis to understand the implementation.

License
--------------------

Copyright (C) 2014 Hisanari Otsu

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.