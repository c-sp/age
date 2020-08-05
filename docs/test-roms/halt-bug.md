
# `HALT` bug


## `HALT` bug and opcode prefetch

Gambatte test rom
[halt/noime_ifandie_halt_lda_3c_dmg08_cgb04c_out3F](
https://github.com/sinamas/gambatte/tree/master/test/hwtests/halt/noime_ifandie_halt_lda_3c_dmg08_cgb04c_out3F.asm):
```
        ; interrupt dispatching disabled

        ld  a, 0x11    ; a = 0x11
        ldh (0x0F), a  ; IF = 0x11
        ldh (0xFF), a  ; IE = 0x11

        halt  ; The next opcode (0x3E) has been prefetched by now.
              ; Interrupt dispatching is currently disabled
              ;  => PC is not incremented and still points to "halt".

3E 3C   ld a, 0x3C   ; PC is incremented and points to byte 0x3E now.
                     ; The prefetched opcode (0x3E) is executed.
                     ; PC still pointing to byte 0x3E changes the actually
                     ; executed instructions to:
3E 3E   ld  a, 0x3E  ; a = 0x3E
3C      inc a        ; a = 0x3F
```

Gambatte test rom
[halt/noime_ifandie_halt_sra_dmg08_cgb04c_outF1](
https://github.com/sinamas/gambatte/tree/master/test/hwtests/halt/noime_ifandie_halt_sra_dmg08_cgb04c_outF1.asm):
```
        ; interrupt dispatching disabled

        ld  a, 0x11    ; a = 0x11
        ld  e, 0x01    ; e = 0x01
        ldh (0x0F), a  ; IF = 0x11
        ldh (0xFF), a  ; IE = 0x11

        halt  ; The next opcode (0xCB) has been prefetched by now.
              ; Interrupt dispatching is currently disabled
              ;  => PC is not incremented and still points to "halt".

CB 2F   sra a     ; PC is incremented and points to byte 0xCB now.
                  ; The prefetched opcode (0xCB) is executed.
                  ; PC still pointing to byte 0xCB changes the actually
                  ; executed instructions to:
CB CB   set 1, e  ; e = 0x03
2F      cpl       ; a = 0xEE

        add a, e  ; a = 0xF1
```


## `HALT` bug, opcode prefetch and interrupt dispatching

Gambatte test rom
[halt/ifandie_ei_halt_sra_dmg08_cgb04c_out0A](
https://github.com/sinamas/gambatte/tree/master/test/hwtests/halt/ifandie_ei_halt_sra_dmg08_cgb04c_out0A.asm):
```
        ; interrupt dispatching disabled

        ld  a, 0x11    ; a = 0x11
        ldh (0x0F), a  ; IF = 0x11
        ldh (0xFF), a  ; IE = 0x11

        ei  ; Interrupt dispatching is enabled after the next instruction
            ; has been executed.

        halt  ; The next opcode (0x3C) has been prefetched by now.
              ; Interrupt dispatching is currently disabled
              ;  => PC is not incremented and still points to "halt".

        ; Before V-Blank interrupt:
        ;
        ; Interrupt dispatching is enabled before the next instruction
        ; (inc a) is executed.
        ; The "halt" instruction's address is pushed onto the stack,
        ; the V-Blank interrupt is dispatched and PC = 0x40.

        ; After V-Blank interrupt (handler returned  to "halt"):
        ;
3C      inc a  ; PC is incremented and points to byte 0x3C now.
               ; The prefetched opcode (0x3C) is executed.
               ; PC still pointing to byte 0x3C changes the actually
               ; executed instructions to:
3C      inc a  ; a = 0x09
3C      inc a  ; a = 0x0A

@40:    ; V-Blank interrupt handler
        sra a  ; a = 0x08
        ret    ; returns to "halt" instruction
               ; (interrupt dispatching is disabled now)
```
