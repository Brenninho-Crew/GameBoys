// =======================================
// GameBoys WASM Loader with .gb Support
// =======================================

const Module = {

    instance: null,
    exports: null,
    memory: null,
    ready: false,
    debug: true,

    async init() {
        try {

            console.log("Loading gameboys.wasm...");

            const response = await fetch("gameboys.wasm");
            if (!response.ok) throw new Error("WASM not found");

            const buffer = await response.arrayBuffer();

            const wasm = await WebAssembly.instantiate(buffer, {
                env: {
                    memory: new WebAssembly.Memory({ initial: 256 }),
                    abort: () => console.error("WASM Abort"),
                    emscripten_notify_memory_growth: () => {}
                }
            });

            this.instance = wasm.instance;
            this.exports = this.instance.exports;
            this.memory = this.exports.memory;

            this.ready = true;
            console.log("WASM Ready");

            if (this.onRuntimeInitialized) {
                this.onRuntimeInitialized();
            }

        } catch (err) {
            console.error("WASM Error:", err);
            alert("Failed to load emulator core.");
        }
    },

    // =========================
    // Memory Helpers
    // =========================

    malloc(size) {
        return this.exports.malloc(size);
    },

    free(ptr) {
        if (this.exports.free) this.exports.free(ptr);
    },

    get HEAPU8() {
        return new Uint8Array(this.memory.buffer);
    },

    // =========================
    // ROM Validation (.gb)
    // =========================

    validateGB(data) {

        if (data.length < 0x150) {
            alert("Invalid ROM: File too small.");
            return null;
        }

        // Nintendo Logo Check (0x104–0x133)
        const nintendoLogo = [
            0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
            0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,
            0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
            0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,
            0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
            0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E
        ];

        for (let i = 0; i < nintendoLogo.length; i++) {
            if (data[0x104 + i] !== nintendoLogo[i]) {
                alert("Invalid Game Boy ROM (Logo mismatch).");
                return null;
            }
        }

        // Extract Title (0x134–0x143)
        let title = "";
        for (let i = 0x134; i <= 0x143; i++) {
            if (data[i] === 0x00) break;
            title += String.fromCharCode(data[i]);
        }

        // Detect Game Boy Color
        const cgbFlag = data[0x143];
        let system = "Game Boy";
        if (cgbFlag === 0x80 || cgbFlag === 0xC0) {
            system = "Game Boy Color";
        }

        return {
            title: title.trim() || "Unknown Title",
            system: system,
            size: data.length
        };
    },

    // =========================
    // Load ROM
    // =========================

    loadROMFile(file, statusCallback) {

        if (!file.name.toLowerCase().endsWith(".gb")) {
            alert("Only .gb files are supported.");
            return;
        }

        const reader = new FileReader();

        reader.onload = () => {

            const data = new Uint8Array(reader.result);

            const info = this.validateGB(data);
            if (!info) return;

            const ptr = this.malloc(data.length);
            this.HEAPU8.set(data, ptr);

            this.exports.loadROM(ptr, data.length);
            this.free(ptr);

            if (statusCallback) {
                statusCallback(
                    "Loaded: " + info.title +
                    " | " + info.system +
                    " | " + (info.size / 1024).toFixed(2) + " KB"
                );
            }

            console.log("ROM Loaded:", info);
        };

        reader.readAsArrayBuffer(file);
    },

    // =========================
    // Emulator Controls
    // =========================

    start() {
        if (!this.ready) return;
        this.exports.start();
    },

    pressButton(id) {
        if (!this.ready) return;
        this.exports.pressButton(id);
    },

    releaseButton(id) {
        if (!this.ready) return;
        this.exports.releaseButton(id);
    }
};

// =======================================
// Auto Init
// =======================================

window.addEventListener("load", () => {
    Module.init();
});