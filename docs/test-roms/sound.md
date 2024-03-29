
*This is still a draft*

*Until now Gameboy test rom analysis has been documented inside the code files.
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

The Gameboy APU has been
[described by Shay Green (blargg)](https://gist.github.com/drhelius/3652407)
in great detail.
[Sinamas](https://github.com/sinamas) has extended that by creating
[Gameboy APU test roms](https://github.com/sinamas/gambatte/tree/master/test/hwtests/sound)
for several corner cases as part of his Gameboy emulator
[Gambatte](https://github.com/sinamas/gambatte).



### Gambatte Test Roms: Custom Modulation

Several
[Gambatte sound tests](https://github.com/sinamas/gambatte/tree/master/test/hwtests/sound)
produce audio output as test result:
a test either finishes with audible sound or with silence.

To achieve this, test roms apply a custom modulation to the duty waveform of
channel 1 or 2.
The modulation is applied by repeatedly changing the channel volume.
Additionally the channel is continuously initialized which each time restarts
the frequency timer and thus prevents any further duty waveform progression.
The resulting audio output is based on one and the same duty waveform
step being modulated in an infinite loop.

Example for an `outaudio0` test (the modulation has no effect on a waveform
input of 0):
```
Duty Waveform 2  -----+                       +-----+-----+-----
                      |                       |
                      |                       |
                      +-----+-----+-----+-----+
                                          .
                              Modulation  +--+  +--+  +--+  +---
                                          .  |  |  |  |  |  |
                                          .  +--+  +--+  +--+
         Result  -----+                   .
                      |                   .
                      |                   .
                      +-----+-----+-----+-+--+--+--+--+--+--+---
                                          .
     duty waveform progression stops here ^
```

Example for an `outaudio1` test:
```
Duty Waveform 2  -----+                       +-----+-----+-----
                      |                       |  .
                      |                       |  .
                      +-----+-----+-----+-----+  .
                                                 .
                                     Modulation  +--+  +--+  +--
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



#### Channel 1 Initial State

Channel 1 is used by the Gameboy boot rom to play the iconic "ding-ding" sound
while the Nintendo logo is displayed.
When reaching `PC 0x100` it produces no audible sound any more due to it's
volume being zero,
but it's frequency timer is still running and iterates over the configured duty
waveform.

For [emulator initialization](#emulator-initialization) the duty waveform
position and frequency timer state can be calculated by determining when the
waveform changes from 0 to 1 and vice versa.

**Gambatte test roms**

1. `ch1_init_pos_1_dmg08_outaudio0_cgb04c_outaudio1`
1. `ch1_init_pos_2_dmg08_cgb04c_outaudio1`
1. `ch1_init_pos_3_dmg08_cgb04c_outaudio1`
1. `ch1_init_pos_4_dmg08_outaudio1_cgb04c_outaudio0`
1. `ch1_init_pos_5_dmg08_outaudio1_cgb04c_outaudio0`
1. `ch1_init_pos_6_dmg08_cgb04c_outaudio0`
1. `ch1_init_pos_7_dmg08_cgb04c_outaudio0`
1. `ch1_init_pos_8_dmg08_outaudio0_cgb04c_outaudio1`

[The modulation described above](#gambatte-test-roms-custom-modulation) is used
to "lock" the duty waveform position starting from a specific cycle.

**Logs**

*Test roms 1, 2, 5, 6 (DMG)*
```yaml
    cycle 44004   <waveform position 3 = 0>
    cycle 44020   NR10 = 0x00  # turn off frequency sweep
    cycle 44040   NR11 = 0x80  # select duty waveform 2 (10000111)
    cycle 44256   <waveform position 4 = 0>

Test rom 1:
    cycle 44480   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 44504   NR14 = 0x80  # Channel 1 init, frequency timer restarted
                               # (4 cycles before waveform position 5)
    <...>         <custom modulation of waveform position 4>  # outaudio0

Test rom 2:
    cycle 44484   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 44508   <waveform position 5 = 1>
    cycle 44508   NR14 = 0x80  # Channel 1 init, frequency timer restarted
    <...>         <custom modulation of waveform position 5>  # outaudio1

Test rom 5:
    cycle 44508   <waveform position 5 = 1>
    cycle 44760   <waveform position 6 = 1>
    cycle 45012   <waveform position 7 = 1>
    cycle 45264   <waveform position 0 = 1>
    cycle 45488   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 45512   NR14 = 0x80  # Channel 1 init, frequency timer restarted
                               # (4 cycles before waveform position 1)
    <...>         <custom modulation of waveform position 0>  # outaudio1

Test rom 6:
    cycle 44508   <waveform position 5 = 1>
    cycle 44760   <waveform position 6 = 1>
    cycle 45012   <waveform position 7 = 1>
    cycle 45264   <waveform position 0 = 1>
    cycle 45492   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 45516   <waveform position 1 = 0>
    cycle 45516   NR14 = 0x80  # Channel 1 init, frequency timer restarted
    <...>         <custom modulation of waveform position 1>  # outaudio0
```

*Test roms 3, 4, 7, 8 (CGB)*
```yaml
    cycle 7736   <waveform position 6 = 1>
    cycle 7880   NR10 = 0x00  # turn off frequency sweep
    cycle 7900   NR11 = 0x80  # select duty waveform 2 (10000111)
    cycle 7988   <waveform position 7 = 1>
    cycle 8240   <waveform position 0 = 1>

Test rom 3:
    cycle 8464   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 8488   NR14 = 0x80  # Channel 1 init, frequency timer restarted
                              # (4 cycles before waveform position 1)
    <...>        <custom modulation of waveform position 0>  # outaudio1

Test rom 4:
    cycle 8468   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 8492   <waveform position 1 = 0>
    cycle 8492   NR14 = 0x80  # Channel 1 init, frequency timer restarted
    <...>        <custom modulation of waveform position 1>  # outaudio0

Test rom 7:
    cycle 8492   <waveform position 1 = 0>
    cycle 8744   <waveform position 2 = 0>
    cycle 8996   <waveform position 3 = 0>
    cycle 9248   <waveform position 4 = 0>
    cycle 9472   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 9496   NR14 = 0x80  # Channel 1 init, frequency timer restarted
                              # (4 cycles before waveform position 5)
    <...>        <custom modulation of waveform position 4>  # outaudio1

Test rom 8:
    cycle 8492   <waveform position 1 = 0>
    cycle 8744   <waveform position 2 = 0>
    cycle 8996   <waveform position 3 = 0>
    cycle 9248   <waveform position 4 = 0>
    cycle 9476   NR12 = 0xC0  # Channel 1, set volume 12
    cycle 9500   <waveform position 5 = 1>
    cycle 9500   NR14 = 0x80  # Channel 1 init, frequency timer restarted
    <...>        <custom modulation of waveform position 5>  # outaudio0
```



#### Frequency Timer Delay on Channel Initialization

The channel 1 and 2 frequency timer for the first duty waveform step is
delayed by 8 cycles on channel initialization.

**Gambatte test roms**

1. `ch1_duty0_pos6_to_pos7_timing_1_dmg08_cgb04c_outaudio0`
1. `ch1_duty0_pos6_to_pos7_timing_2_dmg08_cgb04c_outaudio1`
1. `ch1_duty0_pos6_to_pos7_timing_ds_1_cgb04c_outaudio0`
1. `ch1_duty0_pos6_to_pos7_timing_ds_2_cgb04c_outaudio1`

The tests measure the exact cycle when duty 0 waveform position 7 is reached
by applying
[the modulation described above](#gambatte-test-roms-custom-modulation)
around that cycle.
Based on the cycle difference between initializing channel 1 and reaching
waveform position 7 the initial delay can be calculated.

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



### Gambatte Volume Envelope Tests

Several
[Gambatte sound tests](https://github.com/sinamas/gambatte/tree/master/test/hwtests/sound)
are based on determining the exact cycle of the frame sequencer step handling
the volume envelope.
The process is always the same:

*   Channel 2 is initialized with volume 0 (silence) and incrementing volume
    envelope so that the next volume envelope step (frame sequencer step 7)
    will end that silence by changing the volume from 0 to 1.
*   To determine the exact cycle of that step,
    a test disables the volume envelope around that cycle by setting the volume
    envelope period to zero.
*   Depending on wether or not disabling the volume envelope was successful
    (it was disabled right before or right after the volume envelope step),
    the test finishes either with silence or with audible sound.



#### Frame Sequencer Initialization

Volume updates by envelope occur only once in a complete frame sequencer cycle.
This allows inferring the frame sequencers state during
[emulator initialization](#emulator-initialization).

**Gambatte test roms**

1. `ch2_init_env_counter_timing_1_dmg08_cgb04c_outaudio0`
1. `ch2_init_env_counter_timing_2_dmg08_outaudio1_cgb04c_outaudio0`
1. `ch2_init_env_counter_timing_3_dmg08_outaudio1_cgb04c_outaudio0`
1. `ch2_init_env_counter_timing_4_dmg08_cgb04c_outaudio1`

Since the APU is not switched off and back on by these tests,
the frame sequencer is not reset and it's initial state
(the frame sequencer step occurring on emulator cycle 0)
can be inferred by determining when the next volume envelope step occurs.

**Logs**

*Test roms 1, 2 (DMG)*
```yaml
    cycle     0   <fs step 3>
    cycle  8192   <fs step 4>
    cycle 16384   <fs step 5>
    cycle 24576   <fs step 6>
    cycle 32768   <fs step 7>
    cycle 40960   <fs step 0>
    cycle 44012   NR50 = 0x77  # SO1 volume = SO2 volume = 8
    cycle 44032   NR51 = 0x22  # Channel 2 enabled on SO1 and SO2
    cycle 44052   NR21 = 0x00
    cycle 44072   NR22 = 0x09  # Channel 2 volume 0 on init,
                               # incrementing volume envelope, period 1
    cycle 44092   NR23 = 0x00
    cycle 44112   NR24 = 0x87  # Channel 2 init with volume 0
    cycle 49152   <fs step 1>
    cycle 57344   <fs step 2>
    cycle 65536   <fs step 3>
    cycle 73728   <fs step 4>
    cycle 81920   <fs step 5>
    cycle 90112   <fs step 6>

Test rom 1:
    cycle 98300   NR22 = 0x08  # disable Channel 2 volume envelope
    cycle 98304   <fs step 7>  # Channel 2 volume envelope disabled
    <...>         <test finishes with silence>  # outaudio0

Test rom 2:
    cycle 98304   <fs step 7>  # Channel 2 volume = 1 by volume envelope
    cycle 98304   NR22 = 0x08  # disable Channel 2 volume envelope
    <...>         <test finishes with audible sound>  # outaudio1
```

*Test roms 3, 4 (CGB)*
```yaml
    cycle     0   <fs step 7>
    cycle  7872   NR50 = 0x77  # SO1 volume = SO2 volume = 8
    cycle  7892   NR51 = 0x22  # Channel 2 enabled on SO1 and SO2
    cycle  7912   NR21 = 0x00
    cycle  7932   NR22 = 0x09  # Channel 2 volume 0 on init,
                               # incrementing volume envelope, period 1
    cycle  7952   NR23 = 0x00
    cycle  7972   NR24 = 0x87  # Channel 2 init with volume 0
    cycle  8192   <fs step 0>
    cycle 16384   <fs step 1>
    cycle 24576   <fs step 2>
    cycle 32768   <fs step 3>
    cycle 40960   <fs step 4>
    cycle 49152   <fs step 5>
    cycle 57344   <fs step 6>

Test rom 3:
    cycle 65532   NR22 = 0x08  # disable Channel 2 volume envelope
    cycle 65536   <fs step 7>  # Channel 2 volume envelope disabled
    <...>         <test finishes with silence>  # outaudio0

Test rom 4:
    cycle 65536   <fs step 7>  # Channel 2 volume = 1 by volume envelope
    cycle 65536   NR22 = 0x08  # disable Channel 2 volume envelope
    <...>         <test finishes with audible sound>  # outaudio1
```



#### Frame Sequencer Skips

When switching on the APU while `(cycle + 4) & 0x1000` is true the next frame
sequencer step is skipped.

**Gambatte test roms**

1. `ch2_init_reset_env_counter_timing_1_dmg08_cgb04c_outaudio0`
1. `ch2_init_reset_env_counter_timing_2_dmg08_outaudio1_cgb04c_outaudio0`
1. `ch2_init_reset_env_counter_timing_3_dmg08_cgb04c_outaudio0`
1. `ch2_init_reset_env_counter_timing_4_dmg08_outaudio0_cgb04c_outaudio1`
1. `ch2_init_reset_env_counter_timing_5_dmg08_outaudio0_cgb04c_outaudio1`
1. `ch2_init_reset_env_counter_timing_6_dmg08_cgb04c_outaudio1`
1. `ch2_init_reset_env_counter_timing_7_dmg08_outaudio1_cgb04c_outaudio0`
1. `ch2_init_reset_env_counter_timing_8_dmg08_cgb04c_outaudio1`

The skipping of a frame sequencer step is detected by switching on the APU at
specific cycles and measuring when the next volume envelope step occurs.

**Logs**

*Test roms 1, 2 (DMG)*
```yaml
    cycle  45028   NR52 = 0x00  # APU off
    cycle  45048   NR52 = 0x80  # APU on
                                # cycle 45048   = 1010'1111'1111'1000b
                                # cycle 45048+4 = 1010'1111'1111'1100b
                                # -> no frame sequencer step skipped
    cycle  45108   NR21 = 0x00
    cycle  45128   NR22 = 0x09  # Channel 2 volume 0 on init,
                                # incrementing volume envelope, period 1
    cycle  45148   NR23 = 0x00
    cycle  45168   NR24 = 0x87  # Channel 2 init with volume 0
    cycle  49152   <fs step 0>
    cycle  57344   <fs step 1>
    cycle  65536   <fs step 2>
    cycle  73728   <fs step 3>
    cycle  81920   <fs step 4>
    cycle  90112   <fs step 5>
    cycle  98304   <fs step 6>

Test rom 1:
    cycle 106492   NR22 = 0x08  # disable Channel 2 volume envelope
    cycle 106496   <fs step 7>  # Channel 2 volume envelope disabled
    <...>          <test finishes with silence>  # outaudio0

Test rom 2:
    cycle 106496   <fs step 7>  # Channel 2 volume = 1 by volume envelope
    cycle 106496   NR22 = 0x08  # disable Channel 2 volume envelope
    <...>          <test finishes with audible sound>  # outaudio1
```

*Test roms 5, 6 (DMG)*
```yaml
    cycle  45032   NR52 = 0x00  # APU off
    cycle  45052   NR52 = 0x80  # APU on
                                # cycle 45052   = 1010'1111'1111'1100b
                                # cycle 45052+4 = 1011'0000'0000'0000b
                                # -> one frame sequencer step skipped
    cycle  45112   NR21 = 0x00
    cycle  45132   NR22 = 0x09  # Channel 2 volume 0 on init,
                                # incrementing volume envelope, period 1
    cycle  45152   NR23 = 0x00
    cycle  45172   NR24 = 0x87  # Channel 2 init with volume 0
    cycle  49152   <fs step skipped>
    cycle  57344   <fs step 0>
    cycle  65536   <fs step 1>
    cycle  73728   <fs step 2>
    cycle  81920   <fs step 3>
    cycle  90112   <fs step 4>
    cycle  98304   <fs step 5>
    cycle 106496   <fs step 6>

Test rom 5:
    cycle 114684   NR22 = 0x08  # disable Channel 2 volume envelope
    cycle 114688   <fs step 7>  # Channel 2 volume envelope disabled
    <...>          <test finishes with silence>  # outaudio0

Test rom 6:
    cycle 114688   <fs step 7>  # Channel 2 volume = 1 by volume envelope
    cycle 114688   NR22 = 0x08  # disable Channel 2 volume envelope
    <...>          <test finishes with audible sound>  # outaudio1
```

*Test roms 3, 4, 7, 8 (CGB) produce similar logs just with different cycles.*



#### Volume Envelope Period Increments

The initial volume envelope period is incremented by one if the next volume
envelope step is at most 8196 cycles away.

**Gambatte test roms**

1. `ch2_init_reset_env_counter_timing_9_dmg08_cgb04c_outaudio0`
1. `ch2_init_reset_env_counter_timing_10_dmg08_outaudio1_cgb04c_outaudio0`
1. `ch2_init_reset_env_counter_timing_11_dmg08_outaudio0_cgb04c_outaudio1`
1. `ch2_init_reset_env_counter_timing_12_dmg08_cgb04c_outaudio1`
1. `ch2_init_reset_env_counter_timing_13_dmg08_cgb04c_outaudio0`
1. `ch2_init_reset_env_counter_timing_14_dmg08_outaudio0_cgb04c_outaudio1`
1. `ch2_init_reset_env_counter_timing_15_dmg08_outaudio1_cgb04c_outaudio0`
1. `ch2_init_reset_env_counter_timing_16_dmg08_cgb04c_outaudio1`

The volume envelope period incease is detected by initializing channel 2 at
specific cycles and measuring when the next volume envelope step occurs.

**Logs**

*Test roms 1, 2 (DMG)*
```yaml
    cycle  44008   NR52 = 0x00  # APU off
    cycle  44028   NR52 = 0x80  # APU on
                                # cycle 44028+4 = 1010'1100'0000'0000b
                                # -> no frame sequencer step skipped
    cycle  49152   <fs step 0>
    cycle  57344   <fs step 1>
    cycle  65536   <fs step 2>
    cycle  73728   <fs step 3>
    cycle  81920   <fs step 4>
    cycle  90112   <fs step 5>
    cycle  98236   NR21 = 0x00
    cycle  98256   NR22 = 0x09  # Channel 2 volume 0 on init,
                                # incrementing volume envelope, period 1
    cycle  98276   NR23 = 0x00
    cycle  98296   NR24 = 0x87  # Channel 2 init with volume 0
                                # 8200 cycles until frame sequencer step 7
    cycle  98304   <fs step 6>

Test rom 1:
    cycle 106492   NR22 = 0x08  # disable Channel 2 volume envelope
    cycle 106496   <fs step 7>  # Channel 2 volume envelope disabled
    <...>          <test finishes with silence>  # outaudio0

Test rom 2:
    cycle 106496   <fs step 7>  # Channel 2 volume = 1 by volume envelope
    cycle 106496   NR22 = 0x08  # disable Channel 2 volume envelope
    <...>          <test finishes with audible sound>  # outaudio1
```

*Test roms 3, 4 (DMG)*
```yaml
    cycle  44008   NR52 = 0x00  # APU off
    cycle  44028   NR52 = 0x80  # APU on
                                # cycle 44028+4 = 1010'1100'0000'0000b
                                # -> no frame sequencer step skipped
    cycle  49152   <fs step 0>
    cycle  57344   <fs step 1>
    cycle  65536   <fs step 2>
    cycle  73728   <fs step 3>
    cycle  81920   <fs step 4>
    cycle  90112   <fs step 5>
    cycle  98240   NR21 = 0x00
    cycle  98260   NR22 = 0x09  # Channel 2 volume 0 on init,
                                # incrementing volume envelope, period 1
    cycle  98280   NR23 = 0x00
    cycle  98300   NR24 = 0x87  # Channel 2 init with volume 0
                                # 8196 cycles until frame sequencer step 7
                                # -> initial volume envelope period = 2
    cycle  98304   <fs step 6>
    cycle 106496   <fs step 7>  # first volume envelope step for period 2
    cycle 114688   <fs step 0>
    cycle 122880   <fs step 1>
    cycle 131072   <fs step 2>
    cycle 139264   <fs step 3>
    cycle 147456   <fs step 4>
    cycle 155648   <fs step 5>
    cycle 163840   <fs step 6>

Test rom 3:
    cycle 172028   NR22 = 0x08  # disable Channel 2 volume envelope
    cycle 172032   <fs step 7>  # Channel 2 volume envelope disabled
    <...>          <test finishes with silence>  # outaudio0

Test rom 4:
    cycle 172032   <fs step 7>  # Channel 2 volume = 1 by volume envelope
    cycle 172032   NR22 = 0x08  # disable Channel 2 volume envelope
    <...>          <test finishes with audible sound>  # outaudio1
```

*Test roms 5, 6, 7, 8 (CGB) produce similar logs just with different cycles
and a skipped frame sequencer step right after switching on the APU.*



#### Volume Envelope Test Cycles

The following table contains the cycles at which the APU is switched on and
channel 2 is initialized for `ch2_init_reset_env_counter_timing_*` test roms.

The `type` column contains the type of Gameboy hardware the test is tailored to
and if the test is expected to finish with silence or audible sound.
E.g. `CGB-0` is a test tailored to the Color Gameboy and expected to finish
with silence.

The `steps` column contains the number of framesequencer steps between
switching on the APU and disabling the volume envelope.
If noted as `X+Y`, `X` is the number of frame sequencer steps between switching
on the APU and initializing channel 2 and `Y` is the number of frame sequencer
steps between initializing channel 2 and disabling the volume envelope.

```
+-----------+-------+-------+---------------------+-----------------------+
| test file | type  | steps | APU on @cycle       | channel init @cycle   |
+-----------+-------+-------+---------------------+-----------------------+
|           |       |       |                     |                       |
| timing_1  | DMG-0 |     7 | 1010'1111'1111'1000 |   1011'0000'0111'0000 |
| timing_2  | DMG-1 |     8 | 1010'1111'1111'1000 |   1011'0000'0111'0000 |
|           |       |       |                     |                       |
| timing_5  | DMG-0 |     8 | 1010'1111'1111'1100 |   1011'0000'0111'0100 |
| timing_6  | DMG-1 |     9 | 1010'1111'1111'1100 |   1011'0000'0111'0100 |
|           |       |       |                     |                       |
| timing_3  | CGB-0 |     7 | 0010'1111'1111'1000 |   0011'0000'0111'0000 |
| timing_4  | CGB-1 |     8 | 0010'1111'1111'1000 |   0011'0000'0111'0000 |
|           |       |       |                     |                       |
| timing_7  | CGB-0 |     8 | 0010'1111'1111'1100 |   0011'0000'0111'0100 |
| timing_8  | CGB-1 |     9 | 0010'1111'1111'1100 |   0011'0000'0111'0100 |
|           |       |       |                     |                       |
+-----------+-------+-------+---------------------+-----------------------+
|           |       |       |                     |                       |
| timing_9  | DMG-0 |   6+1 | 1010'1011'1111'1100 | 1'0111'1111'1111'1000 |
| timing_10 | DMG-1 |   6+2 | 1010'1011'1111'1100 | 1'0111'1111'1111'1000 |
|           |       |       |                     |                       |
| timing_11 | DMG-0 |  6+ 9 | 1010'1011'1111'1100 | 1'0111'1111'1111'1100 |
| timing_12 | DMG-1 |  6+10 | 1010'1011'1111'1100 | 1'0111'1111'1111'1100 |
|           |       |       |                     |                       |
| timing_13 | CGB-0 |   7+1 | 0001'1110'1101'0000 |   1111'1111'1111'1000 |
| timing_14 | CGB-1 |   7+2 | 0001'1110'1101'0000 |   1111'1111'1111'1000 |
|           |       |       |                     |                       |
| timing_15 | CGB-0 |  7+ 9 | 0001'1110'1101'0000 |   1111'1111'1111'1100 |
| timing_16 | CGB-1 |  7+10 | 0001'1110'1101'0000 |   1111'1111'1111'1100 |
|           |       |       |                     |                       |
+-----------+-------+-------+---------------------+-----------------------+
```



### Length Counter Timing

If a channel is initialized with length counter,
it is deactivated once the length counter is decremented to zero.

As described by [Shay Green (blargg)](https://gist.github.com/drhelius/3652407)
a length counter is decremented immediately on channel initialization if the
next frame sequencer step does *not* decrement length counters.
This also works with [Frame Sequencer Skips](#frame-sequencer-skips) when
considering step 7 as the one being skipped
(which makes sense as the next step being executed is step 0).

**Gambatte test roms**

1. `ch2_init_reset_length_counter_timing_nr52_1_dmg08_out2_cgb04c_out0`
1. `ch2_init_reset_length_counter_timing_nr52_2_dmg08_cgb04c_out0`
1. `ch2_init_reset_length_counter_timing_nr52_3_dmg08_cgb04c_out2`
1. `ch2_init_reset_length_counter_timing_nr52_4_dmg08_out2_cgb04c_out0`

Sound channel 2 is initialized to automatically deactivate when it's length
counter reaches zero.
NR52 is checked for channel 2 activity around the time length counters are
expected to be decremented.

**Logs**

*Test roms 1, 2 (DMG)*
```yaml
    cycle 44008   NR50 = 0x00
    cycle 44020   NR51 = 0x00  # disable sound output
    cycle 44032   NR52 = 0x00  # APU off
    cycle 44052   NR52 = 0x80  # APU on
                               # cycle 44052+4 = 1010'1100'0001'1000b
                               # -> no frame sequencer step skipped
    cycle 44072   NR21 = 0x3D  # Channel 2 length counter = 3
    cycle 44092   NR22 = 0xF0  # Channel 2 volume = 15
    cycle 44112   NR23 = 0x00
    cycle 44132   NR24 = 0xC7  # Channel 2 init with length counter LC=3
                               # cycle 44132 = 1010'1100'0110'0100b
                               # -> no immediate length counter decrement
    cycle 49152   <fs step 0>  # length counter decrement, LC=2
    cycle 57344   <fs step 1>
    cycle 65536   <fs step 2>  # length counter decrement, LC=1
    cycle 73728   <fs step 3>
    cycle 81916   NR52 == 0x82 # Channel 2 active (checked by test rom 1)
    cycle 81920   <fs step 4>  # length counter decrement, LC=0
    cycle 81920   NR52 == 0x80 # Channel 2 inactive (checked by test rom 2)
```

*Test roms 3, 4 (CGB)*
```yaml
    cycle  7868   NR50 = 0x00
    cycle  7880   NR51 = 0x00  # disable sound output
    cycle  7892   NR52 = 0x00  # APU off
    cycle  7912   NR52 = 0x80  # APU on
                               # cycle 7912+4 = 0001'1110'1110'1100b
                               # -> one frame sequencer step skipped
    cycle  7932   NR21 = 0x3D  # Channel 2 length counter = 3
    cycle  7952   NR22 = 0xF0  # Channel 2 volume = 15
    cycle  7972   NR23 = 0x00
    cycle  7992   NR24 = 0xC7  # Channel 2 init with length counter LC=3
                               # skipped frame sequencer step considered step 7
                               # -> immediate length counter decrement, LC=2
    cycle  8192   <fs step skipped>
    cycle 16384   <fs step 0>  # length counter decrement, LC=1
    cycle 24576   <fs step 1>
    cycle 32764   NR52 == 0x82 # Channel 2 active (checked by test rom 3)
    cycle 32768   <fs step 2>  # length counter decrement, LC=0
    cycle 32768   NR52 == 0x80 # Channel 2 inactive (checked by test rom 4)
```



### Frequency Sweep Overflow Delay

The check for frequency sweep overflow is delayed by 4 cycles.

**Gambatte test roms**

1. `ch1_init_reset_sweep_counter_timing_nr52_1_dmg08_cgb04c_out1`
1. `ch1_init_reset_sweep_counter_timing_nr52_2_dmg08_out0_cgb04c_out1`
1. `ch1_init_reset_sweep_counter_timing_nr52_3_dmg08_out0_cgb04c_out1`
1. `ch1_init_reset_sweep_counter_timing_nr52_4_dmg08_cgb04c_out0`

Sound channel 2 is initialized to automatically deactivate when frequency sweep
overflows.
NR52 is checked for channel 2 activity around the time the frame sequencer is
expected to perform frequency sweep.

**Logs**

*Test roms 1, 2 (DMG)*
```yaml
    cycle 44032   NR52 = 0x00  # APU off
    cycle 44052   NR52 = 0x80  # APU on
                               # cycle 44052+4 = 1010'1100'0001'1000b
                               # -> no frame sequencer step skipped
    cycle 44072   NR10 = 0x20  # Channel 1 frequency sweep up, FC=2
    cycle 44132   NR14 = 0x87  # Channel 1 init,
                               # first frequency sweep will overflow
    cycle 49152   <fs step 0>
    cycle 57344   <fs step 1>
    cycle 65536   <fs step 2>  # FC=1
    cycle 73728   <fs step 3>
    cycle 81920   <fs step 4>
    cycle 90112   <fs step 5>
    cycle 98304   <fs step 6>  # FC=0 -> perform frequency sweep
    cycle 98304   NR52 == 0x81 # Channel 1 active (checked by test rom 1)
    cycle 98308                # frequency sweep overflow check: positive
    cycle 98308   NR52 == 0x80 # Channel 1 inactive (checked by test rom 2)
```

*Test roms 3, 4 (CGB)*
```yaml
    cycle 7892    NR52 = 0x00  # APU off
    cycle 7912    NR52 = 0x80  # APU on
                               # cycle 7912+4 = 0001'1110'1110'1100b
                               # -> one frame sequencer step skipped
    cycle 7932    NR10 = 0x20  # Channel 1 frequency sweep up, FC=2
    cycle 7992    NR14 = 0x87  # Channel 1 init,
                               # first frequency sweep will overflow
    cycle 8192    <fs step skipped>
    cycle 16384   <fs step 0>
    cycle 24576   <fs step 1>
    cycle 32768   <fs step 2>  # FC=1
    cycle 40960   <fs step 3>
    cycle 49152   <fs step 4>
    cycle 57344   <fs step 5>
    cycle 65536   <fs step 6>  # FC=0 -> perform frequency sweep
    cycle 65536   NR52 == 0x81 # Channel 1 active (checked by test rom 3)
    cycle 65540                # frequency sweep overflow check: positive
    cycle 65540   NR52 == 0x80 # Channel 1 inactive (checked by test rom 4)
```



### Initial Frequency Sweep Delay

If channel 1 is initialized with frequency sweep while the next frame sequencer
step 2 or 6 (the frequency sweep steps) is at most 4 cycles away
(8 cycles on CGB),
this first frequency sweep step is skipped.

**Gambatte test roms**

1. `ch1_init_reset_sweep_counter_timing_4_dmg08_outaudio1_cgb04c_outaudio0`
1. `ch1_init_reset_sweep_counter_timing_5_dmg08_xoutaudio1lowpitch_cgb04c_outaudio0`
1. `ch1_init_reset_sweep_counter_timing_6_dmg08_cgb04c_outaudio0`
1. `ch1_init_reset_sweep_counter_timing_10_dmg08_outaudio0_cgb04c_outaudio1`
1. `ch1_init_reset_sweep_counter_timing_11_dmg08_outaudio0_cgb_xoutaudio1lowpitch`
1. `ch1_init_reset_sweep_counter_timing_12_dmg08_cgb04c_outaudio0`

Channel 1 is initialized with frequency sweep configured to deactivate it after
two sweep steps.
In every test there are three sweep steps between initializing channel 1 and
turning off frequency sweep.
Depending on when channel 1 was initialized and if the test finishes with
silence or audible sound the skipping of a sweep step can be detected.

**Logs**

*Test roms 1, 2, 3 (DMG)*
```yaml
    cycle  44032  NR52 = 0x00  # APU off
    cycle  44052  NR52 = 0x80  # APU on
    cycle  49152  <fs step 0>
    cycle  57344  <fs step 1>
    cycle  65472  NR10 = 0x20  # Channel 1 frequency sweep up, FC=2
    cycle  65532  NR14 = 0x87  # Channel 1 init,
                               # first frequency sweep will overflow
    cycle  65536  <fs step 2>  # frequency sweep step skipped
    cycle  73728  <fs step 3>
    cycle  81920  <fs step 4>
    cycle  90112  <fs step 5>
    cycle  98304  <fs step 6>  # FC=1
    cycle 106496  <fs step 7>
    cycle 114688  <fs step 0>
    cycle 122880  <fs step 1>

test rom 1:
    cycle 131068  NR10 = 0x00  # deactivate frequency sweep
    cycle 131072  <fs step 2>  # frequency sweep turned off
    <...>         <test finishes with audible sound>  # outaudio1

test rom 2:
    cycle 131072  <fs step 2>  # FC=0 -> perform frequency sweep
    cycle 131072  NR10 = 0x00  # deactivate frequency sweep
    <...>         <test finishes undefined>  # xoutaudio1lowpitch

test rom 3:
    cycle 131072  <fs step 2>  # FC=0 -> perform frequency sweep
    cycle 131076  NR10 = 0x00  # deactivate frequency sweep
    <...>         <test finishes with silence>  # outaudio0
```

*Test roms 4, 5, 6 (CGB)*
```yaml
    cycle  7892   NR52 = 0x00  # APU off
    cycle  7912   NR52 = 0x80  # APU on
    cycle 16384   <fs step 0>
    cycle 24576   <fs step 1>
    cycle 32700   NR10 = 0x20  # Channel 1 frequency sweep up, FC=2
    cycle 32760   NR14 = 0x87  # Channel 1 init,
                               # first frequency sweep will overflow
    cycle 32768   <fs step 2>  # frequency sweep step skipped
    cycle 40960   <fs step 3>
    cycle 49152   <fs step 4>
    cycle 57344   <fs step 5>
    cycle 65536   <fs step 6>  # FC=1
    cycle 73728   <fs step 7>
    cycle 81920   <fs step 0>
    cycle 90112   <fs step 1>

test rom 4:
    cycle 98300   NR10 = 0x00  # deactivate frequency sweep
    cycle 98304   <fs step 2>  # frequency sweep turned off
    <...>         <test finishes with audible sound>  # outaudio1

test rom 5:
    cycle 98304   <fs step 2>  # FC=0 -> perform frequency sweep
    cycle 98304   NR10 = 0x00  # deactivate frequency sweep
    <...>         <test finishes undefined>  # xoutaudio1lowpitch

test rom 6:
    cycle 98304   <fs step 2>  # FC=0 -> perform frequency sweep
    cycle 98308   NR10 = 0x00  # deactivate frequency sweep
    <...>         <test finishes with silence>  # outaudio0
```
