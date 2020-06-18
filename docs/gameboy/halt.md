
# `HALT` behavior


## Interrupts disabled

Gambatte test rom `halt/noime_ifandie_halt_lda_3c_dmg08_cgb04c_out3F`:
```
       ld a, 11
       ld e, 01
       ldff(0f), a
       ldff(ff), a
       halt
3E 3C  ld a, 3C
```
is executed as:
```
       <...>
       halt
3E 3E  ld a, 3E
3C     inc a     ; a = 3F
```


Gambatte test rom `halt/noime_ifandie_halt_sra_dmg08_cgb04c_outF1`:
```
       ld a, 11
       ld e, 01
       ldff(0f), a
       ldff(ff), a
       halt
CB 2F  sra a
83     add a, e
```
is executed as:
```
       <...>
       halt
CB CB  set 1, e  ; e = 03
2F     cpl       ; a = EE
83     add a, e  ; a = F1
```
