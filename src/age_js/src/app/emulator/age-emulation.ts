import {AgeGbButton} from './age-emulator-keymap';

export interface AgeEmulation {

    /**
     * Run the emulation for a number of emulated cycles equivalent to the time between the last call to
     * {@link emulate()} and the current time.
     *
     * @returns true if the emulated screen has changed, false otherwise
     */
    emulate(): boolean;

    /**
     * @returns the current emulated screen (front-) buffer as 32 bit RGB values
     */
    getScreenBuffer(): Uint8ClampedArray;

    getScreenWidth(): number;

    getScreenHeight(): number;

    buttonDown(button: AgeGbButton): void;

    buttonUp(button: AgeGbButton): void;
}


export class AgeGbEmulation implements AgeEmulation {

    private readonly _screenBufferBytes: number;
    private _lastEmuTime: number;

    constructor(private readonly _emGbModule: EmGbModule,
                romFileContents: ArrayBuffer) {

        const romArray = new Uint8Array(romFileContents, 0, romFileContents.byteLength);
        const bufferPtr = this._emGbModule._gb_allocate_rom_buffer(romArray.length);
        this._emGbModule.HEAPU8.set(romArray, bufferPtr);
        this._emGbModule._gb_new_emulator();

        this._screenBufferBytes = this._emGbModule._gb_get_screen_width() * this._emGbModule._gb_get_screen_height() * 4;
        this._lastEmuTime = Date.now();
    }

    /**
     * @inheritDoc
     */
    emulate(): boolean {
        const now = Date.now();
        const millisElapsed = now - this._lastEmuTime;
        this._lastEmuTime = now;

        // calculate the number of cycles to emulate based on the elapsed time
        // (limit the cycles to emulate to not freeze the browser in case of performance issues)
        const cyclesPerSecond = this._emGbModule._gb_get_cycles_per_second();
        const maxCyclesToEmulate = cyclesPerSecond / 20;
        const cyclesToEmulate = Math.min(maxCyclesToEmulate, cyclesPerSecond * millisElapsed / 1000);

        // emulate
        return this._emGbModule._gb_emulate(cyclesToEmulate);
    }

    /**
     * @inheritDoc
     */
    getScreenBuffer(): Uint8ClampedArray {
        const screenBuffer = this._emGbModule._gb_get_screen_front_buffer();
        return new Uint8ClampedArray(this._emGbModule.HEAPU8.buffer, screenBuffer, this._screenBufferBytes);
    }

    /**
     * @inheritDoc
     */
    getScreenHeight(): number {
        return this._emGbModule._gb_get_screen_height();
    }

    /**
     * @inheritDoc
     */
    getScreenWidth(): number {
        return this._emGbModule._gb_get_screen_width();
    }

    /**
     * @inheritDoc
     */
    buttonDown(button: AgeGbButton): void {
        this._emGbModule._gb_set_buttons_down(button);
    }

    /**
     * @inheritDoc
     */
    buttonUp(button: AgeGbButton): void {
        this._emGbModule._gb_set_buttons_up(button);
    }
}
