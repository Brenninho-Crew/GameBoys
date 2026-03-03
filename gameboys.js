let Module = {
    instance: null,
    memory: null,
    exports: null,

    async init() {
        const response = await fetch("gameboys.wasm");
        const buffer = await response.arrayBuffer();

        const wasmModule = await WebAssembly.instantiate(buffer, {
            env: {
                memory: new WebAssembly.Memory({ initial: 256 }),
                abort: () => console.error("WASM Abort"),
            }
        });

        this.instance = wasmModule.instance;
        this.exports = this.instance.exports;
        this.memory = this.exports.memory;

        console.log("WASM Loaded");
        if (this.onRuntimeInitialized) {
            this.onRuntimeInitialized();
        }
    },

    _malloc(size) {
        return this.exports.malloc(size);
    },

    get HEAPU8() {
        return new Uint8Array(this.memory.buffer);
    },

    _loadROM(ptr, size) {
        this.exports.loadROM(ptr, size);
    },

    _start() {
        this.exports.start();
    },

    _pressButton(button) {
        this.exports.pressButton(button);
    },

    _releaseButton(button) {
        this.exports.releaseButton(button);
    }
};

window.addEventListener("load", () => {
    Module.init();
});