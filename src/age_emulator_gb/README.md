
*This is still a draft*

*Until now Gameboy test rom analysis has been documented within the code.
To make the results more accessible,
from now on I will document my test rom findings in this readme.*


# Gameboy test rom analysis

## Sound

The frame sequencer has been
[described by Shay Green (blargg)](https://gist.github.com/drhelius/3652407).

Frequency sweep, volume sweep and length counters are controlled by the frame
sequencer.

### Length Counter Timing

**Gambatte test roms**

* sound/ch2_init_reset_length_counter_timing_nr52_1_dmg08_out2_cgb04c_out0
* sound/ch2_init_reset_length_counter_timing_nr52_2_dmg08_cgb04c_out0
* sound/ch2_init_reset_length_counter_timing_nr52_3_dmg08_cgb04c_out2
* sound/ch2_init_reset_length_counter_timing_nr52_4_dmg08_out2_cgb04c_out0

**Logs**

This is a combination of all four test logs.
The tests basically do the same thing,
they just read NR52 at different times.

```yaml
    cycle 7892    NR52 = 0x00  # APU off
    cycle 7912    NR52 = 0x80  # APU on (cycle 0001'1110'1110'1000)
    cycle 7932    NR21 = 0x3d  # Channel 2 length counter = 3
    cycle 7992    NR24 = 0xc7  # Channel 2 init with length counter
    cycle 32764   NR52 == 0x82 # Channel 2 active
    cycle 32768   NR52 == 0x80 # Channel 2 inactive

    cycle 44032   NR52 = 0x00  # APU off
    cycle 44052   NR52 = 0x80  # APU on (cycle 1010'1100'0001'0100)
    cycle 44072   NR21 = 0x3d  # Channel 2 length counter = 3
    cycle 44132   NR24 = 0xc7  # Channel 2 init with length counter
    cycle 81916   NR52 == 0x82 # Channel 2 active
    cycle 81920   NR52 == 0x80 # Channel 2 inactive
```

**Conclusion**

* Length counters are decremented on even frame sequencer steps
    (0, 2, 4 and 6).
* The frame sequencer is not reset when switching the APU off and back on,
    it will not restart with step 0.

```yaml
    cycle 0       <fs-0>       # length counter decrement
    cycle 7892    NR52 = 0x00  # APU off
    cycle 7912    NR52 = 0x80  # APU on (cycle 0001'1110'1110'1000)
    cycle 7932    NR21 = 0x3d  # Channel 2 length counter = 3
    cycle 7992    NR24 = 0xc7  # Channel 2 init with length counter LC=3
    cycle 7992                 # immediate length counter decrement
    cycle 8192    <fs-1>
    cycle 16384   <fs-2>       # length counter decrement, LC=1
    cycle 24576   <fs-3>
    cycle 32764   NR52 == 0x82 # Channel 2 active
    cycle 32768   <fs-4>       # length counter decrement, LC=0
    cycle 32768   NR52 == 0x80 # Channel 2 inactive
    cycle 40960   <fs-5>

    cycle 44032   NR52 = 0x00  # APU off
    cycle 44052   NR52 = 0x80  # APU on (cycle 1010'1100'0001'0100)
    cycle 44072   NR21 = 0x3d  # Channel 2 length counter = 3
    cycle 44132   NR24 = 0xc7  # Channel 2 init with length counter LC=3
    cycle 49152   <fs-6>       # length counter decrement, LC=2
    cycle 57344   <fs-7>
    cycle 65536   <fs-0>       # length counter decrement, LC=1
    cycle 73728   <fs-1>
    cycle 81916   NR52 == 0x82 # Channel 2 active
    cycle 81920   <fs-2>       # length counter decrement, LC=0
    cycle 81920   NR52 == 0x80 # Channel 2 inactive
    cycle 90112   <fs-3>
```

### Frequency Sweep Timing

**Gambatte test roms**

1. sound/ch1_init_reset_sweep_counter_timing_nr52_1_dmg08_cgb04c_out1
1. sound/ch1_init_reset_sweep_counter_timing_nr52_2_dmg08_out0_cgb04c_out1
1. sound/ch1_init_reset_sweep_counter_timing_nr52_3_dmg08_out0_cgb04c_out1
1. sound/ch1_init_reset_sweep_counter_timing_nr52_4_dmg08_cgb04c_out0

**Logs**

This is a combination of test logs.
The tests basically do the same thing,
they just read NR52 at different times.

*Test roms 1, 2:*

```yaml
    cycle 44032   NR52 = 0x00  # APU off
    cycle 44052   NR52 = 0x80  # APU on (cycle 1010'1100'0001'0100)
    cycle 44072   NR10 = 0x20  # Channel 1 frequency sweep (up), F-Counter=2
    cycle 44132   NR14 = 0x87  # Channel 1 init, first frequency sweep will overflow
    cycle 98304   NR52 == 0x81 # Channel 1 active
    cycle 98308   NR52 == 0x80 # Channel 1 inactive
```

*Test roms 3, 4:*

```yaml
    cycle 7892    NR52 = 0x00  # APU off
    cycle 7912    NR52 = 0x80  # APU on (cycle 0001'1110'1110'1000)
    cycle 7932    NR10 = 0x20  # Channel 1 frequency sweep (up), F-Counter=2
    cycle 7992    NR14 = 0x87  # Channel 1 init, first frequency sweep will overflow
    cycle 65536   NR52 == 0x81 # Channel 1 active
    cycle 65540   NR52 == 0x80 # Channel 1 inactive
```

**Conclusion**

* Frequency sweep happens on frame sequencer steps 0 and 4.
* The check for frequency sweep overflow is delayed by 4 cycles.
* The frame sequencer is not reset when switching the APU off and back on,
    it will not restart with step 0.

*Test roms 1, 2:*

```yaml
    cycle 0       <fs-0>       # frequency sweep
    cycle 8192    <fs-1>
    cycle 16384   <fs-2>
    cycle 24576   <fs-3>
    cycle 32768   <fs-4>       # frequency sweep
    cycle 40960   <fs-5>
    cycle 44032   NR52 = 0x00  # APU off
    cycle 44052   NR52 = 0x80  # APU on (cycle 1010'1100'0001'0100)
    cycle 44072   NR10 = 0x20  # Channel 1 frequency sweep (up), F-Counter=2
    cycle 44132   NR14 = 0x87  # Channel 1 init, first frequency sweep will overflow
    cycle 49152   <fs-6>
    cycle 57344   <fs-7>
    cycle 65536   <fs-0>       # frequency sweep, F-Counter=1
    cycle 73728   <fs-1>
    cycle 81920   <fs-2>
    cycle 90112   <fs-3>
    cycle 98304   <fs-4>       # frequency sweep, F-Counter=0 -> perform sweep
    cycle 98304   NR52 == 0x81 # Channel 1 active
    cycle 98308                # frequency sweep overflow check: positive
    cycle 98308   NR52 == 0x80 # Channel 1 inactive
    cycle 106496  <fs-5>
```

*Test roms 3, 4:*

```yaml
    cycle 0       <fs-0>       # frequency sweep
    cycle 7892    NR52 = 0x00  # APU off
    cycle 7912    NR52 = 0x80  # APU on (cycle 0001'1110'1110'1000)
    cycle 7932    NR10 = 0x20  # Channel 1 frequency sweep (up), F-Counter=2
    cycle 7992    NR14 = 0x87  # Channel 1 init, first frequency sweep will overflow
    cycle 8192    <fs-1>
    cycle 16384   <fs-2>
    cycle 24576   <fs-3>
    cycle 32768   <fs-4>       # frequency sweep, F-Counter=1
    cycle 40960   <fs-5>
    cycle 49152   <fs-6<
    cycle 57344   <fs-7>
    cycle 65536   <fs-0>       # frequency sweep, F-Counter=0 -> perform sweep
    cycle 65536   NR52 == 0x81 # Channel 1 active
    cycle 65540                # frequency sweep overflow check: positive
    cycle 65540   NR52 == 0x80 # Channel 1 inactive
    cycle 73728   <fs-1>
```
