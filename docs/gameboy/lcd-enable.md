
# Switching on the LCD


## Begin with mode 0

When switching on the LCD,
the first scanline begins with `mode 0` instead of `mode 2` for `80`
4-Mhz-clock cycles.

* [enable_display/nextstat_1_dmg08_cgb04c_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/nextstat_1_dmg08_cgb04c_out84.asm)
* [enable_display/nextstat_2_dmg08_cgb04c_out87](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/nextstat_2_dmg08_cgb04c_out87.asm)


## Scanline clock cycles #1

The following Gambatte test roms check `mode 2` to `mode 3` transition for
scanline 0 to 144 of the first frame after switching on the LCD.

* [enable_display/frame0_m3stat_count_1_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_1_dmg08_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_2_dmg08_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_ds_1_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_ds_1_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_ds_2_cgb04c_out90.asm)

Conclusions:
1. All scanline's `mode 3` is signalled after `80` 4-Mhz-clock cycles regardless
   of the current CGB speed.
1. The first scanline is `2` 4-Mhz-clock cycles shorter
   (detectable only with CGB double speed).

```
4-Mhz clock - event
-------------------
              LCD switched off
          0 - LCD switched on
         76 - STAT = 0x84: scanline 0 m2-end
         80 - STAT = 0x87: scanline 0 m3-begin
        532 - STAT = 0x82: scanline 1 m2-end
        536 - STAT = 0x83: scanline 1 m3-begin, 456 clock cycles after scanline 0 m3-begin
        988 - STAT = 0x82: scanline 2 m2-end
        992 - STAT = 0x83: scanline 2 m3-begin, 456 clock cycles after scanline 1 m3-begin
          <...>
      65284 - STAT = 0x82: scanline 143 m2-end
      65288 - STAT = 0x83: scanline 143 m3-begin
      65740 - STAT = 0x81: scanline 144 m1
      65744 - STAT = 0x81: scanline 144 m1
```
CGB double speed:
```
4-Mhz clock - event
-------------------
              LCD switched off
          0 - LCD switched on
         78 - STAT = 0x84: scanline 0 m2-end
         80 - STAT = 0x87: scanline 0 m3-begin
        532 - STAT = 0x82: scanline 1 m2-end
        534 - STAT = 0x83: scanline 1 m3-begin, 454 (!) clock cycles after scanline 0 m3-begin
        988 - STAT = 0x82: scanline 2 m2-end
        990 - STAT = 0x83: scanline 2 m3-begin, 456 clock cycles after scanline 1 m3-begin
          <...>
      65284 - STAT = 0x82: scanline 143 m2-end
      65286 - STAT = 0x83: scanline 143 m3-begin
      65740 - STAT = 0x81: scanline 144 m1
      65742 - STAT = 0x81: scanline 144 m1
```


## Scanline clock cycles #2

This can be confirmed using the following Gambatte test roms measuring `mode 0`
to `mode 2` transitions.
With CGB double speed not active the STAT LY-match flag for LYC 0 is cleared `2`
4-Mhz-clock cycle before scanline 0 finishes.

* [enable_display/frame0_m2stat_count_1_dmg08_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_1_dmg08_cgb04c_out91.asm)
* [enable_display/frame0_m2stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_2_dmg08_cgb04c_out90.asm)
* [enable_display/frame0_m2stat_count_ds_1_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_ds_1_cgb04c_out91.asm)
* [enable_display/frame0_m2stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_ds_2_cgb04c_out90.asm)

With CGB double speed not active `mode 0` is signalled on v-blank's last
machine cycle. 

* [enable_display/frame1_m2stat_count_1_dmg08_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2stat_count_1_dmg08_cgb04c_out91.asm)
* [enable_display/frame1_m2stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2stat_count_2_dmg08_cgb04c_out90.asm)
* [enable_display/frame1_m2stat_count_ds_1_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2stat_count_ds_1_cgb04c_out91.asm)
* [enable_display/frame1_m2stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2stat_count_ds_2_cgb04c_out90.asm)


## V-Blank

After switching on the LCD the first frame's V-Blank is signalled
`2` 4-Mhz-clock cycles earlier than usual
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
`70222 + 70224 = 140446` 4-Mhz-clock cycles
(detectable only with CGB double speed).

* [enable_display/frame1_m1stat_1_dmg08_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_1_dmg08_cgb04c_out80.asm)
* [enable_display/frame1_m1stat_2_dmg08_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_2_dmg08_cgb04c_out81.asm)
* [enable_display/frame1_m1stat_ds_1_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_1_cgb04c_out80.asm)
* [enable_display/frame1_m1stat_ds_2_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_2_cgb04c_out81.asm)


## LY

LY is incremented `2` 4-Mhz-clock cycles before the respective scanline is
finished.
LY 153 is shorter than usual.
LY 0 starts early.

* [enable_display/frame0_ly_count_1_dmg08_cgb04c_out99](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_ly_count_1_dmg08_cgb04c_out99.asm)
* [enable_display/frame0_ly_count_2_dmg08_cgb04c_out9A](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_ly_count_2_dmg08_cgb04c_out9A.asm)
* [enable_display/frame0_ly_count_ds_1_cgb04c_out99](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_ly_count_ds_1_cgb04c_out99.asm)
* [enable_display/frame0_ly_count_ds_2_cgb04c_out9A](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_ly_count_ds_2_cgb04c_out9A.asm)

LY 153 is visible for just one machine cycle when scanline 153 begins
(two machine cycles for CGB double speed).

* [enable_display/frame1_ly_count_1_dmg08_cgb04c_out99](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_ly_count_1_dmg08_cgb04c_out99.asm)
* [enable_display/frame1_ly_count_2_dmg08_cgb04c_out9A](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_ly_count_2_dmg08_cgb04c_out9A.asm)
* [enable_display/frame1_ly_count_ds_1_cgb04c_out99](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_ly_count_ds_1_cgb04c_out99.asm)
* [enable_display/frame1_ly_count_ds_2_cgb04c_out9A](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_ly_count_ds_2_cgb04c_out9A.asm)
