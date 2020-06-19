
# `HALT` behavior


## Interrupts disabled

Gambatte test rom `halt/noime_ifandie_halt_lda_3c_dmg08_cgb04c_out3F`:
```
        ld  a, 0x11    ; a = 0x11
        ldh (0x0F), a  ; IF = 0x11
        ldh (0xFF), a  ; IE = 0x11

        halt  ; The next opcode (0x3E) has been prefetched by now.
              ; Interrupts are currently disabled
              ;  => PC is not incremented and still points to byte 0x3E.

3E 3C   ld a, 0x3C  ; The prefetched opcode (0x3E) is executed.
                    ; PC still pointing to byte 0x3E changes the actually
                    ; executed instructions to:

3E 3E   ld  a, 0x3E  ; a = 0x3E
3C      inc a        ; a = 0x3F
```


Gambatte test rom `halt/noime_ifandie_halt_sra_dmg08_cgb04c_outF1`:
```
        ld  a, 0x11    ; a = 0x11
        ld  e, 0x01    ; e = 0x01
        ldh (0x0F), a  ; IF = 0x11
        ldh (0xFF), a  ; IE = 0x11

        halt  ; The next opcode (0xCB) has been prefetched by now.
              ; Interrupts are currently disabled
              ;  => PC is not incremented and still points to byte 0xCB.

CB 2F   sra a  ; The prefetched opcode (0xCB) is executed.
               ; PC still pointing to byte 0xCB changes the actually
               ; executed instructions to:

CB CB   set 1, e  ; e = 0x03
2F      cpl       ; a = 0xEE
83      add a, e  ; a = 0xF1
```


## Interrupts disabled, `EI`

Gambatte test rom `halt/ifandie_ei_halt_sra_dmg08_cgb04c_out0A`:
```
        ld  a, 0x11    ; a = 0x11
        ldh (0x0F), a  ; IF = 0x11
        ldh (0xFF), a  ; IE = 0x11

        ei  ; Interrupts are enabled after the next instruction
            ; has been executed.

        halt  ; The next opcode (0x3C) has been prefetched by now.
              ; Interrupts are currently disabled
              ;  => PC is not incremented and still points to byte 0x3C.

        ; Interrupts are enabled before the next instruction (inc a)
        ; is executed.
        ; The V-Blank interrupt is dispatched and PC = 0x40.
        ; Opcode 0x3C (inc a) remains prefetched.

3C      inc a  ; not executed (here)

@40:    ; V-Blank interrupt handler
CB 2F   sra a  ; With opcode 0x3C still prefetched this is actually
               ; executed as:

3C      inc a  ; a = 0x12
CB 2F   sra a  ; a = 0x0A
```
