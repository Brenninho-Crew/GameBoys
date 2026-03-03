#include <SDL.h>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define SCALE 3
#define FPS 60

// =============================
// Globals
// =============================

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;

uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

bool running = false;

// =============================
// Emulator Core Structure
// =============================

class Emulator {
public:

    std::vector<uint8_t> rom;
    bool buttons[8]; // Up, Down, Left, Right, A, B, Start, Select

    Emulator() {
        memset(buttons, 0, sizeof(buttons));
    }

    void loadROM(uint8_t* data, int size) {
        rom.assign(data, data + size);
        printf("ROM Loaded: %d bytes\n", size);
    }

    void reset() {
        printf("Emulator Reset\n");
    }

    void pressButton(int id) {
        if (id >= 0 && id < 8)
            buttons[id] = true;
    }

    void releaseButton(int id) {
        if (id >= 0 && id < 8)
            buttons[id] = false;
    }

    void update() {
        // =============================
        // TEMPORARY GRAPHIC TEST
        // Replace later with real PPU
        // =============================

        static int tick = 0;
        tick++;

        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {

                uint8_t r = (x + tick) % 255;
                uint8_t g = (y + tick) % 255;
                uint8_t b = (tick) % 255;

                framebuffer[y * SCREEN_WIDTH + x] =
                    0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }

        // Example: change color if A button pressed
        if (buttons[4]) {
            for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
                framebuffer[i] = 0xFFFFFFFF; // white flash
            }
        }
    }
};

Emulator emulator;

// =============================
// Rendering
// =============================

void render() {
    emulator.update();

    SDL_UpdateTexture(texture, NULL, framebuffer,
                      SCREEN_WIDTH * sizeof(uint32_t));

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

// =============================
// Main Loop
// =============================

void mainLoop() {
    if (!running) return;

    render();
}

#ifndef __EMSCRIPTEN__
void desktopLoop() {
    SDL_Event event;

    while (running) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        mainLoop();
        SDL_Delay(1000 / FPS);
    }
}
#endif

// =============================
// Exported Functions (Web)
// =============================

extern "C" {

    void loadROM(uint8_t* data, int size) {
        emulator.loadROM(data, size);
    }

    void start() {
        running = true;
    }

    void pressButton(int button) {
        emulator.pressButton(button);
    }

    void releaseButton(int button) {
        emulator.releaseButton(button);
    }
}

// =============================
// Initialization
// =============================

int main() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Init Failed\n");
        return -1;
    }

    window = SDL_CreateWindow(
        "GameBoys Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCALE,
        SCREEN_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(window, -1,
                                   SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    running = true;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    desktopLoop();
#endif

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}