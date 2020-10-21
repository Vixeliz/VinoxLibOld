# Vinox
A simple 2d framework for rendering graphics in a GLESv2 context.

Heavily inspired by [raylib](https://github.com/raysan5/raylib)

What are the goals/Ideas?
--------------------
* Try to keep a small managable base that is flexible but not lacking in features.
* Try to minimize external library dependencies if possible use single header file libraries
* 2D Only(This is just my personal preference I have no need for 3d and if I do I can just directly call opengl)
* **Minimize** reliance on dynamic memory allocation
* Be able to use in any GLES2 context.

Thanks
------
Thank you to the following sources for either inspiration or an amazing header that is used in this project!
* Raylib(inspired by and uses raymath)
* stb_headers(image loading)
* miniaudio(audio)

Other Resource
--------------
* Cute_headers - you can find useful header libs there such as cute_c2

Why GLES2?
----------
It is portable and specifically for my use case I wanted a library that worked with xorg libs on wayland.

Dependencies
------------
* GLES2(With vao extension support; however, always willing to have it be optional if I get around to it)
* EGL
* [glfw](https://github.com/glfw/glfw)(optional for examples)

Plans
-----
* Scripting via lua depending on complexity.
* Golang support
* Audio
* Collision
* Update
* Having an optional single file header library for window/input related things with glfw
* More types of shapes

Notes
------
The examples are coded very poorly. I plan on going back through and cleaning them up but currently I just want to get features in.

Raymath is slightly modified currently just a Vector2Transform

Current Limitations
--------------------
* No audio
* Optimizing changing data to only when it's changed for example camera
