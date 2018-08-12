# C Scripting Language
## What is `CSL`?
`CSL` is a simple lisp-like language for scripting high-level logic
and data structures in a way that interfaces easily to the `C` programming
language.

You don't want to use this language - it's a work-in-progress and not
yet fit to use. You are encouraged to use one of the alternatives listed
below.

## Alternatives
If you really were looking for a scripting language for your `C` programs
then you should avoid `CSL` and use one of the existing scripting
languages. I provide a short list below - you can easily find out more
about each alternative with a brief search on the internet:

1. `Python` is pretty popular as a scripting language, although
   interfacing `Python` to `C` involves writing wrappers around your `C`
   functions if you want to call them from `Python` and, conversely, writing
   some wrappers for your `Python` code if you want to call it from `C`.
   `Python` is strongly typed in a useless sort of way because you only
   discover type errors at runtime, which defeats the purpose of static
   typing in the first place.

   More recently, a little progress has been made in the `Python` community
   towards performing type-checking prior to runtime.

1. `Lua` has enjoyed considerable success as a scripting language for `C`
   and `C++`. Interfacing between `Lua` and `C` is pretty complicated and
   apparently changes very often when `Lua` changes a major version
   number. `Lua` is more suited to running entire scripts from `C` rather
   than running individual functions (although it can do both, the latter
   is more involved). Since that is exactly what most programmers want
   from a scripting language this is no big deal.

   `Lua` has found quite a successful niche as the logic and event
   scripting engine for games. Make of that what you will.

1. `Tcl` is one of the older scripting languages. While it is pretty
   simple to start with and understand it is fairly limited due to its
   type system which uses all data as either strings or lists of strings.
   Nevertheless, it interfaces with `C` extremely well and it does by
   default also come with the `Tk` Graphical User Interface library.

   `Tcl/Tk` has been used with moderate success in many embedded environments,
   although its usage has dropped in the last decade.

1. If you are looking specifically for a lisp-ish language then you can't
   go wrong with either `ECL` (Embeddable Common Lisp) or `GCL` (Gnu
   Common Lisp). They both have exceptional `C` interface capabilities due
   to the way they work. `ECL` and `GCL` both **compile** their input `Lisp`
   source into ANSI-C, which is then passed off to gcc for compilation
   into native code.

   This has superb performance benefits as the scripted logic is then run
   as native code. Another side-effect of compiling to `C` is that the
   script code can contain `C` snippets directly in the source of the
   script; this makes interfacing to `C` a walk-in-the-park when compared
   to the interfacing requirements of the other scripting languages.

   The downside is that you'll have to learn `Lisp` (come to think of it,
   you'll learn `Lisp` anyway, when it's disguised as an imperative
   language).

1. `CINT` is a `C` interpreter. This means that your scripting language
   is the same language as your host language (either `C` or `C++`).
   While I haven't personally used it, reports of its conformance level
   with pre-C99 is quite good. It apparently also makes interfacing with
   `C` a breeze.


## CSL building
If you still want to try out `CSL` in spite of my advice against doing so,
you can compile simply by typing 'make'. On Windows you need to have
`MingW32` or `MingW64` installed and you need to make sure that the gcc is in
the path. You also need to set the environment variable MINGW_LOCATION to
point to the path that `MingW` is installed in.

Further information about the build process is in the file README.build.

At this point in time the development of `CSL` is still in progress. It
will compile and leave a test-case in the `rt/` directory. Running that
executable will cause the test script `rt/test_input.csl` to be executed in
the interpreter. The test script is designed to exercise most of the
functionality that is being added to the interpreter (so it will drop into
the built-in debugger for some of the tests).

Note that the final output of the build is a set of libraries to be used
by the host program. A general interpreter that can be run from the
command line will be produced in due course.

## CSL capabilities
Right now scripts are written in a lisp-ish syntax. The actual syntax is
not going to change. The interpreter (embedded into the application) will
allow scripts to:
1. Call the built in functions (there is no documentation on this yet),
1. Evaluate using the built-in operators (arithmetic, boolean, bitwise),
1. Define global variables using a 'define' built-in function,
1. Create local variables with a 'let' built-in function,
1. Create functions, and then store them as a global symbol (this means
   that functions are by definition anonymous),
1. Evaluate symbols (including functions) from both global and local
   scope,
1. Recursively call functions,
1. Set, clear and generate traps for error conditions,
1. Invoke the built-in debugger to examine program state when a trap is
   unhandled.
1. Call functions in external library.dll or library.so files, subject to
   some restrictions (for example, cannot pass arbitrary-length buffers to
   the external functions, such as `fread`).

Admittedly that's not an impressive list of capabilities, but I did warn
you to use something else!

Right now, I'm just trying to get to a point where the script can define
functions compatible with `C` structs and have enough runtime support to
define arbitrarily large buffers that many of the `C` standard functions
use (`fread`, `fwrite`, etc).

## Missing capabilities... coming soon<sup>(tm)</sup>
This project is now only a few weeks old, and as such is missing a bunch
of critical functionality. I'd advise going with an alternative.

Nevertheless, if you do find yourself messing with this here's what you
are not going to find (yet):
1. Looping capability (can be faked with recursion if primitives `first`
   and `rest` existed),
1. Primitives to enable recursing across a list (superfluous if looping
   constructs existed),
1. Creating byte-buffers needed for interfacing to many `C` libraries,
1. Defining `C` style `structs` so that the scripts can interface to many
   `C` libraries that take structures,
1. A garbage collection strategy. Currently there is no need for one as
   the script executes strictly top-down in the tree, and each return from
   an expression cleans up after itself,
1. Trapping based on signals (SIGSEGV, SIGBUS, etc). While the script
   cannot recover from such a thing, it can dump the runtime and
   call-stack before ending.

I do not know when any of the above will be finished, or even started.
Don't hold your breath.

## Licence
```
Copyright (c) 2018, Lelanthran Manickum
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the project  nor the names of its contributors
      may be used to endorse or promote products erived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY  EXPRESS OR  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL LELANTHRAN  MANICKUM BE  LIABLE FOR  ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING  NEGLIGENCE OR OTHERWISE) ARISING  IN ANY  WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
