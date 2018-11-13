#
# Makefile
#

CIRCLEHOME = ../../circle
SRCDIR = ../src/faux86

OBJS	=  kernel.o main.o CircleHostInterface.o PWMSound.o VCHIQSound.o \
	  $(SRCDIR)/Adlib.o \
	  $(SRCDIR)/Audio.o \
	  $(SRCDIR)/Config.o \
	  $(SRCDIR)/CPU.o \
	  $(SRCDIR)/DisneySoundSource.o \
	  $(SRCDIR)/DMA.o \
	  $(SRCDIR)/DriveManager.o \
	  $(SRCDIR)/InputManager.o \
	  $(SRCDIR)/MemUtils.o \
	  $(SRCDIR)/PCSpeaker.o \
	  $(SRCDIR)/PIC.o \
	  $(SRCDIR)/PIT.o \
	  $(SRCDIR)/Ports.o \
	  $(SRCDIR)/Ram.o \
	  $(SRCDIR)/Renderer.o \
	  $(SRCDIR)/SerialMouse.o \
	  $(SRCDIR)/SoundBlaster.o \
	  $(SRCDIR)/TaskManager.o \
	  $(SRCDIR)/Timing.o \
	  $(SRCDIR)/Video.o \
	  $(SRCDIR)/VM.o

CPPFLAGS = -I$(SRCDIR) -Wall -Werror -O3

#OBJS	= ../src/fake86/main.o

#LIBS	= $(CIRCLEHOME)/lib/usb/libusb.a \
#	  $(CIRCLEHOME)/lib/input/libinput.a \
#	  $(CIRCLEHOME)/lib/fs/libfs.a \
#	  $(CIRCLEHOME)/lib/libcircle.a

LIBS	= $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	  $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
	  $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	  $(CIRCLEHOME)/addon/linux/liblinuxemu.a \
	  $(CIRCLEHOME)/lib/sched/libsched.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/libcircle.a
	  
#LIBS	= $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
#	  $(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
#	  $(CIRCLEHOME)/lib/usb/libusb.a \
#	  $(CIRCLEHOME)/lib/input/libinput.a \
#	  $(CIRCLEHOME)/lib/fs/libfs.a \
#	  $(CIRCLEHOME)/lib/libcircle.a

include $(CIRCLEHOME)/sample/Rules.mk
