
*This is still a draft*

*Until now Gameboy test rom analysis has been documented within the code.
To make the results more accessible,
from now on I will document my test rom findings in this readme.*


# Gameboy Test Rom Analysis

## Emulator Initialization

AGE does not include the Gameboy boot rom.
Instead emulation begins right after a real Gameboy has executed the boot rom:
at `PC 0x0100`.
Thus the emulator has to be initialized to a state where the Gameboy has been
already running for some time.
E.g. the cycle counter's initial value has to be the number of cycles elapsed
when the Gameboy reaches `PC 0x0100`,
it cannot be initialized to just zero.



### Initial Cycle

TODO cycle init using DIV test roms



## Sound

Frequency sweep, volume sweep and length counters are controlled by the frame
sequencer which has been
[described by Shay Green (blargg)](https://gist.github.com/drhelius/3652407).



### Gambatte Test Roms: Custom Modulation

Several Gambatte sound tests produce audio output as test result:
a test either finishes with audible output or with silence.

To achieve this, test roms apply a custom modulation to the duty waveform of
channel 1 or 2.
The modulation is applied by repeatedly changing the channel volume.
Additionally the channel is continuously initialized which each time restarts
the frequency timer and thus prevents any further duty waveform progression.
The resulting audio output is based on one and the same duty waveform
step being modulated in an infinite loop.

Example for `outaudio0` test (the modulation has no effect on a waveform input
of 0):
```
Duty Waveform 2  -----+                       +-----+-----+-----
                      |                       |
                      |                       |
                      +-----+-----+-----+-----+
                                          .
     Modulation                           +--+  +--+  +--+  +---
                                          .  |  |  |  |  |  |
                                          .  +--+  +--+  +--+
         Result  -----+                   .
                      |                   .
                      |                   .
                      +-----+-----+-----+-+--+--+--+--+--+--+---
                                          .
     duty waveform progression stops here ^
```

Example for `outaudio1` test:
```
Duty Waveform 2  -----+                       +-----+-----+-----
                      |                       |  .
                      |                       |  .
                      +-----+-----+-----+-----+  .
                                                 .
     Modulation                                  +--+  +--+  +--
                                                 .  |  |  |  |
                                                 .  +--+  +--+
                                                 .
         Result  -----+                       +--+--+  +--+  +--
                      |                       |  .  |  |  |  |
                      |                       |  .  +--+  +--+
                      +-----+-----+-----+-----+  .
                                                 .
            duty waveform progression stops here ^
```



### Initial Frequency Timer Delay

The channel 1 and 2 frequency timer for the first duty waveform step is
delayed by 8 cycles (4 output samples) on channel initialization.

**Gambatte test roms**

1. ch1_duty0_pos6_to_pos7_timing_1_dmg08_cgb04c_outaudio0
1. ch1_duty0_pos6_to_pos7_timing_2_dmg08_cgb04c_outaudio1
1. ch1_duty0_pos6_to_pos7_timing_ds_1_cgb04c_outaudio0
1. ch1_duty0_pos6_to_pos7_timing_ds_2_cgb04c_outaudio1

**Logs**

*Test roms 1, 2 (DMG)*

```yaml
    cycle 44008   NR52 = 0x00  # APU off
    cycle 44028   NR52 = 0x80  # APU on, duty waveforms reset to position 0
    cycle 44048   NR50 = 0x77  # SO1 volume = SO2 volume = 8
    cycle 44068   NR51 = 0x11  # Channel 1 enabled on SO1 and SO2
    cycle 44088   NR11 = 0x00  # Channel 1 duty 0 (waveform: 10000001)
    cycle 44108   NR12 = 0x80  # Channel 1 volume 8
    cycle 44128   NR13 = 0xC0  # Channel 1 lower frequency bits
    cycle 44148   NR14 = 0x87  # Channel 1 init, 256 cycles per waveform step
    cycle 44148   <waveform position 0>
    cycle 44412   <waveform position 1>  # 256 + 8 cycles after position 0
    cycle 44668   <waveform position 2>  # 256 cycles after position 1
    cycle 44924   <waveform position 3>  # 256 cycles after position 2
    cycle 45180   <waveform position 4>  # 256 cycles after position 3
    cycle 45436   <waveform position 5>  # 256 cycles after position 4
    cycle 45692   <waveform position 6>  # 256 cycles after position 5

Test rom 1:

    cycle 45920   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 45944   NR14 = 0x80  # Channel 1 init, frequency timer restarted
                               # (4 cycles before waveform position 7)
    <...>         <custom modulation of waveform position 6>  # outaudio0

Test rom 2:

    cycle 45924   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 45948   <waveform position 7>
    cycle 45948   NR14 = 0x80  # Channel 1 init, frequency timer restarted
    <...>         <custom modulation of waveform position 7>  # outaudio1
```

*TODO add logs for test roms 3, 4 after emulation of CGB speed switching is
complete (until then cycle numbers are not accurate)*



### Volume Envelope Timing

Volume updates by envelope happen only once in all eight frame sequencer steps.
This makes volume envelope test roms perfect for determining the correct frame
sequencer "alignment" during
[emulator initialization](#emulator-initialization).

**Gambatte test roms**

TODO finish

**Logs**

TODO finish



### Frame Sequencer Step Skipping

When switching on the APU while cycle bit 14 is set the next frame sequencer
step is skipped.

**Gambatte test roms**

TODO finish

**Logs**

TODO finish

```
test      | type  | steps | cycle APU on        | cycle channel init
----------+-------+-------+---------------------+----------------------
          |       |       |                     |
timing_1  | DMG-0 | 7     | 1010'1111'1111'1000 |   1011'0000'0111'0000
timing_2  | DMG-1 | 8     | 1010'1111'1111'1000 |   1011'0000'0111'0000
          |       |       |                     |
timing_5  | DMG-0 | 8     | 1010'1111'1111'1100 |   1011'0000'0111'0100
timing_6  | DMG-1 | 9     | 1010'1111'1111'1100 |   1011'0000'0111'0100
          |       |       |                     |
timing_9  | DMG-0 | 6+1   | 1010'1011'1111'1100 | 1'0111'1111'1111'1000
timing_10 | DMG-1 | 6+2   | 1010'1011'1111'1100 | 1'0111'1111'1111'1000
          |       |       |                     |
timing_11 | DMG-0 | 6+9   | 1010'1011'1111'1100 | 1'0111'1111'1111'1100
timing_12 | DMG-1 | 6+10  | 1010'1011'1111'1100 | 1'0111'1111'1111'1100
          |       |       |                     |
----------+-------+-------+---------------------+----------------------
          |       |       |                     |
timing_3  | CGB-0 | 7     | 0010'1111'1111'1000 |   0011'0000'0111'0000
timing_4  | CGB-1 | 8     | 0010'1111'1111'1000 |   0011'0000'0111'0000
          |       |       |                     |
timing_7  | CGB-0 | 8     | 0010'1111'1111'1100 |   0011'0000'0111'0100
timing_8  | CGB-1 | 9     | 0010'1111'1111'1100 |   0011'0000'0111'0100
          |       |       |                     |
timing_13 | CGB-0 | 7+1   | 0001'1110'1101'0000 |   1111'1111'1111'1000
timing_14 | CGB-1 | 7+2   | 0001'1110'1101'0000 |   1111'1111'1111'1000
          |       |       |                     |
timing_15 | CGB-0 | 7+9   | 0001'1110'1101'0000 |   1111'1111'1111'1100
timing_16 | CGB-1 | 7+10  | 0001'1110'1101'0000 |   1111'1111'1111'1100
          |       |       |                     |
----------+-------+-------+---------------------+----------------------
```



### Length Counter Timing

**Gambatte test roms**

1. sound/ch2_init_reset_length_counter_timing_nr52_1_dmg08_out2_cgb04c_out0
1. sound/ch2_init_reset_length_counter_timing_nr52_2_dmg08_cgb04c_out0
1. sound/ch2_init_reset_length_counter_timing_nr52_3_dmg08_cgb04c_out2
1. sound/ch2_init_reset_length_counter_timing_nr52_4_dmg08_out2_cgb04c_out0

Sound channel 2 is initialized to automatically deactivate when it's length
counter reaches zero.
NR52 is checked for channel 2 activity around the time length counters are
expected to be decremented.

**Logs**

TODO align frame sequencer steps
TODO split logs into DMG and CGB

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

TODO align frame sequencer steps

**Gambatte test roms**

1. sound/ch1_init_reset_sweep_counter_timing_nr52_1_dmg08_cgb04c_out1
1. sound/ch1_init_reset_sweep_counter_timing_nr52_2_dmg08_out0_cgb04c_out1
1. sound/ch1_init_reset_sweep_counter_timing_nr52_3_dmg08_out0_cgb04c_out1
1. sound/ch1_init_reset_sweep_counter_timing_nr52_4_dmg08_cgb04c_out0

Sound channel 2 is initialized to automatically deactivate when frequency sweep
overflows.
NR52 is checked for channel 2 activity around the time the frame sequencer is
expected to perform frequency sweep.

**Logs**

This is a combination of test logs.
The tests basically do the same thing,
they just read NR52 at different times.

*Test roms 1, 2 (DMG):*

```yaml
    cycle 44032   NR52 = 0x00  # APU off
    cycle 44052   NR52 = 0x80  # APU on (cycle 1010'1100'0001'0100)
    cycle 44072   NR10 = 0x20  # Channel 1 frequency sweep (up), F-Counter=2
    cycle 44132   NR14 = 0x87  # Channel 1 init, first frequency sweep overflows
    cycle 98304   NR52 == 0x81 # Channel 1 active
    cycle 98308   NR52 == 0x80 # Channel 1 inactive
```

*Test roms 3, 4 (CGB):*

```yaml
    cycle 7892    NR52 = 0x00  # APU off
    cycle 7912    NR52 = 0x80  # APU on (cycle 0001'1110'1110'1000)
    cycle 7932    NR10 = 0x20  # Channel 1 frequency sweep (up), F-Counter=2
    cycle 7992    NR14 = 0x87  # Channel 1 init, first frequency sweep overflows
    cycle 65536   NR52 == 0x81 # Channel 1 active
    cycle 65540   NR52 == 0x80 # Channel 1 inactive
```

**Conclusion**

* Frequency sweep happens on frame sequencer steps 0 and 4.
* The check for frequency sweep overflow is delayed by 4 cycles.
* The frame sequencer is not reset when switching the APU off and back on,
    it will not restart with step 0.

*Test roms 1, 2 (DMG):*

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
    cycle 44132   NR14 = 0x87  # Channel 1 init, first frequency sweep overflows
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

*Test roms 3, 4 (CGB):*

```yaml
    cycle 0       <fs-0>       # frequency sweep
    cycle 7892    NR52 = 0x00  # APU off
    cycle 7912    NR52 = 0x80  # APU on (cycle 0001'1110'1110'1000)
    cycle 7932    NR10 = 0x20  # Channel 1 frequency sweep (up), F-Counter=2
    cycle 7992    NR14 = 0x87  # Channel 1 init, first frequency sweep overflows
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
