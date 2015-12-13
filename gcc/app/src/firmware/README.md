This folder contains the "main" files. What would be the *.ino files in an Arduino project. Each contains a setup() and loop() method. There can be only one main firmware file compiled into each elf/bin project. 

The Makefile is set up so that if you want to build one of these you invoke as:

make compile FIRMWARE=sd

