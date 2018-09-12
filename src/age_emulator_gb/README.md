
*This is still a draft*

*Until now Gameboy test rom analysis has been documented within the code.
To make the results more accessible,
from now on I will document my test rom findings in this readme.*


# Gameboy test rom analysis

## Sound

### Frame sequencer

The frame sequencer has been
[described by Shay Green (blargg)](https://gist.github.com/drhelius/3652407).

Frequency sweep, volume sweep and length counters are controlled by the frame
sequencer.

---

**Gambatte test roms**

* sound/ch2_init_reset_length_counter_timing_nr52_1_dmg08_out2_cgb04c_out0
* sound/ch2_init_reset_length_counter_timing_nr52_2_dmg08_cgb04c_out0
* sound/ch2_init_reset_length_counter_timing_nr52_3_dmg08_cgb04c_out2
* sound/ch2_init_reset_length_counter_timing_nr52_4_dmg08_out2_cgb04c_out0

**Logs**

    cycle 7892    W-52=0x00: APU off
    cycle 7912    W-52=0x80: APU on (cycle 0001'1110'1110'1000)
    cycle 7932    W-21=0x3d: Channel 2 length counter = 3
    cycle 7992    W-24=0xc7: Channel 2 init with length-counter
    cycle 32764   R-52==0x82: Channel 2 active
    cycle 32768   R-52==0x80: Channel 2 inactive

    cycle 44032   W-52=0x00: APU off
    cycle 44052   W-52=0x80: APU on (cycle 1010'1100'0001'0100)
    cycle 44072   W-21=0x3d: Channel 2 length counter = 3
    cycle 44132   W-24=0xc7: Channel 2 init with length-counter
    cycle 81916   R-52==0x82: Channel 2 active
    cycle 81920   R-52==0x80: Channel 2 inactive

**Conclusion**

    cycle 0       -> fs-0 (length counter decrement)
    cycle 7892    W-52=0x00: APU off
    cycle 7912    W-52=0x80: APU on (cycle 0001'1110'1110'1000)
    cycle 7932    W-21=0x3d: Channel 2 length counter = 3
    cycle 7992    W-24=0xc7: Channel 2 init with length-counter
    cycle 7992    -> immediate length-counter decrement since the
                     last fs (fs-0) decremented length counter
    cycle 8192    -> fs-1
    cycle 16384   -> fs-2 (length counter decrement)
    cycle 24576   -> fs-3
    cycle 32764   R-52==0x82: Channel 2 active
    cycle 32768   -> fs-4 (length counter decrement)
    cycle 32768   R-52==0x80: Channel 2 inactive
    cycle 40960   -> fs-5

    cycle 44032   W-52=0x00: APU off
    cycle 44052   W-52=0x80: APU on (cycle 1010'1100'0001'0100)
    cycle 44072   W-21=0x3d: Channel 2 length counter = 3
    cycle 44132   W-24=0xc7: Channel 2 init with length-counter
    cycle 49152   -> fs-6 (length counter decrement)
    cycle 57344   -> fs-7
    cycle 65536   -> fs-0 (length counter decrement)
    cycle 73728   -> fs-1
    cycle 81916   R-52==0x82: Channel 2 active
    cycle 81920   -> fs-2 (length counter decrement)
    cycle 81920   R-52==0x80: Channel 2 inactive
    cycle 90112   -> fs-3
    cycle 98304   -> fs-4 (length counter decrement)

* The frame sequencer is not reset when switching the APU off and back on,
    it will not restart with step 0.
* Frame sequencer steps from cycle 0 to the initial cycle the emulator is
    started with cannot be ignored.
