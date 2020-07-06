
# Initial LCD state

## CGB

```
     0   <PC = 0x0150>
    40   LY = 144
   292   <scanline 145>
   748   <scanline 146>
  1204   <scanline 147>
  1660   <scanline 148>
  2116   <scanline 149>
  2572   <scanline 150>
  3028   <scanline 151>
  3484   <scanline 152>
  3940   <scanline 153>
  4396   <scanline 0>
  4644   STAT = 0x87 (mode 3, LY match)
  4648   STAT = 0x84 (mode 0, LY match)
  4852   <scanline 1>
``` 

Gambatte test roms:
* [display_startstate/ly_dmg08_out00_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/ly_dmg08_out00_cgb04c_out90.asm)
* [display_startstate/stat_1_cgb04c_out87](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_1_cgb04c_out87.asm)
* [display_startstate/stat_2_cgb04c_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_2_cgb04c_out84.asm)


## DMG

```
     0   <PC = 0x0150>
    40   LY = 0
    52   STAT = 0x85 (mode 1, LY match)
    56   <scanline 0>
    56   STAT = 0x86 (mode 2, LY match)
   512   <scanline 1>
``` 

Gambatte test roms:
* [display_startstate/ly_dmg08_out00_cgb04c_out90](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/ly_dmg08_out00_cgb04c_out90.asm)
* [display_startstate/stat_1_dmg08_out85](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_1_dmg08_out85.asm)
* [display_startstate/stat_2_dmg08_out84](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/display_startstate/stat_2_dmg08_out84.asm)
