#include <SDL.h>
#include <vector>
#include <cstdint>
#include <emscripten.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define SCALE 3

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;

std::vector<uint8_t> romData;
bool running = false;

uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

// =============================
// Dummy Emulator Core
// =============================

class Emulator {
public:
    void loadROM(uint8_t* data, int size) {
        romData.assign(data, data + size);
        printf("ROM Loaded: %d bytes\n", size);
    }

    void update() {
        // Simulação temporária
        static int color = 0;
        color = (color + 1) % 255;

        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            framebuffer[i] = 0xFF000000 | (color << 16); // vermelho animado
        }
    }

    void pressButton(int button) {
        printf("Button Pressed: %d\n", button);
    }

    void releaseButton(int button) {
        printf("Button Released: %d\n", button);
    }
};

Emulator emulator;

// =============================
// Rendering
// =============================

void render() {
    emulator.update();

    SDL_UpdateTexture(texture, NULL, framebuffer, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

// =============================
// Main Loop (Web compatible)
// =============================

void mainLoop() {
    if (!running) return;
    render();
}

// =============================
// Exported Functions (JS)
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
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "GameBoys",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCALE,
        SCREEN_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (true) {
        mainLoop();
        SDL_Delay(16);
    }
#endif

    return 0;
}