
# Initial LCD state

The initial LCD state may be important to get right as not every rom restarts
the Gameboy's LCD.


## CGB

By reading `LY` and `STAT` at specific times we can identify the initial LCD
state in M-cycle "resolution":
there may be an error of up to `3` T4-cycles cycles due to one M-cycle
consisting of `4` T4-cycles.

However,
since different `SCX` values extend `mode 3` duration by `SCX & 7` T4-cycles,
accurate identification of the initial LCD state is possible.

```
T4-cycle - event
----------------
       0 - <PC = 0x0100>
      40 - LY = 144
     289 - <scanline 145>
    3937 - <scanline 153>
    4393 - <scanline 0, mode 2 begins>
    4473 - <scanline 0, mode 3 begins>

    4644 - STAT = 0x87 (mode 3, LY match)
    4645 - <scanline 0, mode 0 begins for SCX = 0>
    4646 - <scanline 0, mode 0 begins for SCX = 1>
    4647 - <scanline 0, mode 0 begins for SCX = 2>
    4648 - <scanline 0, mode 0 begins for SCX = 3>
    4648 - STAT = 0x84 (mode 0, LY match) for SCX 0, 2, 3
    4648 - STAT = 0x87 (mode 3, LY match) for SCX 5
    4649 - <scanline 0, mode 0 begins for SCX = 4>
    4650 - <scanline 0, mode 0 begins for SCX = 5>
    4651 - <scanline 0, mode 0 begins for SCX = 6>
    4652 - <scanline 0, mode 0 begins for SCX = 7>
    4652 - STAT = 0x84 (mode 0, LY match)

    4849 - <scanline 1>
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
T4-cycle - event
----------------
           <scanline 153>
       0 - <PC = 0x0100>
      40 - LY = 0
      52 - STAT = 0x85 (mode 1, LY match)
      56 - STAT = 0x84 (mode 0, LY match)
           <scanline 0>
``` 
* [display_startstate/ly_dmg08_out00_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/ly_dmg08_out00_cgb04c_out90.asm)
* [display_startstate/stat_1_dmg08_out85](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_1_dmg08_out85.asm)
* [display_startstate/stat_2_dmg08_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_2_dmg08_out84.asm)
