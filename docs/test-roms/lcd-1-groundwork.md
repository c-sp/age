
# LCD groundwork

## LCD M-cycle alignment

Prerequisites: [clocks, cycles and state][l-a1]

* align the LCD (working at T4-cycles) to M-cycles
* TL;DR: TODO

### CGB

By reading `LY` and `STAT` at specific times we can identify the initial LCD
state at `PC = 0x0100` with M-cycle accuracy.

By just reading `STAT` there may be an error of up to `3` T4-cycles cycles
though due to one M-cycle consisting of `4` T4-cycles.
However,
since `SCX` extends `mode 3` duration by up to `7` T4-cycles (`SCX & 7`),
more accurate identification of the initial LCD state is possible.

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

### DMG

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



## LCD restart

### Scanline 0 initial mode

Prerequisites: [clocks, cycles and state][l-a1]

When restarting the LCD,
scanline 0 begins with `mode 0` instead of `mode 2` for `77-80` T4-cycles.
```
DMG & CGB single speed

T4-cycle - event
----------------
           LCD switched off
       0 - LCD switched on (LYC = 0)
      76 - STAT = 0x84: scanline 0 mode 0, LY match
      80 - STAT = 0x87: scanline 0 mode 3, LY match
```
* [enable_display/nextstat_1_dmg08_cgb04c_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/nextstat_1_dmg08_cgb04c_out84.asm)
* [enable_display/nextstat_2_dmg08_cgb04c_out87](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/nextstat_2_dmg08_cgb04c_out87.asm)


### Scanline 0 duration

Prerequisites: [clocks, cycles and state][l-a1]

Scanline 0 is `2-3` T4-cycles cycles shorter than usual right after restarting
the LCD.
This affects either mode 3 or mode 0 (not the initial mode 0 replacing mode 2).

Detecting this requires CGB double speed though
(compare to single speed log below).
```
CGB double speed

T4-cycle - event
----------------
           LCD switched off
       0 - LCD switched on (LYC = 0)
       <...>
      78 - STAT = 0x84: scanline 0 mode 0, LY match
      80 - STAT = 0x87: scanline 0 mode 3, LY match
       <...>
     452 - STAT = 0x84: scanline 0 mode 0, LY match
     454 - STAT = 0x82: scanline 1 mode 2, 454(!) cycles after LCD restarted
       <...>
     532 - STAT = 0x82: scanline 1 mode 2
     534 - STAT = 0x83: scanline 1 mode 3, at cycle 80 + 454(!)
       <...>
     908 - STAT = 0x80: scanline 1 mode 0
     910 - STAT = 0x82: scanline 2 mode 2, at cycle 454 + 456
       <...>
     988 - STAT = 0x82: scanline 2 mode 2
     990 - STAT = 0x83: scanline 2 mode 3, at cycle 534 + 456
       <...>
```
* [enable_display/frame0_m2stat_count_ds_1_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_ds_1_cgb04c_out91.asm)
* [enable_display/frame0_m2stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_ds_2_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_ds_1_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_ds_1_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_ds_2_cgb04c_out90.asm)

```
DMG & CGB single speed

T4-cycle - event
----------------
           LCD switched off
       0 - LCD switched on (LYC = 0)
       <...>
      76 - STAT = 0x84: scanline 0 mode 2
      80 - STAT = 0x87: scanline 0 mode 3
       <...>
     452 - STAT = 0x80: scanline 0 mode 0
     456 - STAT = 0x82: scanline 1 mode 2, 456 cycles after LCD restarted
       <...>
     532 - STAT = 0x82: scanline 1 mode 2
     536 - STAT = 0x83: scanline 1 mode 3, at cycle 80 + 456
       <...>
     908 - STAT = 0x80: scanline 1 mode 0
     912 - STAT = 0x82: scanline 2 mode 2, 456 cycles after scanline 1 mode 2
       <...>
     988 - STAT = 0x82: scanline 2 mode 2
     992 - STAT = 0x83: scanline 2 mode 3, at cycle 534 + 456
       <...>
```
* [enable_display/frame0_m2stat_count_1_dmg08_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_1_dmg08_cgb04c_out91.asm)
* [enable_display/frame0_m2stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_2_dmg08_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_1_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_1_dmg08_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_2_dmg08_cgb04c_out90.asm)



[l-a1]: ../age/cycles-state.md
