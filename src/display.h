#ifndef DISPLAY_H
#define DISPLAY_H

#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;

/* The SDL window where the simulated panel is shown. */
class Display {
  public:
    Display(
        int width, int height, int win_width, int win_height, bool headless);
    ~Display();

    /* Framebuffer the panel model draws into, one ARGB pixel per element. */
    uint32_t *pixels();

    /* Copy the framebuffer to the window. */
    void present();

    void set_title(const char *title);

    /* Handle one pending window event, true if the window was closed. */
    bool poll_quit();

    bool is_headless() const {
        return headless;
    }

  private:
    int width;
    int height;
    bool headless;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    SDL_Surface *screen = nullptr;
};

#endif
