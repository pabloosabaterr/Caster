/*
 * Caster simulator
 * Copyright 2021 Wenting Zhang
 * Copyright 2026 Pablo Sabater
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <SDL.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "display.h"

Display::Display(int width, int height, int scale)
    : width(width), height(height) {
    window = SDL_CreateWindow("Caster",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              width * scale,
                              height * scale,
                              SDL_SWSURFACE);
    if (!window) {
        fprintf(stderr, "cannot create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    uint32_t flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    if (!(renderer = SDL_CreateRenderer(window, -1, flags))) {
        fprintf(stderr, "cannot create renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    screen = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                  width,
                                  height,
                                  32,
                                  0x00FF0000,
                                  0x0000FF00,
                                  0x000000FF,
                                  0xFF000000);

    if (!screen) {
        fprintf(stderr, "cannot create surface: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                width,
                                height);
    if (!texture) {
        fprintf(stderr, "cannot create texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    /* The panel starts white */
    SDL_FillRect(screen, nullptr, 0xFFFFFFFF);
}

Display::~Display() {
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

uint32_t *Display::pixels() {
    return static_cast<uint32_t *>(screen->pixels);
}

void Display::present() {
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
    SDL_SetWindowTitle(window, title);
}

bool Display::poll_quit() {
    SDL_Event event;
    return SDL_PollEvent(&event) && event.type == SDL_QUIT;
}
