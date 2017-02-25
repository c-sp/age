[![build status](/../badges/master/build.svg)](https://gitlab.com/csprenger/age/commits/master)

# AGE - Another Gameboy Emulator

AGE is a [Gameboy](https://en.wikipedia.org/wiki/Game_Boy) emulator aiming at accuracy and usability. It is written in [C++](https://en.wikipedia.org/wiki/C%2B%2B) and makes use of [Qt](https://www.qt.io) to be platform independent.

AGE allows you to:
- run ***Gameboy*** and ***Gameboy Color*** roms.
- improve the ***visual quality*** with image filters.
- ***reduce flicker*** for some Gameboy roms.
- automatically store and load ***savegames*** (if supported by the Gameboy rom).
- configure ***buttons*** and ***hotkeys*** as you like.
- improve ***audio quality*** (applying a low pass filter with Kaiser window).
- compile for Windows and Linux (and Mac? I did not try it yet) since AGE is not based on any OS specific code.

I do not expect AGE to be finished (in terms of "I will not continue working on it") any time soon. For details on my future plans please see the [milestones](../milestones).

## Improve visual quality

AGE can scale the Gameboy's low resolution visuals up to current screen resolutions without making them look blurry or blocky. I slightly modified the [scale2x](http://www.scale2x.it/) graphics effect to achieve that. The following two images illustrate AGE's image filtering capabilities. They show a scene from Nintendo's [The Legend of Zelda: Link’s Awakening DX](https://en.wikipedia.org/wiki/The_Legend_of_Zelda:_Link's_Awakening).

| AGE's default Image Filter | No Image Filter |
|:--------------------------:|:---------------:|
|![AGE - Zelda, image filter](/readme_files/age_zelda_image_filter.png)*Applying AGE's default image filters makes the scene look much better on current screen resolutions.*|![AGE - Zelda, no image filter](/readme_files/age_zelda_no_image_filter.png)*The scene looks "blocky" which is caused by the Gameboy's low resolution.*|

## Reduce flicker

Some Gameboy games make use of fast changing graphics to create a transparency effect. On current screens this results in noticable flicker. AGE can migitate the flicker by blending frames together. The following two images demonstrate that. They show a scene from Nintendo's [The Legend of Zelda: Link’s Awakening DX](https://en.wikipedia.org/wiki/The_Legend_of_Zelda:_Link's_Awakening). Note that the black ball is supposed to depict a dog.

| No Flicker | Flicker |
|:----------:|:-------:|
|![AGE - Zelda, no flicker](/readme_files/age_zelda_no_flicker.png)*Blending two frames prevents flicker on the dog's chain and shadow.*|![AGE - Zelda, flicker](/readme_files/age_zelda_flicker.png)*The dog's chain and shadow flicker since they are visible only on every other frame.*|

## Settings and savegames

AGE stores all settings and savegames in a subdirectory called `.age_emulator` it creates in your home directory.

# The AGE CI pipeline

AGE is continuously being built and tested using GitLab's [CI pipeline](/../pipelines) with [docker-age-ci](https://gitlab.com/csprenger/docker-age-ci), which is based on [docker-qt-gcc](https://gitlab.com/csprenger/docker-qt-gcc), a docker image I created to build [Qt](https://www.qt.io) applications.

# History

AGE has been my spare time project since around 1998 when I was 16 years old. My first attempts to develop a Gameboy emulator were done with [QBasic](https://en.wikipedia.org/wiki/QBasic) which was the first programming language I learned on the PC (coming from the C64 I already new some Basic and Assembler). I soon extended the [QBasic](https://en.wikipedia.org/wiki/QBasic) code with a self written x86 assembler library but never emulated more than the Gameboy's CPU.

After taking a detour over Visual Basic, which I found not really suited for developing a Gameboy emulator, I reached C++ and the [Win32 API](https://msdn.microsoft.com/de-de/library/windows/desktop/ff818516(v=vs.85).aspx). Over time my emulator improved and could even run Gameboy Color roms. The last big step was moving to [Qt](https://www.qt.io) to not depend on the [Win32 API](https://msdn.microsoft.com/de-de/library/windows/desktop/ff818516(v=vs.85).aspx) any more and allow for compiling AGE on different operating systems.

AGE has been published for a while under the name "SimGBE" (Simple Gameboy Emulator), but when the project lay dormant for some time I took the site down. With platforms like GitLab I now feel I can publish AGE in a way that it is useful for interested people even when I'm not working on it for a while.

