# 2dGLESFramework
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


Current Limitations
--------------------
* Different types of geometry aren't batched together for example
triangles are a seperate batch from quads and spheres are a seperate batch from those,
etc
* No audio
* Texture system is static right now ie can't load your own texture this is being worked on
* Optimizing changing data to only when it's changed ex: camera
