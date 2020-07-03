
# LCD mode 2

## Mode 2 interrupt timing

LCD mode 2 interrupt is triggered `2` 4-Mhz-clock cycles before actually
starting mode 2.
It is also triggered `2` 4-Mhz-clock cycles before entering v-blank.

* [enable_display/frame0_m2irq_count_1_dmg08_cgb04c_out98](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2irq_count_1_dmg08_cgb04c_out98.asm)
* [enable_display/frame0_m2irq_count_2_dmg08_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2irq_count_2_dmg08_cgb04c_out91.asm)
* [enable_display/frame0_m2irq_count_ds_1_cgb04c_out98](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2irq_count_ds_1_cgb04c_out98.asm)
* [enable_display/frame0_m2irq_count_ds_2_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame0_m2irq_count_ds_2_cgb04c_out91.asm)

LCD mode 2 interrupt is not triggered early for scanline 0.

* [enable_display/frame1_m2irq_count_1_dmg08_cgb04c_out98](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2irq_count_1_dmg08_cgb04c_out98.asm)
* [enable_display/frame1_m2irq_count_2_dmg08_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2irq_count_2_dmg08_cgb04c_out91.asm)
* [enable_display/frame1_m2irq_count_ds_1_cgb04c_out98](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2irq_count_ds_1_cgb04c_out98.asm)
* [enable_display/frame1_m2irq_count_ds_2_cgb04c_out91](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/enable_display/frame1_m2irq_count_ds_2_cgb04c_out91.asm)
