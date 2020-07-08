
# Initial LCD state

The initial LCD state may be important to get right as not every rom restarts
the Gameboy's LCD.


## CGB

By reading `LY` and `STAT` at specific times we can identify the initial LCD
state in "machine cycle resolution":
there may be an error of up to `3` 4-Mhz-clock cycles due to one machine cycle
consisting of `4` 4-Mhz-clock cycles.

However,
since different `SCX` values extend `mode 3` duration by `SCX & 7` 4-Mhz-clock
cycles,
cycle accurate identification of the initial LCD state is possible.

```
4-Mhz clock - event
-------------------
          0 - <PC = 0x0100>
         40 - LY = 144
        292 - <scanline 145>
        748 - <scanline 146>
          <...>
       3940 - <scanline 153>
       4396 - <scanline 0>
       4644 - STAT = 0x87 (mode 3, LY match)

       4648 - STAT = 0x87 (mode 3, LY match) for SCX 5
       4648 - STAT = 0x84 (mode 0, LY match) for SCX 0, 2, 3

       4652 - STAT = 0x84 (mode 0, LY match)
       4852 - <scanline 1>
``` 
* [display_startstate/ly_dmg08_out00_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/ly_dmg08_out00_cgb04c_out90.asm)
* [display_startstate/stat_1_cgb04c_out87](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_1_cgb04c_out87.asm)
* [display_startstate/stat_2_cgb04c_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_2_cgb04c_out84.asm)
* [display_startstate/stat_scx2_1_cgb04c_out87](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_scx2_1_cgb04c_out87.asm)
* [display_startstate/stat_scx2_2_cgb04c_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_scx2_2_cgb04c_out84.asm)
* [display_startstate/stat_scx3_1_cgb04c_out87](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_scx3_1_cgb04c_out87.asm)
* [display_startstate/stat_scx3_2_cgb04c_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_scx3_2_cgb04c_out84.asm)
* [display_startstate/stat_scx5_1_cgb04c_out87](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_scx5_1_cgb04c_out87.asm)
* [display_startstate/stat_scx5_2_cgb04c_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_scx5_2_cgb04c_out84.asm)


## DMG

```
4-Mhz clock - event
-------------------
              <scanline 153>
          0 - <PC = 0x0100>
         40 - LY = 0
         52 - STAT = 0x85 (mode 1, LY match)
         56 - STAT = 0x84 (mode 0, LY match)
         60 - <scanline 0>
        516 - <scanline 1>
``` 
* [display_startstate/ly_dmg08_out00_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/ly_dmg08_out00_cgb04c_out90.asm)
* [display_startstate/stat_1_dmg08_out85](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_1_dmg08_out85.asm)
* [display_startstate/stat_2_dmg08_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_2_dmg08_out84.asm)
