# doseffects

At the turn of the millenium, I wrote some graphical effects for DOS using 
the famous VGA mode 13h.

I finally cleaned them up (to some extent), and combined into one monolithic package.

The effects combined here are all written in pure C. On cleaning them up, I got rid 
of the remaining inline Assembly. I have a lot more effects written in Assembler in a 
[separate repository](https://github.com/efliks/demoscene-legacy).

The code is horrible, because I wrote it in my teens. I am in the process of making
it a bit nicer, but I cannot promise miracles.

A quick demonstration video: [https://youtu.be/82xVEE4bYbs](https://youtu.be/82xVEE4bYbs).

## Installation instructions

To compile the graphical effects, install [Open Watcom 2.0](https://github.com/open-watcom/open-watcom-v2) 
on your DOS PC, or your VM running DOS. For me, it is a VirtualBox VM with Win98. 

I put here a CMakeLists.txt, but only so I can do development on a modern Linux PC.
Some modules for accessing the hardware had to be mocked with dummies. The code
compiles with GCC and runs, but nothing happens on the screen, which is expected.

To make it actually run, copy the files to your DOS PC, except CMakeLists.txt and 
the dummy folder.

Then, simply invoke in the main folder:

    c:\doseffec > wmake -f makefile.wc

Run the vgademo.exe executable. Switching to the next graphical effect is done by 
pressing any key.
