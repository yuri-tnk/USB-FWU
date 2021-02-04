###########################################################################
#
# Make file for USB Firmware Upgrader ( for GNU make )
#
# Processor: LPC2146
#
# Name:   usb_fwu
#
# Compiler: ARM(c) RVC 2.2
#
#  No dependencies support. Call 'make clean' and than 'make' to rebuild all
#
###########################################################################

# =========  General =============



# Project output file name(image) without extention

IMAGE = usb_fwu

# Pathes
# Pathes

# If you make utility not support path name with space("Program Files")
# use  TOOLROOT = XX:/progra~2/XXX  instead

TOOLROOT  = D:/"Program Files"/ARM/RVCT
PRJPATH   = F:/TNKernel_v21/LPC214X_USB_FWU/RVCT
PRJPATH_MSDOS  = F:\TNKernel_v21\LPC214X_USB_FWU\RVCT


TOOLSPATH = $(TOOLROOT)/Programs/2.2/349/win_32-pentium
LIBPATH   = $(TOOLROOT)/Data/2.2/349/lib

PRJPATH_MSDOS  = F:\TNKernel_v21\LPC214X_USB_FWU\RVCT
DEBUG_MSDOS    = $(PRJPATH_MSDOS)\Debug
OBJ_CLR        = $(PRJPATH_MSDOS)\OBJ

DEBUG         = $(PRJPATH)/DEBUG
OBJ           = $(PRJPATH)/OBJ
FILESPATH     = $(PRJPATH)/../usb_fwu

# Tools

ASM = $(TOOLSPATH)/armasm
CC  = $(TOOLSPATH)/armcc
LINKER = $(TOOLSPATH)/armlink

# Assembler options

ASMOPTS  = --cpu ARM7TDMI -g --apcs /noswst/interwork

# Compiler options

CCOPTS    = -c --fpu None -Ospace -Ono_inline --cpu ARM7TDMI --apcs /noswst/interwork

OBJECTS = \
  $(OBJ)/fwu_startup_hardware.o\
  $(OBJ)/fwu_utils.o\
  $(OBJ)/fwu.o\
  $(OBJ)/fwu_int.o\
  $(OBJ)/fwu_queue.o\
  $(OBJ)/fwu_usb_descr.o\
  $(OBJ)/fwu_usb_ep.o\
  $(OBJ)/fwu_usb_hw.o\
  $(OBJ)/fwu_usb_req.o\
  $(OBJ)/fwu_asm_armcc.o\
  $(OBJ)/fwu_init.o\
  $(OBJ)/fwu_usb_thumb.o\
  $(OBJ)/fwu_startup_armcc.o

OBJECTS_CLR =  \
  $(OBJ_CLR)\fwu_startup_hardware.o\
  $(OBJ_CLR)\fwu_utils.o\
  $(OBJ_CLR)\fwu.o\
  $(OBJ_CLR)\fwu_int.o\
  $(OBJ_CLR)\fwu_queue.o\
  $(OBJ_CLR)\fwu_usb_descr.o\
  $(OBJ_CLR)\fwu_usb_ep.o\
  $(OBJ_CLR)\fwu_usb_hw.o\
  $(OBJ_CLR)\fwu_usb_req.o\
  $(OBJ_CLR)\fwu_asm_armcc.o\
  $(OBJ_CLR)\fwu_init.o\
  $(OBJ_CLR)\fwu_usb_thumb.o\
  $(OBJ_CLR)\fwu_startup_armcc.o

all: $(PRJPATH)/$(IMAGE).axf


.PHONY:   clean

clean:
	cmd /c del $(OBJECTS_CLR)

#--- Compiling/Assembling ---

#  Asm

$(OBJ)/fwu_startup_hardware.o: $(PRJPATH)/fwu_startup_hardware.s
	$(ASM) $(ASMOPTS) $(PRJPATH)/fwu_startup_hardware.s -o $(OBJ)/fwu_startup_hardware.o
$(OBJ)/fwu_startup_armcc.o: $(PRJPATH)/fwu_startup_armcc.s
	$(ASM) $(ASMOPTS) $(PRJPATH)/fwu_startup_armcc.s -o $(OBJ)/fwu_startup_armcc.o
$(OBJ)/fwu_asm_armcc.o: $(FILESPATH)/fwu_asm_armcc.s
	$(ASM) $(ASMOPTS) $(FILESPATH)/fwu_asm_armcc.s -o $(OBJ)/fwu_asm_armcc.o

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

# C THUMB - not needs for this compiler

$(OBJ)/fwu_init.o: $(FILESPATH)/fwu_init.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu_init.c -o $(OBJ)/fwu_init.o
$(OBJ)/fwu_usb_thumb.o: $(FILESPATH)/fwu_usb_thumb.c
	$(CC) $(CCOPTS) $(FILESPATH)/fwu_usb_thumb.c -o $(OBJ)/fwu_usb_thumb.o


# Linking

$(PRJPATH)/$(IMAGE).axf: $(OBJECTS)
	cmd /c del $(DEBUG_MSDOS)\$(IMAGE).axf
	cmd /c del $(DEBUG_MSDOS)\$(IMAGE).hex
	cmd /c del $(DEBUG_MSDOS)\$(IMAGE).lst
	$(LINKER) --noscanlib --map --symbols \
	--list $(DEBUG)/$(IMAGE).lst \
	--entry reset --noremove --callgraph \
	--scatter $(PRJPATH)/fwu_lpc2146_flash.scat \
	--info sizes --info totals -o $(DEBUG)/$(IMAGE).axf $(OBJECTS)
	$(TOOLSPATH)/fromelf --nodebug --i32 $(DEBUG)/$(IMAGE).axf -o $(DEBUG)/$(IMAGE).hex


