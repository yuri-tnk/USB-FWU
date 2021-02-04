# =============================================================================
#
# Make file for USB Firmware Upgrader
#
# Processor: LPC214x (LPC2146)
#
# Output:    usb_fwu
#
# Compiler: GCC ARM  3.4.3 (GNUARM + Cygwin) or 4.10
#
# !!!! Do not use  GCC ARM 4.0.1. It makes a few problems  !!!!
#
#  No dependencies support. Call 'make clean' and than 'make' to rebuild all
#
# =============================================================================

# =========  General =============

FORMAT = ihex

# =========  Pathes ==============

ROOTDIR  = D:/CYGWIN
INCLUDES =

TOOLSROOT = D:/GNUARM
TOOLS     = $(TOOLSROOT)/bin/arm-elf-

PRJROOTPATH = F:/TNKernel_v21/LPC214X_USB_FWU
PRJPATH   = $(PRJROOTPATH)/GCC
FILESPATH = $(PRJROOTPATH)/usb_fwu

DEBUG         = $(PRJPATH)/DEBUG
OBJ           = $(PRJPATH)/OBJ


#===========  Files  ==============

#--  Project target file name (without extension).

IMAGE = usb_fwu

#--  Linker script

LDSCRIPT = $(PRJPATH)/fwu_lpc2146_gcc.ld

#-- List of objects

OBJECTS = \
  $(OBJ)/startup_hardware_init.o\
  $(OBJ)/fwu_utils.o\
  $(OBJ)/fwu.o\
  $(OBJ)/fwu_int.o\
  $(OBJ)/fwu_queue.o\
  $(OBJ)/fwu_usb_descr.o\
  $(OBJ)/fwu_usb_ep.o\
  $(OBJ)/fwu_usb_hw.o\
  $(OBJ)/fwu_usb_req.o\
  $(OBJ)/fwu_asm.o\
  $(OBJ)/fwu_init.o\
  $(OBJ)/fwu_usb_thumb.o\
  $(OBJ)/fwu_startup_gcc.o

# ---------------------------------------------------------------------------
#  Tools, tools options(flags)
# ---------------------------------------------------------------------------

SHELL   = D:/cygwin/bin/sh
CC      = $(TOOLS)gcc
AS      = $(TOOLS)as
CPP     = $(TOOLS)g++
LD      = $(TOOLS)ld
OBJCOPY = $(TOOLS)objcopy
OBJDUMP = $(TOOLS)objdump
SIZE    = $(TOOLS)size
NM      = $(TOOLS)nm
REMOVE  = rm -f
COPY    = cp


OPTFLAGS = -mcpu=arm7tdmi -mhard-float
# //-mfpu=hard
# -mfpu=soft -msoft-float
DEBUG =

#-- Assembler flags

ADEFS =
ASMOPTS  =  --traditional-format -march=armv4t -mthumb-interwork -EL

#-- Compiler  flags

CDEF =

CCOPTS  = $(OPTFLAGS) $(INCLUDES) $(CDEF) -c \
     -mlittle-endian -Wall -gdwarf-2 -std=c99 \
     -Os -march=armv4t -mthumb-interwork -mlittle-endian -fno-builtin

CCOPTSTH  = $(OPTFLAGS) $(INCLUDES) $(CDEF) -c \
     -mlittle-endian -Wall -gdwarf-2 -std=c99 \
     -Os -march=armv4t -mthumb -mthumb-interwork -mlittle-endian -fno-builtin

#-- Linker    flags

LDFLAGS =  -nostartfiles -Wl,-Map=$(OBJ)/$(IMAGE).map,--cref  -lc \
           $(NEWLIBLPC) -lm -lc -lgcc  -T$(LDSCRIPT)


#-- Combine all necessary flags and optional flags.

ALL_CFLAGS =  -I. $(CFLAGS)
ALL_ASFLAGS = -I. $(ASFLAGS)

# ---------------------------------------------------------------------------
#  Build
# ---------------------------------------------------------------------------

all: $(PRJPATH)/$(IMAGE).elf  sizeafter


#--- Compiling/Assembling ---

#  Asm

$(OBJ)/startup_hardware_init.o: $(PRJPATH)/startup_hardware_init.s
	$(AS) $(ASMOPTS) $(PRJPATH)/startup_hardware_init.s -o $(OBJ)/startup_hardware_init.o
$(OBJ)/fwu_startup_gcc.o: $(PRJPATH)/fwu_startup_gcc.s
	$(AS) $(ASMOPTS) $(PRJPATH)/fwu_startup_gcc.s -o $(OBJ)/fwu_startup_gcc.o
$(OBJ)/fwu_asm.o: $(FILESPATH)/fwu_asm.s
	$(AS) $(ASMOPTS) $(FILESPATH)/fwu_asm.s -o $(OBJ)/fwu_asm.o

# C ARM

$(OBJ)/fwu_utils.o: $(FILESPATH)/fwu_utils.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu_utils.c -o $(OBJ)/fwu_utils.o
$(OBJ)/fwu.o: $(FILESPATH)/fwu.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu.c -o $(OBJ)/fwu.o
$(OBJ)/fwu_int.o: $(FILESPATH)/fwu_int.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu_int.c -o $(OBJ)/fwu_int.o
$(OBJ)/fwu_queue.o: $(FILESPATH)/fwu_queue.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu_queue.c -o $(OBJ)/fwu_queue.o
$(OBJ)/fwu_usb_descr.o: $(FILESPATH)/fwu_usb_descr.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu_usb_descr.c -o $(OBJ)/fwu_usb_descr.o
$(OBJ)/fwu_usb_ep.o: $(FILESPATH)/fwu_usb_ep.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu_usb_ep.c -o $(OBJ)/fwu_usb_ep.o
$(OBJ)/fwu_usb_hw.o: $(FILESPATH)/fwu_usb_hw.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu_usb_hw.c -o $(OBJ)/fwu_usb_hw.o
$(OBJ)/fwu_usb_req.o: $(FILESPATH)/fwu_usb_req.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu_usb_req.c -o $(OBJ)/fwu_usb_req.o

# C THUMB

$(OBJ)/fwu_init.o: $(FILESPATH)/fwu_init.c
	$(CC) $(CCOPTSTH) $(FILESPATH)/fwu_init.c -o $(OBJ)/fwu_init.o
$(OBJ)/fwu_usb_thumb.o: $(FILESPATH)/fwu_usb_thumb.c
	$(CC) $(CCOPTSTH) $(FILESPATH)/fwu_usb_thumb.c -o $(OBJ)/fwu_usb_thumb.o


#-- Linking ---

$(PRJPATH)/$(IMAGE).elf: $(OBJECTS)

	$(REMOVE)  $(OBJ)/$(IMAGE).elf
	$(REMOVE)  $(OBJ)/$(IMAGE).hex
	$(REMOVE)  $(OBJ)/$(IMAGE).lst
	$(REMOVE)  $(OBJ)/$(IMAGE).map

	$(CC) $(ALL_CFLAGS) $(OBJECTS) --output $(OBJ)/$(notdir $@) $(LDFLAGS)

	$(OBJCOPY) -O$(FORMAT)    $(OBJ)/$(IMAGE).elf  $(OBJ)/$(IMAGE).hex
#	$(NM) -n $(OBJ)/$(IMAGE).elf

#--- Display size of file.

#HEXSIZE = $(SIZE) --target=$(FORMAT) $(OBJ)/$(IMAGE).hex
ELFSIZE = $(SIZE) -A $(OBJ)/$(IMAGE).elf

sizeafter:
	@if [ -f $(OBJ)/$(IMAGE).elf ]; then echo; $(ELFSIZE); echo; fi


# ===========================================================================

.PHONY:   clean

clean:

	$(REMOVE) $(OBJECTS)

# ===========================================================================
# ===========================================================================
# ===========================================================================



