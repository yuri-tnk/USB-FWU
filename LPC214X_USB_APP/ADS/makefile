###########################################################################
#
# Make file for TNKernel USB loadable application ( for GNU make )
#
# Processor: LPC2146
#
# Example:   usb_app
#
# Compiler: ARM(c) ADS 1.2
#
#  No dependencies support. Call 'make clean' and than 'make' to rebuild all
#
###########################################################################

# =========  General =============

# Project output file name(image) without extention

IMAGE = usb_app

# Pathes

#If you make utility not support path name with space("Program Files")
# use  TOOLROOT = XX:/progra~2/ARM/ADSv1_2 instead

TOOLROOT  = D:/"Program Files"/ARM/ADSv1_2
PRJPATH   = F:/TNKernel_v21/LPC214X_USB_APP/ADS

TOOLSPATH = $(TOOLROOT)/Bin
LIBPATH   = $(TOOLROOT)/Lib

PRJPATH_MSDOS  = F:\TNKernel_v21\LPC214X_USB_APP\ADS
DEBUG_MSDOS    = $(PRJPATH_MSDOS)\Debug
OBJ_CLR        = $(PRJPATH_MSDOS)\OBJ

DEBUG         = $(PRJPATH)/DEBUG
OBJ           = $(PRJPATH)/OBJ
TNKERNELPATH  = $(PRJPATH)/../../TNKernel
FILESPATH = $(PRJPATH)/../usb_app


# Tools

ASM = $(TOOLSPATH)/armasm
CC  = $(TOOLSPATH)/armcc
LINKER = $(TOOLSPATH)/armlink

# Assembler options

ASMOPTS  = -li -g -apcs /noswst -cpu 4T

# Compiler options

CCOPTS  = -c -fpu None -O2 -Ono_inline -g -apcs /noswst -li -cpu 4T


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
  $(OBJ)/tn_port_asm_armcc.o\
  $(OBJ)/utils.o\
  $(OBJ)/tn_usb_descr.o\
  $(OBJ)/tn_usb_ep.o\
  $(OBJ)/tn_usb_hw.o\
  $(OBJ)/tn_usb_req.o\
  $(OBJ)/tn_user.o\
  $(OBJ)/usb_app.o\
  $(OBJ)/s_itoa_armcc.o\
  $(OBJ)/big_flash_size_imit.o\
  $(OBJ)/startup_hardware_init.o\
  $(OBJ)/tnkernel_startup_app_armcc.o


OBJECTS_CLR =  \
  $(OBJ_CLR)\tn.o\
  $(OBJ_CLR)\tn_utils.o\
  $(OBJ_CLR)\tn_dqueue.o\
  $(OBJ_CLR)\tn_event.o\
  $(OBJ_CLR)\tn_mem.o\
  $(OBJ_CLR)\tn_mutex.o\
  $(OBJ_CLR)\tn_port.o\
  $(OBJ_CLR)\tn_sem.o\
  $(OBJ_CLR)\tn_tasks.o\
  $(OBJ_CLR)\tn_port_asm_armcc.o\
  $(OBJ_CLR)\utils.o\
  $(OBJ_CLR)\tn_usb_descr.o\
  $(OBJ_CLR)\tn_usb_ep.o\
  $(OBJ_CLR)\tn_usb_hw.o\
  $(OBJ_CLR)\tn_usb_req.o\
  $(OBJ_CLR)\tn_user.o\
  $(OBJ_CLR)\usb_app.o\
  $(OBJ_CLR)\s_itoa_armcc.o\
  $(OBJ_CLR)\big_flash_size_imit.o\
  $(OBJ_CLR)\startup_hardware_init.o\
  $(OBJ_CLR)\tnkernel_startup_app_armcc.o

all: $(PRJPATH)/$(IMAGE).axf


.PHONY:   clean

clean:
	cmd /c del $(OBJECTS_CLR)

# Compiling/Assembling

$(OBJ)/tn_port_asm_armcc.o: $(TNKERNELPATH)/tn_port_asm_armcc.s
	$(ASM) $(ASMOPTS) $(TNKERNELPATH)/tn_port_asm_armcc.s -o $(OBJ)/tn_port_asm_armcc.o
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
$(OBJ)/s_itoa_armcc.o: $(FILESPATH)/s_itoa_armcc.s
	$(ASM) $(ASMOPTS) $(FILESPATH)/s_itoa_armcc.s -o $(OBJ)/s_itoa_armcc.o
$(OBJ)/startup_hardware_init.o: $(PRJPATH)/startup_hardware_init.s
	$(ASM) $(ASMOPTS) $(PRJPATH)/startup_hardware_init.s -o $(OBJ)/startup_hardware_init.o
$(OBJ)/tnkernel_startup_app_armcc.o: $(PRJPATH)/tnkernel_startup_app_armcc.s
	$(ASM) $(ASMOPTS) $(PRJPATH)/tnkernel_startup_app_armcc.s -o $(OBJ)/tnkernel_startup_app_armcc.o
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


# Linking

$(PRJPATH)/$(IMAGE).axf: $(OBJECTS)
	cmd /c del $(DEBUG_MSDOS)\$(IMAGE).axf
	cmd /c del $(DEBUG_MSDOS)\$(IMAGE).hex
	cmd /c del $(DEBUG_MSDOS)\$(IMAGE).lst
	$(LINKER) -scanlib -libpath $(LIBPATH)/ARMLIB,$(LIBPATH) -map -symbols \
	-list $(DEBUG)/$(IMAGE).lst \
	-scatter $(PRJPATH)/lpc2146_app_flash.scat \
	-entry reset -noremove -callgraph \
	-info sizes -info totals -o $(DEBUG)/$(IMAGE).axf $(OBJECTS)
	$(TOOLSPATH)/fromelf -nodebug -i32 $(DEBUG)/$(IMAGE).axf -o $(DEBUG)/$(IMAGE).hex



