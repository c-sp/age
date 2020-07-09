# AGE documentation

AGE documentation is still in a very early stage.
If you want me to prioritize anything just let me know.


## Clocks & cycles

An **M-cycle (machine cycle)** is the duration of one CPU operation,
e.g. one memory read or write.
DMG & CGB: 1048576 M-cycles per second,
CGB double speed: 2097152 M-cycles per second.

There are 4 **T-cycles (time cycles)** per machine cycle.

For easier event managing and real time synchronization AGE makes use of
**T4-cycles (4Mhz time cycles)**.
T4-cycles are fixed at 4Mhz even when emulating CGB double speed.


## Test rom analyses

* [`HALT` bug](halt-bug.md)
* [Interrupt dispatching](interrupt-dispatch.md)
* [LCD - initial state](lcd-init.md)
* [LCD - `STAT` mode timing](lcd-stat-mode-timing.md)
* [Serial transfer](serial.md)
* [Sound circuit (old)](sound.md)


## Links

* [Cycle-Accurate Game Boy Docs (AntonioND)](
  https://github.com/AntonioND/giibiiadvance/tree/master/docs)
* [Game Boy: Complete Technical Reference (Joonas Javanainen)](
  https://gekkio.fi/files/gb-docs/gbctr.pdf)
* [Pan Docs](https://gbdev.io/pandocs/)
