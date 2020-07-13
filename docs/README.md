# AGE documentation

AGE documentation is still in a very early stage.
If you want me to prioritize anything just let me know.


## Test rom analyses

Before diving into test rom analyses please read about
[clocks, cycles and state][l-a1].

* [`HALT` bug][l-tr1]
* [Interrupt dispatching][l-tr2]
* [LCD - groundwork][l-tr3]
* [LCD - `STAT` mode timing][l-tr4]
* [Serial transfer][l-tr5]
* [Sound circuit (old)][l-tr6]


## Links

* [Cycle-Accurate Game Boy Docs (AntonioND)][l-e1]
* [Game Boy: Complete Technical Reference (Joonas Javanainen)][l-e2]
* [Pan Docs][l-e3]



[l-a1]: age/cycles-state.md

[l-tr1]: test-roms/halt-bug.md
[l-tr2]: test-roms/interrupt-dispatch.md
[l-tr3]: test-roms/lcd-1-groundwork.md
[l-tr4]: test-roms/lcd-stat-mode-timing.md
[l-tr5]: test-roms/serial.md
[l-tr6]: test-roms/sound.md

[l-e1]: https://github.com/AntonioND/giibiiadvance/tree/master/docs
[l-e2]: https://gekkio.fi/files/gb-docs/gbctr.pdf
[l-e3]: https://gbdev.io/pandocs/
