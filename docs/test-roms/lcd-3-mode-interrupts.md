
# LCD interrupts

## LCD mode 0 IRQ timing

Prerequisites:
[LCD M-cycle alignment][l-t1],
[LCD modes][l-t2]

* The mode 0 interrupt request is delayed by one T4-cycle:
  it occurs one T4-cycle after mode 3 ends.
* Getting mode 0 IRQ timing right allows us to examine [`HALT` timing][l-t3] at
  T4-cycle edges instead of M-cycle edges.

```
CGB single speed, SCX = 2

T4-cycle - event
----------------
   12240 - aligned frame to clock cycle 12233
   12748 - write STAT = 0x08 (8)
   12768 - write IE 0x02 (2)
   12784 - write IF 0x00 (0)
   12792 - enable interrupt dispatching after this CPU instruction
   12796 - interrupt dispatching enabled
   12944 - interrupt 0x02 (2) requested on clock cycle 12944
   12944 -     * mode 0 IRQ happened on scanline 1
   12944 -     * next mode 0 IRQ in 456 clock cycles (13400)
   12944 - dispatching interrupt, current PC = 0x018C (396), [PC] = 0x00 (0)
   12960 - clear IF flag 0x02 (2)
   12964 - interrupt dispatching disabled
   12964 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
   13144 - read STAT = 0x88 (136)
   13148 - read STAT = 0x8A (138), 204 cycles after interrupt

CGB single speed, SCX = 3

T4-cycle - event
----------------
   12240 - aligned frame to clock cycle 12233
   12748 - write STAT = 0x08 (8)
   12768 - write IE 0x02 (2)
   12784 - write IF 0x00 (0)
   12792 - enable interrupt dispatching after this CPU instruction
   12796 - interrupt dispatching enabled
   12948 - interrupt 0x02 (2) requested on clock cycle 12945
   12948 -     * mode 0 IRQ happened on scanline 1
   12948 -     * next mode 0 IRQ in 453 clock cycles (13401)
   12948 - dispatching interrupt, current PC = 0x018D (397), [PC] = 0x00 (0)
   12964 - clear IF flag 0x02 (2)
   12968 - interrupt dispatching disabled
   12968 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
   13144 - read STAT = 0x88 (136)
   13148 - read STAT = 0x8A (138), 200 cycles after interrupt
```
* [m0int_m0stat/m0int_m0stat_scx2_1_dmg08_cgb04c_out0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/m0int_m0stat/m0int_m0stat_scx2_1_dmg08_cgb04c_out0.asm)
* [m0int_m0stat/m0int_m0stat_scx2_2_dmg08_cgb04c_out2](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/m0int_m0stat/m0int_m0stat_scx2_2_dmg08_cgb04c_out2.asm)
* [m0int_m0stat/m0int_m0stat_scx3_1_dmg08_cgb04c_out0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/m0int_m0stat/m0int_m0stat_scx3_1_dmg08_cgb04c_out0.asm)
* [m0int_m0stat/m0int_m0stat_scx3_2_dmg08_cgb04c_out2](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/m0int_m0stat/m0int_m0stat_scx3_2_dmg08_cgb04c_out2.asm)

```
CGB double speed, SCX = 0

T4-cycle - event
----------------
   12236 - aligned frame to clock cycle 12233
   13174 - write STAT = 0x08 (8)
   13184 - write IE 0x02 (2)
   13188 - enable interrupt dispatching after this CPU instruction
   13190 - interrupt dispatching enabled
   13398 - interrupt 0x02 (2) requested on clock cycle 13398
   13398 -     * mode 0 IRQ happened on scanline 2
   13398 -     * next mode 0 IRQ in 456 clock cycles (13854)
   13398 - dispatching interrupt, current PC = 0x01DA (474), [PC] = 0x00 (0)
   13406 - clear IF flag 0x02 (2)
   13408 - interrupt dispatching disabled
   13408 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
   13600 - read STAT = 0x88 (136)
   13602 - read STAT = 0x8A (138), 204 cycles after interrupt


CGB double speed, SCX = 5

T4-cycle - event
----------------
   12236 - aligned frame to clock cycle 12233
   13174 - write STAT = 0x08 (8)
   13184 - write IE 0x02 (2)
   13188 - enable interrupt dispatching after this CPU instruction
   13190 - interrupt dispatching enabled
   13404 - interrupt 0x02 (2) requested on clock cycle 13403
   13404 -     * mode 0 IRQ happened on scanline 2
   13404 -     * next mode 0 IRQ in 455 clock cycles (13859)
   13404 - dispatching interrupt, current PC = 0x01DD (477), [PC] = 0x00 (0)
   13412 - clear IF flag 0x02 (2)
   13414 - interrupt dispatching disabled
   13414 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
   13600 - read STAT = 0x88 (136)
   13602 - read STAT = 0x8A (138), 198 cycles after interrupt
```
TODO CGB double speed: `enable_display/frame*_m0irg_count_scx*`
     are probably better suited for this
* [m0int_m0stat/m0int_m0stat_ds_1_cgb04c_out0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/m0int_m0stat/m0int_m0stat_ds_1_cgb04c_out0.asm)
* [m0int_m0stat/m0int_m0stat_ds_2_cgb04c_out2](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/m0int_m0stat/m0int_m0stat_ds_2_cgb04c_out2.asm)
* [m0int_m0stat/m0int_m0stat_scx5_ds_1_cgb04c_out0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/m0int_m0stat/m0int_m0stat_scx5_ds_1_cgb04c_out0.asm)
* [m0int_m0stat/m0int_m0stat_scx5_ds_2_cgb04c_out2](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/m0int_m0stat/m0int_m0stat_scx5_ds_2_cgb04c_out2.asm)


## LCD mode 2 interrupt timing

LCD mode 2 interrupt is triggered one M-cycle before the `STAT` mode 2 flag can
be seen.
It is also triggered one M-cycle before the `STAT` v-blank flag can be seen.

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


## TODO m2int_m3stat/scx/m2int_m3stat

* mode 3 extended by one T4-cycle for CGB double speed
  (otherwise IRQ and LCD alignment does not add up)

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
12940 - read STAT = 0xA3 (163)   <-- 252 cycles after mode 2 interrupt
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
12942 - read STAT = 0xA0 (160)   <-- 254 cycles after mode 2 interrupt
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
12942 - read STAT = 0xA0 (160)   <-- 254 cycles after mode 2 interrupt
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
12944 - read STAT = 0xA0 (160)   <-- 256 cycles after mode 2 interrupt
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
12942 - read STAT = 0xA3 (163)   <-- 254 cycles after mode 2 interrupt
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
12944 - read STAT = 0xA0 (160)   <-- 256 cycles after mode 2 interrupt
```



## TODO m2int_m0irq/m2int_m0irq_scx*_ifw*

`m2int_m0irq_scx3_ifw_1_dmg08_cgb04c_out2`:
m2 int - m0 irq > 252 cycles
```
12236 - aligned frame to clock cycle 12233
12320 - read STAT = 0x87 (135)
12352 - write STAT = 0x20 (32)
12372 - write IE 0x02 (2)
12388 - write IF 0x00 (0)
12396 - enable interrupt dispatching after this CPU instruction
12400 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x01B1 (433), [PC] = 0x00 (0)
12704 - clear IF flag 0x02 (2)
12708 - interrupt dispatching disabled
12708 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12740 - write STAT = 0x08 (8)
12940 - write IF 0x00 (0)
12948 - interrupt 0x02 (2) requested on clock cycle 12945
12948 -     * mode 0 IRQ happened on scanline 1
12948 -     * next mode 0 IRQ in 453 clock cycles (13401)
12952 - read IF 0xE2 (226)
```

`m2int_m0irq_scx3_ifw_2_dmg08_cgb04c_out0`:
m2 int - m0 irq <= 256 cycles
```
12236 - aligned frame to clock cycle 12233
12320 - read STAT = 0x87 (135)
12352 - write STAT = 0x20 (32)
12372 - write IE 0x02 (2)
12388 - write IF 0x00 (0)
12396 - enable interrupt dispatching after this CPU instruction
12400 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x01B1 (433), [PC] = 0x00 (0)
12704 - clear IF flag 0x02 (2)
12708 - interrupt dispatching disabled
12708 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12740 - write STAT = 0x08 (8)
12944 - write IF 0x00 (0)
12948 - interrupt 0x02 (2) requested on clock cycle 12945
12948 -     * mode 0 IRQ happened on scanline 1
12948 -     * next mode 0 IRQ in 453 clock cycles (13401)
12956 - read IF 0xE2 (226)
```

`m2int_m0irq_scx3_ifw_3_dmg08_cgb04c_out8`:
same as above: m2 int - m0 irq > 252 cycles,
just this time the irq is cancelled by overwriting IF

`m2int_m0irq_scx3_ifw_4_dmg08_cgb04c_out0`:
same as above: m2 int - m0 irq <= 256 cycles,
just this time the irq is cancelled by overwriting IF

`m2int_m0irq_scx3_ifw_ds_1_cgb04c_out2`:
m2 int - m0 irq > 254 cycles
```
12234 - aligned frame to clock cycle 12233
12334 - write STAT = 0x20 (32)
12344 - write IE 0x02 (2)
12352 - write IF 0x00 (0)
12356 - enable interrupt dispatching after this CPU instruction
12358 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x021B (539), [PC] = 0x00 (0)
12696 - clear IF flag 0x02 (2)
12698 - interrupt dispatching disabled
12698 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12714 - write STAT = 0x08 (8)
12942 - write IF 0x00 (0)
12946 - interrupt 0x02 (2) requested on clock cycle 12945
12946 -     * mode 0 IRQ happened on scanline 1
12946 -     * next mode 0 IRQ in 455 clock cycles (13401)
12948 - read IF 0xE2 (226)
```

`m2int_m0irq_scx3_ifw_ds_2_cgb04c_out0`:
m2 int - m0 irq <= 256 cycles
```
12234 - aligned frame to clock cycle 12233
12334 - write STAT = 0x20 (32)
12344 - write IE 0x02 (2)
12352 - write IF 0x00 (0)
12356 - enable interrupt dispatching after this CPU instruction
12358 - interrupt dispatching enabled
12688 - interrupt 0x02 (2) requested on clock cycle 12688
12688 -     * mode 2 IRQ happened on scanline 0
12688 -     * next mode 2 IRQ in 456 clock cycles (13144)
12688 - dispatching interrupt, current PC = 0x021B (539), [PC] = 0x00 (0)
12696 - clear IF flag 0x02 (2)
12698 - interrupt dispatching disabled
12698 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
12714 - write STAT = 0x08 (8)
12944 - write IF 0x00 (0)
12946 - interrupt 0x02 (2) requested on clock cycle 12945
12946 -     * mode 0 IRQ happened on scanline 1
12946 -     * next mode 0 IRQ in 455 clock cycles (13401)
12950 - read IF 0xE2 (226)
```


---

`m2int_m0irq_scx4_ifw_1_dmg08_cgb04c_out2`:
m2 int - m0 irq > 256 cycles

`m2int_m0irq_scx4_ifw_2_dmg08_cgb04c_out0`:
m2 int - m0 irq <= 260 cycles

---

conclusion:
* scx 3: m2 int - m0 irq = 256 cycles
* scx 4: m2 int - m0 irq = 257 cycles



[l-t1]: lcd-1-groundwork.md#lcd-m-cycle-alignment
[l-t2]: lcd-2-modes.md
[l-t3]: halt-timing.md
