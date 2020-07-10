
halt/late_m0int_halt_m0stat_scx2_*
halt/late_m0int_halt_m0stat_scx3_*

* requires accurate mode 0 interrupts (delayed by one t4-cycle)
    1. display_startstate/*
    2. m0int_m0stat/*
* HALT bug with IME=1 for interrupt during HALT
    => late_m0int_halt_m0stat_scx*: return to HALT, no int dispatch on next irq
* HALT (when successful) takes at least 3 m-cycles on DMG
    => late_m0int_halt_m0stat_scx3_2b_dmg08_cgb04c_out2


```
T4-cycle - event
----------------
   12236 - aligned frame to clock cycle 12233
   12352 - write STAT = 0x20 (32)
   12368 - write IF 0x00 (0)
   12388 - write IE 0x02 (2)
   12396 - enable interrupt dispatching after this CPU instruction
   12400 - interrupt dispatching enabled
   12688 - interrupt 0x02 (2) requested on clock cycle 12687
   12688 -     * next mode 2 IRQ in 451 clock cycles (13143)
   12688 - dispatching interrupt, current PC = 0x01B1 (433), [PC] = 0x00 (0)
   12704 - clear IF flag 0x02 (2)
   12708 - interrupt dispatching disabled
   12708 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)
   12728 - read STAT = 0xA2 (162)
   12760 - write STAT = 0x08 (8)
   12768 - enable interrupt dispatching after this CPU instruction
   12772 - interrupt dispatching enabled

   12932 - HALTed (*_scx2_1*)
   12936 - HALTed (*_scx2_2*)
   12940 - HALTed (*_scx2_3*)
   12944 - interrupt 0x02 (2) requested on clock cycle 12943
   12948 -     * HALT termination M-cycle (CGB)
   12948 -     * next mode 0 IRQ in 451 clock cycles (13399)
   12948 - dispatching interrupt, current PC = 0x1034 (4148), [PC] = 0x00 (0)
   12964 - clear IF flag 0x02 (2)
   12968 - interrupt dispatching disabled
   12968 - interrupt 0x02 (2) (lcd) dispatched to 0x0048 (72)

   13144 - read STAT = 0x88 (136) (*_scx2_1a_*, *_scx2_2a_*)
   13148 - read STAT = 0x8A (138) (*_scx2_1b_*, *_scx2_2b_*)
   13216 - read STAT = 0x8A (138) (*_scx2_3a_*)
   13220 - read STAT = 0x8A (138) (*_scx2_3b_*)
```
