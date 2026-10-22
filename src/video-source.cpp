#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "video-source.h"
#include "log.h"
#include "panel.h"
#include "stb_image.h"
#include "stb_image_resize.h"

void print_buffer(uint8_t *buf, size_t bytes) {
    for (size_t i = 0; i < bytes / 16; i++) {
        for (int j = 0; j < 16; j++) {
            log_debug("%02x ", buf[i * 16 + j]);
        }
        log_debug("\n");
    }
}

uint8_t VideoSource::rgb2y(uint32_t r, uint32_t g, uint32_t b, int x, int y) {
    switch (panel) {
    case PanelType::KALEIDO3: {
        int c = (x + (DISP_HEIGHT - y)) % 3;
        if (c == 0)
            return r;
        else if (c == 1)
            return b;
        else
            return g;
    }
    case PanelType::MONOCHROME: {
        r = r * 77;
        g = g * 150;
        b = b * 29;
        uint32_t yy = r + g + b;
        return (yy >> 8);
    }
    }
}

// Wrapper around stbi_load() to prevent segfault when there's no image
static uint8_t *load_image(const char *fn, int *x, int *y, int *n) {
    uint8_t *data = stbi_load(fn, x, y, n, 0);
    if (!data) {
        fprintf(stderr,
                "error: cannot load image '%s': %s\n",
                fn,
                stbi_failure_reason());
        exit(EXIT_FAILURE);
    }

    return data;
}

VideoSource::VideoSource(SourceType source, PanelType panel)
    : panel(panel), source(source) {
}

void VideoSource::source_box() {
    memset(pixels, 0xff, DISP_WIDTH * DISP_HEIGHT);
    bool xright = (box_x == DISP_WIDTH - 21);
    bool ydown = (box_y == DISP_HEIGHT - 21);
    bool yup = (box_y == 0);
    bool xleft = (box_x == 0);
    if (box_dir == 0) {
        // Right down
        if (ydown)
            box_dir = 1; // right up
        if (xright)
            box_dir = 2; // left down
        if (ydown && xright)
            box_dir = 3;
    } else if (box_dir == 1) {
        // Right up
        if (yup)
            box_dir = 0; // right down
        if (xright)
            box_dir = 3; // left up
        if (yup && xright)
            box_dir = 2;
    } else if (box_dir == 2) {
        // Left down
        if (ydown)
            box_dir = 3; // left up
        if (xleft)
            box_dir = 0; // right down
        if (ydown && xleft)
            box_dir = 1;
    } else if (box_dir == 3) {
        // Left up
        if (yup)
            box_dir = 2; // left down
        if (xleft)
            box_dir = 1; // right up
        if (yup && xleft)
            box_dir = 0;
    }

    if (box_dir == 0) {
        // Right down
        box_x++;
        box_y++;
    } else if (box_dir == 1) {
        // Right up
        box_x++;
        box_y--;
    } else if (box_dir == 2) {
        // Left down
        box_x--;
        box_y++;
    } else if (box_dir == 3) {
        // Left up
        box_x--;
        box_y--;
    }

    if (!box_loaded) {
        int w, h, n;
        uint8_t *data = load_image("texture.jpg", &w, &h, &n);
        log_debug("Input image size %d x %d (%d channels)\n", w, h, n);
        if (w != 20 || h != 20) {
            fprintf(stderr,
                    "error: texture.jpg must be 20 x 20, got %d x %d\n",
                    w,
                    h);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < 20 * 20; i++) {
            box_texture[i] = data[i * n];
        }
        box_loaded = true;
        log_info("Image loaded\n");
    }

    uint32_t rptr = 0;
    for (int i = box_y; i < box_y + 20; i++) {
        for (int j = box_x; j < box_x + 20; j++) {
            pixels[i * DISP_WIDTH + j] = box_texture[rptr++];
        }
    }
    frame_counter++;
}

void VideoSource::source_fixed_border() {
    memset(pixels, 0xff, DISP_WIDTH * DISP_HEIGHT);
    for (int i = 0; i < DISP_WIDTH; i++) {
        pixels[i] = 0x00;
        pixels[(DISP_HEIGHT - 1) * DISP_WIDTH + i] = 0x00;
    }
    for (int i = 0; i < DISP_HEIGHT; i++) {
        pixels[DISP_WIDTH * i] = 0x00;
        pixels[DISP_WIDTH * i + DISP_WIDTH - 1] = 0x00;
    }
}

void VideoSource::source_fixed_color() {
    memset(pixels, 0x00, DISP_WIDTH * DISP_HEIGHT / 4);
    memset(pixels + DISP_WIDTH * DISP_HEIGHT / 4,
           0x55,
           DISP_WIDTH * DISP_HEIGHT / 4);
    memset(pixels + DISP_WIDTH * DISP_HEIGHT / 2,
           0xaa,
           DISP_WIDTH * DISP_HEIGHT / 4);
    memset(pixels + DISP_WIDTH * DISP_HEIGHT / 4 * 3,
           0xff,
           DISP_WIDTH * DISP_HEIGHT / 4);
}

/* Full screen fixed color, reverse, 200 frames later */
void VideoSource::source_fixed_swap() {
    uint8_t colors[4];
    if (frame_counter < 100) {
        colors[0] = 0x00;
        colors[1] = 0x55;
        colors[2] = 0xaa;
        colors[3] = 0xff;
    } else {
        colors[0] = 0xff;
        colors[1] = 0xaa;
        colors[2] = 0x55;
        colors[3] = 0x00;
    }
    memset(pixels, colors[0], DISP_WIDTH * DISP_HEIGHT / 4);
    memset(pixels + DISP_WIDTH * DISP_HEIGHT / 4,
           colors[1],
           DISP_WIDTH * DISP_HEIGHT / 4);
    memset(pixels + DISP_WIDTH * DISP_HEIGHT / 2,
           colors[2],
           DISP_WIDTH * DISP_HEIGHT / 4);
    memset(pixels + DISP_WIDTH * DISP_HEIGHT / 4 * 3,
           colors[3],
           DISP_WIDTH * DISP_HEIGHT / 4);
    frame_counter++;
    if (frame_counter == 200)
        frame_counter = 0;
}

void VideoSource::source_blink() {
    if (frame_counter) {
        memset(pixels, 0x00, DISP_WIDTH * DISP_HEIGHT);
        frame_counter = 0;
    } else {
        memset(pixels, 0xff, DISP_WIDTH * DISP_HEIGHT);
        frame_counter = 1;
    }
}

void VideoSource::source_image() {
    if ((frame_counter % 100) != 0) {
        frame_counter++;
        if (frame_counter == 400)
            frame_counter = 0;
        return;
    }
    frame_counter++;
    char fn[32];
    snprintf(fn, sizeof(fn), "test%d.jpg", frame_counter / 100);
    int x, y, n;
    uint8_t *data = load_image(fn, &x, &y, &n);
    log_debug("Input image size %d x %d (%d channels)\n", x, y, n);
    uint8_t *scaled = (uint8_t *)calloc(1, DISP_WIDTH * DISP_HEIGHT * n);
    float scalex, scaley;
    scalex = (float)DISP_WIDTH / (float)x;
    scaley = (float)DISP_HEIGHT / (float)y;
    // Fit the image into destination size keeping aspect ratio
    int outh, outw, outstride, outoffset;
    if (scalex > scaley) {
        outh = DISP_HEIGHT;
        outw = x * scaley;
        outoffset = ((DISP_WIDTH - outw) / 2) * n;
        outstride = x * n;
    } else {
        outw = DISP_WIDTH;
        outh = y * scalex;
        outoffset = ((DISP_HEIGHT - outh) / 2) * n * DISP_WIDTH;
        outstride = 0;
    }
    stbir_resize_uint8(
        data, x, y, 0, scaled + outoffset, outw, outh, outstride, n);
    free(data);
    uint8_t *rdptr = scaled;
    uint8_t *wrptr = pixels;
    if (n == 4) {
        for (int y = 0; y < DISP_HEIGHT; y++) {
            for (int x = 0; x < DISP_WIDTH; x++) {
                uint32_t r = *rdptr++;
                uint32_t g = *rdptr++;
                uint32_t b = *rdptr++;
                // uint32_t a = *rdptr++;
                uint32_t yy = rgb2y(r, g, b, x, y);
                *wrptr++ = yy;
            }
        }
    } else if (n == 3) {
        for (int y = 0; y < DISP_HEIGHT; y++) {
            for (int x = 0; x < DISP_WIDTH; x++) {
                uint32_t r = *rdptr++;
                uint32_t g = *rdptr++;
                uint32_t b = *rdptr++;
                uint32_t yy = rgb2y(r, g, b, x, y);
                *wrptr++ = yy;
            }
        }
    } else if (n == 1) {
        memcpy(wrptr, rdptr, DISP_WIDTH * DISP_HEIGHT);
    }
    free(scaled);
}

void VideoSource::next_frame() {
    switch (source) {
    case SourceType::BOX:
        source_box();
        break;
    case SourceType::FIXED_BORDER:
        source_fixed_border();
        break;
    case SourceType::FIXED_COLOR:
        source_fixed_color();
        break;
    case SourceType::FIXED_SWAP:
        source_fixed_swap();
        break;
    case SourceType::BLINK:
        source_blink();
        break;
    case SourceType::IMAGE:
        source_image();
        break;
    }
}

void VideoSource::reset() {
    x_counter = 0;
    y_counter = 0;
    next_frame();
}

void VideoSource::apply(VideoBus &bus) {
    bus.vsync = (y_counter == 0) ? 1 : 0;
    bus.valid = 1;
    if (bus.ready) {
        uint32_t output = 0;
        for (int i = 0; i < 4; i++) {
            uint32_t p = pixels[y_counter * DISP_WIDTH + x_counter++];
            p = p << (3 - i) * 8;
            output |= p;
        }
        bus.pixel = output;

        if (x_counter == DISP_WIDTH) {
            x_counter = 0;
            y_counter++;
            if (y_counter == DISP_HEIGHT) {
                y_counter = 0;
                next_frame();
            }
        }
    }
}
