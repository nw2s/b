

# nw2s::b - A microcontroller-based modular synth control framework
# Copyright (C) 2013 Scott Wilson (thomas.scott.wilson@gmail.com)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# The original version of this file can be found at https://github.com/pauldreik/arduino-due-makefile
# By Paul Dreik 20130503 http://www.pauldreik.se/



# Which serial port to use 
# Add a file with SUBSYSTEMS=="usb", ATTRS{product}=="Arduino Due Prog. Port", ATTRS{idProduct}=="003d",
# ATTRS{idVendor}=="2341", SYMLINK+="arduino_due" in /etc/udev/rules.d/ to get this working
PORT = /dev/arduino_due

# If you want to verify the bossac upload, define this to -v. Leave blank otherwise.
VERIFY = -v

# Locations of tools
CXX = 		../tools/gcc-arm-none-eabi-4.8.3-2014q1/bin/arm-none-eabi-g++
CC = 		../tools/gcc-arm-none-eabi-4.8.3-2014q1/bin/arm-none-eabi-gcc
C = 		$(CC)
SAM = 		../arduino/sam
CMSIS = 	../arduino/sam/system/CMSIS
LIBSAM = 	../arduino/sam/system/libsam
TMPDIR = 	./build
BINDIR =	./bin
AR = 		../tools/gcc-arm-none-eabi-4.8.3-2014q1/bin/arm-none-eabi-ar 
OBJCOPY =  	../tools/gcc-arm-none-eabi-4.8.3-2014q1/bin/arm-none-eabi-objcopy



# Defines
DEFINES:=-Dprintf=iprintf -DF_CPU=84000000L -DARDUINO=152 -D__SAM3X8E__ -DUSB_PID=0x003e -DUSB_VID=0x2341 -DUSBCON

# Includes
INCLUDES = 	-Isrc -Isrc/devices -Isrc/util -Isrc/drivers -I$(LIBSAM) -I$(CMSIS)/CMSIS/Include/ -I$(CMSIS)/Device/ATMEL/ \
			-I$(SAM)/cores/arduino -I$(SAM)/variants/arduino_due_x -I$(SAM)/libraries/SPI -I$(SAM)/libraries/Wire \
			-Isrc/drivers/usbhost -Isrc/drivers/usbhost -Isrc/libraries 

# Compilation flags common to both c and c++
COMMON_FLAGS = -g -Os -w -ffunction-sections -fdata-sections -nostdlib --param max-inline-insns-single=500 -mcpu=cortex-m3  -mthumb

CFLAGS = $(COMMON_FLAGS)
CXXFLAGS = $(COMMON_FLAGS) -fno-rtti -fno-exceptions 

# Let the results be named after the project
VERSION = 1.2
FIRMWARE = sd
PROJNAME = nw2s-b-$(VERSION)-$(FIRMWARE)
	
# nw2s-b source files
SRCFILES =												\
			src/drivers/dac/mcp4822.cpp					\
			src/drivers/pwm/pca9685.cpp					\
			src/drivers/sd/File.cpp						\
			src/drivers/sd/SD.cpp						\
			src/drivers/sd/utility/Sd2Card.cpp			\
			src/drivers/sd/utility/SdFile.cpp			\
			src/drivers/sd/utility/SdVolume.cpp			\
			src/drivers/usbhost/adk.cpp					\
			src/drivers/usbhost/hid2.cpp				\
			src/drivers/usbhost/hidboot.cpp				\
			src/drivers/usbhost/KeyboardController.cpp	\
			src/drivers/usbhost/MouseController.cpp		\
			src/drivers/usbhost/parsetools.cpp			\
			src/drivers/usbhost/Usb.cpp					\
			src/util/b.cpp								\
			src/util/Entropy.cpp						\
			src/util/EventManager.cpp					\
			src/util/IO.cpp								\
			src/util/JSONUtil.cpp						\
			src/util/Key.cpp							\
			src/util/NoteStack.cpp						\
			src/util/SignalData.cpp						\
			src/util/Timers.cpp							\
			src/util/SDFirmware.cpp						\
			src/libraries/aJSON/aJSON.cpp				\
			src/libraries/aJSON/utility/stringbuffer.c	\
			src/devices/Arc.cpp							\
			src/devices/AudioDevice.cpp					\
			src/devices/BinaryArc.cpp					\
			src/devices/Clock.cpp						\
			src/devices/DrumTrigger.cpp					\
			src/devices/GameOfLife.cpp					\
			src/devices/Gate.cpp						\
			src/devices/Grid.cpp						\
			src/devices/GridNoteSequencer.cpp			\
			src/devices/GridOto.cpp						\
			src/devices/GridTrigger.cpp					\
			src/devices/Loop.cpp						\
			src/devices/Oscillator.cpp					\
			src/devices/RatchetDivider.cpp				\
			src/devices/Sequence.cpp					\
			src/devices/ShiftRegister.cpp				\
			src/devices/SlideRuleGrid.cpp				\
			src/devices/Trigger.cpp						\
			src/devices/UsbMidi.cpp						\
			src/devices/UsbPS3.cpp						\
			src/firmware/$(FIRMWARE)Main.cpp
			
OBJFILES = $(addsuffix .o,$(addprefix $(TMPDIR)/,$(notdir $(SRCFILES))))

#These source files are the ones forming core.a
CORESRCXX = $(shell ls ${SAM}/cores/arduino/*.cpp ${SAM}/cores/arduino/USB/*.cpp ${SAM}/variants/arduino_due_x/variant.cpp )  \
			${SAM}/libraries/SPI/SPI.cpp								\
			${SAM}/libraries/Wire/Wire.cpp

CORESRC = $(shell ls ${SAM}/cores/arduino/*.c )

#convert the core source files to object files. assume no clashes.
COREOBJSXX = $(addprefix $(TMPDIR)/core/,$(notdir $(CORESRCXX)) )
COREOBJSXX := $(addsuffix .o,$(COREOBJSXX))
COREOBJS = $(addprefix $(TMPDIR)/core/,$(notdir $(CORESRC)) )
COREOBJS := $(addsuffix .o,$(COREOBJS))

default:
	@echo default rule, does nothing. Try make compile or make upload.

#This rule is good to just make sure stuff compiles, without having to wait
#for bossac.
compile: $(TMPDIR)/$(PROJNAME).elf $(BINDIR)/$(PROJNAME).bin

#This is a make rule template to create object files from the source files.
# arg 1=src file
# arg 2=object file
# arg 3= XX if c++, empty if c
define OBJ_template
$(2): $(1)
	$(C$(3)) -MD -c $(C$(3)FLAGS) $(DEFINES) $(INCLUDES) $(1) -o $(2)
endef
#now invoke the template both for c++ sources
$(foreach src,$(CORESRCXX), $(eval $(call OBJ_template,$(src),$(addsuffix .o,$(addprefix $(TMPDIR)/core/,$(notdir $(src)))),XX) ) )

#...and for c sources:
$(foreach src,$(CORESRC), $(eval $(call OBJ_template,$(src),$(addsuffix .o,$(addprefix $(TMPDIR)/core/,$(notdir $(src)))),) ) )

#and our own c++ sources
$(foreach src,$(SRCFILES), $(eval $(call OBJ_template,$(src),$(addsuffix .o,$(addprefix $(TMPDIR)/,$(notdir $(src)))),XX) ) )


clean:
	test ! -d $(TMPDIR) || rm -rf $(TMPDIR)

.PHONY: upload default

$(TMPDIR):
	mkdir -p $(TMPDIR)

$(TMPDIR)/core:
	mkdir -p $(TMPDIR)/core

# Include the dependencies for our own files
-include $(OBJFILES:.o=.d)

# Create the core library from the core objects. Do this EXACTLY as the
# arduino IDE does it, seems *really* picky about this.
$(TMPDIR)/core.a: $(TMPDIR)/core $(COREOBJS) $(COREOBJSXX)
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/wiring_shift.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/wiring_analog.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/itoa.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/cortex_handlers.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/hooks.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/wiring.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/WInterrupts.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/syscalls_sam3.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/iar_calls_sam3.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/wiring_digital.c.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/Print.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/USARTClass.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/WString.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/USBCore.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/CDC.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/HID.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/wiring_pulse.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/UARTClass.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/main.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/cxxabi-compat.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/Stream.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/RingBuffer.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/IPAddress.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/Reset.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/WMath.cpp.o 
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/variant.cpp.o
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/SPI.cpp.o
	$(AR) rcs $(TMPDIR)/core.a $(TMPDIR)/core/Wire.cpp.o

#link our own object files with core to form the elf file
$(TMPDIR)/$(PROJNAME).elf: $(TMPDIR)/core.a $(TMPDIR)/core/syscalls_sam3.c.o $(OBJFILES) 
	$(CXX) -Os -Wl,--gc-sections -mcpu=cortex-m3 -T$(SAM)/variants/arduino_due_x/linker_scripts/gcc/flash.ld -Wl,-Map,$(NEWMAINFILE).map -o $@ -L$(TMPDIR) -lm -lgcc -mthumb -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--warn-unresolved-symbols -Wl,--start-group $(TMPDIR)/core/syscalls_sam3.c.o $(OBJFILES) $(SAM)/variants/arduino_due_x/libsam_sam3x8e_gcc_rel.a $(TMPDIR)/core.a -Wl,--end-group

#copy from the hex to our bin file (why?)
$(BINDIR)/$(PROJNAME).bin: $(TMPDIR)/$(PROJNAME).elf 
	$(OBJCOPY) -O binary $< $@

#upload to the arduino by first resetting it (stty) and the running bossac
upload: $(BINDIR)/$(PROJNAME).bin
	stty -F $(PORT) cs8 1200 hupcl
	../tools/bossac -U false -e -w $(VERIFY) -b $(BINDIR)/$(PROJNAME).bin -R

#to view the serial port with screen.
monitor:
	screen $(PORT) 115200
	