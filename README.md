# rGWB
An implementation of Martti Mäntylä's Geometric WorkBench

![Test case](https://github.com/mfernba/rGWB/blob/master/Screenshots/Cylindrical%20part%20with%20holes%20.png)

This is a working implementation of Martti Mäntylä's Geometric WorkBenchk. The Geometric Workbench (also known as GWB) is a polyhedral boundary representation geometric modeler for 2-Manifold solids. 

For a long time, the source code developed for the book could be found at Martti Mäntylä's personal web page (http://www.cs.hut.fi/~mam/), but this code is old and incomplete, and the code in book sometimes is buggy (I suppose it's objective was to be illustrative -and really, it is-, not correct). 

This implementations allows:

  - Sweep solids by extrusion or along a path (csmsweep.h, csmshape2d.h, csmbasicshape2d.h)
  - Quadrics solids (csmquadrics.h)
  - Split algorithm (csmsplit.h)
  - Boolean Operations (csmsetop.h)  
  - Explode solids with several shells (csmexplode.h)
  
Look at Screenshots to get some sense of what can be done.

![Test case](https://github.com/mfernba/rGWB/blob/master/Screenshots/Virtual%20sector%20test.png)

# Implementation notes

IMPORTANT: The project has been developed using xcode and some parts of the system are tied to other tools I developed in the past, so don't expect (for now) that it compiles for you. You will need to provide your own implementation for the csmdebug.c.inl module and for the csmviewer.c.h module (or comment both). Also, the visualization extensions must be provided by you (I use an own developed scene graph). This only affects a few modules and should be easy to make it compile (I tested that).

I have reimplemented the core of the modeler by using code exposed on the the book and taking (and extending) some parts from original Mäntylä source code (part of the euler operators to match the expected algorithm behaviour and the vertex neighborhood classification strategy -which I needed to complete-). 

The last resort of the null edge joining algorithm is based on: Bhardwaj, A. & Malik, I. (1997). Java Applet for Constructive Solid Geometry. Technical report, Computer Science Department, Cornell University. I was pointed to this source by the paper: "Reflections on the implementation of boolean operations with polyhedral solids" by Herbert J. Koelman from SARC BV.

Mäntylä's algorithm need maximal faces, so there is a simplification strategy both in solid generation and after solid operations in order to eliminate unessential elements.

A perturbation strategy is applied if the algorithm doesn't produce an output (it can't join all null edges). This could produce an incorrect output in some cases, but it's a last resort. Also, it reduces the possibility to get a non-manifold solid.

"Boundary representation modeling techniques" from Ian Stroud was also an unvaluable source of information, providing guidance about how to orient the implementation.
