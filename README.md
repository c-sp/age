[![build status](/../badges/master/build.svg)](https://gitlab.com/csprenger/age/commits/master)

# AGE - Another Gameboy Emulator

AGE is a Gameboy emulator aiming at accuracy and usability.

AGE allows you to:
- run ***Gameboy*** and ***Gameboy Color*** roms.
- improve the ***visual quality*** with image filters.
- ***reduce flicker*** for some Gameboy roms.
- automatically store and load ***savegames*** (if supported by the respective Gameboy rom).
- configure ***buttons*** and ***hotkeys*** as you like.
- improve ***audio quality*** (applying a low pass filter with Kaiser window).
- compile for Windows and Linux (and Mac? I did not try it yet) since it is not based on any OS specific code.

## Improve visual quality

AGE can scale the Gameboy's low resolution visuals up to current screen resolutions without making them look blurry or blocky. I slightly modified the [scale2x](http://www.scale2x.it/) graphics effect to achieve that.

The following images show a scene from Nintendo's [The Legend of Zelda: Link’s Awakening DX](https://en.wikipedia.org/wiki/The_Legend_of_Zelda:_Link's_Awakening).

| No Image Filter | AGE's default Image Filter |
|:---------------:|:--------------------------:|
|![AGE - Zelda, no image filter](/readme_files/age_zelda_no_image_filter.png)*The scene looks "blocky" with the Gameboy's low resolution.*|![AGE - Zelda, image filter](/readme_files/age_zelda_image_filter.png)*Applying AGE's default image filters makes the scene look much better on current screen resolutions.*|

## Reduce flicker

Some Gameboy games make use of fast changing graphics to create a blur effect. On current screens this results in noticable flicker. AGE can migitate the flicker by blending a configurable number of frames together.

The following images show a scene from Nintendo's [The Legend of Zelda: Link’s Awakening DX](https://en.wikipedia.org/wiki/The_Legend_of_Zelda:_Link's_Awakening). Note that the black ball is supposed to depict a dog.

| Flicker | No Flicker |
|:-------:|:----------:|
|![AGE - Zelda, flicker](/readme_files/age_zelda_flicker.png)*The dog's chain and shadow flicker since they are visible only on every other frame.*|![AGE - Zelda, no flicker](/readme_files/age_zelda_no_flicker.png)*Blending two frames causes the dog's chain and shadow to not flicker any more.*|

## Settings and savegames

AGE stores settings and savegames in a subdirectory called `.age_emulator` it creates in your home directory.

## The AGE CI pipeline

AGE is continuously being built using GitLab's [CI pipeline](/../pipelines) with [docker-qt-gcc](https://gitlab.com/csprenger/docker-qt-gcc), a docker image I created to build [Qt](https://www.qt.io) applications.

# Future plans

I do not expect AGE to be finished any time soon. My plans for AGE are to:
- *(Graphics)* migrate the rendering code from the old OpenGL fixed function pipeline to shaders.
- *(CI)* extend the [CI pipeline](/../pipelines) by regression tests.
- *(CI)* create a build job for the AGE doxygen documentation and deploy it where it is visible to the public.
- *(Gameboy)* improve the Gameboy emulation accuracy more and more.
- *(Gameboy, UI)* emulate a Game Link Cable over TCP/IP to allow for playing games over the network (maybe even the internet?).
- *(Documentation)* explain how the modified [scale2x](http://www.scale2x.it/) graphics effect works and why I modified it.
- create an installer so that everyone can easily download and install AGE.

# History

AGE has been my spare time project since around 1998 when I was 16 years old. My first attempts to develop a Gameboy emulator were done with [QBasic](https://en.wikipedia.org/wiki/QBasic) which was the first programming language I learned on the PC (coming from the C64 I already new some Basic and Assembler). I soon extended the [QBasic](https://en.wikipedia.org/wiki/QBasic) code with a self written x86 assembler library but never emulated more than the Gameboy's CPU.

After taking a detour over Visual Basic, which I found not really suited for developing a Gameboy emulator, I reached C++ and the [Win32 API](https://msdn.microsoft.com/de-de/library/windows/desktop/ff818516(v=vs.85).aspx). Over time my emulator improved and could even run Gameboy Color roms. The last big step was moving to [Qt](https://www.qt.io) to not depend on the [Win32 API](https://msdn.microsoft.com/de-de/library/windows/desktop/ff818516(v=vs.85).aspx) any more and allow for compiling AGE on different operating systems.

AGE has been published for a while under the name "SimGBE" (Simple Gameboy Emulator), but when the project lay dormant for some time I took the site down. With platforms like GitLab I now feel I can publish AGE in a way that it is useful for interested people even when I'm not working on it for a while.

