#include <SDL2/SDL.h>
#include <vinox/vinox.h>

int main(void) {

    SDL_GLContext context;
    SDL_Window *window;

   // SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Vinox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(0);

    context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

    if (vinoxInit() == -1) {
        printf("Init of vinox failed. aborting\n");
        return -1;
    }

    Camera camera;
    camera.position = (Vec2) { 500.0f, 500.0f };
    camera.scale = 1.0f;
    camera.rotation = 0.0f;
    
    SDL_Event e;
    int run = 1;
    while(run) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        run = 0;
                        break;
                    default:
                        break;
                    }
            } else if (e.type == SDL_QUIT) {
                run = 0;
            }
        }
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        
        vinoxBeginDrawing(camera, width, height);
        vinoxEndDrawing();

        SDL_GL_SwapWindow(window);
    }
    
    SDL_GL_DeleteContext(context);
}
