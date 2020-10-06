# Vinox
A simple 2d framework for rendering graphics in a GLESv2 context.

Why?
-----
Mainly for learning GLESv2 more however I also wanted to make a framework like raylib that doesn't
make assumptions about windowing or input. All functions are purely for rendering and should be called
inside any GLESv2 context up the the user. For example glfw could be used for desktop or SDL2, etc.

What are the goals?
--------------------
I am primarly making this for gamedev as it is what I enjoy doing; however, I also hope to be able to
make some tools with it as I would like to make some things like a pixel art editor and a port of sfxr.

Why GLES2?
----------
It is portable and specifically for my use case I wanted a library that worked with xorg libs on wayland.

Dependencies
------------
* GLES2(With vao extension; however, always willing to have it be optional if I get around to it)
* EGL
* CGLM(will have option for static lib soon)
* glfw(optional for examples)

Current Limitations
--------------------
* Different types of geometry aren't batched together for example triangles are a seperate batch from quads
and spheres are a seperate batch from those, etc
* No audio
* Texture system is static right now ie can't load your own texture this is simple to fix just hasn't been
done
* Optimizing changing data to only when it's changed for example camera
