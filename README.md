mandelbrot
==========

A small collection of programs rendering visual representations of the Mandelbrot-set.
This all started when doing an assignment for the course "Grundlagen der Informatik 3" (rough translation: Foundations of computer science 3) on the "Technische Universität Darmstadt". The assignment involved creating a program to visualize the Mandelbrot set using C, and then, based on that program, creating a SSE-accelerated version.
Since the outcome looked pretty cool and it was fun to write the code, I decided to share the results along with all other Mandelbrot-programs that I did or will create.

#### Overview of implementations ####
Folder name  | Programming language | Description |
------------ | -------------------- | ----------- |
C            | C                    | The "normal C" implementation originally created for the assignment. Doesn't use complex.h, since that wasn't allowed. Includes a GUI and a CLI version.
C+SSE+OpenMP | C (with with SSE intrinsics and OpenMP) | Based on the normal C implementation. Uses SSE intrinsics to accelerate the rendering. Also uses OpenMP to render with multiple threads. Includes a GUI and a CLI version.
C-OpenCL     | OpenCL (C for host)  | This implementation started as a test as I began experimenting with OpenCL for my Raytracer. It is based on the normal C implementation, I basically just "glued" the OpenCL implementation on top. Just a little experiment, probably includes tons of memory leaks. Includes a GUI and a CLI version. Of course you need proper OpenCL support on your host system to run it.
NULLC        | NULLC                | Just a small hacky implementation written in one of my favorite scripting languages, a language called "NULLC". Only includes a GUI version, I recommend to start it from the SuperCalc-IDE. Could probably be made faster by not using "img.DrawPoint" for the pixels.

#### Can I contribute? ####
Of course you can! Whether you want to add your own program or want to improve an existing implementation, every contribution is welcome. Just make sure your implementation runs well on at least Windows (using MinGW or something similar is ok), Linux, and OS X, if possible.
Oh, and it would be nice if all implementations used the same coloring scheme, but you can make that configurable if you want. Just make sure the "standard" coloring scheme is the default.

#### License ####

Everything in this repository is licensed under the GPLv2, which can be found in the LICENSE-file.
