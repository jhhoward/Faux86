# Faux86: A portable, open-source 8086 PC emulator for bare metal Raspberry Pi
Faux86 is designed to be run 'bare metal' on a Raspberry Pi. This means that the emulator runs directly on the hardware so no OS needs to booted on the Pi. 

## Features
- 8086 and 80186 instruction set emulation
- CGA / EGA / VGA emulation is mostly complete
- PC speaker, Adlib and Soundblaster sound emulation
- Serial mouse emulation

## Usage with Raspberry Pi
By default Faux86 boots from a floppy image dosboot.img which in the emulator is mounted as drive A. The SD card will be mounted as drive C and any connected USB mass storage device will be mounted as D.
Since MS-DOS is accessing the SD card directly, it does not work for large SD card types. I have found the best solution is to use a small capacity SD card and flash the image as a 32MB card.
USB keyboard and mouse should be plugged in before booting - hot swapping of devices is not supported.

## Credits
Faux86 was originally based on the Fake86 emulator by Mike Chambers. 
http://fake86.rubbermallet.org
A lot of the code has been shuffled around or rewritten in C++ but the core CPU emulation remains mostly the same.

Faux86 uses the Circle library to interface with the Raspberry Pi
https://github.com/rsta2/circle



  