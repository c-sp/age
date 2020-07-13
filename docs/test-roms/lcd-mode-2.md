
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


## m2int_m3stat

m2int_scx8_m3stat_ds_1_cgb04c_out3
```
12242 - aligned frame to clock cycle 12233
12312 - read STAT = 0x86 (134)
12326 - read STAT = 0x87 (135)
12342 - write STAT = 0x20 (32)
12352 - write IE 0x02 (2)
12362 - write SCX = 0x08 (8)
12366 - enable interrupt dispatching after this CPU instruction
12368 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x0214 (532), [PC] = 0x00 (0)
12696 - clear IF flag 0x02 (2)
12698 - interrupt dispatching disabled
12698 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12940 - read STAT = 0xA3 (163)   <-- 252 cycles after interrupt
```
m2int_scx8_m3stat_ds_2_cgb04c_out0
```
12242 - aligned frame to clock cycle 12233
12312 - read STAT = 0x86 (134)
12326 - read STAT = 0x87 (135)
12342 - write STAT = 0x20 (32)
12352 - write IE 0x02 (2)
12362 - write SCX = 0x08 (8)
12366 - enable interrupt dispatching after this CPU instruction
12368 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x0214 (532), [PC] = 0x00 (0)
12696 - clear IF flag 0x02 (2)
12698 - interrupt dispatching disabled
12698 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12942 - read STAT = 0xA0 (160)   <-- 254 cycles after interrupt
```

---

m2int_scx1_m3stat_ds_1_cgb04c_out3
```
12242 - aligned frame to clock cycle 12233
12312 - read STAT = 0x86 (134)
12326 - read STAT = 0x87 (135)
12342 - write STAT = 0x20 (32)
12352 - write IE 0x02 (2)
12362 - write SCX = 0x01 (1)
12366 - enable interrupt dispatching after this CPU instruction
12368 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x0214 (532), [PC] = 0x00 (0)
12696 - clear IF flag 0x02 (2)
12698 - interrupt dispatching disabled
12698 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12942 - read STAT = 0xA0 (160)   <-- 254 cycles after interrupt
```
m2int_scx1_m3stat_ds_2_cgb04c_out0
```
12242 - aligned frame to clock cycle 12233
12312 - read STAT = 0x86 (134)
12326 - read STAT = 0x87 (135)
12342 - write STAT = 0x20 (32)
12352 - write IE 0x02 (2)
12362 - write SCX = 0x01 (1)
12366 - enable interrupt dispatching after this CPU instruction
12368 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x0214 (532), [PC] = 0x00 (0)
12696 - clear IF flag 0x02 (2)
12698 - interrupt dispatching disabled
12698 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12944 - read STAT = 0xA0 (160)   <-- 256 cycles after interrupt
```

---

m2int_scx2_m3stat_ds_1_cgb04c_out3
```
12242 - aligned frame to clock cycle 12233
12312 - read STAT = 0x86 (134)
12326 - read STAT = 0x87 (135)
12342 - write STAT = 0x20 (32)
12352 - write IE 0x02 (2)
12362 - write SCX = 0x02 (2)
12366 - enable interrupt dispatching after this CPU instruction
12368 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x0214 (532), [PC] = 0x00 (0)
12696 - clear IF flag 0x02 (2)
12698 - interrupt dispatching disabled
12698 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12942 - read STAT = 0xA3 (163)   <-- 254 cycles after interrupt
```
m2int_scx2_m3stat_ds_2_cgb04c_out0
```
12242 - aligned frame to clock cycle 12233
12312 - read STAT = 0x86 (134)
12326 - read STAT = 0x87 (135)
12342 - write STAT = 0x20 (32)
12352 - write IE 0x02 (2)
12362 - write SCX = 0x02 (2)
12366 - enable interrupt dispatching after this CPU instruction
12368 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x0214 (532), [PC] = 0x00 (0)
12696 - clear IF flag 0x02 (2)
12698 - interrupt dispatching disabled
12698 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12944 - read STAT = 0xA0 (160)   <-- 256 cycles after interrupt
```
