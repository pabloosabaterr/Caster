#include <algorithm>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "video-source.h"
#include "log.h"
#include "stb_image.h"
#include "stb_image_resize.h"
#include "util.h"

static constexpr int FRAMES_PER_IMAGE = 100;
static constexpr int IMAGE_COUNT = 2;

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
        int c = (x + (height - y)) % 3;
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
    die("unknown panel type");
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

VideoSource::VideoSource(SourceType source,
                         PanelType panel,
                         std::pair<uint64_t, uint64_t> res)
    : width(res.first), height(res.second), pixels(width * height),
      panel(panel), source(source) {
}

void VideoSource::source_box() {
    std::fill(pixels.begin(), pixels.end(), 0xff);
    bool xright = (box_x == width - 21);
    bool ydown = (box_y == height - 21);
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
            pixels[i * width + j] = box_texture[rptr++];
        }
    }
    frame_counter++;
}

void VideoSource::source_fixed_border() {
    std::fill(pixels.begin(), pixels.end(), 0xff);
    for (int i = 0; i < width; i++) {
        pixels[i] = 0x00;
        pixels[(height - 1) * width + i] = 0x00;
    }
    for (int i = 0; i < height; i++) {
        pixels[width * i] = 0x00;
        pixels[width * i + width - 1] = 0x00;
    }
}

void VideoSource::source_fixed_color() {
    size_t quarter = pixels.size() / 4;
    std::fill_n(pixels.begin(), quarter, 0x00);
    std::fill_n(pixels.begin() + quarter, quarter, 0x55);
    std::fill_n(pixels.begin() + 2 * quarter, quarter, 0xaa);
    std::fill_n(pixels.begin() + 3 * quarter, quarter, 0xff);
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
    size_t quarter = pixels.size() / 4;
    for (int i = 0; i < 4; i++)
        std::fill_n(pixels.begin() + i * quarter, quarter, colors[i]);
    frame_counter++;
    if (frame_counter == 200)
        frame_counter = 0;
}

void VideoSource::source_blink() {
    if (frame_counter) {
        std::fill(pixels.begin(), pixels.end(), 0x00);
        frame_counter = 0;
    } else {
        std::fill(pixels.begin(), pixels.end(), 0xff);
        frame_counter = 1;
    }
}

void VideoSource::source_image() {
    if ((frame_counter % FRAMES_PER_IMAGE) != 0) {
        frame_counter++;
        if (frame_counter == FRAMES_PER_IMAGE * IMAGE_COUNT)
            frame_counter = 0;
        return;
    }
    frame_counter++;
    char fn[32];
    snprintf(fn, sizeof(fn), "test%d.jpg", frame_counter / FRAMES_PER_IMAGE);
    int x, y, n;
    uint8_t *data = load_image(fn, &x, &y, &n);
    log_debug("Input image size %d x %d (%d channels)\n", x, y, n);
    uint8_t *scaled = (uint8_t *)calloc(1, width * height * n);
    float scalex, scaley;
    scalex = (float)width / (float)x;
    scaley = (float)height / (float)y;
    // Fit the image into destination size keeping aspect ratio
    int outh, outw, outstride, outoffset;
    if (scalex > scaley) {
        outh = height;
        outw = x * scaley;
        outoffset = ((width - outw) / 2) * n;
        outstride = width * n;
    } else {
        outw = width;
        outh = y * scalex;
        outoffset = ((height - outh) / 2) * n * width;
        outstride = 0;
    }
    stbir_resize_uint8(
        data, x, y, 0, scaled + outoffset, outw, outh, outstride, n);
    free(data);
    uint8_t *rdptr = scaled;
    uint8_t *wrptr = pixels.data();
    if (n == 4 || n == 3) {
        for (int py = 0; py < height; py++) {
            for (int px = 0; px < width; px++) {
                uint32_t r = rdptr[0];
                uint32_t g = rdptr[1];
                uint32_t b = rdptr[2];
                rdptr += n;
                *wrptr++ = rgb2y(r, g, b, px, py);
            }
        }
    } else if (n == 1) {
        memcpy(wrptr, rdptr, width * height);
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
            uint32_t p = pixels[y_counter * width + x_counter++];
            p = p << (3 - i) * 8;
            output |= p;
        }
        bus.pixel = output;

        if (x_counter == width) {
            x_counter = 0;
            y_counter++;
            if (y_counter == height) {
                y_counter = 0;
                next_frame();
            }
        }
    }
}
