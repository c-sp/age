
# LCD `STAT` mode timing

LCD mode timing can be examined with the help of several Gambatte test roms.
Most of these roms just read the `STAT` register each `114` M-cycles
(`456` T4-cycles) and compare it to some expected value.
By using a different initial M-cycle "offset" for the first `STAT` read
different mode transitions can be observed.


## Scanline 0 starts with mode 0

When restarting the LCD,
the first scanline starts with `mode 0` instead of `mode 2` for `77-80`
T4-cycles.
```
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


## mode 0 - mode 2

Scanline 0 is `2-3` T4-cycles cycles shorter than usual.
Detecting this requires CGB double speed
(compare to single speed log below).
```
CGB double speed

T4-cycle - event
----------------
           LCD switched off
       0 - LCD switched on (LYC = 0)
       <...>
     452 - STAT = 0x84: scanline 0 mode 0, LY match
     454 - STAT = 0x82: scanline 1 mode 2, 454(!) cycles after LCD switched on
       <...>
     908 - STAT = 0x80: scanline 1 mode 0
     910 - STAT = 0x82: scanline 2 mode 2
       <...>
    1364 - STAT = 0x80: scanline 2 mode 0
    1366 - STAT = 0x82: scanline 3 mode 2
       <...>
   65204 - STAT = 0x80: scanline 142 mode 0
   65206 - STAT = 0x82: scanline 143 mode 2
       <...>
   65660 - STAT = 0x80: scanline 143 mode 0
   65662 - STAT = 0x81: scanline 144 mode 1
       <...>
   70220 - STAT = 0x85: scanline 153 mode 1, LY match
   70222 - STAT = 0x86: scanline 0 mode 2, LY match
       <...>
   70676 - STAT = 0x84: scanline 0 mode 0, LY match
   70678 - STAT = 0x82: scanline 1 mode 2
       <...>
   71132 - STAT = 0x80: scanline 1 mode 0
   71134 - STAT = 0x82: scanline 2 mode 2
       <...>
  135428 - STAT = 0x80: scanline 142 mode 0
  135430 - STAT = 0x82: scanline 143 mode 2
       <...>
  135884 - STAT = 0x80: scanline 143 mode 0
  135886 - STAT = 0x81: scanline 144 mode 1
```
* [enable_display/frame0_m2stat_count_ds_1_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_ds_1_cgb04c_out91.asm)
* [enable_display/frame0_m2stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_ds_2_cgb04c_out90.asm)
* [enable_display/frame1_m2stat_count_ds_1_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2stat_count_ds_1_cgb04c_out91.asm)
* [enable_display/frame1_m2stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2stat_count_ds_2_cgb04c_out90.asm)

```
T4-cycle - event
----------------
           LCD switched off
       0 - LCD switched on (LYC = 0)
       <...>
     452 - STAT = 0x80: scanline 0 mode 0
     456 - STAT = 0x82: scanline 1 mode 2
       <...>
   65204 - STAT = 0x80: scanline 142 mode 0
   65208 - STAT = 0x82: scanline 143 mode 2
       <...>
   65660 - STAT = 0x80: scanline 143 mode 0
   65664 - STAT = 0x81: scanline 144 mode 1
       <...>
   70220 - STAT = 0x84: scanline 153 mode 0, LY match
   70224 - STAT = 0x86: scanline 0 mode 2, LY match
       <...>
   70676 - STAT = 0x80: scanline 0 mode 0
   70680 - STAT = 0x82: scanline 1 mode 2
       <...>
  135428 - STAT = 0x80: scanline 142 mode 0
  135432 - STAT = 0x82: scanline 143 mode 2
       <...>
  135884 - STAT = 0x80: scanline 143 mode 0
  135888 - STAT = 0x81: scanline 144 mode 1
```
* [enable_display/frame0_m2stat_count_1_dmg08_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_1_dmg08_cgb04c_out91.asm)
* [enable_display/frame0_m2stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_2_dmg08_cgb04c_out90.asm)
* [enable_display/frame1_m2stat_count_1_dmg08_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2stat_count_1_dmg08_cgb04c_out91.asm)
* [enable_display/frame1_m2stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2stat_count_2_dmg08_cgb04c_out90.asm)


## mode 2 - mode 3

The following Gambatte test roms check `mode 2` to `mode 3` transition for
scanline 0 to 144 of the first frame after restarting the LCD.

* [enable_display/frame0_m3stat_count_1_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_1_dmg08_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_2_dmg08_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_ds_1_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_ds_1_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_ds_2_cgb04c_out90.asm)

```
T4-cycle - event
-------------------
           LCD switched off
       0 - LCD switched on
      76 - STAT = 0x84: scanline 0 mode 2
      80 - STAT = 0x87: scanline 0 mode 3
     532 - STAT = 0x82: scanline 1 mode 2
     536 - STAT = 0x83: scanline 1 mode 3, at cycle 80 + 456
     988 - STAT = 0x82: scanline 2 mode 2
     992 - STAT = 0x83: scanline 2 mode 3, at cycle 534 + 456
       <...>
   65284 - STAT = 0x82: scanline 143 mode 2
   65288 - STAT = 0x83: scanline 143 mode 3
   65740 - STAT = 0x81: scanline 144 mode 1
   65744 - STAT = 0x81: scanline 144 mode 1
```
CGB double speed:
```
T4-cycle - event
-------------------
           LCD switched off
       0 - LCD switched on (LYC = 0)
      78 - STAT = 0x84: scanline 0 mode 0, LY match
      80 - STAT = 0x87: scanline 0 mode 3, LY match
     532 - STAT = 0x82: scanline 1 mode 2
     534 - STAT = 0x83: scanline 1 mode 3, at cycle 80 + 454(!)
     988 - STAT = 0x82: scanline 2 mode 2
     990 - STAT = 0x83: scanline 2 mode 3, at cycle 534 + 456
       <...>
   65284 - STAT = 0x82: scanline 143 mode 2
   65286 - STAT = 0x83: scanline 143 mode 3
   65740 - STAT = 0x81: scanline 144 mode 1
   65742 - STAT = 0x81: scanline 144 mode 1
```
Conclusions:
1. All scanlines' `mode 3` is signalled after `79-80` T4-cycles regardless of
   the current CGB speed.
1. The first scanline is `2-3` T4-cycles shorter
   (detectable only with CGB double speed).


## V-Blank

After restarting the LCD the first frame's V-Blank is signalled
`2` T4-cycles earlier than usual
(detectable only with CGB double speed).

* [enable_display/frame0_m1stat_1_dmg08_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_1_dmg08_cgb04c_out80.asm)
* [enable_display/frame0_m1stat_2_dmg08_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_2_dmg08_cgb04c_out81.asm)
* [enable_display/frame0_m1stat_ds_1_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_1_cgb04c_out80.asm)
* [enable_display/frame0_m1stat_ds_2_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_2_cgb04c_out81.asm)

The second frame's V-Blank is signalled at
`70222 + 70224 = 140446` T4-cycles
(detectable only with CGB double speed).

* [enable_display/frame1_m1stat_1_dmg08_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_1_dmg08_cgb04c_out80.asm)
* [enable_display/frame1_m1stat_2_dmg08_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_2_dmg08_cgb04c_out81.asm)
* [enable_display/frame1_m1stat_ds_1_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_1_cgb04c_out80.asm)
* [enable_display/frame1_m1stat_ds_2_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_2_cgb04c_out81.asm)
