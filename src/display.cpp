#include <SDL.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "display.h"
#include "util.h"

Display::Display(int width, int height, int scale, bool headless)
    : width(width), height(height), headless(headless) {
    screen = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                  width,
                                  height,
                                  32,
                                  0x00FF0000,
                                  0x0000FF00,
                                  0x000000FF,
                                  0xFF000000);

    if (!screen)
        die("cannot create surface: %s", SDL_GetError());
    /* The panel starts white */
    SDL_FillRect(screen, nullptr, 0xFFFFFFFF);

    if (headless)
        return;

    window = SDL_CreateWindow("Caster",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              width * scale,
                              height * scale,
                              SDL_SWSURFACE);
    if (!window)
        die("cannot create window: %s", SDL_GetError());

    uint32_t flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    if (!(renderer = SDL_CreateRenderer(window, -1, flags)))
        die("cannot create renderer: %s", SDL_GetError());

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                width,
                                height);
    if (!texture)
        die("cannot create texture: %s", SDL_GetError());

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
}

Display::~Display() {
    SDL_FreeSurface(screen);
    if (texture)
        SDL_DestroyTexture(texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
}

uint32_t *Display::pixels() {
    return static_cast<uint32_t *>(screen->pixels);
}

void Display::present() {
    if (headless)
        return;

    void *texture_pixels;
    int texture_pitch;

    SDL_LockTexture(texture, nullptr, &texture_pixels, &texture_pitch);
    uint8_t *dst = static_cast<uint8_t *>(texture_pixels);
    uint8_t *src = static_cast<uint8_t *>(screen->pixels);
    for (int y = 0; y < height; y++) {
        memcpy(dst, src, width * 4);
        dst += texture_pitch;
        src += screen->pitch;
    }
    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void Display::set_title(const char *title) {
    if (headless)
        return;

    SDL_SetWindowTitle(window, title);
}

bool Display::poll_quit() {
    if (headless)
        return false;

    SDL_Event event;
    return SDL_PollEvent(&event) && event.type == SDL_QUIT;
}
