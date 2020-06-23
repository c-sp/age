
# Serial transfer

For the sake of simplicity we only look at DMG serial transfer.
On CGB serial transfer speed can be changed by `KEY1` and `SC` settings.
However, this comes down to simply using different clock bits to calculate
serial transfer state.


## Serial transfer timing

Serial transfer completes after clock bit 8 was switched 16 times.
The bit's initial state is irrelevant.
Note that bit 8 is switched every time bit 7 goes low
(this will be relevant for `DIV` resets).

### Serial transfer started with clock bit 8 low

* [serial/nopx1_start_wait_read_if_1_dmg08_cgb04c_outE0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/nopx1_start_wait_read_if_1_dmg08_cgb04c_outE0.asm)
* [serial/nopx1_start_wait_read_if_2_dmg08_cgb04c_outE8](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/nopx1_start_wait_read_if_2_dmg08_cgb04c_outE8.asm)
```
binary clock         clock - instruction / event
----------------------------------------------
1011 1100 1110 1000  48360 - write SB 0x00 (0), old SB was: 0xFF (255)
1011 1100 1111 1100  48380 - write SC 0x81 (129)
1011 1100 1111 1100  48380 - starting serial transfer:
        |                        * 512 clock cycles per transferred bit
        |                        * 4 clock cycles until first clock bit change
        +------------------- clock bit changing 16 times

1011 1101 0000 0000          clock bit change  #1
1011 1110 0000 0000          clock bit change  #2
1011 1111 0000 0000          clock bit change  #3
1100 0000 0000 0000          clock bit change  #4
1100 0001 0000 0000          clock bit change  #5
1100 0010 0000 0000          clock bit change  #6
1100 0011 0000 0000          clock bit change  #7
1100 0100 0000 0000          clock bit change  #8
1100 0101 0000 0000          clock bit change  #9
1100 0110 0000 0000          clock bit change #10
1100 1111 0000 0000          clock bit change #11
1100 1000 0000 0000          clock bit change #12
1100 1001 0000 0000          clock bit change #13
1100 1010 0000 0000          clock bit change #14
1100 1011 0000 0000          clock bit change #15

1100 1011 1111 1100  52220 - read IF 0xE0 (224)
1100 1100 0000 0000  52224 - clock bit change #16
1100 1100 0000 0000  52224 - serial transfer finished
1100 1100 0000 0000  52224 - interrupt requested: 0x08 (8)
1100 1100 0000 0000  52224 - read IF 0xE8 (232)
```

### Serial transfer started with clock bit 8 high

* [serial/nopx2_start_wait_read_if_1_dmg08_cgb04c_outE0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/nopx2_start_wait_read_if_1_dmg08_cgb04c_outE0.asm)
* [serial/nopx2_start_wait_read_if_2_dmg08_cgb04c_outE8](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/nopx2_start_wait_read_if_2_dmg08_cgb04c_outE8.asm)

```
binary clock         clock - instruction / event
----------------------------------------------
1011 1100 1110 1100  48364 - write SB 0x00 (0), old SB was: 0xFF (255)
1011 1101 0000 0000  48384 - write SC 0x81 (129)
1011 1101 0000 0000  48384 - starting serial transfer:
        |                        * 512 clock cycles per transferred bit
        |                        * 256 clock cycles until first clock bit change
        +------------------- clock bit changing 16 times

1011 1110 0000 0000          clock bit change  #1
1011 1111 0000 0000          clock bit change  #2
1100 0000 0000 0000          clock bit change  #3
1100 0001 0000 0000          clock bit change  #4
1100 0010 0000 0000          clock bit change  #5
1100 0011 0000 0000          clock bit change  #6
1100 0100 0000 0000          clock bit change  #7
1100 0101 0000 0000          clock bit change  #8
1100 0110 0000 0000          clock bit change  #9
1100 1111 0000 0000          clock bit change #10
1100 1000 0000 0000          clock bit change #11
1100 1001 0000 0000          clock bit change #12
1100 1010 0000 0000          clock bit change #13
1100 1011 0000 0000          clock bit change #14
1100 1100 0000 0000          clock bit change #15

1100 1100 1111 1100  52476 - read IF 0xE0 (224)
1100 1101 0000 0000  52480 - clock bit change #16
1100 1101 0000 0000  52480 - serial transfer finished
1100 1101 0000 0000  52480 - interrupt requested: 0x08 (8)
1100 1101 0000 0000  52480 - read IF 0xE8 (232)
```


## Reading `SB` during serial transfer

Already transferred bits can be read from `SB`.

* [serial/start_wait_read_sb_1_dmg08_cgb04c_out7F](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_wait_read_sb_1_dmg08_cgb04c_out7F.asm)
* [serial/start_wait_read_sb_2_dmg08_cgb04c_outFF](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_wait_read_sb_2_dmg08_cgb04c_outFF.asm)
```
clock - instruction / event
---------------------------
48176 - write SB 0x00 (0), old SB was: 0xFF (255)
48196 - write SC 0x81 (129)
48196 - starting serial transfer:
48196 -     * 512 clock cycles per transferred bit
48196 -     * 188 clock cycles until first clock bit change
        <...>
52220 - read SB 0x7F (127)
52224 - serial transfer finished
52224 - interrupt requested: 0x08 (8)
52224 - read SB 0xFF (255)
```


## serial transfer `DIV` alignment

TODO describe this

* [serial/div_write_start_wait_read_if_1_dmg08_cgb04c_outE0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/div_write_start_wait_read_if_1_dmg08_cgb04c_outE0.asm)
* [serial/div_write_start_wait_read_if_2_dmg08_cgb04c_outE8](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/div_write_start_wait_read_if_2_dmg08_cgb04c_outE8.asm)


## `DIV` reset during serial transfer

### bit 7 going low due to `DIV` reset

TODO describe this

* [serial/start_late_div_write_wait_read_if_2a_dmg08_cgb04c_outE0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_late_div_write_wait_read_if_2a_dmg08_cgb04c_outE0.asm)
* [serial/start_late_div_write_wait_read_if_2b_dmg08_cgb04c_outE8](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_late_div_write_wait_read_if_2b_dmg08_cgb04c_outE8.asm)
* [serial/start_late_div_write_wait_read_if_4_dmg08_cgb04c_outE8](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_late_div_write_wait_read_if_4_dmg08_cgb04c_outE8.asm)

### bit 7 NOT going low due to `DIV` reset

TODO describe this

* [serial/start_late_div_write_wait_read_if_1a_dmg08_cgb04c_outE0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_late_div_write_wait_read_if_1a_dmg08_cgb04c_outE0.asm)
* [serial/start_late_div_write_wait_read_if_1b_dmg08_cgb04c_outE8](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_late_div_write_wait_read_if_1b_dmg08_cgb04c_outE8.asm)
* [serial/start_late_div_write_wait_read_if_3a_dmg08_cgb04c_outE0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_late_div_write_wait_read_if_3a_dmg08_cgb04c_outE0.asm)
* [serial/start_late_div_write_wait_read_if_3b_dmg08_cgb04c_outE8](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_late_div_write_wait_read_if_3b_dmg08_cgb04c_outE8.asm)


## Interrupt timing

Triggering a serial transfer interrupt during interrupt dispatching has
different timings on CGB and on DMG.

TODO examine this

[serial/start_wait_trigger_int8_read_if_1_dmg08_cgb04c_outE8](
https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_wait_trigger_int8_read_if_1_dmg08_cgb04c_outE8.asm)
```
48216 - write SB 0xE0 (224), old SB was: 0xFF (255)
48236 - write SC 0x81 (129)
48236 - starting serial transfer:
48236 -     * 512 clock cycles per transferred bit
48236 -     * 148 clock cycles until first clock bit change
48236 -     * div offset 0x0000 (0)
48236 -     * finishes on clock cycle 52224
52188 - write IF 0x08 (8)
52196 - enable interrupts after this CPU instruction
52200 - interrupts enabled
52200 - dispatching interrupt, PC = 0x13EB (5099)
52216 - clearing IF interrupt bit 0x08 (8), interrupts disabled
52220 - interrupt 0x08 (8) (serial transfer) dispatched to 0x0058 (88)
52224 - serial transfer finished
52224 - interrupt requested: 0x08 (8)
52264 - read IF 0xE8 (232)
```

[serial/start_wait_trigger_int8_read_if_2_dmg08_outE8_cgb04c_outE0](
https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_wait_trigger_int8_read_if_2_dmg08_outE8_cgb04c_outE0.asm)
```
48216 - write SB 0xE0 (224), old SB was: 0xFF (255)
48236 - write SC 0x81 (129)
48236 - starting serial transfer:
48236 -     * 512 clock cycles per transferred bit
48236 -     * 148 clock cycles until first clock bit change
48236 -     * div offset 0x0000 (0)
48236 -     * finishes on clock cycle 52224
52192 - write IF 0x08 (8)
52200 - enable interrupts after this CPU instruction
52204 - interrupts enabled
52204 - dispatching interrupt, PC = 0x13EC (5100)
52220 - clearing IF interrupt bit 0x08 (8), interrupts disabled
52224 - serial transfer finished
52224 - interrupt requested: 0x08 (8)
52224 - interrupt 0x08 (8) (serial transfer) dispatched to 0x0058 (88)
52268 - read IF 0xE8 (232)
```

[serial/start_wait_trigger_int8_read_if_3_dmg08_cgb04c_outE0](
https://github.com/sinamas/gambatte/tree/master/test/hwtests/serial/start_wait_trigger_int8_read_if_3_dmg08_cgb04c_outE0.asm)
```
48216 - write SB 0xE0 (224), old SB was: 0xFF (255)
48236 - write SC 0x81 (129)
48236 - starting serial transfer:
48236 -     * 512 clock cycles per transferred bit
48236 -     * 148 clock cycles until first clock bit change
48236 -     * div offset 0x0000 (0)
48236 -     * finishes on clock cycle 52224
52196 - write IF 0x08 (8)
52204 - enable interrupts after this CPU instruction
52208 - interrupts enabled
52208 - dispatching interrupt, PC = 0x13ED (5101)
52224 - serial transfer finished
52224 - interrupt requested: 0x08 (8)
52224 - clearing IF interrupt bit 0x08 (8), interrupts disabled
52228 - interrupt 0x08 (8) (serial transfer) dispatched to 0x0058 (88)
52272 - read IF 0xE0 (224)
```
