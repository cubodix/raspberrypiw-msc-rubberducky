# raspberrypiw-msc-rubberducky
 a rubberducky (target: Win7-10 x86-x64) for raspberry pi W with (constant / ROM) storage, not sure if it works with standard raspberry pi

# compiling instructions
- modify the .ima example file (don't change the name) with a disk editor, i recommend WinImage, but you cloud use any other editor that supports fat12, y.bat inside fat12_disk.ima is first to be ran
- run a script from build_script, depending which OS do you have (right now script is only available in windows), this script will convert fat12_disk.ima in a C++ array header file
- open msc_rubber_ducky.ino in arduino IDE and compile it, remember that raspberry's led builtin will turn on (at 3.90625 FPS) when host (computer) reads or tries to write msc disk
