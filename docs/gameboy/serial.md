
Gambatte test rom serial/start_wait_read_sb_1_dmg08_cgb04c_out7F:
```
                   11824: write SB 0x00 (0), old SB was: 0xFF (255)
10 1110 0100 0100  11844: write SC 0x81 (129)
10 1110 0100 0100  11844: starting serial transfer, 512 clocks per bit, 68 clocks into sio, 0 clocks extended

10 1111 0000 0000         high-1
11 0000 0000 0000         low-1
11 0001 0000 0000         high-2
11 0010 0000 0000         low-2
11 0011 0000 0000         high-3
11 0100 0000 0000         low-3
11 0101 0000 0000         high-4
11 0110 0000 0000         low-4
11 0111 0000 0000         high-5
11 1000 0000 0000         low-5
11 1001 0000 0000         high-6
11 1010 0000 0000         low-6
11 1011 0000 0000         high-7
11 1100 0000 0000         low-7
11 1101 0000 0000         high-8

11 1101 1111 1100  15868: read SB 0x7F (127)
11 1110 0000 0000         low-8
11 1110 0000 0000  15872: serial transfer finished
11 1110 0000 0000  15872: interrupt requested: 0x08 (8)
```

Gambatte test rom serial/start_wait_read_sb_2_dmg08_cgb04c_outFF:
```
                   11824: write SB 0x00 (0), old SB was: 0xFF (255)
10 1110 0100 0100  11844: write SC 0x81 (129)
10 1110 0100 0100  11844: starting serial transfer, 512 clocks per bit, 68 clocks into sio, 0 clocks extended

10 1111 0000 0000         high-1
11 0000 0000 0000         low-1
11 0001 0000 0000         high-2
11 0010 0000 0000         low-2
11 0011 0000 0000         high-3
11 0100 0000 0000         low-3
11 0101 0000 0000         high-4
11 0110 0000 0000         low-4
11 0111 0000 0000         high-5
11 1000 0000 0000         low-5
11 1001 0000 0000         high-6
11 1010 0000 0000         low-6
11 1011 0000 0000         high-7
11 1100 0000 0000         low-7
11 1101 0000 0000         high-8

11 1110 0000 0000         low-8
11 1110 0000 0000  15872: serial transfer finished
11 1110 0000 0000  15872: interrupt requested: 0x08 (8)
11 1110 0000 0000  15872: read SB 0xFF (255)
```


## Serial transfer duration

Gambatte test rom serial/nopx1_start_wait_read_if_1_dmg08_cgb04c_outE0:
```
                   12008: write SB 0x00 (0), old SB was: 0xFF (255)
10 1110 1111 1100  12028: write SC 0x81 (129)
10 1110 1111 1100  12028: starting serial transfer, 512 clocks per bit, 252 clocks into sio, 0 clocks extended

10 1111 0000 0000         flip  1
11 0000 0000 0000         flip  2
11 0001 0000 0000         flip  3
11 0010 0000 0000         flip  4
11 0011 0000 0000         flip  5
11 0100 0000 0000         flip  6
11 0101 0000 0000         flip  7
11 0110 0000 0000         flip  8
11 0111 0000 0000         flip  9
11 1000 0000 0000         flip 10
11 1001 0000 0000         flip 11
11 1010 0000 0000         flip 12
11 1011 0000 0000         flip 13
11 1100 0000 0000         flip 14
11 1101 0000 0000         flip 15

11 1101 1111 1100  15868: read IF 0xE0 (224)
11 1110 0000 0000         flip 16
11 1110 0000 0000  15872: serial transfer finished
11 1110 0000 0000  15872: interrupt requested: 0x08

    => serial transfer finished 3844 clock cycles after write to SC
```

Gambatte test rom serial/nopx1_start_wait_read_if_2_dmg08_cgb04c_outE8:
```
                   12008: write SB 0x00 (0), old SB was: 0xFF (255)
10 1110 1111 1100  12028: write SC 0x81 (129)
10 1110 1111 1100  12028: starting serial transfer, 512 clocks per bit, 252 clocks into sio, 0 clocks extended

10 1111 0000 0000         flip  1
11 0000 0000 0000         flip  2
11 0001 0000 0000         flip  3
11 0010 0000 0000         flip  4
11 0011 0000 0000         flip  5
11 0100 0000 0000         flip  6
11 0101 0000 0000         flip  7
11 0110 0000 0000         flip  8
11 0111 0000 0000         flip  9
11 1000 0000 0000         flip 10
11 1001 0000 0000         flip 11
11 1010 0000 0000         flip 12
11 1011 0000 0000         flip 13
11 1100 0000 0000         flip 14
11 1101 0000 0000         flip 15

11 1110 0000 0000         flip 16
11 1110 0000 0000  15872: serial transfer finished
11 1110 0000 0000  15872: interrupt requested: 0x08
11 1110 0000 0000  15872: read IF 0xE8 (232)

    => serial transfer finished 3844 clock cycles after write to SC
```

Gambatte test rom serial/nopx2_start_wait_read_if_1_dmg08_cgb04c_outE0:

```
                   12012: write SB 0x00 (0), old SB was: 0xFF (255)
10 1111 0000 0000  12032: write SC 0x81 (129)
10 1111 0000 0000  12032: starting serial transfer, 512 clocks per bit, 256 clocks into sio, 256 clocks extended

11 0000 0000 0000         flip  1
11 0001 0000 0000         flip  2
11 0010 0000 0000         flip  3
11 0011 0000 0000         flip  4
11 0100 0000 0000         flip  5
11 0101 0000 0000         flip  6
11 0110 0000 0000         flip  7
11 0111 0000 0000         flip  8
11 1000 0000 0000         flip  9
11 1001 0000 0000         flip 10
11 1010 0000 0000         flip 11
11 1011 0000 0000         flip 12
11 1100 0000 0000         flip 13
11 1101 0000 0000         flip 14
11 1110 0000 0000         flip 15

11 1110 1111 1100  16124: read IF 0xE0 (224)
11 1111 0000 0000         flip 16
11 1111 0000 0000  16128: serial transfer finished
11 1111 0000 0000  16128: interrupt requested: 0x08

    => serial transfer finished 4096 clock cycles after write to SC
```

Gambatte test rom serial/nopx2_start_wait_read_if_2_dmg08_cgb04c_outE8:
```
                   12012: write SB 0x00 (0), old SB was: 0xFF (255)
10 1111 0000 0000  12032: write SC 0x81 (129)
10 1111 0000 0000  12032: starting serial transfer, 512 clocks per bit, 256 clocks into sio, 256 clocks extended

11 0000 0000 0000         flip  1
11 0001 0000 0000         flip  2
11 0010 0000 0000         flip  3
11 0011 0000 0000         flip  4
11 0100 0000 0000         flip  5
11 0101 0000 0000         flip  6
11 0110 0000 0000         flip  7
11 0111 0000 0000         flip  8
11 1000 0000 0000         flip  9
11 1001 0000 0000         flip 10
11 1010 0000 0000         flip 11
11 1011 0000 0000         flip 12
11 1100 0000 0000         flip 13
11 1101 0000 0000         flip 14
11 1110 0000 0000         flip 15

11 1111 0000 0000         flip 16
11 1111 0000 0000  16128: serial transfer finished
11 1111 0000 0000  16128: interrupt requested: 0x08
11 1111 0000 0000  16128: read IF 0xE8 (232)

    => serial transfer finished 4096 clock cycles after write to SC
```


## `DIV`-aligned serial transfer

Gambatte test rom serial/div_write_start_wait_read_if_1_dmg08_cgb04c_outE0.gbc

Gambatte test rom serial/div_write_start_wait_read_if_2_dmg08_cgb04c_outE8.gbc
