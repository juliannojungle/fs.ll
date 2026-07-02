#include "HAL.h"
#include <stdlib.h>
#include <SDL2/SDL.h>

void Delay(unsigned int milliseconds) {
    unsigned int start = SDL_GetTicks();
    SDL_Event event;
    while (SDL_GetTicks() - start < milliseconds) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            exit(0);
        SDL_Delay(1);
    }
}
void STDIOInitAll(void) {}
