
# Switching on the LCD


## First frame

### Begin with mode 0

When switching on the LCD,
the first scanline begins with mode `0` instead of mode `2` for `80`
4-Mhz-clock cycles.

* [enable_display/nextstat_1_dmg08_cgb04c_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/nextstat_1_dmg08_cgb04c_out84.asm)
* [enable_display/nextstat_2_dmg08_cgb04c_out87](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/nextstat_2_dmg08_cgb04c_out87.asm)


### First Mode 3

When switching on the LCD,
mode 3 for LY 0 is signalled after `80` 4-Mhz-clock cycles regardless of the
current CGB speed.

The following Gambatte test roms check mode `2` to mode `3` transition for all
scanlines of the first frame.

* [enable_display/frame0_m3stat_count_1_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_1_dmg08_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_2_dmg08_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_ds_1_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_ds_1_cgb04c_out90.asm)
* [enable_display/frame0_m3stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m3stat_count_ds_2_cgb04c_out90.asm)


### Short first scanline

When switching on the LCD,
the first scanline is `2` 4-Mhz-clock cycles shorter than usual
(`454` instead of `456` 4-Mhz-clock cycles).

* (1)[enable_display/frame0_m2stat_count_1_dmg08_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_1_dmg08_cgb04c_out91.asm)
* [enable_display/frame0_m2stat_count_2_dmg08_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_2_dmg08_cgb04c_out90.asm)
* (1)[enable_display/frame0_m2stat_count_ds_1_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_ds_1_cgb04c_out91.asm)
* [enable_display/frame0_m2stat_count_ds_2_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2stat_count_ds_2_cgb04c_out90.asm)

(1) seems to require correct STAT LY-match-flag timings:
STAT LY-match-flag cleared before scanline finished.


### Early first V-Blank

The first frame's V-Blank is signalled one machine cycle earlier for CGB double
speed mode.
That would be `2` 4-Mhz-clock cycles regardless of the current CGB speed.

* [enable_display/frame0_m1stat_1_dmg08_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_1_dmg08_cgb04c_out80.asm)
* [enable_display/frame0_m1stat_2_dmg08_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_2_dmg08_cgb04c_out81.asm)
* [enable_display/frame0_m1stat_ds_1_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_1_cgb04c_out80.asm)
* [enable_display/frame0_m1stat_ds_2_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_2_cgb04c_out81.asm)


## Second frame

### V-Blank

The second frame's V-Blank is signalled `70222 + 70224 = 140446`
4-Mhz-clock cycles after switching on the LCD.

* [enable_display/frame1_m1stat_1_dmg08_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_1_dmg08_cgb04c_out80.asm)
* [enable_display/frame1_m1stat_2_dmg08_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_2_dmg08_cgb04c_out81.asm)
* [enable_display/frame1_m1stat_ds_1_cgb04c_out80](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_1_cgb04c_out80.asm)
* [enable_display/frame1_m1stat_ds_2_cgb04c_out81](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m1stat_ds_2_cgb04c_out81.asm)
