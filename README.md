# rGWB
An implementation of Martti Mäntyllä's Geometric WorkBench

![Test case](https://github.com/mfernba/rGWB/Screenshots/virtual sector test.png)

IMPORTANT: Some parts of the system are tied to other tools I developed in the past, so don't expect (for now) that it compiles for you. 
You will need to provide your own implementation for the csmdebug.c.inl module and for the csmviewer.c.h module (or comment both). 
Also, the visualization extensions must be provided by you (I use an own developed scene graph). This only affects a few modules.

This is a working implementation of Martti Mäntyllä's Geometric WorkBenchk. The Geometric Workbench (also known as GWB) is a 
polyhedral boundary representation geometric modeler for 2-Manifold solids. 

For a long time, the source code developed for the book could be found at Martti Mäntyllä's personal web page (http://www.cs.hut.fi/~mam/), but this code is old and incomplete, and the code in book sometimes is buggy (I suppose it's objective was to be illustrative -and really, it is-, not correct). 

I have studied the book and some of the papers it refers to, also I have read "Boundary Representation Modelling Techniques" by Ian Stroud. Starting from the code of the book (and original Mäntyllä source code, specially for vertex neighbour classification), I have reimplemented the core of the modeler.

This implementations allows:

  - Sweep solids by extrusion or along a path (csmsweep.h, csmshape2d.h, csmbasicshape2d.h)
  - Quadrics solids (csmquadrics.h)
  - Split algorithm (csmsplit.h)
  - Boolean Operations (csmsetop.h)  
  - Explode solids with several shells (csmexplode.h)
  
Look at Screenshots to get some sense of what can be done.

Non-manifold solids: This is a 2-manifold modeler, but non-manifold solids can be produced as a consequence of the solid operation used. In such case, the algorithms may fail if you use the solid in another operations. This is an extension I will do in the future.

I have used xcode to do the development, and the code is marked as objective-c for compiling the c source. Well, I will improve that in
the future.
