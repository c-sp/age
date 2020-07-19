
Prerequisites:
[LCD mode 0 IRQ timing][l-t1]



halt/late_m0int_halt_m0stat_scx2_*
halt/late_m0int_halt_m0stat_scx3_*

* HALT bug with IME=1 for interrupt during HALT
    => late_m0int_halt_m0stat_scx*: return to HALT, no int dispatch on next irq
* HALT (when successful) takes at least 3 m-cycles on DMG
    => late_m0int_halt_m0stat_scx3_2b_dmg08_cgb04c_out2



[l-t1]: lcd-3-mode-interrupts.md#lcd-mode-0-irq-timing
