
halt/late_m0int_halt_m0stat_scx2_*
halt/late_m0int_halt_m0stat_scx3_*

* requires accurate mode 0 interrupts (delayed by one t4-cycle)
    1. display_startstate/*
    2. m0int_m0stat/*
* HALT bug with IME=1 for interrupt during HALT
    => late_m0int_halt_m0stat_scx*: return to HALT, no int dispatch on next irq
* HALT (when successful) takes at least 3 m-cycles on DMG
    => late_m0int_halt_m0stat_scx3_2b_dmg08_cgb04c_out2
