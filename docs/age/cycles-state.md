

# Clocks, cycles and state

There seem to be many terms being used with regard to Gameboy timing:
m-cycle, t-cycle, clock cycle, oscillation, ...

For AGE I tried to use only the most common terms and use as few different
terms as possible.


## Cycles

An **M-cycle (machine cycle)** is the duration of one CPU operation,
e.g. one memory read or write.
Do not confuse this with a CPU instruction,
which can consist of multiple CPU operations.
There are 1048576 M-cycles per second for DMG and CGB.
For CGB double speed we have 2097152 M-cycles per second.

There are 4 **T-cycles (time cycles)** per machine cycle.
Depending on the CGB speed the T-cycle frequency is either 4 Mhz or 8 Mhz.

For easier event handling and easier real time synchronization AGE uses
**T4-cycles (4 Mhz time cycles)** instead of T-cycles.
T4-cycles are fixed at 4 Mhz even in CGB double speed mode.

```
DMG & CGB regular speed

 M-cycle  |               |               |               |
T4-cycle  |   |   |   |   |   |   |   |   |   |   |   |   |
 T-cycle  |   |   |   |   |   |   |   |   |   |   |   |   |
    time  +---+---+---+---+---+---+---+---+---+---+---+---+
```
```
CGB double speed

 M-cycle  |       |       |       |       |       |       |
T4-cycle  |   |   |   |   |   |   |   |   |   |   |   |   |
 T-cycle  | | | | | | | | | | | | | | | | | | | | | | | | |
    time  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```


## Reading and writing memory
