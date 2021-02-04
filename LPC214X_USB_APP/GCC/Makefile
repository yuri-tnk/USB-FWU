# =============================================================================
#
# Make file for TNKernel USB loadable application
#
# Processor: LPC2146
#
# Example:   usb_app
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
INCLUDES = -I ./ -I ../../TNKernel

TOOLSROOT = D:/GNUARM
TOOLS     = $(TOOLSROOT)/bin/arm-elf-

PRJPATH   = F:/TNKernel_v21/LPC214X_USB_APP/GCC

DEBUG         = $(PRJPATH)/DEBUG
OBJ           = $(PRJPATH)/OBJ
TNKERNELPATH  = $(PRJPATH)/../../TNKernel
FILESPATH     = $(PRJPATH)/../usb_app


#===========  Files  ==============

#--  Project target file name (without extension).

IMAGE = usb_app

#--  Linker script

LDSCRIPT = $(PRJPATH)/lpc2146_app_gcc.ld

#-- List of objects

OBJECTS = \
  $(OBJ)/tn.o\
  $(OBJ)/tn_utils.o\
  $(OBJ)/tn_dqueue.o\
  $(OBJ)/tn_event.o\
  $(OBJ)/tn_mem.o\
  $(OBJ)/tn_mutex.o\
  $(OBJ)/tn_port.o\
  $(OBJ)/tn_sem.o\
  $(OBJ)/tn_tasks.o\
  $(OBJ)/tn_port_asm.o\
  $(OBJ)/utils.o\
  $(OBJ)/tn_usb_descr.o\
  $(OBJ)/tn_usb_ep.o\
  $(OBJ)/tn_usb_hw.o\
  $(OBJ)/tn_usb_req.o\
  $(OBJ)/tn_user.o\
  $(OBJ)/usb_app.o\
  $(OBJ)/s_itoa.o\
  $(OBJ)/big_flash_size_imit.o\
  $(OBJ)/startup_hardware_init.o\
  $(OBJ)/tnkernel_app_startup_gcc.o

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

DEBUG =

#-- Assembler flags

ADEFS =

#ASMOPTS  = $(OPTFLAGS) $(INCLUDES) -D_GNU_ASSEMBLER_ \
#           -mlittle-endian -nostdinc -x assembler-with-cpp \
#           $(ADEFS)  -Wa,-gstabs


ASMOPTS  =  --traditional-format -march=armv4t -EL

# -Wa,-adhlns=$(<:.S=.lst),-g$(DEBUG)

#-- Compiler  flags

CDEF =

CCOPTS  = $(OPTFLAGS) $(INCLUDES) $(CDEF) -c \
     -mlittle-endian -Wall -gdwarf-2 -std=c99 \
     -O1 -march=armv4t -mlittle-endian -fno-builtin

#-- Linker    flags

LDFLAGS =  -nostartfiles -Wl,-Map=$(OBJ)/$(IMAGE).map,--cref  -lc \
           $(NEWLIBLPC) -lm -lc -lgcc -T$(LDSCRIPT)


#-- Combine all necessary flags and optional flags.

ALL_CFLAGS =  -I. $(CFLAGS)
ALL_ASFLAGS = -I. $(ASFLAGS)

# ---------------------------------------------------------------------------
#  Build
# ---------------------------------------------------------------------------

all: $(PRJPATH)/$(IMAGE).elf  sizeafter


#--- Compiling/Assembling ---

$(OBJ)/tn_port_asm.o: $(TNKERNELPATH)/tn_port_asm.s
	$(AS) $(ASMOPTS) $(TNKERNELPATH)/tn_port_asm.s -o $(OBJ)/tn_port_asm.o
$(OBJ)/utils.o: $(FILESPATH)/utils.c
	$(CC) $(CCOPTS) $(FILESPATH)/utils.c -o $(OBJ)/utils.o
$(OBJ)/tn_usb_descr.o: $(FILESPATH)/tn_usb_descr.c
	$(CC) $(CCOPTS) $(FILESPATH)/tn_usb_descr.c -o $(OBJ)/tn_usb_descr.o
$(OBJ)/tn_usb_ep.o: $(FILESPATH)/tn_usb_ep.c
	$(CC) $(CCOPTS) $(FILESPATH)/tn_usb_ep.c -o $(OBJ)/tn_usb_ep.o
$(OBJ)/tn_usb_hw.o: $(FILESPATH)/tn_usb_hw.c
	$(CC) $(CCOPTS) $(FILESPATH)/tn_usb_hw.c -o $(OBJ)/tn_usb_hw.o
$(OBJ)/tn_usb_req.o: $(FILESPATH)/tn_usb_req.c
	$(CC) $(CCOPTS) $(FILESPATH)/tn_usb_req.c -o $(OBJ)/tn_usb_req.o
$(OBJ)/tn_user.o: $(FILESPATH)/tn_user.c
	$(CC) $(CCOPTS) $(FILESPATH)/tn_user.c -o $(OBJ)/tn_user.o
$(OBJ)/usb_app.o: $(FILESPATH)/usb_app.c
	$(CC) $(CCOPTS) $(FILESPATH)/usb_app.c -o $(OBJ)/usb_app.o
$(OBJ)/big_flash_size_imit.o: $(FILESPATH)/big_flash_size_imit.c
	$(CC) $(CCOPTS) $(FILESPATH)/big_flash_size_imit.c -o $(OBJ)/big_flash_size_imit.o
$(OBJ)/s_itoa.o: $(FILESPATH)/s_itoa.s
	$(AS) $(ASMOPTS) $(FILESPATH)/s_itoa.s -o $(OBJ)/s_itoa.o
$(OBJ)/startup_hardware_init.o: $(PRJPATH)/startup_hardware_init.s
	$(AS) $(ASMOPTS) $(PRJPATH)/startup_hardware_init.s -o $(OBJ)/startup_hardware_init.o
$(OBJ)/tnkernel_app_startup_gcc.o: $(PRJPATH)/tnkernel_app_startup_gcc.s
	$(AS) $(ASMOPTS) $(PRJPATH)/tnkernel_app_startup_gcc.s -o $(OBJ)/tnkernel_app_startup_gcc.o
$(OBJ)/tn.o: $(TNKERNELPATH)/tn.c
	$(CC) $(CCOPTS) $(TNKERNELPATH)/tn.c -o $(OBJ)/tn.o
$(OBJ)/tn_dqueue.o: $(TNKERNELPATH)/tn_dqueue.c
	$(CC) $(CCOPTS) $(TNKERNELPATH)/tn_dqueue.c -o $(OBJ)/tn_dqueue.o
$(OBJ)/tn_event.o: $(TNKERNELPATH)/tn_event.c
	$(CC) $(CCOPTS) $(TNKERNELPATH)/tn_event.c -o $(OBJ)/tn_event.o
$(OBJ)/tn_mem.o: $(TNKERNELPATH)/tn_mem.c
	$(CC) $(CCOPTS) $(TNKERNELPATH)/tn_mem.c -o $(OBJ)/tn_mem.o
$(OBJ)/tn_mutex.o: $(TNKERNELPATH)/tn_mutex.c
	$(CC) $(CCOPTS) $(TNKERNELPATH)/tn_mutex.c -o $(OBJ)/tn_mutex.o
$(OBJ)/tn_port.o: $(TNKERNELPATH)/tn_port.c
	$(CC) $(CCOPTS) $(TNKERNELPATH)/tn_port.c -o $(OBJ)/tn_port.o
$(OBJ)/tn_sem.o: $(TNKERNELPATH)/tn_sem.c
	$(CC) $(CCOPTS) $(TNKERNELPATH)/tn_sem.c -o $(OBJ)/tn_sem.o
$(OBJ)/tn_tasks.o: $(TNKERNELPATH)/tn_tasks.c
	$(CC) $(CCOPTS) $(TNKERNELPATH)/tn_tasks.c -o $(OBJ)/tn_tasks.o
$(OBJ)/tn_utils.o: $(TNKERNELPATH)/tn_utils.c
	$(CC) $(CCOPTS) $(TNKERNELPATH)/tn_utils.c -o $(OBJ)/tn_utils.o

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



