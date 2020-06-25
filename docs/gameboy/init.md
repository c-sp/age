
# Initial state

Gameboy emulators usually begin execution at `PC = 0x0100` as this is the
usual entry point for Gameboy Roms.
Before that the Gameboy boot rom has been running which is responsible for
showing the scrolling Nintendo logo and playing the famous sound effect.

Several parts of a Gameboy emulator have to be initialized accordingly.
Examples are:
* the clock cycle at `PC = 0x0100` is not zero (or any equivalent value)
* the sound circuit has already been used

The following chapters try to identify initial emulator state with the help
of several test roms.


## Clock & DIV

TODO


## LCD

TODO


## Sound circuit

TODO
