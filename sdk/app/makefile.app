###############################################################################
# BEEP-8 SDK Application Makefile (makefile.app)
#
# This Makefile defines common build rules and variables for BEEP-8 applications.
# It is intended to be included from individual sample Makefiles.
#
# Key Features:
# - Automatically collects all .c and .cpp files in the current directory
# - Converts PNG images in ./data/import/ into C source via png2c, and compiles them
# - Generates BEEP-8 ROM (.b8) and optional listing file (.lst)
# - Supports Windows command prompt (DOS window), macOS, Linux, and WSL
# - Uses relative paths for portability across sample projects
#
# Notes:
# - Set `EXPORT_LIST = 1` in the including Makefile to enable .lst file generation
# - Object files are collected into OBJS_UNSORTED and deduplicated via OBJS_SORTED
# - Custom tool paths (e.g., png2c, genb8rom) are resolved based on $(OS)/$(HW)
# - To build a project, just include this file and define `PROJECT := <name>`
#
# Example sample Makefile:
#   PROJECT := hello
#   # EXPORT_LIST = 1
#   include ../makefile.app
###############################################################################

OBJDIR = ./obj
EXPORTDIR = ./data/export

HTTP_PORT ?= 443
API_PORT ?= 8082
Q ?= @

OBJS_UNSORTED =

PNGS     = $(wildcard ./data/import/*.png)
PNGS_CPP = $(patsubst %.png,%.png.cpp,$(PNGS))
PNGS_EXPORT_CPP = $(subst import,export,$(PNGS_CPP))
PNG_OBJS = $(patsubst %.cpp,%.o,$(PNGS_EXPORT_CPP))
OBJS_UNSORTED += $(PNG_OBJS)

C_SRC = $(foreach src,$(wildcard *.c),$(realpath $(src)))
OBJS_UNSORTED += $(patsubst %.c,$(OBJDIR)/%.o,$(C_SRC))

CPP_SRC = $(wildcard *.cpp)
OBJS_UNSORTED += $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRC))

TOP = $(abspath ../../../)

ifeq ($(OS),Windows_NT)
  DRIVE_LETTERS := A B C D E F G H I J K L M N O P Q R S T U V W X Y Z

  define check_drive_letter
  ifeq ($$(findstring $1:,$$(TOP)), $1:)
  TOP := $$(subst $1:,,$$(TOP))
  endif
  endef

  $(foreach letter,$(DRIVE_LETTERS),$(eval $(call check_drive_letter,$(letter))))
endif

B8ROM_IMAGE = $(abspath $(OBJDIR)/$(PROJECT).bin)
B8LIB_TOP   = $(abspath ../../b8lib)
B8HELPER_TOP= $(abspath ../../b8helper)
TOOL_TOP    = $(TOP)/tool

# png2c
PNG2C = $(TOOL_TOP)/png2c/$(OS)/$(HW)/png2c

# bin2c
BIN2C = $(TOOL_TOP)/bin2c_py/bin2c.py

# genb8rom
GENB8ROM=$(TOOL_TOP)/genb8rom/$(OS)/$(HW)/genb8rom
ifeq ($(OS),Windows_NT)
	EXE_GENB8ROM = cd ./romfs & $(GENB8ROM) -i "*" -o $(abspath $(OBJDIR)/romfs.bin)
else
	EXE_GENB8ROM = cd ./romfs ; $(GENB8ROM) -i "*" -o $(abspath $(OBJDIR)/romfs.bin)
endif

# relb8rom
RELB8ROM = $(TOOL_TOP)/relb8rom/$(OS)/$(HW)/relb8rom

ifeq ($(OS),Windows_NT)
BUSYBOX     = $(TOOL_TOP)/busybox/busybox.exe
endif

B8LIB_INC = $(B8LIB_TOP)/include
B8LIB_LIB = $(B8LIB_TOP)/lib
B8LIB_SRC = $(B8LIB_TOP)/src

B8HELPER_INC = $(B8HELPER_TOP)/include
B8HELPER_LIB = $(B8HELPER_TOP)/lib
B8HELPER_SRC = $(B8HELPER_TOP)/src

B8LIB_CRT	= $(B8LIB_SRC)/crt
B8LDSCRIPT= $(B8LIB_CRT)/beep8.ld
GNUARM_TOP = $(TOP)/gnuarm

OBJS_UNSORTED += $(OBJDIR)/bootloader.o $(OBJDIR)/crt0.o
OBJS_SORTED = $(sort $(OBJS_UNSORTED))

DEPS = $(OBJS_SORTED:.o=.d)
LST_FILE = $(abspath $(OBJDIR)/$(PROJECT).lst)
ELF = $(abspath $(OBJDIR)/$(PROJECT).out)
B8  = $(abspath $(OBJDIR)/$(PROJECT).b8)

INCLUDES += $(B8LIB_INC)
INCLUDES += $(B8HELPER_INC)

include	$(B8LIB_TOP)/makefile.top

LIBDIR=
LIBS += -lnosys -lm

LIBS += -L$(B8LIB_TOP)/lib
LIBS += -lb8

LIBS += -L$(B8HELPER_TOP)/lib
LIBS += -lb8helper

define MAKEB8LIB
	@$(MAKE) -f $(B8LIB_SRC)/b8/Makefile -C $(B8LIB_SRC)/b8 --no-print-directory
endef

define B8PLAYER
	$(EXEC_B8PLAYER)
endef

define CLEANB8LIB
	@$(MAKE) clean -f $(B8LIB_SRC)/b8/Makefile -C $(B8LIB_SRC)/b8 --no-print-directory
endef

define MAKEB8HELPER
	@$(MAKE) -f $(B8HELPER_SRC)/Makefile -C $(B8HELPER_SRC) --no-print-directory
endef

define CLEANB8HELPER
	@$(MAKE) clean -f $(B8HELPER_SRC)/Makefile -C $(B8HELPER_SRC) --no-print-directory
endef

.PHONY: all clean mostlyclean lib helper info

all: $(B8)

$(B8) : lib $(OBJS_SORTED)
	+@$(MAKE) -f $(B8LIB_SRC)/b8/Makefile -C $(B8LIB_SRC)/b8 --no-print-directory
	+@$(MAKE) -f $(B8HELPER_SRC)/Makefile -C $(B8HELPER_SRC) --no-print-directory
	@echo $(ESC_INFO)linking $(ELF) $(ESC_RESET)
	$(Q)$(LD) -T$(B8LDSCRIPT) $(LDFLAGS) $(OBJS_SORTED) $(LIBDIR) $(LIBS) -o $@
	$(Q)@$(SIZE) --totals --common $(sort $(OBJS_SORTED))
	$(Q)$(OCOPY) -R .noinit -R .comment --strip-debug -O binary $@ $(B8ROM_IMAGE)
ifeq ($(EXPORT_LIST),1)
	$(ODUMP) -Mreg-names-std -d -C $@ > $(LST_FILE)
	@echo $(ESC_INFO) BEEP-8 ROM LISTING FILE:
	$(LS) $(LST_FILE)
endif 
	@echo $(ESC_RESET)
	@echo $(ESC_INFO) BEEP-8 PROGRAM ROM:
	$(LS) $(B8ROM_IMAGE)
	@echo $(ESC_RESET)
	@echo making BEEP-8 RELEASE ROM IMAGE
	$(MKDIR) romfs
	@echo $(ESC_ASM)
	$(EXE_GENB8ROM)
	$(RELB8ROM) -i $(B8ROM_IMAGE) -r $(OBJDIR)/romfs.bin -o $(B8)
	@echo $(ESC_RESET)
	@echo $(ESC_INFO) BEEP-8 RELEASE ROM IMAGE
	$(LS) $(B8)
	$(call ON_AFTER_BUILD)
	@echo $(ESC_RESET)

run: all
ifeq ($(OS),Windows_NT)
	$(TOOL_TOP)/scripts/upload.bat $(B8) $(HTTP_PORT) $(API_PORT)

else
	$(TOOL_TOP)/scripts/upload.sh $(B8) $(HTTP_PORT) $(API_PORT)
endif

runp: all
	$(call B8PLAYER)

lib:
	+$(call MAKEB8LIB)

cleanlib:
	$(call CLEANB8LIB)

helper:
	$(call MAKEB8HELPER)

cleanhelper:
	$(call CLEANB8HELPER)

$(OBJDIR)/bootloader.o: $(B8LIB_CRT)/bootloader.S
	$(call ASM)

$(OBJDIR)/crt0.o: $(B8LIB_CRT)/crt0.c
	$(call COMPILE_C)

$(OBJDIR)/%.o: %.S
	$(call ASM)

$(OBJDIR)/%.o: %.cpp
	$(call COMPILE_CPP)

$(EXPORTDIR)/%.o: $(EXPORTDIR)/%.cpp
	$(call COMPILE_CPP)

$(OBJDIR)/%.o: %.c
	$(call COMPILE_C)

./data/export/%.png.cpp: ./data/import/%.png
	$(MKDIR) $(dir $@)
	@echo $(ESC_INFO)
	@echo png2c $< to $@
	$(PNG2C) $< > $@
	@echo $(ESC_RESET)

clean:
	$(call CLEAN)
	$(TOUCH) Makefile $(PNGS)
	@$(RM) $(OBJS_SORTED) $(OBJDIR) ./data/export/*.d

mostlyclean: clean

.PHONY: info
info:
	@echo "PROJECT  = $(PROJECT)"
	@echo "OS     = $(OS)"
	@echo "HW     = $(HW)"
	@echo "CROSS  = $(CROSS)"
	@echo "CCACHE = $(CCACHE)"
	@echo "CC     = $(CC)"
	@echo "CXX    = $(CXX)"
	@echo "AS     = $(AS)"
	@echo "LD     = $(LD)"
	@echo "SIZE     = $(SIZE)"
	@echo "GCCVER = $(GCCVER)"
	@echo "TOUCH  = $(TOUCH)"
	@echo "MKDIR  = $(MKDIR)"
	@echo "CP     = $(CP)"
	@echo "RM     = $(RM)"
	@echo "WHICH  = $(WHICH)"
	@echo "CAT    = $(CAT)"
	@echo "BUSYBOX= $(BUSYBOX)"
	@echo "PYTHON = $(PYTHON)"
	@echo "TOP      = $(TOP)"
	@echo "TOOL_TOP = $(TOOL_TOP)"
	@echo "PNG2C    = $(PNG2C)"
	@echo "PNGS     = $(PNGS)"
	@echo "GENB8ROM = $(GENB8ROM)"
	@echo "RELB8ROM = $(RELB8ROM)" 
ifdef CCACHE
ifneq ($(CCACHE),)
	$(CCACHE) -sv
endif
endif

ifeq ($(filter $(MAKECMDGOALS), clean), )
ifeq ($(filter $(MAKECMDGOALS), mostlyclean), )
-include $(DEPS)
endif
endif
